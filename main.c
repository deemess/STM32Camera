#include "stm32f10x.h"
#include "misc.h"
#include <stdio.h>
#include "lcd.h"
//#include "plane.h"
#include "camera\dcmi_ov9655.h"


#define LINES 80

uint8_t video[320*2*LINES];

void InitCameraClock()
{

	GPIO_InitTypeDef GPIO_InitStructure;


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	// Output clock on MCO pin ---------------------------------------------

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	// pick one of the clocks to spew
	RCC_MCOConfig(RCC_MCO_PLLCLK_Div2); // Put on MCO pin the: System clock selected
	//RCC_MCOConfig(RCC_MCOSource_HSE); // Put on MCO pin the: freq. of external crystal
	//RCC_MCOConfig(RCC_MCOSource_PLLCLK_Div2); // Put on MCO pin the: System clock selected


	/*
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	TIM_TimeBaseInitTypeDef TIM_BaseConfig;
	TIM_OCInitTypeDef TIM_OCConfig;

	//init timer
	// Enable timer clock  - use TIMER3
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	TIM_BaseConfig.TIM_Prescaler = (uint16_t) 2;
	TIM_BaseConfig.TIM_Period = 10;
	TIM_BaseConfig.TIM_ClockDivision = 0;
	TIM_BaseConfig.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM1, &TIM_BaseConfig);

	TIM_OCConfig.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCConfig.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCConfig.TIM_Pulse = 5;
	TIM_OCConfig.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM1, &TIM_OCConfig);

	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIM1, ENABLE);

	TIM_Cmd(TIM1, ENABLE);
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
	*/
}

int main(void)
{
	char buff[32];
	uint8_t reg[32];
	uint32_t i;
	uint32_t p;
	uint8_t line=0;
	uint32_t skipLines=0;
	uint32_t framePart=0;

	OV9655_IDTypeDef id;

	SystemInit();
	SysTick_Config(16777215);
	SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

	LCD_Initializtion();
	LCD_Clear(Blue);
	//GUI_Image((uint16_t*)image_data_plane);
	GUI_Text(COL(0), LINE(line++), "Camera test application", White, Blue);
	GUI_Text(COL(0), LINE(line++), "Initializing camera...", White, Blue);

	//Init camera clock
	InitCameraClock();
	DCMI_Control_IO_Init();

	sprintf(buff, "Camera init: 0x%02x", cameraInit());
	GUI_Text(COL(0), LINE(line++), buff, White, Blue);

	DCMI_OV9655_SetPrescaler(16);//8
	DCMI_OV9655_ReadID(&id);
	sprintf(buff, "Camera ID: 0x%02x 0x%02x 0x%02x 0x%02x", id.Manufacturer_ID1, id.Manufacturer_ID2, id.PID, id.Version);
	GUI_Text(COL(0), LINE(line++), buff, White, Blue);

	for(i=0; i<32; i++) {
		reg[i] = DCMI_SingleRandomRead(OV9655_DEVICE_WRITE_ADDRESS, i);
	}

	for(i=0; i<32; i+=8) {
		sprintf(buff, "%02x %02x %02x %02x %02x %02x %02x %02x", reg[i], reg[i+1], reg[i+2], reg[i+3] ,reg[i+4], reg[i+5], reg[i+6], reg[i+7]);
		GUI_Text(COL(0), LINE(line++), buff, White, Blue);
	}


	while(1)
	{
		//reset video buffer pointer
		p=0;

		//wait for frame start
		while((CAMERA_PORT->IDR & CAMERA_VSYNC) == 0) ; //wait for VSYNC high
		while(CAMERA_PORT->IDR & CAMERA_VSYNC) ; //wait for VSYNC low

		//skip lines
		while (skipLines > 0) {
			//wait for HREF (horisontal line start)
			while((CAMERA_PORT->IDR & CAMERA_HREF) == 0) ; //wait for HREF high
			while(CAMERA_PORT->IDR & CAMERA_HREF) ; //wait for HREF low
			skipLines--;
		}

		//read first 80 lines
		while(p < 320*2*LINES) {
			//wait for HREF (horisontal line start)
			while((CAMERA_PORT->IDR & CAMERA_HREF) == 0) ; //wait for HREF high

			//read line
			for(i=0; i<640; i++) {
				//wait for pixel clock
				while((CAMERA_PXCLCPORT->IDR & CAMERA_PXCLC) == 0) ; //wait for PXCLC high

				//read data
				video[p++] = CAMERA_DATAPORT->IDR;//GPIO_ReadInputData(CAMERA_DATAPORT);
			}

		}

		//send first 80 lines to LCD
		LCD_SetCursor(0, framePart*80);
		GUI_Image((uint16_t*)video, 320*LINES);

		framePart++;
		if(framePart > 2) framePart = 0;

		switch(framePart) {
		case 0:
			skipLines = 0;
			break;
		case 1:
			skipLines = 80;
			break;
		case 2:
			skipLines = 160;
			break;
		}

	}

}


#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
