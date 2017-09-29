#ifndef FBC_FACE_TEST_SEETAFACEENGINE_HPP_
#define FBC_FACE_TEST_SEETAFACEENGINE_HPP_

//#include <boost/python.hpp>
#include <string>
#include <time.h>
#include <fstream>
#include <sstream>  
#include <vector>
#include <stdlib.h>
#include <face_detection.h>
#include <face_alignment.h>
#include <face_identification.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include "spdlog/spdlog.h"
#include <memory>

namespace spd = spdlog;
using namespace std;
//using namespace boost::python;
using namespace spdlog;
extern "C"{

struct driverfeat;

int getCropFace(const cv::Mat &src_img, cv::Mat &dst_img, seeta::FaceDetection &detector, seeta::FaceAlignment &alignment, seeta::FaceIdentification &face_recognizer);

void getFeature(cv::Mat &img, float *feat, seeta::FaceIdentification &face_recognizer);

int loadfacelib(string libfile, vector<driverfeat> &dfeat);

int updatelib(string libfile, driverfeat df);

long faceRecognize(cv::Mat &img, string libfile);

long faceRecognizeWithFile(const string filepath, const string libfile);

int createfacelib(const string path_rawimages, const string path_cropimages);

std::string get_image_name(std::string name);
}

#endif // FBC_FACE_TEST_SEETAFACEENGINE_HPP_
