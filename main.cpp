#pragma warning(disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

void InverseImage(BYTE* Img, BYTE* Out, int W, int H)
{
	int ImgSize = W * H;
	for (int i = 0; i < ImgSize; i++)
	{
		Out[i] = 255 - Img[i];
	}
}

// 밝기 조절
void BrightnessAdj(BYTE* Img, BYTE* Out, int W, int H, int Val)
{
	int ImgSize = W * H;
	for (int i = 0; i < ImgSize; i++)
	{
		if (Img[i] + Val > 255)
		{
			Out[i] = 255;
		}
		else if (Img[i] + Val < 0)
		{
			Out[i] = 0;
		}
		else 	Out[i] = Img[i] + Val;
	}
}

// Contrast 조절
void ContrastAdj(BYTE* Img, BYTE* Out, int W, int H, double Val)
{
	int ImgSize = W * H;
	for (int i = 0; i < ImgSize; i++)
	{
		if (Img[i] * Val > 255.0)
		{
			Out[i] = 255;
		}
		else 	Out[i] = (BYTE)(Img[i] * Val);
	}
}

void ObtainHistogram(BYTE* Img, int* Histo, int W, int H)
{
	int ImgSize = W * H;
	for (int i = 0; i < ImgSize; i++) {
		Histo[Img[i]]++;
	}
}

void ObtainAHistogram(int* Histo, int* AHisto)
{
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j <= i; j++) {
			AHisto[i] += Histo[j];
		}
	}
}

// 스트레칭
void HistogramStretching(BYTE* Img, BYTE* Out, int* Histo, int W, int H)
{
	int ImgSize = W * H;
	BYTE Low, High;
	for (int i = 0; i < 256; i++) {
		if (Histo[i] != 0) {
			Low = i;
			break;
		}
	}
	for (int i = 255; i >= 0; i--) {
		if (Histo[i] != 0) {
			High = i;
			break;
		}
	}
	for (int i = 0; i < ImgSize; i++) {
		Out[i] = (BYTE)((Img[i] - Low) / (double)(High - Low) * 255.0);
	}
}

// 평탄화
void HistogramEqualization(BYTE* Img, BYTE* Out, int* AHisto, int W, int H)
{
	int ImgSize = W * H;
	int Nt = W * H, Gmax = 255;
	double Ratio = Gmax / (double)Nt;
	BYTE NormSum[256];
	for (int i = 0; i < 256; i++) {
		NormSum[i] = (BYTE)(Ratio * AHisto[i]);
	}
	for (int i = 0; i < ImgSize; i++)
	{
		Out[i] = NormSum[Img[i]];
	}
}

// 이진화
void Binarization(BYTE* Img, BYTE* Out, int W, int H, BYTE Threshold)
{
	int ImgSize = W * H;
	for (int i = 0; i < ImgSize; i++) {
		if (Img[i] < Threshold) Out[i] = 0;
		else Out[i] = 255;
	}
}

// Gonzalez 자동 이진화 알고리즘
int GonzalezBinThresh(int* Histo) {
	int Min = 255, Max = 0;
	for (int i = 0; i < 256; i++) {
		if (Histo[i] != 0) {
			Min = (i < Min) ? i : Min;
			Max = (i > Max) ? i : Max;
		}
	}
	int T = (Min + Max) / 2;
	int Init;
	// 영상의 최소값, 최대값 구하기
	do {
		Init = T;
		int Sum1 = 0, Sum2 = 0, Cnt1 = 0, Cnt2 = 0;
		double Avg1, Avg2;

		for (int i = 0; i <= T; i++) {
			Sum1 += i * Histo[i];
			Cnt1 += Histo[i];
		}
		Avg1 = (double)Sum1 / Cnt1;

		for (int i = T + 1; i < 256; i++) {
			Sum2 += i * Histo[i];
			Cnt2 += Histo[i];
		}
		Avg2 = (double)Sum2 / Cnt2;
		//새로운 경계값
		T = (Avg1 + Avg2) / 2;
	} while (abs(T - Init) > 3);

	return T;
}

