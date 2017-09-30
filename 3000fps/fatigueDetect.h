#include<opencv2/opencv.hpp>
#include "LBF.h"
#include "LBFRegressor.h"
#include"variables.h"

using namespace std;
using namespace cv;

void ReadGlobalParamFromFile(string path);

cv::Mat boundingBoxImg(cv::Mat & cImg, vector<cv::Point> &featurePoints, double scale);

int fatiguepredict(cv::Mat &frame, cv::CascadeClassifier &fcc, LBFRegressor &regressor, cv::HOGDescriptor &eyeHog, const int &eyeHogSize,
                cv::Ptr<cv::ml::SVM> &rightEyeStatusSVM, cv::Ptr<cv::ml::SVM> &mouthChinStatusSVM, cv::HOGDescriptor &mouthChinHog, const int &mouthChinHogSize);
