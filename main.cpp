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
		if (Img[i] + Val > 255) {
			Out[i] = 255;
		}
		else if (Img[i] + Val < 0) {
			Out[i] = 0;
		}
		else Out[i] = Img[i] + Val;
	}
}

// Contrast 조절
void ContrastAdj(BYTE* Img, BYTE* Out, int W, int H, double Val)
{
	int ImgSize = W * H;
	for (int i = 0; i < ImgSize; i++)
	{
		if (Img[i] * Val > 255.0) {
			Out[i] = 255;
		}
		else Out[i] = (BYTE)(Img[i] * Val);
	}
}

void ObtainHistogram(BYTE* Img, int* Histo, int W, int H) {
	int ImgSize = W * H;
	for (int i = 0; i < ImgSize; i++) {
		Histo[Img[i]]++;
	}
}

void ObtainAHisto(int* Histo, int* AHisto) {
	for (int i = 0; i < 255; i++) {
		for (int j = 0; j <= i; j++) {
			AHisto[i] += Histo[j];
		}
	}
}
// 스트레칭
void HistogramStretching(BYTE* Img, BYTE* Out, int* Histo, int W, int H) {
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
void HistogramEqualization(BYTE* Img, BYTE* Out, int* AHisto, int W, int H) {
	int ImgSize = W * H;
	int Nt =W * H, Gmax = 255;
	double Ratio = Gmax / (double)Nt;
	BYTE NormSum[256];
	for (int i = 0; i < 256; i++) {
		NormSum[i] = (BYTE)(Ratio * AHisto[i]);
	}
	for (int i = 0; i < ImgSize; i++) {
		Out[i] = NormSum[Img[i]];
	}
}

// 이진화
void Binarization(BYTE* Img, BYTE* Out, int W, int H, BYTE Threshold) {
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
			Sum1 += i*Histo[i];
			Cnt1 += Histo[i];
		}
		Avg1 = (double)Sum1 / Cnt1;

		for (int i = T + 1; i < 256; i++) {
			Sum2 += i*Histo[i];
			Cnt2 += Histo[i];
		}
		Avg2 = (double)Sum2 / Cnt2;
		//새로운 경계값
		T = (Avg1 + Avg2) / 2;
	} while(abs(T-Init) > 3);

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
					SumProduct += Img[(i+m)*W + (j+n)] * Kennel[m+1][n+1]  ;
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

void main()
{
	BITMAPFILEHEADER hf; // 14Bytes
	BITMAPINFOHEADER hInfo; //40Bytes
	RGBQUAD hRGB[256]; // (256 * 4Bytes)
	FILE* fp;
	fp = fopen("lenna.bmp", "rb");
	if (fp == NULL) return;
	fread(&hf, sizeof(BITMAPFILEHEADER), 1, fp);
	fread(&hInfo, sizeof(BITMAPINFOHEADER), 1, fp);
	fread(hRGB, sizeof(RGBQUAD), 256, fp);
	int ImgSize = hInfo.biWidth * hInfo.biHeight;
	BYTE* Image = (BYTE*)malloc(ImgSize);
	BYTE* Output = (BYTE*)malloc(ImgSize);
	fread(Image, sizeof(BYTE), ImgSize, fp);
	fclose(fp);

	int Histo[256] = { 0 };
	int AHisto[255] = { 0 };

	// ObtainHistogram(Image, Histo, hInfo.biWidth, hInfo.biHeight);
	// ObtainAHisto(Histo, AHisto);
	// HistogramEqualization(Image, Output, AHisto, hInfo.biWidth, hInfo.biHeight);
	// int Thres = GonzalezBinThresh(Histo);
	// Binarization(Image, Output, hInfo.biWidth, hInfo.biHeight, Thres);

	// AverageConv(Image, Output, hInfo.biWidth, hInfo.biHeight);
	// GaussAvgConv(Image, Output, hInfo.biWidth, hInfo.biHeight);
	Prewitt_X_Conv(Image, Output, hInfo.biWidth, hInfo.biHeight);
	Binarization(Output, Output, hInfo.biWidth, hInfo.biHeight, 80);

	// HistogramStretching(Image, Output, Histo, hInfo.biWidth, hInfo.biHeight);
	// InverseImage(Image, Output, hInfo.biWidth, hInfo.biHeight);
	// BrightnessAdj(Image, Output, hInfo.biWidth, hInfo.biHeight, 70);
	// ContrastAdj(Image, Output, hInfo.biWidth, hInfo.biHeight, 1.5);

	fp = fopen("output.bmp", "wb");
	fwrite(&hf, sizeof(BYTE), sizeof(BITMAPFILEHEADER), fp);
	fwrite(&hInfo, sizeof(BYTE), sizeof(BITMAPINFOHEADER), fp);
	fwrite(hRGB, sizeof(RGBQUAD), 256, fp);
	fwrite(Output, sizeof(BYTE), ImgSize, fp);
	fclose(fp);
	free(Image);
	free(Output);
}