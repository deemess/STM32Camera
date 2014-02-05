#include "dcmi_ov9655.h"

/*
******************************************************************************
  Function:       DCMI_Control_IO_Init
  Description:    Init DCMI module power control
  Calls:          void
  Called By:
  Input:          void
  Output:         void
  Return:
  Others:
******************************************************************************
*/
void DCMI_Control_IO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	//Enable clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	CAMERA_CLOCK;
	CAMERA_PXCLCCLOCK;

	//Camera Pixel clock
	GPIO_InitStructure.GPIO_Pin = CAMERA_PXCLC;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init( CAMERA_PXCLCPORT , &GPIO_InitStructure);

	//Camera HREV VSYNC
	GPIO_InitStructure.GPIO_Pin = CAMERA_HREF, CAMERA_VSYNC;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(CAMERA_PORT, &GPIO_InitStructure);

	//Camera data bus
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(CAMERA_DATAPORT, &GPIO_InitStructure);

}

 
/**
  * @brief  Reset the OV9655 SCCB registers.
  * @param  None
  * @retval None
  */
void DCMI_OV9655_Reset(void)
{
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS, OV9655_COM7, SCCB_REG_RESET);    
}

/**
  * @brief  Set the QVGA size(240*320).
  * @param  None
  * @retval None
  */
