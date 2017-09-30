#pragma once
#include<string>
#include <time.h>
#include<opencv2/opencv.hpp>
#include"LBF.h"
using namespace std;

//3000fps模型路径
string modelPath = "model/";
//训练数据路径
string dataPath = "./../../Datasets/";
//人脸检测模型文件
string cascadeName = "haarcascade_frontalface_alt.xml";
//眼部状态支持�模型文件
string rightEyeStatusSVMModelFile = "EyeOCSVMHog1_3.xml";
string mouthChinStatusSVMModelFile =/* "MouthOCSVM1_0.xml"*/ "MouthOCSVM_HOG_1_0.xml";

//string fatigueWarningAudio = "fatiguewaring.wav";
string yawnWarningAudio = "yawnWarning.wav";
string severeFatigueWarningAudio = "severeFatigueWarning.wav";
//全局参数
Params global_params;
//人脸放缩比例
double scaleForFaceDetection = 0.2;
//double scaleForFaceAlignment = 0.2;
//原始帧
//cv::Mat frame;
//1/2原始帧
cv::Mat cImg;
//灰度图像
cv::Mat gImg;
//缩小后的灰度图像
cv::Mat gSmallImgForFaceDetection;
//cv::Mat gSmallImgForFaceAlignment;
//脸部矩形区域
vector<cv::Rect> faces;

vector<cv::Point> allPoints;

vector<cv::Point> mouthChinPoints;
//特征点坼特征点坐标
vector<cv::Point> rightEyePoints;
//嘴部及下巴特征点坼部矩形放缩比例
double eyeRectScale = 1.3;
//帧数
int frameCount = 0;
//睁眼帧计数
int openEyeFrameCount = 0;
//闭眼帧计数
int closeEyeFrameCount = 0;
//上一秒时间
time_t lastSecond = 0;
//perclose阈值
float percloseThreshold = 0.6;
//张嘴帧计数
int openMouthFrameCount = 0;
//闭嘴帧计数
int closeMouthFrameCount = 0;
//上一帧中嘴巴的状态,0为张嘴，1为闭嘴
int lastMouthStatus = 1;
//开始张嘴时间
time_t startOpenMouthTime;
//哈欠帧计数
int yawnFrameCount=0;
//当前哈欠帧计数
int currentOpenMouthFrameCount = 0;
//哈欠时长阈值
int yawnContinueTimeThreshold = 2;
//哈欠帧比例阈值
float yawnFrameRateThreshold = 0.2;
//哈欠比例
float yawnFrameRate;
//闭眼比例
float perclose;

//显示参数信息

//嘴部状态
string mouthChinStatus;
cv::Scalar mouthChinStatusColor;
//眼睛状态
string eyeStatus;
cv::Scalar eyeStatusColor;
//perclose文本缓冲
char perclose_buf[32];
//显示perclose的颜色
cv::Scalar perclose_color(0, 255, 0);
//yawnFrameRate缓冲
char yawnFrameRate_buf[32];
//yawnFrameRate颜色
cv::Scalar yawnFrameRateColor(0, 255, 0);
//哈欠
char yawnCount_buf[32];
//哈欠计数
int yawnCount = 0;
//上一秒的帧计数
int frameCountInLastSecond = 0;
//上一人脸的区域
cv::Rect lastFaceRect(0,0,0,0);
//帧频字符
char fps_buf[32];
//人脸检测时间
double faceDetectionTime;
//人脸检测时间字符串
char faceDetectionTime_buf[16];
//特征点定位时间
double faceAlignmentTime;
//特征点定位时间字符串
char faceAlignmentTime_buf[16];
//闭嘴帧计数
int interuptCloseMouthFrameCount = 0;
//嘴部图像标准化候的长和宽
const int normalizedMouthWidth = 48;
const int normalizedMouthHeight = 48;

