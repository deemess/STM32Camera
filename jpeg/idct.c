#include <math.h>
#include "idct.h"

// inverse real DCT
void idct(short data[8][8], short pixel[8][8])
{
	//CACHE_ALIGN float rows[8][8];
	float rows[8][8];
	unsigned i;

	static const float // Ci = cos(i*PI/16);
        C1 = 0.98078528040323044912618223613424,
        C2 = 0.92387953251128675612818318939679,
        C3 = 0.83146961230254523707878837761791,
        C4 = 0.70710678118654752440084436210485,
        C5 = 0.55557023301960222474283081394853,
        C6 = 0.38268343236508977172845998403040,
        C7 = 0.19509032201612826784828486847702;

	/* transform rows */
	for (i = 0; i < 8; i++)
	{
		const short x0 = data[i][0];
		const short x4 = data[i][4];
		const short x2 = data[i][2];
		const short x6 = data[i][6];

		const float t0 = C4*(x0 + x4);
		const float t4 = C4*(x0 - x4);
		const float t2 = (C2*x2 + C6*x6);
		const float t6 = (C6*x2 - C2*x6);

		const short x1 = data[i][1];
		const short x3 = data[i][3];
		const short x5 = data[i][5];
		const short x7 = data[i][7];

		const float e0 = t0 + t2;
		const float e3 = t0 - t2;
		const float e1 = t4 + t6;
		const float e2 = t4 - t6;

		const float o0 = (C1*x1 + C5*x5 + C3*x3 + C7*x7);
		const float o1 = (C3*x1 - C1*x5 - C7*x3 - C5*x7);
		const float o2 = (C5*x1 + C7*x5 - C1*x3 + C3*x7);
		const float o3 = (C7*x1 + C3*x5 - C5*x3 - C1*x7);

		rows[i][0] = e0 + o0;
		rows[i][7] = e0 - o0;
		rows[i][1] = e1 + o1;
		rows[i][6] = e1 - o1;
		rows[i][2] = e2 + o2;
		rows[i][5] = e2 - o2;
		rows[i][3] = e3 + o3;
		rows[i][4] = e3 - o3;
	}

	/* transform columns */
	for (i = 0; i < 8; i++)
	{
		const float x0 = rows[0][i];
		const float x4 = rows[4][i];
		const float x2 = rows[2][i];
		const float x6 = rows[6][i];

		const float t0 = C4*(x0 + x4);
		const float t4 = C4*(x0 - x4);
		const float t2 = (C2*x2 + C6*x6);
		const float t6 = (C6*x2 - C2*x6);

		const float x1 = rows[1][i];
		const float x3 = rows[3][i];
		const float x5 = rows[5][i];
		const float x7 = rows[7][i];

		const float e0 = t0 + t2;
		const float e3 = t0 - t2;
		const float e1 = t4 + t6;
		const float e2 = t4 - t6;

		const float o0 = (C1*x1 + C5*x5 + C3*x3 + C7*x7);
		const float o1 = (C3*x1 - C1*x5 - C7*x3 - C5*x7);
		const float o2 = (C5*x1 + C7*x5 - C1*x3 + C3*x7);
		const float o3 = (C7*x1 + C3*x5 - C5*x3 - C1*x7);

		pixel[0][i] = (e0 + o0)/4;
		pixel[7][i] = (e0 - o0)/4;
		pixel[1][i] = (e1 + o1)/4;
		pixel[6][i] = (e1 - o1)/4;
		pixel[2][i] = (e2 + o2)/4;
		pixel[5][i] = (e2 - o2)/4;
		pixel[3][i] = (e3 + o3)/4;
		pixel[4][i] = (e3 - o3)/4;
	}
}

void idct3(short data[8][8], short pixel[8][8])
{
	//CACHE_ALIGN short rows[8][8];
	short rows[8][8];
	unsigned i;

	static const short // Ci = cos(i*PI/16)*(1<<14);
        C1 = 16070,
        C2 = 15137,
        C3 = 13623,
        C4 = 11586,
        C5 = 9103,
        C6 = 6270,
        C7 = 3196;

	/* transform rows */
	for (i = 0; i < 8; i++)
	{
		const short x0 = data[i][0];
		const short x4 = data[i][4];
		const short t0 = C4*(x0 + x4) >> 14;
		const short t4 = C4*(x0 - x4) >> 14;

		const short x2 = data[i][2];
		const short x6 = data[i][6];
		const short t2 = (C2*x2 + C6*x6) >> 14;
		const short t6 = (C6*x2 - C2*x6) >> 14;

		const short e0 = t0 + t2;
		const short e3 = t0 - t2;
		const short e1 = t4 + t6;
		const short e2 = t4 - t6;

		const short x1 = data[i][1];
		const short x3 = data[i][3];
		const short x5 = data[i][5];
		const short x7 = data[i][7];
		const short o0 = (C1*x1 + C5*x5 + C3*x3 + C7*x7) >> 14;
		const short o1 = (C3*x1 - C1*x5 - C7*x3 - C5*x7) >> 14;
		const short o2 = (C5*x1 + C7*x5 - C1*x3 + C3*x7) >> 14;
		const short o3 = (C7*x1 + C3*x5 - C5*x3 - C1*x7) >> 14;

		rows[i][0] = e0 + o0;
		rows[i][7] = e0 - o0;
		rows[i][1] = e1 + o1;
		rows[i][6] = e1 - o1;
		rows[i][2] = e2 + o2;
		rows[i][5] = e2 - o2;
		rows[i][3] = e3 + o3;
		rows[i][4] = e3 - o3;
	}

	/* transform columns */
	for (i = 0; i < 8; i++)
	{
		const short x0 = rows[0][i];
		const short x4 = rows[4][i];
		const short t0 = C4*(x0 + x4) >> 16;
		const short t4 = C4*(x0 - x4) >> 16;

		const short x2 = rows[2][i];
		const short x6 = rows[6][i];
		const short t2 = (C2*x2 + C6*x6) >> 16;
		const short t6 = (C6*x2 - C2*x6) >> 16;

		const short e0 = t0 + t2;
		const short e3 = t0 - t2;
		const short e1 = t4 + t6;
		const short e2 = t4 - t6;

		const short x1 = rows[1][i];
		const short x3 = rows[3][i];
		const short x5 = rows[5][i];
		const short x7 = rows[7][i];
		const short o0 = (C1*x1 + C5*x5 + C3*x3 + C7*x7) >> 16;
		const short o1 = (C3*x1 - C1*x5 - C7*x3 - C5*x7) >> 16;
		const short o2 = (C5*x1 + C7*x5 - C1*x3 + C3*x7) >> 16;
		const short o3 = (C7*x1 + C3*x5 - C5*x3 - C1*x7) >> 16;

		pixel[0][i] = e0 + o0;
		pixel[7][i] = e0 - o0;
		pixel[1][i] = e1 + o1;
		pixel[6][i] = e1 - o1;
		pixel[2][i] = e2 + o2;
		pixel[5][i] = e2 - o2;
		pixel[3][i] = e3 + o3;
		pixel[4][i] = e3 - o3;
	}
}