void DCMI_OV9655_QVGASizeSetup_adv() {
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x12, 0x10);    // COM7       set output to QVGA mode
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x11, 0x81);    // CLKRC      set the system clock division
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x6b, 0x0a);    // DBLV       band gap reference adjustment
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x6a, 0x41);    // 		   LSB of Banding Filter (effective only when COM11[0] is high)
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x3b, 0x09);    // COM11      use half frame and manual banding filter mode
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x13, 0xe0);    // COM8       enable fast AGC/AEC, enable AEC, and enable Banding filter
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x01, 0x80);	 // BLUE	   Blue channel gain setting, default 0x80
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x02, 0x80);	 // RED		   Red channel gain setting, default 0x80
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x00, 0x00);	 // GAIN	   AGC, range 0x00 to 0xFF
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x10, 0x00);    // AECH       exposure value set default 0x40
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x13, 0xe5);    // COM8       now enable AGC/AEC
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x39, 0x43);    // OPON       ???
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x38, 0x12);    // ACOM       ???
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x37, 0x00);	 // ADC		   ???
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x35, 0x91);    // RSVD	   ???
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x0e, 0x20);    // COM5       ???
	//DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x1e, 0x30);    // MVFP       Mirror/VFlip enable, mirror and vflip enable for self-portrait (hardware OV9650_BO)
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x1e, 0x00);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x04, 0x00);    // COM1       no line skip option
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x0c, 0x00);    // COM3       set to default
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x0d, 0x00);	 // COM4	   default to 0x00

	// window start and stop for output size control
	// HSTOP[REG18h] is the high 8 bit of H STOP (low 3 bits are at HREF[5:3])
	// Because HREF[5:3] = 0b100, HSTOP = 0x5A*2^3 + 4 = 0x2D0
	// HSTART[REG17h] is the high 8-bit of H START (low 3 bits are at HREF[2:0])
	// Because HREF[2:0] = 0b100, HSTART = 0x32*2^3 + 4 = 0x190
	// Therefore HSTOP-HSTART = (0x2D0+4)-(0x190+4) = 0x140 (320 dec.)
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,OV9655_HREF, 0xA4);// HREF       window control
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,OV9655_HSTOP, 0x5A);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,OV9655_HSTART, 0x32);

	// VSTRT      window control
	// VREF[REG03h] contains the start and end low 3 bits for VSTOP and VSTRT
	// VSTRT[REG19h] = 0x31*2^3 = 0x188
	// VSTOP[REG1Ah] = 0x4F*2^3 = 0x278
	// Therefore VSTOP - VSTRT = (0x278+2)-(0x188+2) = 0xF0 (240 dec.)
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,OV9655_VREF, 0x12);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,OV9655_VSTART, 0x31);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,OV9655_VSTOP, 0x4F);

	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x14, 0x1a);    // COM9       automatic gain control ceilings
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x15, 0x10);    // COM10      set up pclk, vsync and href signals
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x3f, 0xa6);	 // EDGE	   what edge enhancement ???

	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x41, 0x02);	 // COM16	   Color matrix coefficient double option enable ???
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x42, 0xc1);	 // COM17	   edge enhancement option, tri-state output etc
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x1b, 0x00);    // PSHFT      set pclk _delay_ms from href

	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x16, 0x06);	 // RSVD	   another reserved register!
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x33, 0xe2);    // CHLF       array current control
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x34, 0xbf);    // ARBLM      array reference control

	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x96, 0x04);	 // RSVD	   again, reserved register
		DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x3a, 0x0c); 	 // TSLB	   line buffer test option for UV output and sequence setting

	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x8e, 0x00);	 // COM24	   Reserved register
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x3c, 0x77);    // COM12      href options

	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x8b,0x06);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x94,0x88);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x95,0x88);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x40,0xc1);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x29,0x3f);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x0f,0x42);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x3d,0x92);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x69,0x40);	 // Manual Banding Filter MSB
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x5c,0xb9);	 // REG[0x59-0x61] all RSVD
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x5d,0x96);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x5e,0x10);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x59,0xc0);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x5a,0xaf);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x5f,0xe0);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x60,0x8c);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x61,0x20);

	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x5b,0x55);	 // REG[0x59-0x61] all RSVD
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x43,0xf0);	 // REG[0x43-0x4E] all RSVD
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x44,0x10);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x45,0x68);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x46,0x96);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x47,0x60);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x48,0x80);

	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xa5,0xd9);	 // RSVD
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xa4,0x74);	 // RSVD

	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x8d,0x02);	 // color bar test mode disable, digital AWB enable

	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x13,0xe7);	 // AGC, AWB, AEC all enabled here

	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x4f,0x3a);	 // REG[4F-58] are Matrix coefficients
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x50,0x3d);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x51,0x03);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x52,0x12);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x53,0x26);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x54,0x38);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x55,0x40);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x56,0x40);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x57,0x40);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x58,0x0d);

	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x8c,0x23);	 // De-noise enable, white-pixel erase enable
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x3e,0x02);

	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xa9,0xb8);	 // REG[A8-AA] all RSVD
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xaa,0x92);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xab,0x0a);

	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x8f,0xdf);	 // Digital BLC offset sign???

	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x90,0x00);	 // Digital BLC B channel offset value set default
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x91,0x00);	 // Digital BLC R channel offset value set default
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x9f,0x00);	 // Digital BLC Gb channel offset value set default
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xa0,0x00);	 // Digital BLC Gr channel offset value set default

	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x24,0x98);	 // AEW stable operating region upper limit
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x25,0x88);	 // AEW stable operating region lower limit
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x26,0xd3);	 // AGC/AEC fast mode operating region
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x2a,0x00);	 // Horizontal dummy pixel insert MSB
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x2b,0x00);	 // Horizontal dummy insert LSB
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x2d,0x00);	 // Vertical LSB of dummy lines

	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x6c,0x40);	 // REG[6C-7B] are Gamma Curve setup GSP
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x6d,0x30);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x6e,0x4b);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x6f,0x60);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x70,0x70);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x71,0x70);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x72,0x70);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x73,0x70);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x74,0x60);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x75,0x60);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x76,0x50);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x77,0x48);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x78,0x3a);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x79,0x2e);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x7a,0x28);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x7b,0x22);

	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x7c,0x04);	 // REG[7C-8A] are Gamma Curve setup GST
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x7d,0x07);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x7e,0x10);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x7f,0x28);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x80,0x36);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x81,0x44);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x82,0x52);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x83,0x60);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x84,0x6c);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x85,0x78);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x86,0x8c);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x87,0x9e);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x88,0xbb);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x89,0xd2);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x8a,0xe6);
}

