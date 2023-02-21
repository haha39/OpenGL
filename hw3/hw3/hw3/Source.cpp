/***************************
4108056041 陳璽文 第三次作業10/27
***************************/

#include <Windows.h>
#include <GL\glew.h>
#include <GL\freeglut.h>
#include <iostream>

#define width 1540
#define height 440
#define halfWidth 770.0f
#define halfHeight 220.0f

static GLfloat spin = 0.0;
static GLint mouseClick = 0;
static GLint keyboardpress = 0;


class Poly
{
protected:
	float startX, startY;
	void drawC(int(*coordinate)[2], int len)
	{
		if (keyboardpress == 1) {		//keyboard press 1, should use glDrawArrays() to draw polygons
			float* buffer = (float*)malloc(2 * len * sizeof(float));
			for (int i = 0; i < len; i++) {
				*(buffer + 2*i) = ((coordinate[i][0] - startX) / startX);
				*(buffer + 2*i+1) = ((startY - coordinate[i][1]) / startY);
			}
			glEnableClientState(GL_VERTEX_ARRAY);

			glVertexPointer(2, GL_FLOAT, 0, buffer);
			glDrawArrays(GL_POLYGON, 0, len);
			glDisableClientState(GL_VERTEX_ARRAY);
			free(buffer);
		}
		else if (keyboardpress == 2) {	//keyboard press 2, should use glDrawElements() to draw polygons
			float* buffer = (float*)malloc(2 * len * sizeof(float));
			for (int i = 0; i < len; i++) {
				*(buffer + 2 * i) = ((coordinate[i][0] - startX) / startX);
				*(buffer + 2 * i + 1) = ((startY - coordinate[i][1]) / startY);
			}
			unsigned int* offset = (unsigned int*)malloc(len * sizeof(unsigned int));
			for (int i = 0; i < len; i++) {
				*(offset + i) = i;
			}
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(2, GL_FLOAT, 0, buffer);
			glDrawElements(GL_POLYGON, len, GL_UNSIGNED_INT, offset);
			glDisableClientState(GL_VERTEX_ARRAY);
			free(offset);
			free(buffer);
		}
		else if (keyboardpress == 3) {	//keyboard press 3, should use glMultiDrawArrays() to draw polygons

		}
		else if (keyboardpress == 4) {	//keyboard press 4, should use glMultiDrawElements() to draw polygons

		}
		else {
			glBegin(GL_POLYGON);
			for (int i = 0; i < len; i++)
			{
				glVertex2f(((coordinate[i][0] - startX) / startX), ((startY - coordinate[i][1]) / startY));
			}
			glEnd();
		}
	}
public:
	Poly(float w, float h) {
		startX = w / 2;
		startY = h / 2;
	}
};

class Tri
{
protected:
	float startX, startY;
	void drawE(int(*coordinate)[2], int move)
	{
		if (move == 0)
		{
			glBegin(GL_TRIANGLES);
			for (int i = 0; i < 3; i++)
			{
				glVertex2f(((coordinate[i][0] - startX) / startX), ((startY - coordinate[i][1]) / startY));
			}
			glEnd();
		}
		else
		{
			glBegin(GL_TRIANGLES);
			for (int i = 0; i < 3; i++)
			{
				glVertex2f(((coordinate[i][0] + move - startX) / startX), ((startY - coordinate[i][1]) / startY));
			}
			glEnd();
		}
	}
public:
	Tri(float w, float h) {
		startX = w / 2;
		startY = h / 2;
	}
};


