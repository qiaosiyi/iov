#include<iostream>
#include "fatigueDetect.h"
/*
#include<opencv2/opencv.hpp>
#include "LBF.h"
#include "LBFRegressor.h"
#include"variables.h"

using namespace std;
using namespace cv;
*/

void ReadGlobalParamFromFile(string path) {
	cout << "Loading GlobalParam..." << endl;
	ifstream fin;
	fin.open(path);
	fin >> global_params.bagging_overlap;
	fin >> global_params.max_numtrees;
	fin >> global_params.max_depth;
	fin >> global_params.max_numthreshs;
	fin >> global_params.landmark_num;
	fin >> global_params.initial_num;
	fin >> global_params.max_numstage;

	for (int i = 0; i< global_params.max_numstage; i++) {
		fin >> global_params.max_radio_radius[i];
	}

	for (int i = 0; i < global_params.max_numstage; i++) {
		fin >> global_params.max_numfeats[i];
	}
	cout << "Loading GlobalParam end" << endl;
	fin.close();
}

cv::Mat boundingBoxImg(cv::Mat & cImg, vector<cv::Point> &featurePoints, double scale = 1.0)
{
	cv::RotatedRect rotatedRect = cv::minAreaRect(featurePoints);
	rotatedRect.size.height *= scale;
	rotatedRect.size.width *= scale;
	Point2f vertices[4];
	rotatedRect.points(vertices);
	
	vector<cv::Point2f> rotatedVertices;
	for (int i = 0; i < 4; i++)
	{
		line(cImg, vertices[i], vertices[(i + 1) % 4], Scalar(255));
		rotatedVertices.push_back(vertices[i]);
	}
	
	std::sort(rotatedVertices.begin(), rotatedVertices.end(),
		[](const cv::Point2f p0, const cv::Point2f p1)
		-> bool
	{
		return p0.x < p1.x;
	});

	cv::Rect rect;
	rect.width = rotatedRect.size.width;
	rect.height = rotatedRect.size.height;
	float rotatedAngle = rotatedRect.angle;
	if (abs(rotatedAngle)>45)
	{
		rotatedAngle += 90;
		rect.width = rotatedRect.size.height;
		rect.height = rotatedRect.size.width;
	}

	cv::Mat rotatedMat = cv::getRotationMatrix2D(rotatedRect.center, rotatedAngle, 1);
	cv::Mat cRotatedImg;
	cv::warpAffine(cImg, cRotatedImg, rotatedMat, cImg.size());
	cv::Mat rotatedMatf;
	rotatedMat.convertTo(rotatedMatf, CV_64FC1);
	cv::Point leftTopPoint = rotatedVertices[0].y < rotatedVertices[1].y ? rotatedVertices[0] : rotatedVertices[1];
	rect.x = rotatedMatf.at<double>(0, 0)*leftTopPoint.x + rotatedMatf.at<double>(0, 1)*leftTopPoint.y + rotatedMatf.at<double>(0, 2);
	rect.y = rotatedMatf.at<double>(1, 0)*leftTopPoint.x + rotatedMatf.at<double>(1, 1)*leftTopPoint.y + rotatedMatf.at<double>(1, 2);
	return cRotatedImg(rect);
}