void DCMI_OV9655_QVGASizeSetup(void)
{  
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x00, 0x00);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x01, 0x80);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x02, 0x80);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x03, 0x02);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x04, 0x03);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x09, 0x01);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x0b, 0x57);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x0e, 0x61);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x0f, 0x40);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x11, 0x01);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x12, 0x62);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x13, 0xc7);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x14, 0x3a);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x16, 0x24);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x17, 0x18);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x18, 0x04);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x19, 0x01);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x1a, 0x81);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x1e, 0x00); /*0x20*/
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x24, 0x3c);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x25, 0x36);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x26, 0x72);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x27, 0x08);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x28, 0x08);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x29, 0x15);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x2a, 0x00);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x2b, 0x00);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x2c, 0x08);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x32, 0x12);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x33, 0x00);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x34, 0x3f);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x35, 0x00);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x36, 0x3a);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x38, 0x72);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x39, 0x57);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x3a, 0xcc);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x3b, 0x04);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x3d, 0x99);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x3e, 0x02);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x3f, 0xc1);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x40, 0xc0);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x41, 0x41);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x42, 0xc0);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x43, 0x0a);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x44, 0xf0);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x45, 0x46);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x46, 0x62);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x47, 0x2a);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x48, 0x3c);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x4a, 0xfc);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x4b, 0xfc);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x4c, 0x7f);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x4d, 0x7f);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x4e, 0x7f);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x4f, 0x98);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x50, 0x98);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x51, 0x00);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x52, 0x28);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x53, 0x70);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x54, 0x98);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x58, 0x1a);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x59, 0x85);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x5a, 0xa9);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x5b, 0x64);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x5c, 0x84);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x5d, 0x53);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x5e, 0x0e);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x5f, 0xf0);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x60, 0xf0);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x61, 0xf0);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x62, 0x00);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x63, 0x00);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x64, 0x02);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x65, 0x20);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x66, 0x00);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x69, 0x0a);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x6b, 0x5a);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x6c, 0x04);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x6d, 0x55);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x6e, 0x00);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x6f, 0x9d);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x70, 0x21);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x71, 0x78);
	_delay_ms(TIMEOUT);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x72, 0x11);
	_delay_ms(TIMEOUT);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x73, 0x01);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x74, 0x10);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x75, 0x10);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x76, 0x01);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x77, 0x02);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x7A, 0x12);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x7B, 0x08);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x7C, 0x16);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x7D, 0x30);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x7E, 0x5e);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x7F, 0x72);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x80, 0x82);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x81, 0x8e);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x82, 0x9a);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x83, 0xa4);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x84, 0xac);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x85, 0xb8);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x86, 0xc3);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x87, 0xd6);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x88, 0xe6);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x89, 0xf2);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x8a, 0x24);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x8c, 0x80);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x90, 0x7d);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x91, 0x7b);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x9d, 0x02);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x9e, 0x02);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x9f, 0x7a);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xa0, 0x79);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xa1, 0x40);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xa4, 0x50);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xa5, 0x68);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xa6, 0x4a);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xa8, 0xc1);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xa9, 0xef);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xaa, 0x92);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xab, 0x04);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xac, 0x80);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xad, 0x80);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xae, 0x80);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xaf, 0x80);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xb2, 0xf2);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xb3, 0x20);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xb4, 0x20);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xb5, 0x00);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xb6, 0xaf);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xb6, 0xaf);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xbb, 0xae);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xbc, 0x7f);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xbd, 0x7f);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xbe, 0x7f);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xbf, 0x7f);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xbf, 0x7f);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xc0, 0xaa);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xc1, 0xc0);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xc2, 0x01);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xc3, 0x4e);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xc6, 0x05);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xc7, 0x81);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xc9, 0xe0);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xca, 0xe8);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xcb, 0xf0);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xcc, 0xd8);
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xcd, 0x93);
	_delay_ms(TIMEOUT);
}