/////////////////////////////國//////////////////////////////////////
class A :protected Poly
{
private:
	int a1[5][2] = { {7,63},{2,190},{6,196},{17,195},{17,60} };
	int a2[5][2] = { {17,60},{17,77},{139,48},{148,38},{134,32} };
	int a3[4][2] = { {139,48},{139,190},{156,201},{148,38} };
	int a4[5][2] = { {139,190},{129,193},{138,216},{146,216},{156,201} };
	int a5[5][2] = { {17,77},{17,117},{40,104},{40,89},{28,70} };
	int a6[5][2] = { {40,89},{40,104},{102,78},{17,117},{28,77} };
	int a7[5][2] = { {40,89},{40,104},{104,77},{98,67},{81,73} };
	int a8[4][2] = { {81,50},{78,78},{98,67},{93,50} };
	int a9[4][2] = { {98,67},{104,77},{131,77},{126,64} };
	int a10[4][2] = { {100,52},{106,67},{126,64},{115,48} };
	int a11[4][2] = { {17,170},{17,195},{129,180},{130,169} };
	int a12[5][2] = { {55,97},{22,129},{23,137},{43,128},{57,109} };
	int a13[7][2] = { {43,128},{26,146},{28,160},{75,122},{103,93},{87,89},{57,109} };
	int a14[5][2] = { {26,146},{28,160},{96,134},{98,120},{75,122} };
	int a15[6][2] = { {81,73},{87,89},{130,161},{137,147},{103,93},{102,75} };
	int a16[4][2] = { {115,88},{60,172},{94,169},{125,100} };
	/*int a1[][2] = { {,},{,},{,},{,},{,} };*/

public:
	A(float w, float h) :Poly(width, height) {}
	void displayC()
	{
		drawC(a1, sizeof(a1) / sizeof(a1[0]));
		drawC(a2, sizeof(a2) / sizeof(a2[0]));
		drawC(a3, sizeof(a3) / sizeof(a3[0]));
		drawC(a4, sizeof(a4) / sizeof(a4[0]));
		drawC(a5, sizeof(a5) / sizeof(a5[0]));
		drawC(a6, sizeof(a6) / sizeof(a6[0]));
		drawC(a7, sizeof(a7) / sizeof(a7[0]));
		drawC(a8, sizeof(a8) / sizeof(a8[0]));
		drawC(a9, sizeof(a9) / sizeof(a9[0]));
		drawC(a10, sizeof(a10) / sizeof(a10[0]));
		drawC(a11, sizeof(a11) / sizeof(a11[0]));
		drawC(a12, sizeof(a12) / sizeof(a12[0]));
		drawC(a13, sizeof(a13) / sizeof(a13[0]));
		drawC(a14, sizeof(a14) / sizeof(a14[0]));
		drawC(a15, sizeof(a15) / sizeof(a15[0]));
		drawC(a16, sizeof(a16) / sizeof(a16[0]));

	}
};
/////////////////////////////立//////////////////////////////////////
class B :protected Poly
{
private:
	int b1[5][2] = { {270,40},{269,59},{285,78},{311,73},{312,66} };
	int b2[6][2] = { {227,90},{222,107},{232,113},{310,89},{311,73},{285,78} };
	int b3[6][2] = { {310,89},{334,91},{340,86},{340,77},{332,73},{311,73} };
	int b4[6][2] = { {291,94},{251,126},{249,143},{260,146},{287,112},{310,89} };
	int b5[4][2] = { {287,112},{262,143},{313,111},{323,104} };
	int b6[5][2] = { {323,104},{313,110},{325,111},{343,112},{339,106} };
	int b7[4][2] = { {325,111},{275,165},{296,163},{343,112} };
	int b8[7][2] = { {224,174},{240,189},{369,180},{369,170},{359,162},{296,163},{275,165} };
	/*int b[][2] = { {,},{,},{,},{,},{,} };*/

public:
	B(float w, float h) :Poly(width, height) {}
	void displayC()
	{
		drawC(b1, sizeof(b1) / sizeof(b1[0]));
		drawC(b2, sizeof(b2) / sizeof(b2[0]));
		drawC(b3, sizeof(b3) / sizeof(b3[0]));
		drawC(b4, sizeof(b4) / sizeof(b4[0]));
		drawC(b5, sizeof(b5) / sizeof(b5[0]));
		drawC(b6, sizeof(b6) / sizeof(b6[0]));
		drawC(b7, sizeof(b7) / sizeof(b7[0]));
		drawC(b8, sizeof(b8) / sizeof(b8[0]));
		//drawC(b, sizeof(b) / sizeof(b[0]));
	}
};
/////////////////////////////中//////////////////////////////////////
class C :protected Poly
{
private:
	int c1[4][2] = { {403,70},{400,100},{421,135}/*,{409,124},{427,135},{427,131},{423,135},{420,86},{416,82}*/,{418,70} };
	int c2[4][2] = { {418,70},{418,77},{490,68},{484,56} };
	int c3[4][2] = { {484,38},{484,56},{500,56},{500,34} };
	int c4[4][2] = { {484,56},{490,68},{550,68},{541,56} };
	int c5[4][2] = { {426,70},{435,87},{444,87},{479,69} };
	int c6[4][2] = { {490,68},{481,261},{500,253},{500,34} };
	int c7[4][2] = { {490,94},{456,114},{451,131},{489,123} };
	int c8[4][2] = { {541,56},{490,94},{500,99},{550,68} };
	int c9[4][2] = { {500,108},{500,122},{519,120},{523,109} };

	/*int c[][2] = { {,},{,},{,},{,},{,} };*/

public:
	C(float w, float h) :Poly(width, height) {}
	void displayC()
	{
		drawC(c1, sizeof(c1) / sizeof(c1[0]));
		drawC(c2, sizeof(c2) / sizeof(c2[0]));
		drawC(c3, sizeof(c3) / sizeof(c3[0]));
		drawC(c4, sizeof(c4) / sizeof(c4[0]));
		drawC(c5, sizeof(c5) / sizeof(c5[0]));
		drawC(c6, sizeof(c6) / sizeof(c6[0]));
		drawC(c7, sizeof(c7) / sizeof(c7[0]));
		drawC(c8, sizeof(c8) / sizeof(c8[0]));
		drawC(c9, sizeof(c9) / sizeof(c9[0]));
		//drawC(c1, sizeof(c1) / sizeof(c1[0]));
	}
};
/////////////////////////////興//////////////////////////////////////
class D :protected Poly
{
private:
	int d1[7][2] = { {617,51},{613,58},{617,60},{617,130},{631,129},{631,57},{627,51} };
	int d2[6][2] = {/* {627,51},*/{639,56},/*{639,127}*/{639,136},{645,144},{649,124},{649,56},{644,47} };
	int d2_5[4][2] = { {627,51},{635,51},{639,56},{644,47} };
	int d3[5][2] = { {655,50},{660,52},{660,137},{669,131},{670,43} };
	int d4[6][2] = { {670,43},{669,50},{705,48},{705,35},{701,31},{691,31} };
	int d5[4][2] = { {669,64},{669,81},{684,82},{684,62} };
	int d6[5][2] = { {677,82},{669,91},{669,114},{679,118},{680,82} };
	int d7[4][2] = { {684,45},{684,144},{696,136},{696,45} };
	int d8[6][2] = { {700,43},{701,53},{733,53},{741,33},{730,33},{716,43} };
	int d9[4][2] = { {696,45},{696,76},{701,67},{701,45} };
	int d10[4][2] = { {701,67},{696,76},{724,98},{731,90} };
	int d11[4][2] = { {680,95},{680,102},{684,102},{684,94} };
	int d12[3][2] = { {696,90},{731,90},{707,86} };
	int d13[4][2] = { {696,90},{706,111},{713,111},{702,94} };
	int d14[4][2] = { {718,53},{708,136},{718,122},{733,53} };
	int d15[9][2] = { {554,145},{554,161},{565,169},{679,120},{768,121},{771,113},{763,104},{753,110},{617,130} };
	int d16[6][2] = { {594,163},{599,180},{615,196},{616,169} };
	int d17[4][2] = { {616,169},{615,196},{648,177},{656,162} };
	int d18[3][2] = { {662,160},{662,163},{686,153} };
	int d19[4][2] = { {720,146},{760,200},{765,190},{768,175} };

