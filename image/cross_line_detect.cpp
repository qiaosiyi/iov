//g++ -o main main.cpp -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc -lopencv_videoio
#include<opencv/cv.h>
#include<opencv/cxcore.h>
#include<opencv/highgui.h>
#include"mylinedetect.h"

#include<cstdio>
#include<iostream>
using namespace std;

int main(int argc, char **argv) {
	bool flag = 0;
	IplImage* pFrame = NULL;
	IplImage* pCutFrame = NULL;
	IplImage* pCutFrImg = NULL;
	
	CvCapture* pCapture = NULL;
	
	CvMemStorage* storage = cvCreateMemStorage();
	CvSeq* lines = NULL;
		
    	pCapture = cvCreateCameraCapture(0);
	
	int nFrmNum = 0;
	
	int CutHeight = 120;
	
//	cvNamedWindow("video", 1);
//	cvNamedWindow("BWmode", 1);
	
//	cvMoveWindow("video", 300, 0);
//	cvMoveWindow("BWmode", 300, 520);

	/*if (!(pCapture = cvCaptureFromFile("eg.avi"))) {
		fprintf(stderr, "Can not open video file\n");
		return -2;
	}*/

	while (pFrame = cvQueryFrame(pCapture)) {
		cvSetImageROI(pFrame, cvRect(0, CutHeight, pFrame->width, pFrame->height - CutHeight));
		nFrmNum++;
		if (nFrmNum == 1) {
			pCutFrame = cvCreateImage(cvSize(pFrame->width, pFrame->height - CutHeight), pFrame->depth, pFrame->nChannels);
			cvCopy(pFrame, pCutFrame, 0);
			pCutFrImg = cvCreateImage(cvSize(pCutFrame->width, pCutFrame->height), IPL_DEPTH_8U, 1);
			cvCvtColor(pCutFrame, pCutFrImg, CV_BGR2GRAY);
		}
		else {
			cvCopy(pFrame, pCutFrame, 0);
		
			cvCvtColor(pCutFrame, pCutFrImg, CV_BGR2GRAY);
			cvThreshold(pCutFrImg, pCutFrImg, 80, 255.0, CV_THRESH_BINARY);
			cvErode(pCutFrImg, pCutFrImg, 0, 2);
			cvDilate(pCutFrImg, pCutFrImg, 0, 2);
			cvCanny(pCutFrImg, pCutFrImg, 50, 120);


			lines = cvHoughLines2(pCutFrImg, storage, CV_HOUGH_PROBABILISTIC, 1, CV_PI / 180, 100, 15, 15);
//			printf("Lines number: %d\n", lines->total);
			for (int i = 0; i<lines->total; i++) {
				CvPoint* line = (CvPoint*)cvGetSeqElem(lines, i);
				double k = ((line[0].y - line[1].y)*1.0 / (line[0].x - line[1].x));
				cout << "nFrmNum " << nFrmNum << " 's k = " << k << endl;
				if (!(abs(k) < 0.1))
				{
					cvLine(pFrame, line[0], line[1], CV_RGB(255, 0, 0), 6, CV_AA);
					float mid = abs((line[0].x + line[1].x) / 2);
					if (mid >= 200 && mid <= 250)
					{
						flag = 1;
						cout << "cross line" << endl;
						break;
					}
					else
						continue;
				}

			}

			cvResetImageROI(pFrame);
//			cvShowImage("video", pFrame);
//			cvShowImage("BWmode", pCutFrImg);
//			int temp = cvWaitKey(2);
//			if (temp == 32) {
//				while (cvWaitKey() == -1);
//			}
//			else if (temp >= 0) {
//				break;
//			}
		}
	}
//	cvDestroyWindow("video");
//	cvDestroyWindow("BWmode");
	cvReleaseImage(&pCutFrImg);
	cvReleaseImage(&pCutFrame);
	cvReleaseCapture(&pCapture);

	return 0;
}