/**
  * @brief  Set the QQVGA size(120*160).
  * @param  None
  * @retval None
  */
void DCMI_OV9655_QQVGASizeSetup(void)
{
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x00, 0x00);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x01, 0x80);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x02, 0x80);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x03, 0x02);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x04, 0x03);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x09, 0x01);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x0b, 0x57);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x0e, 0x61);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x0f, 0x40);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x11, 0x01);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x12, 0x62);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x13, 0xc7);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x14, 0x3a);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x16, 0x24);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x17, 0x18);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x18, 0x04);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x19, 0x01);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x1a, 0x81);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x1e, 0x20);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x24, 0x3c);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x25, 0x36);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x26, 0x72);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x27, 0x08);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x28, 0x08);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x29, 0x15);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x2a, 0x00);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x2b, 0x00);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x2c, 0x08);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x32, 0xa4);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x33, 0x00);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x34, 0x3f);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x35, 0x00);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x36, 0x3a);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x38, 0x72);
  _delay_ms(TIMEOUT);
  //DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x39, 0x57);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x39, 0x43);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x3a, 0xcc);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x3b, 0x04);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x3d, 0x99);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x3e, 0x0e); 
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x3f, 0xc1);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x40, 0xc0);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x41, 0x41);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x42, 0xc0);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x43, 0x0a);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x44, 0xf0);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x45, 0x46);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x46, 0x62);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x47, 0x2a);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x48, 0x3c);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x4a, 0xfc);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x4b, 0xfc);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x4c, 0x7f);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x4d, 0x7f);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x4e, 0x7f);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x4f, 0x98);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x50, 0x98);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x51, 0x00);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x52, 0x28);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x53, 0x70);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x54, 0x98);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x58, 0x1a);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x59, 0x85);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x5a, 0xa9);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x5b, 0x64);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x5c, 0x84);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x5d, 0x53);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x5e, 0x0e);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x5f, 0xf0);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x60, 0xf0);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x61, 0xf0);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x62, 0x00);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x63, 0x00);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x64, 0x02);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x65, 0x20);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x66, 0x00);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x69, 0x0a);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x6b, 0x5a);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x6c, 0x04);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x6d, 0x55);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x6e, 0x00);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x6f, 0x9d);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x70, 0x21);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x71, 0x78);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x72, 0x22); 
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x73, 0x02);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x74, 0x10);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x75, 0x10); 
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x76, 0x01);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x77, 0x02);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x7A, 0x12);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x7B, 0x08);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x7C, 0x16);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x7D, 0x30);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x7E, 0x5e);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x7F, 0x72);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x80, 0x82);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x81, 0x8e);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x82, 0x9a);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x83, 0xa4);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x84, 0xac);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x85, 0xb8);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x86, 0xc3);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x87, 0xd6);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x88, 0xe6);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x89, 0xf2);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x8a, 0x24);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x8c, 0x80);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x90, 0x7d);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x91, 0x7b);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x9d, 0x02);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x9e, 0x02);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x9f, 0x7a);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xa0, 0x79);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xa1, 0x40);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xa4, 0x50);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xa5, 0x68);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xa6, 0x4a);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xa8, 0xc1);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xa9, 0xef);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xaa, 0x92);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xab, 0x04);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xac, 0x80);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xad, 0x80);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xae, 0x80);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xaf, 0x80);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xb2, 0xf2);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xb3, 0x20);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xb4, 0x20);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xb5, 0x00);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xb6, 0xaf);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xb6, 0xaf);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xbb, 0xae);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xbc, 0x7f);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xbd, 0x7f);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xbe, 0x7f);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xbf, 0x7f);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xbf, 0x7f);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xc0, 0xaa);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xc1, 0xc0);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xc2, 0x01);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xc3, 0x4e);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xc6, 0x05);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xc7, 0x82);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xc9, 0xe0);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xca, 0xe8);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xcb, 0xf0);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xcc, 0xd8);
  _delay_ms(TIMEOUT);
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0xcd, 0x93);
  _delay_ms(TIMEOUT);
}