	/*int d[][2] = { {,},{,},{,},{,},{,} };*/

public:
	D(float w, float h) :Poly(width, height) {}
	void displayC()
	{
		drawC(d1, sizeof(d1) / sizeof(d1[0]));
		drawC(d2, sizeof(d2) / sizeof(d2[0]));
		drawC(d2_5, sizeof(d2_5) / sizeof(d2_5[0]));
		drawC(d3, sizeof(d3) / sizeof(d3[0]));
		drawC(d4, sizeof(d4) / sizeof(d4[0]));
		drawC(d5, sizeof(d5) / sizeof(d5[0]));
		drawC(d6, sizeof(d6) / sizeof(d6[0]));
		drawC(d7, sizeof(d7) / sizeof(d7[0]));
		drawC(d8, sizeof(d8) / sizeof(d8[0]));
		drawC(d9, sizeof(d9) / sizeof(d9[0]));
		drawC(d10, sizeof(d10) / sizeof(d10[0]));
		drawC(d11, sizeof(d11) / sizeof(d11[0]));
		drawC(d12, sizeof(d12) / sizeof(d12[0]));
		drawC(d13, sizeof(d13) / sizeof(d13[0]));
		drawC(d14, sizeof(d14) / sizeof(d14[0]));
		drawC(d15, sizeof(d15) / sizeof(d15[0]));
		drawC(d16, sizeof(d16) / sizeof(d16[0]));
		drawC(d17, sizeof(d17) / sizeof(d17[0]));
		drawC(d18, sizeof(d18) / sizeof(d18[0]));
		drawC(d19, sizeof(d19) / sizeof(d19[0]));

		/*drawC(d, sizeof(d) / sizeof(d[0]));*/
	}
};
/////////////////////////////大//////////////////////////////////////
class E :protected Poly
{
private:
	int e1[4][2] = { {909,36},{893,90},{902,90},{914,36} };
	int e2[4][2] = { {909,36},{907,90},{911,90},{914,36} };
	int e3[8][2] = { {820,108},{823,124},{830,132},{847,125},{859,113},{928,96},{933,89},{931,78} };
	int e4[8][2] = { {886,105},{878,128},{836,167},{844,163},{847,168},{868,164},{886,140},{897,99} };
	int e5[3][2] = { {907,130},{953,175},{963,152} };
	int e6[3][2] = { {938,152},{915,166},{948,166} };

	/*int e[][2] = { {,},{,},{,},{,},{,} };*/

public:
	E(float w, float h) :Poly(width, height) {}
	void displayC()
	{
		drawC(e1, sizeof(e1) / sizeof(e1[0]));
		drawC(e2, sizeof(e2) / sizeof(e2[0]));
		drawC(e3, sizeof(e3) / sizeof(e3[0]));
		drawC(e4, sizeof(e4) / sizeof(e4[0]));
		drawC(e5, sizeof(e5) / sizeof(e5[0]));
		drawC(e6, sizeof(e6) / sizeof(e6[0]));

		/*drawC(e, sizeof(e) / sizeof(e[0]));*/
	}
};
/////////////////////////////學//////////////////////////////////////
class F :protected Poly
{
private:
	int f1[7][2] = { {1005,121},{988,140},{984,146},{984,162},{988,168},{995,168},{1005,147} };
	int f2[6][2] = { {1005,126},{1136,90},{1128,104},{1118,102},{1023,131},{1005,147} };
	int f3[6][2] = { {1136,90},{1128,104},{1109,124},{1127,113},{1148,110},{1151,102} };
	int f4[4][2] = { {1038,26},{1015,55},{1013,69},{1046,39} };
	int f5[4][2] = { {1018,40},{1018,125},{1030,120},{1027,45} };
	int f6[4][2] = { {1027,53},{1027,74},{1040,66},{1040,52} };
	int f7[4][2] = { {1074,26},{1027,90},{1027,103},{1082,37} };
	int f8[4][2] = { {1042,54},{1045,61},{1085,61},{1086,51} };
	int f9[4][2] = { {1073,61},{1050,115},{1068,111},{1079,71} };
	int f10[4][2] = { {1045,76},{1045,84},{1076,95},{1077,90} };
	int f11[5][2] = { {1079,44},{1088,50},{1118,50},{1124,32},{1115,30} };
	int f12[8][2] = { {1106,50},{1103,56},{1079,58},{1079,90},{1093,90},{1087,108},{1106,103},{1118,50} };
	int f13[7][2] = { {1083,115},{1060,131},{1065,137},{1078,138},{1087,138},{1094,126},{1092,119} };
	int f14[8][2] = { {1078,138},{1072,144},{1071,149},{1038,153},{1034,159},{1040,162},{1081,152},{1087,138} };
	int f15[7][2] = { {1087,138},{1081,152},{1124,155},{1128,150},{1125,145},{1095,144},{1087,138} };
	int f16[4][2] = { {1081,152},{1079,203}/*,{1025,241},{982,241},{1025,254}*/,{1092,215},{1094,150} };
	int f17[4][2] = { {1079,203},{1025,241}/*,{982,241}*/,{1025,254},{1092,215} };
	int f18[3][2] = { {1025,241},{982,241},{1025,254} };

