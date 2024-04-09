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

//박스 평활화
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


int main()
{
	BITMAPFILEHEADER hf; 
	BITMAPINFOHEADER hInfo; 
	RGBQUAD hRGB[256];
	FILE* fp;
	fp = fopen("lenna_gauss.bmp", "rb");
	if (fp == NULL) {
		printf("File not found!\n");
		return -1;
	}
	fread(&hf, sizeof(BITMAPFILEHEADER), 1, fp);
	fread(&hInfo, sizeof(BITMAPINFOHEADER), 1, fp);
	fread(hRGB, sizeof(RGBQUAD), 256, fp);
	int ImgSize = hInfo.biWidth * hInfo.biHeight;
	BYTE* Image = (BYTE*)malloc(ImgSize);
	BYTE* Temp = (BYTE*)malloc(ImgSize); // �ӽù迭
	BYTE* Output = (BYTE*)malloc(ImgSize);
	fread(Image, sizeof(BYTE), ImgSize, fp);
	fclose(fp);

	int Histo[256] = { 0 };
	int AHisto[256] = { 0 };

	/* ... */

	//GaussAvrConv(Image, Output, hInfo.biWidth, hInfo.biHeight);
	
	/* ... */

	/* Median filtering */
	//BYTE temp[9];
	//int W = hInfo.biWidth, H = hInfo.biHeight;
	//int i, j;
	//for (i = 1; i < H - 1; i++) {
	//	for (j = 1; j < W - 1; j++) {
	//		temp[0] = Image[(i - 1) * W + j-1];
	//		temp[1] = Image[(i - 1) * W + j];
	//		temp[2] = Image[(i - 1) * W + j+1];
	//		temp[3] = Image[i * W + j-1];
	//		temp[4] = Image[i * W + j];
	//		temp[5] = Image[i * W + j+1];
	//		temp[6] = Image[(i + 1) * W + j-1];
	//		temp[7] = Image[(i + 1) * W + j];
	//		temp[8] = Image[(i + 1) * W + j+1];
	//		// Output[i * W + j] = Median(temp, 9);
	//		// Output[i * W + j] = MaxPooling(temp, 9);
	//		Output[i * W + j] = MinPooling(temp, 9);
	//	}
	//}
	/* Median filtering */

	AverageConv(Image, Output, hInfo.biWidth, hInfo.biHeight);

	SaveBMPFile(hf, hInfo, hRGB, Output, hInfo.biWidth, hInfo.biHeight, "output_average.bmp");


	free(Image);
	free(Output);
	free(Temp);
	return 0;
}