/**
  * @brief  Read the OV9655 Manufacturer identifier.
  * @param  OV9655ID: pointer to the OV9655 Manufacturer identifier. 
  * @retval None
  */
void DCMI_OV9655_ReadID(OV9655_IDTypeDef* OV9655ID)
{
  OV9655ID->Manufacturer_ID1 = DCMI_SingleRandomRead(OV9655_DEVICE_WRITE_ADDRESS, OV9655_MIDH);
  _delay_ms(TIMEOUT);
  OV9655ID->Manufacturer_ID2 = DCMI_SingleRandomRead(OV9655_DEVICE_WRITE_ADDRESS, OV9655_MIDL);
  _delay_ms(TIMEOUT);
  OV9655ID->Version = DCMI_SingleRandomRead(OV9655_DEVICE_WRITE_ADDRESS, OV9655_VER);
  _delay_ms(TIMEOUT);
  OV9655ID->PID = DCMI_SingleRandomRead(OV9655_DEVICE_WRITE_ADDRESS, OV9655_PID);
  _delay_ms(TIMEOUT);
}

/**
  * @brief  Set the Internal Clock Prescaler.
  * @param  OV9655_Prescaler: the new value of the prescaler. 
  *         This parameter can be a value between 0x0 and 0x1F
  * @retval None
  */
void DCMI_OV9655_SetPrescaler(uint8_t OV9655_Prescaler)
{
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS, OV9655_CLKRC, OV9655_Prescaler);
}

/**
  * @brief  Select the Output Format.
  * @param  OV9655_OuputFormat: the Format of the ouput Data.  
  *         This parameter can be one of the following values:
  *           @arg OUTPUT_FORMAT_RAWRGB_DATA 
  *           @arg OUTPUT_FORMAT_RAWRGB_INTERP    
  *           @arg OUTPUT_FORMAT_YUV              
  *           @arg OUTPUT_FORMAT_RGB    
  * @retval None
  */
void DCMI_OV9655_SelectOutputFormat(uint8_t OV9655_OuputFormat)
{
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS, OV9655_COM7, OV9655_OuputFormat);
}

/**
  * @brief  Select the Output Format Resolution.
  * @param  OV9655_FormatResolution: the Resolution of the ouput Data. 
  *         This parameter can be one of the following values:
  *           @arg FORMAT_CTRL_15fpsVGA 
  *           @arg FORMAT_CTRL_30fpsVGA_NoVArioPixel    
  *           @arg FORMAT_CTRL_30fpsVGA_VArioPixel     
  * @retval None
  */
void DCMI_OV9655_SelectFormatResolution(uint8_t OV9655_FormatResolution)
{
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS, OV9655_COM7, OV9655_FormatResolution);
}

/**
  * @brief  Set the new value of OV9655 registers
  * @param  OV9655_Register: the OV9655 Register to be configured. 
  * @param  Register_Val: The new value to be set 
  * @retval None
  */
void DCMI_OV9655_SetRegister(uint8_t OV9655_Register, uint8_t Register_Val)
{
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS, OV9655_Register, Register_Val);
}

/**
  * @brief  Select the HREF Control signal option
  * @param  OV9665_HREFControl: the HREF Control signal option.
  *         This parameter can be one of the following value:
  *           @arg OV9665_HREFControl_Opt1: HREF edge offset to data output. 
  *           @arg OV9665_HREFControl_Opt2: HREF end 3 LSB    
  *           @arg OV9665_HREFControl_Opt3: HREF start 3 LSB      
  * @retval None
  */