	/*int f[][2] = { {1,},{1,},{1,},{1,},{1,} };*/

public:
	F(float w, float h) :Poly(width, height) {}
	void displayC()
	{
		drawC(f1, sizeof(f1) / sizeof(f1[0]));
		drawC(f2, sizeof(f2) / sizeof(f2[0]));
		drawC(f3, sizeof(f3) / sizeof(f3[0]));
		drawC(f4, sizeof(f4) / sizeof(f4[0]));
		drawC(f5, sizeof(f5) / sizeof(f5[0]));
		drawC(f6, sizeof(f6) / sizeof(f6[0]));
		drawC(f7, sizeof(f7) / sizeof(f7[0]));
		drawC(f8, sizeof(f8) / sizeof(f8[0]));
		drawC(f9, sizeof(f9) / sizeof(f9[0]));
		drawC(f10, sizeof(f10) / sizeof(f10[0]));
		drawC(f11, sizeof(f11) / sizeof(f11[0]));
		drawC(f12, sizeof(f12) / sizeof(f12[0]));
		drawC(f13, sizeof(f13) / sizeof(f13[0]));
		drawC(f14, sizeof(f14) / sizeof(f14[0]));
		drawC(f15, sizeof(f15) / sizeof(f15[0]));
		drawC(f16, sizeof(f16) / sizeof(f16[0]));
		drawC(f17, sizeof(f17) / sizeof(f17[0]));
		drawC(f18, sizeof(f18) / sizeof(f18[0]));

		/*drawC(f, sizeof(f) / sizeof(f[0]));*/
	}
};
/////////////////////////////logo 白色線線///////////////////////////
class G :protected Poly
{
private:
	int g1[4][2] = { {1286,13},{1290,24},{1366,63},{1377,59} };
	int g2[4][2] = { {1366,63},{1433,140},{1437,131},{1377,59} };
	int g3[4][2] = { {1433,140},{1457,190},{1456,178},{1437,131} };

	/*int g[][2] = { {1,},{1,},{1,},{1,},{1,} };*/

public:
	G(float w, float h) :Poly(width, height) {}
	void displayC()
	{
		drawC(g1, sizeof(g1) / sizeof(g1[0]));
		drawC(g2, sizeof(g2) / sizeof(g2[0]));
		drawC(g3, sizeof(g3) / sizeof(g3[0]));

		/*drawC(g, sizeof(g) / sizeof(g[0]));*/
	}
};
/////////////////////////////logo 紅色圈圈///////////////////////////
class H :protected Poly
{
private:
	int h1[5][2] = { {1366,63},{1361,91},{1373,104},{1389,110},{1402,104} };
	int h2[4][2] = { {1377,59},{1414,96},{1413,64},{1403,60} };
	/*int h[][2] = { {1,},{1,},{1,},{1,},{1,} };*/

public:
	H(float w, float h) :Poly(width, height) {}
	void displayC()
	{
		drawC(h1, sizeof(h1) / sizeof(h1[0]));
		drawC(h2, sizeof(h2) / sizeof(h2[0]));

		/*drawC(h, sizeof(h) / sizeof(h[0]));*/
	}
};

class I :protected Poly
{
private:
	int i1[6][2] = { {1366,63},{1349,81},{1344,107},{1373,133},{1402,132},{1420,124} };
	int i2[4][2] = { {1377,59},{1429,116},{1433,76},{1403,60} };

	/*int i[][2] = { {1,},{1,},{1,},{1,},{1,} };*/

public:
	I(float w, float h) :Poly(width, height) {}
	void displayC()
	{
		drawC(i1, sizeof(i1) / sizeof(i1[0]));
		drawC(i2, sizeof(i2) / sizeof(i2[0]));

		/*drawC(i, sizeof(i) / sizeof(i[0]));*/
	}
};

class J :protected Poly
{
private:
	int j1[7][2] = { {1366,63},{1349,80},{1346,107},{1352,135},{1372,150},{1406,154},{1433,137} };
	int j2[6][2] = { {1377,59},{1437,131},{1447,114},{1433,76},{1413,64},{1403,60} };

	/*int h[][2] = { {1,},{1,},{1,},{1,},{1,} };*/

public:
	J(float w, float h) :Poly(width, height) {}
	void displayC()
	{
		drawC(j1, sizeof(j1) / sizeof(j1[0]));
		drawC(j2, sizeof(j2) / sizeof(j2[0]));

		/*drawC(h, sizeof(h) / sizeof(h[0]));*/
	}
};
/////////////////////////////logo 藍色咚咚///////////////////////////
class K :protected Poly
{
private:
	int k1[5][2] = { {1290,24},{1298,115},{1346,107},{1349,80},{1366,63} };
	int k2[9][2] = { {1298,115},{1291,155},{1266,225},{1333,200},{1395,195},{1395,157},{1372,150},{1352,135},{1346,107} };
	int k3[8][2] = { {1281,3},{1286,13},{1377,59},{1403,60},{1413,64},{1433,76},{1507,3},{1396,18} };
	int k4[4][2] = { {1507,3},{1433,76},{1447,114},{1487,59} };
	int k5[6][2] = { {1487,59},{1447,114},{1437,131},{1456,178},{1457,190},{1488,114}, };
	int k6[5][2] = { {1488,114},{1456,178},{1457,190},{1499,202},{1494,159} };
	int k7[3][2] = { {1494,159},{1499,202},{1523,230} };
	int k8[6][2] = { {1433,140},{1406,154},{1395,155},{1395,195},{1499,202},{1457,190} };