// 박스 평활화
void AverageConv(BYTE* Img, BYTE* Out, int W, int H) {
	double Kennel[3][3] = { 0.11111,0.11111 ,0.11111,
							0.11111,0.11111 ,0.11111,
							0.11111,0.11111 ,0.11111 };
	double SumProduct = 0.0;
	// margin을 두기 위해 1부터 시작해서 W(H) -1 전에 종료
	for (int i = 1; i < H - 1; i++) {
		for (int j = 1; j < W - 1; j++) { // 여기까지 center 화소를 나타냄
			for (int m = -1; m <= 1; m++) {
				for (int n = -1; n <= 1; n++) { // center 화소의 주변부를 계산하기 위한 for문 2개
					SumProduct += Img[(i + m) * W + (j + n)] * Kennel[m + 1][n + 1];
				}
			}
			Out[i * W + j] = (BYTE)SumProduct;
			SumProduct = 0.0;
		}
	}
}

// 가우시안 평활화
void GaussAvgConv(BYTE* Img, BYTE* Out, int W, int H) {
	double Kennel[3][3] = { 0.0625,0.125 ,0.0625,
							0.125,0.25 ,0.125,
							0.0625,0.125 ,0.0625 };
	double SumProduct = 0.0;
	// margin을 두기 위해 1부터 시작해서 W(H) -1 전에 종료
	for (int i = 1; i < H - 1; i++) {
		for (int j = 1; j < W - 1; j++) { // 여기까지 center 화소를 나타냄
			for (int m = -1; m <= 1; m++) {
				for (int n = -1; n <= 1; n++) { // center 화소의 주변부를 계산하기 위한 for문 2개
					SumProduct += Img[(i + m) * W + (j + n)] * Kennel[m + 1][n + 1];
				}
			}
			Out[i * W + j] = (BYTE)SumProduct;
			SumProduct = 0.0;
		}
	}
}

// Prewitt 마스크 X
void Prewitt_X_Conv(BYTE* Img, BYTE* Out, int W, int H) {
	double Kennel[3][3] = { -1.0,0 ,1.0,
							-1.0,0 ,1.0,
							-1.0,0 ,1.0 };
	double SumProduct = 0.0;
	// margin을 두기 위해 1부터 시작해서 W(H) -1 전에 종료
	for (int i = 1; i < H - 1; i++) {
		for (int j = 1; j < W - 1; j++) { // 여기까지 center 화소를 나타냄
			for (int m = -1; m <= 1; m++) {
				for (int n = -1; n <= 1; n++) { // center 화소의 주변부를 계산하기 위한 for문 2개
					SumProduct += Img[(i + m) * W + (j + n)] * Kennel[m + 1][n + 1];
				}
			}
			// 0~765 ===>> 0~255
			Out[i * W + j] = abs((long)SumProduct) / 3;
			SumProduct = 0.0;
		}
	}
}

// Prewitt 마스크 Y
void Prewitt_Y_Conv(BYTE* Img, BYTE* Out, int W, int H) {
	double Kennel[3][3] = { -1.0,-1.0,-1.0,
							0.0,0.0,0.0,
							1.0,1.0,1.0 };
	double SumProduct = 0.0;
	// margin을 두기 위해 1부터 시작해서 W(H) -1 전에 종료
	for (int i = 1; i < H - 1; i++) {
		for (int j = 1; j < W - 1; j++) { // 여기까지 center 화소를 나타냄
			for (int m = -1; m <= 1; m++) {
				for (int n = -1; n <= 1; n++) { // center 화소의 주변부를 계산하기 위한 for문 2개
					SumProduct += Img[(i + m) * W + (j + n)] * Kennel[m + 1][n + 1];
				}
			}
			// 0~765 ===>> 0~255
			Out[i * W + j] = abs((long)SumProduct) / 3;
			SumProduct = 0.0;
		}
	}
}