void DCMI_OV9655_HREFControl(uint8_t OV9665_HREFControl)
{
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS, OV9655_HREF, OV9665_HREFControl);
}

/**
  * @brief  Select the RGB format option
  * @param  OV9665_RGBOption: the RGB Format option.
  *         This parameter can be one of the following value:
  *           @arg RGB_NORMAL
  *           @arg RGB_565  
  *           @arg RGB_555    
  * @retval None
  */
void DCMI_OV9655_SelectRGBOption(uint8_t OV9665_RGBOption)
{
  DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS, OV9655_COM15, OV9665_RGBOption);
}

/**
  * @brief  Writes a byte at a specific Camera register
  * @param  Device: OV9655 write address.
  * @param  Addr: OV9655 register address. 
  * @param  Data: data to be written to the specific register 
  * @retval 0x00 if write operation is OK. 
  *         0xFF if timeout condition occured (device not connected or bus error).
  */
uint8_t DCMI_SingleRandomWrite(uint8_t Device, uint16_t Addr, uint8_t Data)
{
	//if(I2C_WriteAddr(I2C2, buff, 2, Device) == Error)
	if(I2C_WriteAddr(&Data, 1, Device, Addr) == Error)
		return 0xFF;

	return 0x00;
/*
  uint32_t timeout = DCMI_TIMEOUT_MAX;
  
  // Generate the Start Condition 
  I2C_GenerateSTART(I2C2, ENABLE);
	//I2C1->DR = Device & 0xFFFE;

  // Test on I2C1 EV5 and clear it 
  timeout = DCMI_TIMEOUT_MAX; 
  while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT))
  {
    // If the timeout _delay_ms is exeeded, exit with error code
    if ((timeout--) == 0) return 0xFF;
  }
   
	
	
  // Send DCMI selcted device slave Address for write 
  I2C_Send7bitAddress(I2C2, Device, I2C_Direction_Transmitter);
 
  // Test on I2C1 EV6 and clear it 
  timeout = DCMI_TIMEOUT_MAX; 
  while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
  {
    // If the timeout _delay_ms is exeeded, exit with error code
    if ((timeout--) == 0) return 0xFF;
  }
 
  // Send I2C1 location address LSB 
  I2C_SendData(I2C2, (uint8_t)(Addr));

  // Test on I2C1 EV8 and clear it 
  timeout = DCMI_TIMEOUT_MAX; 
  while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
  {
    // If the timeout _delay_ms is exeeded, exit with error code
    if ((timeout--) == 0) return 0xFF;
  }
  
  // Send Data 
  I2C_SendData(I2C2, Data);    

  // Test on I2C1 EV8 and clear it 
  timeout = DCMI_TIMEOUT_MAX; 
  while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
  {
    // If the timeout _delay_ms is exeeded, exit with error code
    if ((timeout--) == 0) return 0xFF;
  }  
 
  // Send I2C1 STOP Condition 
  I2C_GenerateSTOP(I2C2, ENABLE);
  
  // If operation is OK, return 0 
  return 0;
*/
}

/**
  * @brief  Reads a byte from a specific Camera register 
  * @param  Device: OV9655 write address.
  * @param  Addr: OV9655 register address. 
  * @retval data read from the specific register or 0xFF if timeout condition 
  *         occured. 
  */