	/*int k[][2] = { {1,},{1,},{1,},{1,},{1,} };*/

public:
	K(float w, float h) :Poly(width, height) {}
	void displayC()
	{
		drawC(k1, sizeof(k1) / sizeof(k1[0]));
		drawC(k2, sizeof(k2) / sizeof(k2[0]));
		drawC(k3, sizeof(k3) / sizeof(k3[0]));
		drawC(k4, sizeof(k4) / sizeof(k4[0]));
		drawC(k5, sizeof(k5) / sizeof(k5[0]));
		drawC(k6, sizeof(k6) / sizeof(k6[0]));
		drawC(k7, sizeof(k7) / sizeof(k7[0]));
		drawC(k8, sizeof(k8) / sizeof(k8[0]));

		/*drawC(k, sizeof(k) / sizeof(k[0]));*/
	}
};
/////////////////////////////N//////////////////////////////////////
class L :protected Tri
{
private:
	int l1[3][2] = { {14,353},{14,419},{25,419} };
	int l2[3][2] = { {14,353},{25,353},{25,419} };
	int l3[3][2] = { {25,353},{25,367},{58,419} };
	int l4[3][2] = { {58,419},{58,405},{25,353} };
	int l5[3][2] = { {58,353},{58,419},{69,419} };
	int l6[3][2] = { {58,353},{69,353},{69,419} };

	/*int l[3][2] = { {,},{,},{,} };*/

public:
	L(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//N
		drawE(l1, 0);
		drawE(l2, 0);
		drawE(l3, 0);
		drawE(l4, 0);
		drawE(l5, 0);
		drawE(l6, 0);
		//l
		drawE(l1, 375);
		drawE(l2, 375);
		//h
		drawE(l1, 498);
		drawE(l2, 498);
		//H
		drawE(l1, 759);
		drawE(l2, 759);
		//H
		drawE(l1, 804);
		drawE(l2, 804);
		/*drawC(f);*/
	}
};
/////////////////////////////a//////////////////////////////////////
class M :protected Tri
{
private:
	int m1[3][2] = { {88,384},{96,385},{98,372} };
	int m2[3][2] = { {98,372},{95,378},{122,372} };
	int m3[3][2] = { {95,378},{122,372},{130,378} };
	int m4[3][2] = { {122,372},{122,418},{130,378} };
	int m5[3][2] = { {130,378},{122,418},{130,418} };
	int m6[3][2] = { {122,390},{122,396},{107,396} };
	int m7[3][2] = { {122,390},{99,393},{99,401} };
	int m8[3][2] = { {87,411},{99,393},{99,401} };
	int m9[3][2] = { {87,411},{99,420},{99,401} };
	int m10[3][2] = { {113,414},{99,420},{99,414} };
	int m11[3][2] = { {122,406},{122,412},{99,420} };

	/*int m[3][2] = { {,},{,},{,} };*/

public:
	M(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		drawE(m1, 0);
		drawE(m2, 0);
		drawE(m3, 0);
		drawE(m4, 0);
		drawE(m5, 0);
		drawE(m6, 0);
		drawE(m7, 0);
		drawE(m8, 0);
		drawE(m9, 0);
		drawE(m10, 0);
		drawE(m11, 0);

		drawE(m1, 238);
		drawE(m2, 238);
		drawE(m3, 238);
		drawE(m4, 238);
		drawE(m5, 238);
		drawE(m6, 238);
		drawE(m7, 238);
		drawE(m8, 238);
		drawE(m9, 238);
		drawE(m10, 238);
		drawE(m11, 238);
		/*drawC(m);*/
	}
};
/////////////////////////////t//////////////////////////////////////
class N :protected Tri
{
private:
	int n1[3][2] = { {144,372},{144,379},{168,379} };
	int n2[3][2] = { {168,372},{144,372},{168,379} };
	int n3[3][2] = { {160,355},{150,360},{150,417} };
	int n4[3][2] = { {160,355},{160,420},{150,417} };
	int n5[3][2] = { {160,413},{160,420},{170,417} };
	/*int n6[3][2] = { {58,353},{69,353},{69,419} };

	int n[3][2] = { {,},{,},{,} };*/

public:
	N(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		drawE(n1, 0);
		drawE(n2, 0);
		drawE(n3, 0);
		drawE(n4, 0);
		drawE(n5, 0);

		drawE(n1, 1316);
		drawE(n2, 1316);
		drawE(n3, 1316);
		drawE(n4, 1316);
		drawE(n5, 1316);

		/*drawC(n); */
	}
};
/////////////////////////////i//////////////////////////////////////
class O :protected Tri
{
private:
	int o1[3][2] = { {182,354},{182,364},{191,364} };
	int o2[3][2] = { {182,354},{191,354},{191,364} };
	int o3[3][2] = { {182,372},{182,420},{191,420} };
	int o4[3][2] = { {182,372},{191,372},{191,420} };