// Sobel 마스크 X
void Sobel_X_Conv(BYTE* Img, BYTE* Out, int W, int H) {
	double Kennel[3][3] = { -1.0,0 ,1.0,
							-2.0,0 ,2.0,
							-1.0,0 ,1.0 };
	double SumProduct = 0.0;
	// margin을 두기 위해 1부터 시작해서 W(H) -1 전에 종료
	for (int i = 1; i < H - 1; i++) {
		for (int j = 1; j < W - 1; j++) { // 여기까지 center 화소를 나타냄
			for (int m = -1; m <= 1; m++) {
				for (int n = -1; n <= 1; n++) { // center 화소의 주변부를 계산하기 위한 for문 2개
					SumProduct += Img[(i + m) * W + (j + n)] * Kennel[m + 1][n + 1];
				}
			}
			// 0~1020 ===>> 0~255
			Out[i * W + j] = abs((long)SumProduct) / 4;
			SumProduct = 0.0;
		}
	}
}

// Sobel 마스크 Y
void Sobel_Y_Conv(BYTE* Img, BYTE* Out, int W, int H) {
	double Kennel[3][3] = { -1.0,-2.0,-1.0,
							0.0,0.0,0.0,
							1.0,2.0,1.0 };
	double SumProduct = 0.0;
	// margin을 두기 위해 1부터 시작해서 W(H) -1 전에 종료
	for (int i = 1; i < H - 1; i++) {
		for (int j = 1; j < W - 1; j++) { // 여기까지 center 화소를 나타냄
			for (int m = -1; m <= 1; m++) {
				for (int n = -1; n <= 1; n++) { // center 화소의 주변부를 계산하기 위한 for문 2개
					SumProduct += Img[(i + m) * W + (j + n)] * Kennel[m + 1][n + 1];
				}
			}
			// 0~1020 ===>> 0~255
			Out[i * W + j] = abs((long)SumProduct) / 4;
			SumProduct = 0.0;
		}
	}
}

// 라플라시안
void Laplace_Conv(BYTE* Img, BYTE* Out, int W, int H) {
	double Kennel[3][3] = { -1.0,-1.0,-1.0,
							-1.0,8.0,-1.0,
							-1.0,-1.0,-1.0 };
	double SumProduct = 0.0;
	// margin을 두기 위해 1부터 시작해서 W(H) -1 전에 종료
	for (int i = 1; i < H - 1; i++) {
		for (int j = 1; j < W - 1; j++) { // 여기까지 center 화소를 나타냄
			for (int m = -1; m <= 1; m++) {
				for (int n = -1; n <= 1; n++) { // center 화소의 주변부를 계산하기 위한 for문 2개
					SumProduct += Img[(i + m) * W + (j + n)] * Kennel[m + 1][n + 1];
				}
			}
			// 0~2040 ===>> 0~255
			Out[i * W + j] = abs((long)SumProduct) / 8;
			SumProduct = 0.0;
		}
	}
}

// 라플라시안이 노이즈에 얼마나 민감한지 확인
void Laplace_Conv_DC(BYTE* Img, BYTE* Out, int W, int H) {
	double Kennel[3][3] = { -1.0,-1.0,-1.0,
							-1.0,9.0,-1.0,
							-1.0,-1.0,-1.0 };
	double SumProduct = 0.0;
	// margin을 두기 위해 1부터 시작해서 W(H) -1 전에 종료
	for (int i = 1; i < H - 1; i++) {
		for (int j = 1; j < W - 1; j++) { // 여기까지 center 화소를 나타냄
			for (int m = -1; m <= 1; m++) {
				for (int n = -1; n <= 1; n++) { // center 화소의 주변부를 계산하기 위한 for문 2개
					SumProduct += Img[(i + m) * W + (j + n)] * Kennel[m + 1][n + 1];
				}
			}

			if (SumProduct > 255.0) Out[i * W + j] = 255;
			else if (SumProduct < 0.0) Out[i * W + j] = 0;
			else Out[i * W + j] = (BYTE)SumProduct;
			SumProduct = 0.0;
		}
	}
}