int fatiguepredict(cv::Mat &frame, cv::CascadeClassifier &fcc, LBFRegressor &regressor, cv::HOGDescriptor &eyeHog, const int &eyeHogSize,
		cv::Ptr<cv::ml::SVM> &rightEyeStatusSVM, cv::Ptr<cv::ml::SVM> &mouthChinStatusSVM, cv::HOGDescriptor &mouthChinHog, const int &mouthChinHogSize){
		allPoints.clear();
		rightEyePoints.clear();
		mouthChinPoints.clear();
		
		frame.copyTo(cImg);
		cv::cvtColor(cImg, gImg, CV_BGR2GRAY);
		cv::resize(gImg, gSmallImgForFaceDetection, cv::Size(scaleForFaceDetection*gImg.size().width, scaleForFaceDetection*gImg.size().height));

		cv::equalizeHist(gSmallImgForFaceDetection, gSmallImgForFaceDetection);
		int res = 0;
		cv::Mat gDetectRegion;
		if (lastFaceRect.area()!=0)
		{
			gDetectRegion = gSmallImgForFaceDetection(lastFaceRect);
		}
		else
		{
			gDetectRegion = gSmallImgForFaceDetection;
		}
	
		clock_t begin = clock();
		fcc.detectMultiScale(gDetectRegion, faces,
			1.1, 2, 0
			//|CV_HAAR_FIND_BIGGEST_OBJECT
			//|CV_HAAR_DO_ROUGH_SEARCH
			| CV_HAAR_SCALE_IMAGE
			,
			Size(gSmallImgForFaceDetection.size().width/5, gSmallImgForFaceDetection.size().height/3));
		clock_t end = clock();
		faceDetectionTime = double(end - begin) / CLOCKS_PER_SEC;
	
		if (!faces.empty())
		{
			sort(faces.begin(), faces.end(),
				[](const cv::Rect &face0, const cv::Rect &face1)
				-> bool
			{
				return face0.size().area() > face1.size().area();
			});
			cv::Rect face = faces[0];
			face.x += lastFaceRect.x;
			face.y += lastFaceRect.y;
			lastFaceRect.width = face.width*1.3;
			lastFaceRect.height = face.height*1.3;
			lastFaceRect.x = face.x-face.width*0.15;
			lastFaceRect.y = face.y-face.height*0.15;
			if (lastFaceRect.x<0)
			{
				lastFaceRect.x = 0;
			}
			if (lastFaceRect.y<0)
			{
				lastFaceRect.y = 0;
			}
			if (lastFaceRect.x+lastFaceRect.width>gSmallImgForFaceDetection.size().width)
			{
				lastFaceRect.width = gSmallImgForFaceDetection.size().width - lastFaceRect.x-1;
			}
			if (lastFaceRect.y+lastFaceRect.height>gSmallImgForFaceDetection.size().height)
			{
				lastFaceRect.height = gSmallImgForFaceDetection.size().height - lastFaceRect.y-1;
			}

			BoundingBox boundingbox;
			boundingbox.start_x = face.x;//*(scaleForFaceAlignment/scaleForFaceDetection);
			boundingbox.start_y = face.y;//*(scaleForFaceAlignment/scaleForFaceDetection);
			boundingbox.width = (face.width - 1);//*(scaleForFaceAlignment/scaleForFaceDetection);
			boundingbox.height = (face.height - 1);//*(scaleForFaceAlignment/scaleForFaceDetection);
			boundingbox.centroid_x = boundingbox.start_x + boundingbox.width / 2.0;
			boundingbox.centroid_y = boundingbox.start_y + boundingbox.height / 2.0;
	
			begin = clock();
			Mat_<double> current_shape = regressor.Predict(gSmallImgForFaceDetection, boundingbox, 1);
			end = clock();
			faceAlignmentTime = double(end - begin) / CLOCKS_PER_SEC;
			for (int i = 0;i < global_params.landmark_num;i++) {
				allPoints.push_back(cv::Point(current_shape(i, 0)/scaleForFaceDetection, current_shape(i, 1)/scaleForFaceDetection));
				
			}
		
			rightEyePoints.push_back(allPoints[4]);
			rightEyePoints.push_back(allPoints[5]);
			rightEyePoints.push_back(allPoints[6]);
			rightEyePoints.push_back(allPoints[7]);
			rightEyePoints.push_back(allPoints[8]);
			rightEyePoints.push_back(allPoints[9]);
		
			mouthChinPoints.push_back(allPoints[1]);
			mouthChinPoints.push_back(allPoints[2]);
			mouthChinPoints.push_back(allPoints[16]);
			mouthChinPoints.push_back(allPoints[17]);
			mouthChinPoints.push_back(allPoints[18]);
			mouthChinPoints.push_back(allPoints[19]);
		
			cv::Mat cRightEyeImg = boundingBoxImg(cImg, rightEyePoints, eyeRectScale);
			
			cv::Mat gRightEyeImg;
			cv::cvtColor(cRightEyeImg, gRightEyeImg, CV_BGR2GRAY);
		
			cv::resize(gRightEyeImg, gRightEyeImg, cv::Size(48, 16));
			cv::equalizeHist(gRightEyeImg,gRightEyeImg);
		
			std::vector<float> eyeHogDescriptor;
			eyeHog.compute(gRightEyeImg, eyeHogDescriptor);
			cv::Mat eyeHogFeature(1, eyeHogSize, CV_32FC1);
			for (size_t i=0;i<eyeHogDescriptor.size();i++)
			{
				eyeHogFeature.at<float>(0, i) = eyeHogDescriptor[i];
			}
			
			float rightEyeLabel = rightEyeStatusSVM->predict(eyeHogFeature);
			eyeStatus = "EYE STATUS:";
			if (0== rightEyeLabel)
			{
				eyeStatus += "OPEN";
				eyeStatusColor = cv::Scalar(0,255,0);
				openEyeFrameCount++;
			}
			else
			{
				eyeStatus += "CLOSE";
				eyeStatusColor = cv::Scalar(0,0,255);
				closeEyeFrameCount++;
			}
			
			
			cv::Mat cMouthChinImg = boundingBoxImg(cImg,mouthChinPoints);
			cv::Mat gMouthChinImg;
			cv::cvtColor(cMouthChinImg, gMouthChinImg, CV_BGR2GRAY);
			cv::resize(gMouthChinImg, gMouthChinImg, cv::Size(normalizedMouthWidth, normalizedMouthHeight));
			cv::equalizeHist(gMouthChinImg, gMouthChinImg);
		
			std::vector<float> mouthChinHogDescriptor;
			mouthChinHog.compute(gMouthChinImg, mouthChinHogDescriptor);
			cv::Mat mouthChinHogFeature(1, mouthChinHogSize, CV_32FC1);
		
			for (size_t i = 0;i<mouthChinHogDescriptor.size();i++)
			{
				mouthChinHogFeature.at<float>(0, i ) = mouthChinHogDescriptor[i];
			}
			
			float mouthChinLabel = mouthChinStatusSVM->predict(mouthChinHogFeature);
			mouthChinStatus = "MOUTH STATUS:";
			if (0 == mouthChinLabel)
			{
				mouthChinStatus += "OPEN";
				mouthChinStatusColor = cv::Scalar(0, 0, 255);
				openMouthFrameCount++;
			}
			else
			{
				mouthChinStatus += "CLOSE";
				mouthChinStatusColor = cv::Scalar(0, 255, 0);
				closeMouthFrameCount++;
			}
			
			if (lastMouthStatus!=static_cast<int>(mouthChinLabel))
			{
				if (0 == mouthChinLabel)
				{
					startOpenMouthTime = time(0);
					currentOpenMouthFrameCount++;
					interuptCloseMouthFrameCount = 0;
				}
				else
				{
					interuptCloseMouthFrameCount++;
				}
				lastMouthStatus = static_cast<int>(mouthChinLabel);
			}
			else
			{
				if (0==mouthChinLabel)
				{
					currentOpenMouthFrameCount++;
				}
				else
				{
					if (interuptCloseMouthFrameCount>0)
					{
						interuptCloseMouthFrameCount++;
						if (interuptCloseMouthFrameCount >= 3)
						{
							time_t endOpenMouthTime = time(0);
							if (endOpenMouthTime - startOpenMouthTime > yawnContinueTimeThreshold)
							{
								yawnFrameCount += currentOpenMouthFrameCount;
								yawnCount++;
								cout << "yawn" << endl;
							}
							currentOpenMouthFrameCount = 0;
							interuptCloseMouthFrameCount = 0;
						}
					}
				}
			}
			
			 
			time_t now_time = time(0);
			struct tm *lt = localtime(&now_time);
		//	char time_buf[32];

		//	sprintf(time_buf, "%d/%d/%d:%d:%02d:%d",lt->tm_year+1900,lt->tm_mon,lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);
		/************************************************************************/	
			// predict fatigue******************	
			if (lastSecond!=now_time)
			{
				cout << "FPS: "<<frameCount - frameCountInLastSecond << endl;
				sprintf(fps_buf,"FPS:%d",frameCount-frameCountInLastSecond);
				frameCountInLastSecond = frameCount;
				if (0 == now_time % 5)
				{
					perclose = static_cast<float>(closeEyeFrameCount) / static_cast<float>(openEyeFrameCount + closeEyeFrameCount);
					openEyeFrameCount = 0;
					closeEyeFrameCount = 0;
					
					cout << "PERCLOSE: " << perclose << endl;
					perclose_color = cv::Scalar(0,255,0);
					if (perclose > 0.3){
						return 1;
}
					if (perclose > percloseThreshold)
					{
						perclose_color = cv::Scalar(0,0,255);
						res += 1;
					}
				}
				
				if (0 == now_time%30)
				{
					yawnFrameRate = static_cast<float>(yawnFrameCount) / static_cast<float>(openMouthFrameCount+closeMouthFrameCount);
					openMouthFrameCount = 0;
					closeMouthFrameCount = 0;
					yawnFrameCount = 0;
					yawnCount = 0;
					
					cout << "YAWN FRAME RATE: " << yawnFrameRate << endl;
					yawnFrameRateColor = cv::Scalar(0,255,0);
					if (yawnFrameRate>yawnFrameRateThreshold)
					{
						yawnFrameRateColor = cv::Scalar(0,0,255);
						res += 2;
					}
				}
				lastSecond = now_time;
			}
			/*
			sprintf(perclose_buf, "PERCLOSE:%.2f", perclose);
			sprintf(yawnFrameRate_buf, "YAWN FRAME RATE:%.2f", yawnFrameRate);
			sprintf(yawnCount_buf, "YAWN COUNT:%d", yawnCount);
			sprintf(faceDetectionTime_buf, "FDT:%.3lf", faceDetectionTime);
			sprintf(faceAlignmentTime_buf, "FAT:%.3lf", faceAlignmentTime);
			*/
			frameCount++;
		}
		else
		{
			lastFaceRect = cv::Rect(0,0,0,0);
		}
	return 0;
}