	/*int o[3][2] = { {,},{,},{,} };*/

public:
	O(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//i
		drawE(o1, 0);
		drawE(o2, 0);
		drawE(o3, 0);
		drawE(o4, 0);
		//n
		drawE(o3, 85);
		drawE(o4, 85);
		//u
		drawE(o3, 421);
		drawE(o4, 421);
		//n
		drawE(o3, 448);
		drawE(o4, 448);
		//g
		drawE(o3, 542);
		drawE(o4, 542);
		//i
		drawE(o1, 719);
		drawE(o2, 719);
		drawE(o3, 719);
		drawE(o4, 719);
		//n
		drawE(o3, 746);
		drawE(o4, 746);
		//g
		drawE(o3, 840);
		drawE(o4, 840);
		//n
		drawE(o3, 964);
		drawE(o4, 964);
		//i
		drawE(o1, 1021);
		drawE(o2, 1021);
		drawE(o3, 1021);
		drawE(o4, 1021);
		//r
		drawE(o3, 1164);
		drawE(o4, 1164);
		//i
		drawE(o1, 1256);
		drawE(o2, 1256);
		drawE(o3, 1256);
		drawE(o4, 1256);



		/*drawC(n); */
	}
};
/////////////////////////////o//////////////////////////////////////
class P :protected Tri
{
private:
	int p1[3][2] = { {229,370},{229,377},{219,372} };
	int p2[3][2] = { {219,372},{207,385},{229,377} };
	int p11[3][2] = { {229,370},{229,377},{240,372} };
	int p12[3][2] = { {240,372},{229,377},{251,385} };
	int p3[3][2] = { {207,385},{215,385},{229,370} };
	int p13[3][2] = { {243,385},{251,385},{229,370} };
	int p4[3][2] = { {207,407},{207,385},{215,385} };
	int p14[3][2] = { {251,407},{251,385},{243,385} };
	int p5[3][2] = { {207,407},{215,417},{215,385} };
	int p15[3][2] = { {251,407},{243,418},{243,385} };
	int p6[3][2] = { {207,407},{229,422},{229,414} };
	int p16[3][2] = { {251,407},{229,422},{229,414} };
	int p7[3][2] = { {207,407},{215,417},{229,422} };
	int p17[3][2] = { {251,407},{243,418},{229,422} };

	/*int p[3][2] = { {000,000},{000,000},{000,000} };*/

public:
	P(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//o
		drawE(p1, 0);
		drawE(p2, 0);
		drawE(p3, 0);
		drawE(p11, 0);
		drawE(p12, 0);
		drawE(p13, 0);
		drawE(p4, 0);
		drawE(p14, 0);
		drawE(p5, 0);
		drawE(p15, 0);
		drawE(p6, 0);
		drawE(p16, 0);
		drawE(p7, 0);
		drawE(p17, 0);
		//g
		drawE(p1, 482);
		drawE(p2, 482);
		drawE(p3, 482);
		drawE(p4, 482);
		drawE(p5, 482);
		drawE(p6, 482);
		drawE(p7, 482);
		//g
		drawE(p1, 780);
		drawE(p2, 780);
		drawE(p3, 780);
		drawE(p4, 780);
		drawE(p5, 780);
		drawE(p6, 780);
		drawE(p7, 780);
		//e
		drawE(p1, 1079);
		drawE(p2, 1079);
		drawE(p3, 1079);
		drawE(p4, 1079);
		drawE(p5, 1079);
		drawE(p6, 1079);
		drawE(p7, 1079);
		/*drawC(f);*/
	}
};
/////////////////////////////n//////////////////////////////////////
class Q :protected Tri
{
private:
	int q1[3][2] = { {276,378},{276,386},{292,372} };
	int q2[3][2] = { {292,372},{276,386},{292,378} };
	int q3[3][2] = { {292,372},{292,378},{307,377} };
	int q4[3][2] = { {307,377},{301,383},{301,418} };
	int q5[3][2] = { {301,418},{309,418},{307,377} };
	int q6[3][2] = { {292,378},{301,383},{307,377} };