void SaveBMPFile(BITMAPFILEHEADER hf, BITMAPINFOHEADER hInfo,
	RGBQUAD* hRGB, BYTE* Output, int W, int H, const char* FileName)
{
	FILE* fp = fopen(FileName, "wb");
	fwrite(&hf, sizeof(BYTE), sizeof(BITMAPFILEHEADER), fp);
	fwrite(&hInfo, sizeof(BYTE), sizeof(BITMAPINFOHEADER), fp);
	fwrite(hRGB, sizeof(RGBQUAD), 256, fp);
	fwrite(Output, sizeof(BYTE), W * H, fp);
	fclose(fp);
}

void swap(BYTE* a, BYTE* b)
{
	BYTE temp = *a;
	*a = *b;
	*b = temp;
}

// 중앙값 구하는 함수
BYTE Median(BYTE* arr, int size)
{
	// 오름차순 정렬
	const int S = size;
	for (int i = 0; i < size - 1; i++) // pivot index
	{
		for (int j = i + 1; j < size; j++) // 비교대상 index
		{
			if (arr[i] > arr[j]) 	swap(&arr[i], &arr[j]);
		}
	}
	return arr[S / 2];
}

BYTE MaxPooling(BYTE* arr, int size)
{
	// 오름차순 정렬
	const int S = size;
	for (int i = 0; i < size - 1; i++) // pivot index
	{
		for (int j = i + 1; j < size; j++) // 비교대상 index
		{
			if (arr[i] > arr[j]) 	swap(&arr[i], &arr[j]);
		}
	}
	return arr[S - 1];
}

BYTE MinPooling(BYTE* arr, int size)
{
	// 오름차순 정렬
	const int S = size;
	for (int i = 0; i < size - 1; i++) // pivot index
	{
		for (int j = i + 1; j < size; j++) // 비교대상 index
		{
			if (arr[i] > arr[j]) 	swap(&arr[i], &arr[j]);
		}
	}
	return arr[0];
}

int push(short* stackx, short* stacky, int arr_size, short vx, short vy, int* top)
{
	if (*top >= arr_size) return(-1);
	(*top)++;
	stackx[*top] = vx;
	stacky[*top] = vy;
	return(1);
}

int pop(short* stackx, short* stacky, short* vx, short* vy, int* top)
{
	if (*top == 0) return(-1);
	*vx = stackx[*top];
	*vy = stacky[*top];
	(*top)--;
	return(1);
}


// GlassFire 알고리즘을 이용한 라벨링 함수
void m_BlobColoring(BYTE* CutImage, int height, int width)
{
	int i, j, m, n, top, area, Out_Area, index, BlobArea[1000];
	long k;
	short curColor = 0, r, c;
	//	BYTE** CutImage2;
	Out_Area = 1;

	// 스택으로 사용할 메모리 할당
	short* stackx = new short[height * width];
	short* stacky = new short[height * width];
	short* coloring = new short[height * width];

	int arr_size = height * width;


	for (k = 0; k < height * width; k++) coloring[k] = 0;  

	for (i = 0; i < height; i++)
	{
		index = i * width;
		for (j = 0; j < width; j++)
		{
			if (coloring[index + j] != 0 || CutImage[index + j] != 255) continue;
			r = i; c = j; top = 0; area = 1;
			curColor++;

			while (1)
			{
			GRASSFIRE:
				for (m = r - 1; m <= r + 1; m++)
				{
					index = m * width;
					for (n = c - 1; n <= c + 1; n++)
					{
						if (m < 0 || m >= height || n < 0 || n >= width) continue;

						if ((int)CutImage[index + n] == 255 && coloring[index + n] == 0)
						{
							coloring[index + n] = curColor; 
							if (push(stackx, stacky, arr_size, (short)m, (short)n, &top) == -1) continue;
							r = m; c = n; area++;
							goto GRASSFIRE;
						}
					}
				}
				if (pop(stackx, stacky, &r, &c, &top) == -1) break;
			}
			if (curColor < 1000) BlobArea[curColor] = area;
		}
	}

	float grayGap = 255.0f / (float)curColor;

	// 가장 면적이 넓은 영역을 찾아내기 위함
	for (i = 1; i <= curColor; i++)
	{
		if (BlobArea[i] >= BlobArea[Out_Area]) Out_Area = i;
	}
	// CutImage 배열 클리어
	for (k = 0; k < width * height; k++) CutImage[k] = 255;

	// coloring에 저장된 라벨링 결과중 (Out_Area에 저장된) 영역이 가장 큰 것
	for (k = 0; k < width * height; k++)
	{
		if (coloring[k] == Out_Area) CutImage[k] = 0;  // 가장 큰 것만 저장
		//if (BlobArea[coloring[k]] > 500) CutImage[k] = 0; 
		//CutImage[k] = (unsigned char)(coloring[k] * grayGap);
	}

	delete[] coloring;
	delete[] stackx;
	delete[] stacky;
}