uint8_t DCMI_SingleRandomRead(uint8_t Device, uint16_t Addr)
{
	uint8_t data = 0;

	//if(I2C_ReadAddr(I2C2, &data, 1, Device, Addr) == Error)
	if(I2C_ReadAddr(&data, 1, Device, Addr) == Error)
		return 0xFF;

	return data;
/*
  uint32_t timeout = DCMI_TIMEOUT_MAX;
  uint8_t Data = 0;

  // Generate the Start Condition
  I2C_GenerateSTART(I2C2, ENABLE);

  // Test on I2C1 EV5 and clear it
  timeout = DCMI_TIMEOUT_MAX; //
  while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT))
  {
    // If the timeout _delay_ms is exeeded, exit with error code
    if ((timeout--) == 0) return 0xFF;
  } 
  
  // Send DCMI selcted device slave Address for write
  I2C_Send7bitAddress(I2C2, Device, I2C_Direction_Transmitter);
 
  // Test on I2C1 EV6 and clear it
  timeout = DCMI_TIMEOUT_MAX; // Initialize timeout value
  while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
  {
    // If the timeout _delay_ms is exeeded, exit with error code
    if ((timeout--) == 0) return 0xFF;
  } 

  // Send I2C1 location address LSB
  I2C_SendData(I2C2, (uint8_t)(Addr));

  // Test on I2C1 EV8 and clear it
  timeout = DCMI_TIMEOUT_MAX; // Initialize timeout value
  while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
  {
    // If the timeout _delay_ms is exeeded, exit with error code
    if ((timeout--) == 0) return 0xFF;
  } 
  
  // Clear AF flag if arised
  I2C2->SR1 |= (uint16_t)0x0400; 

  // Generate the Start Condition
  I2C_GenerateSTART(I2C2, ENABLE);
  
  // Test on I2C1 EV6 and clear it
  timeout = DCMI_TIMEOUT_MAX; // Initialize timeout value
  while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT))
  {
    // If the timeout _delay_ms is exeeded, exit with error code
    if ((timeout--) == 0) return 0xFF;
  } 
  
  // Send DCMI selcted device slave Address for write
  I2C_Send7bitAddress(I2C2, Device, I2C_Direction_Receiver);
   
  // Test on I2C1 EV6 and clear it
  timeout = DCMI_TIMEOUT_MAX; // Initialize timeout value
  while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
  {
    // If the timeout _delay_ms is exeeded, exit with error code
    if ((timeout--) == 0) return 0xFF;
  }  
 
  // Prepare an NACK for the next data received
  I2C_AcknowledgeConfig(I2C2, DISABLE);

  // Test on I2C1 EV7 and clear it
  timeout = DCMI_TIMEOUT_MAX; // Initialize timeout value
  while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED))
  {
    // If the timeout _delay_ms is exeeded, exit with error code
    if ((timeout--) == 0) return 0xFF;
  }   
    
  // Prepare Stop after receiving data
  I2C_GenerateSTOP(I2C2, ENABLE);

  // Receive the Data
  Data = I2C_ReceiveData(I2C2);

  // return the read data
  return Data;
*/
}

/**
  * @}
  */ 


uint8_t cameraInit() {
	I2C_LowLevel_Init();

	// Reset and check the presence of the OV9655 camera module
	if (DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x12, 0x80))
	{
		return (0xFF);
	}
	_delay_ms(TIMEOUT);

	// OV9655 Camera size setup //
	//DCMI_OV9655_QVGASizeSetup_adv();
	DCMI_OV9655_QVGASizeSetup();

	// Set the RGB565 mode //
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS, OV9655_COM7, 0x63); //0x63
	_delay_ms(TIMEOUT);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS, OV9655_COM15, 0x10);
	_delay_ms(TIMEOUT);
	/*DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS, 0xC0, 0x00);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x4f,0x3a);	 // REG[4F-58] are Matrix coefficients
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x50,0x3d);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x51,0x03);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x52,0x12);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x53,0x26);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x54,0x38);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x55,0x40);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x56,0x40);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x57,0x40);
	DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS,0x58,0x0d);*/
  /* Invert the HRef signal*/
  //DCMI_SingleRandomWrite(OV9655_DEVICE_WRITE_ADDRESS, OV9655_COM10, 0x08);

  /* Configure the DCMI to interface with the OV9655 camera module */
  //DCMI_Config();

  return (0x00);
}

/*********** Portions COPYRIGHT 2012 Embest Tech. Co., Ltd.*****END OF FILE****/