	/*int [3][2] = { {000,000},{000,000},{000,000} };*/

public:
	Q(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//n
		drawE(q1, 0);
		drawE(q2, 0);
		drawE(q3, 0);
		drawE(q4, 0);
		drawE(q5, 0);
		drawE(q6, 0);
		//n
		drawE(q1, 363);
		drawE(q2, 363);
		drawE(q3, 363);
		drawE(q4, 363);
		drawE(q5, 363);
		drawE(q6, 363);
		//n
		drawE(q1, 661);
		drawE(q2, 661);
		drawE(q3, 661);
		drawE(q4, 661);
		drawE(q5, 661);
		drawE(q6, 661);
		//n
		drawE(q1, 878);
		drawE(q2, 878);
		drawE(q3, 878);
		drawE(q4, 878);
		drawE(q5, 878);
		drawE(q6, 878);

		/*drawC();*/
	}
};
/////////////////////////////C//////////////////////////////////////
class R :protected Tri
{
private:
	int r1[13][3][2] =
	{
		/**/{{496,402},{487,397},{482,418}},
		{ {487,397},{482,418},{469,414} },
		{ {482,418},{469,414},{454,419} },
		{ {469,414},{454,419},{450,406} },
		{ {454,419},{450,406},{435,396} },
		{ {450,406},{435,396},{436,377} },
		{ {435,396},{436,377},{445,376} },
		{ {436,377},{445,376},{455,354} },
		{ {445,376},{455,354},{458,362} },
		{ {455,354},{458,362},{478,354} },
		{ {458,362},{478,354},{477,362} },
		{ {478,354},{477,362},{496,372} },
		{ {477,362},{496,372},{487,375} }
	};
	/*int [3][2] = { {000,000},{000,000},{000,000} };*/

public:
	R(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//C
		for (int i = 0; i < 13; i++)
		{
			drawE(r1[i], 0);
		}

		/*drawC();*/
	}
};
/////////////////////////////h//////////////////////////////////////
class S :protected Tri
{
private:
	int s1[8][3][2] =
	{
		{{520,387},{520,377},{530,371} },
		{ {520,387},{530,371},{530,377} },

		{ {530,371},{530,377},{545,371} },
		{ {530,377},{540,377},{545,371} },

		{ {545,371},{540,377},{553,381} },
		{ {540,377},{553,381} ,{545,384} },

		{ {553,381} ,{545,384},{553,419} },
		{ {545,384},{553,419},{545,420} },

	};
	/*int [3][2] = { {000,000},{000,000},{000,000} };*/

public:
	S(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//C
		for (int i = 0; i < 8; i++)
		{
			drawE(s1[i], 0);
		}

		/*drawC();*/
	}
};
/////////////////////////////u//////////////////////////////////////
class T :protected Tri
{
private:
	int t1[6][3][2] =
	{
		{{605,413},{604,406},{590,421} },
		{ {604,406},{590,421},{590,414} },

		{ {590,421},{590,414},{571,409} },
		{ {590,414},{571,409},{580,409} },

		{ {571,409},{580,409},{571,372} },
		{ {580,409},{571,372},{580,372} },

	};
	/*int [3][2] = { {000,000},{000,000},{000,000} };*/

public:
	T(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//C
		for (int i = 0; i < 6; i++)
		{
			drawE(t1[i], 0);
		}

		/*drawC();*/
	}
};
/////////////////////////////g//////////////////////////////////////
class U :protected Tri
{
private:
	int u1[9][3][2] =
	{
		{{709,420},{709,413},{724,406} },
		{ {709,420},{724,414},{724,406} },

		{ {709,377},{709,370},{724,386} },
		{ {709,370},{724,386},{724,378} },

		{ {688,424},{698,424},{707,439} },
		{ {698,424},{707,439},{703,433} },

		{ {703,433},{707,439},{719,431} },
		{ {707,439},{719,431},{732,420} },

		{ {724,414},{719,431},{732,420} },
	};
	/*int [3][2] = { {000,000},{000,000},{000,000} };*/

public:
	U(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//C
		for (int i = 0; i < 9; i++)
		{
			drawE(u1[i], 0);
			drawE(u1[i], 298);

		}

		/*drawC();*/
	}
};
/////////////////////////////H//////////////////////////////////////
class V :protected Tri
{
private:
	int v1[2][3][2] =
	{
		{{783,380},{783,391},{820,380} },
		{ {783,391},{820,380},{820,391} }

	};
	/*int [3][2] = { {000,000},{000,000},{000,000} };*/

public:
	V(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//C
		for (int i = 0; i < 2; i++)
		{
			drawE(v1[i], 0);
		}

		/*drawC();*/
	}
};
/////////////////////////////s//////////////////////////////////////
class W :protected Tri
{
private:
	int w1[12][3][2] =
	{
		{ {844,405},{855,404},{858,421} },
		{ {855,404},{858,421},{859,413} },

		{ {858,421},{859,413},{874,421} },
		{ {859,413},{874,421},{884,413} },

		{ {874,421},{884,413},{877,404} },
		{ {884,413},{877,404},{882,396} },

		{ {877,404},{882,396},{851,394} },
		{ {882,396},{851,394},{846,381} },

		{ {846,381},{855,387},{854,373} },
		{ {854,373},{856,379},{875,372} },

		{ {872,378},{875,372},{877,385} },
		{ {875,372},{877,385},{885,383} },

		//{ {858,421},{874,421},{859,413} },
		//{ {875,372},{877,385},{885,383} },
	};
	/*int [3][2] = { {000,000},{000,000},{000,000} };*/

public:
	W(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//C
		for (int i = 0; i < 12; i++)
		{
			drawE(w1[i], 0);
			drawE(w1[i], 537);

		}

		/*drawC();*/
	}
};
/////////////////////////////U//////////////////////////////////////
class X :protected Tri
{
private:
	int x1[7][3][2] =
	{
		{ {1070,354},{1081,354},{1074,408} },
		{ {1081,354},{1074,408},{1081,416} },

		{ {1081,416},{1081,404},{1099,421} },
		{ {1118,416},{1118,404},{1099,421} },

		{ {1089,410},{1111,410},{1099,421} },

		{ {1127,354},{1116,354},{1125,408} },
		{ {1116,354},{1125,408},{1118,416} },

	};

public:
	X(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//C
		for (int i = 0; i < 7; i++)
		{
			drawE(x1[i], 0);
		}

		/*drawC();*/
	}
};
/////////////////////////////v//////////////////////////////////////
class Y :protected Tri
{
private:
	int y1[5][3][2] =
	{
		{ {1237,371},{1228,371},{1250,409} },
		{ {1228,371},{1250,409},{1246,419} },

		{ {1250,409},{1246,419},{1255,419} },

		{ {1265,371},{1275,371},{1250,409} },
		{ {1275,371},{1250,409},{1255,419} },

	};

public:
	Y(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//C
		for (int i = 0; i < 5; i++)
		{
			drawE(y1[i], 0);
			drawE(y1[i], 265);

		}

		/*drawC();*/
	}
};
/////////////////////////////v//////////////////////////////////////
class Z :protected Tri
{
private:
	int z1[15][3][2] =
	{
		//o
		{ {1292,398},{1291,392},{1332,397} },
		{ {1291,392},{1332,397},{1321,392} },

		{ {1332,397},{1321,392},{1328,381} },
		{ {1321,392},{1328,381},{1321,375} },

		{ {1308,378},{1308,371},{1321,375} },
		{ {1308,378},{1321,375},{1322,384} },

		{ {1308,414},{1308,422},{1322,405} },
		{ {1308,422},{1322,405},{1330,409} },
		//r
		{ {1354,378},{1355,386},{1364,372} },
		{ {1355,386},{1364,372},{1372,381} },
		{ {1364,372},{1372,381},{1374,374} },
		//y
		{ {1517,430},{1512,419},{1503,439} },
		{ {1503,439},{1508,428},{1496,438} },
		{ {1503,431},{1496,438},{1503,431} },

		{ {1512,419},{1522,419},{1517,430} },
	};

public:
	Z(float w, float h) :Tri(width, height) {}
	void displayC()
	{
		//C
		for (int i = 0; i < 15; i++)
		{
			drawE(z1[i], 0);
		}

		/*drawC();*/
	}
};