int main()
{
	cv::CascadeClassifier fcc;
	fcc.load((modelPath+cascadeName).c_str());
	
	ReadGlobalParamFromFile(modelPath + "LBF.model");

	LBFRegressor regressor;
	regressor.Load(modelPath + "LBF.model");
	
	cv::Ptr<cv::ml::SVM> rightEyeStatusSVM=cv::ml::SVM::load<cv::ml::SVM>((modelPath+rightEyeStatusSVMModelFile).c_str());

	cv::HOGDescriptor eyeHog(cv::Size(48, 16), cv::Size(16, 16), cv::Size(8, 8), cv::Size(8, 8), 9);
	const int eyeHogSize = eyeHog.getDescriptorSize();

	cv::Ptr<cv::ml::SVM> mouthChinStatusSVM = cv::ml::SVM::load<cv::ml::SVM>((modelPath+mouthChinStatusSVMModelFile).c_str());
//void fatiguepredict(cv::Mat &frame, cv::CascadeClassifier &fcc, LBFRegressor &regressor, cv::HOGDescriptor &eyeHog, const int &eyeHogSize,
//                 cv::Ptr<cv::ml::SVM> &rightEyeStatusSVM, cv::Ptr<cv::ml::SVM> &mouthChinStatusSVM, cv::HOGDescriptor &mouthChinHog, const int &mouthChinHogSize)
	cv::HOGDescriptor mouthChinHog(cv::Size(48, 48), cv::Size(16, 16), cv::Size(8, 8), cv::Size(8, 8), 9);
	const int mouthChinHogSize = mouthChinHog.getDescriptorSize();
	
	cv::Mat frame;

	cv::VideoCapture cap;
	cap.open(1);

	if (!cap.isOpened())
	{
		cap.open(0);
		if (!cap.isOpened())
		{
			cout << "fail to open camera" << endl;
			return -1;
		}
	}

	while (true)
	{
		cap >> frame;
		fatiguepredict(frame, fcc, regressor, eyeHog, eyeHogSize, rightEyeStatusSVM, mouthChinStatusSVM, mouthChinHog, mouthChinHogSize);
	}
	return 0;
}