int main()
{
	BITMAPFILEHEADER hf; 
	BITMAPINFOHEADER hInfo; 
	RGBQUAD hRGB[256];
	FILE* fp;
	fp = fopen("coin.bmp", "rb");
	if (fp == NULL) {
		printf("File not found!\n");
		return -1;
	}
	fread(&hf, sizeof(BITMAPFILEHEADER), 1, fp);
	fread(&hInfo, sizeof(BITMAPINFOHEADER), 1, fp);
	fread(hRGB, sizeof(RGBQUAD), 256, fp);
	int ImgSize = hInfo.biWidth * hInfo.biHeight;
	BYTE* Image = (BYTE*)malloc(ImgSize);
	BYTE* Temp = (BYTE*)malloc(ImgSize); 
	BYTE* Output = (BYTE*)malloc(ImgSize);
	fread(Image, sizeof(BYTE), ImgSize, fp);
	fclose(fp);

	int Histo[256] = { 0 };
	int AHisto[256] = { 0 };

	/* ... */

	//GaussAvrConv(Image, Output, hInfo.biWidth, hInfo.biHeight);
	
	/* ... */

	/* Median filtering */
	//int Length = 5;  // 마스크의 한 변의 길이
	//int Margin = Length / 2; // 마진 값 두기 (마스크의 길이가 커지면 당연히 커진다)
	//int WSize = Length * Length;  // 마스크의 크기
	//BYTE* temp = (BYTE*)malloc(sizeof(BYTE) * WSize); // 임시 변수에 마스크 크기만큼 동적할당
	//int W = hInfo.biWidth, H = hInfo.biHeight; // W,H 선언 및 값 할당
	//int i, j, m, n; // 반복문에 사용될 변수 선언
	//for (i = Margin; i < H - Margin; i++) {
	//	for (j = Margin; j < W - Margin; j++) { // i,j로 지정된 center값을 반복문으로 돌림
	//		for (m = -Margin; m <= Margin; m++) { 
	//			for (n = -Margin; n <= Margin; n++) { // m,n으로 마스크 크기의 가로, 세로만큼 반복문 돌림 
	//				// temp 배열에 이미지 값(center를 중심으로 한 값들)을 넣음
	//				temp[(m + Margin) * Length + (n + Margin)] = Image[(i + m) * W + j + n];
	//			}
	//		}
	//		Output[i * W + j] = Median(temp, WSize); // center에 median값 넣기
	//	}
	//}
	//free(temp); // 할당한 temp free처리
	/* Median filtering */

	Binarization(Image, Output, hInfo.biWidth, hInfo.biHeight, 100);

	SaveBMPFile(hf, hInfo, hRGB, Output, hInfo.biWidth, hInfo.biHeight, "output_bin.bmp");


	free(Image);
	free(Output);
	free(Temp);
	return 0;
}