void My_Display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();


	if (mouseClick == 1) {		//mouse left click, should Rotate the scene by z-axis
		glRotatef(spin, 0.0, 0.0, 1.0);
	}
	else if (mouseClick == 2) {	//mouse middle click, should Rotate the scene by y-axis
		glRotatef(spin, 0.0, 1.0, 0.0);
	}
	else {						//mouse right click, should Rotate the scene by x-axis
		glRotatef(spin, 1.0, 0.0, 0.0);
	}






	float chColor[3] = {0.0, 0.40f, 0.50f};
	glColor3fv(chColor);







	/////////////////////////////國//////////////////////////////////////
	A a(width, height);
	a.displayC();
	/////////////////////////////立//////////////////////////////////////
	B b(width, height);
	b.displayC();
	/////////////////////////////中//////////////////////////////////////
	C c(width, height);
	c.displayC();
	/////////////////////////////興//////////////////////////////////////
	D d(width, height);
	d.displayC();
	/////////////////////////////大//////////////////////////////////////
	E e(width, height);
	e.displayC();
	/////////////////////////////學//////////////////////////////////////
	F f(width, height);
	f.displayC();
	/////////////////////////////logo 白色線線//////////////////////////////////////
	float whiteline[3] = { 0.99f, 0.99f, 0.99f };
	glColor3fv(whiteline);
	//glColor3f(0.99f, 0.99f, 0.99f);
	G g(width, height);
	g.displayC();
	/////////////////////////////logo 紅色圈圈//////////////////////////////////////
	float redcircleone[3] = { 0.34f, 0.0f, 0.0f };
	glColor3fv(redcircleone);
	//glColor3f(0.34f, 0.0f, 0.0f);
	J j(width, height);
	j.displayC();

	float redcircletwo[3] = { 0.59f, 0.19f, 0.19f };
	glColor3fv(redcircletwo);
	//glColor3f(0.59f, 0.19f, 0.19f);
	I i(width, height);
	i.displayC();

	float redcirclethree[3] = { 0.99f, 0.19f, 0.19f };
	glColor3fv(redcirclethree);
	//glColor3f(0.99f, 0.19f, 0.19f);
	H h(width, height);
	h.displayC();

	float redcirclefour[3] = { 0.19f, 0.39f, 0.59f };
	glColor3fv(redcirclefour);
	//glColor3f(0.19f, 0.39f, 0.59f);
	K k(width, height);
	k.displayC();
	/////////////////////////////N//////////////////////////////////////
	float enColor[3] = { 0.0, 0.40f, 0.50f };
	glColor3fv(enColor);
	//glColor3f(0.0, 0.40f, 0.50f);
	L l(width, height);
	l.displayC();
	/////////////////////////////a//////////////////////////////////////
	M m(width, height);
	m.displayC();
	/////////////////////////////t//////////////////////////////////////
	N n(width, height);
	n.displayC();
	/////////////////////////////i//////////////////////////////////////
	O o(width, height);
	o.displayC();
	/////////////////////////////o//////////////////////////////////////
	P p(width, height);
	p.displayC();
	/////////////////////////////n//////////////////////////////////////
	Q q(width, height);
	q.displayC();
	/////////////////////////////C//////////////////////////////////////
	R r(width, height);
	r.displayC();
	/////////////////////////////h//////////////////////////////////////
	S s(width, height);
	s.displayC();
	/////////////////////////////u//////////////////////////////////////
	T t(width, height);
	t.displayC();
	/////////////////////////////g//////////////////////////////////////
	U u(width, height);
	u.displayC();
	/////////////////////////////H//////////////////////////////////////
	V v(width, height);
	v.displayC();
	/////////////////////////////s//////////////////////////////////////
	W w(width, height);
	w.displayC();
	/////////////////////////////U//////////////////////////////////////
	X x(width, height);
	x.displayC();
	/////////////////////////////v//////////////////////////////////////
	Y y(width, height);
	y.displayC();
	/////////////////////////////ery//////////////////////////////////////
	Z z(width, height);
	z.displayC();



	glPopMatrix();



	glutSwapBuffers();
}

void spinDisplay(void)
{
	spin = spin + 0.5;
	if (spin > 360.0)
		spin = spin - 360.0;
	glutPostRedisplay();
}

void init(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_FLAT);
}

void reshape(int w, int h)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, width, 0.0, height, 0, 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void mouse(int button, int state, int x, int y)
{
	switch (button) {
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN) {
			mouseClick = 1;
			glutIdleFunc(spinDisplay);
		}
		break;
	case GLUT_MIDDLE_BUTTON:
		if (state == GLUT_DOWN) {
			mouseClick = 2;
			glutIdleFunc(spinDisplay);
		}
		break;
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN) {
			mouseClick = 3;
			glutIdleFunc(spinDisplay);
		}
		break;
	default:
		break;
	}
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case '1':
		keyboardpress = 1;
		break;
	case '2':
		keyboardpress = 2;
		break;
	case '3':
		keyboardpress = 3;
		break;
	case '4':
		keyboardpress = 4;
		break;
	default:
		keyboardpress = 0;
		break;
	}
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);	// Initialize GLUT
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);	// Set up some memory buffers for our display
	glutInitWindowPosition(300, 300);
	glutInitWindowSize(width, height);	// Set the window size
	glutCreateWindow("HW3");	// Create the window with the title "HW2"
	init();
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glutDisplayFunc(My_Display);
	//glutReshapeFunc(changeViewPort);	// Bind the two functions (above) to respond when necessary
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);


	// Very important!  This initializes the entry points in the OpenGL driver so we can 
	// call all the functions in the API.
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "GLEW error");
		return 1;
	}

	glutMainLoop();
	return 0;
}







