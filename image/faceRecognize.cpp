#define BOOST_PYTHON_SOURCE
/*
#include <boost/python.hpp>
#include <string>
#include <time.h>
#include <fstream>
#include <sstream>  
#include <vector>
#include <face_detection.h>
#include <face_alignment.h>
#include <face_identification.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <opencv2/imgcodecs.hpp>
*/
#include <faceRecognize.h>
#include <dirent.h>  
#include <sys/types.h>  
#include <sys/stat.h> 
#include <stdio.h> 

//extern "C"
//{

struct driverfeat{
	unsigned long id;
	float feat[2048];
};

const string base_dir = "/home/pi/";
const string seeta_fd_frontal = "/home/pi/data_iov/model/seeta_fd_frontal_v1.0.bin";
const string seeta_fa = "/home/pi/data_iov/model/seeta_fa_v1.1.bin";
const string seeta_fr = "/home/pi/data_iov/model/seeta_fr_v1.0.bin";
float* feat_sdk; //= new float[feat_size * 20];
int feat_size = 2048;
float sim_threshold = 0.5;


int getCropFace(const cv::Mat &src_img, cv::Mat &dst_img, seeta::FaceDetection &detector, seeta::FaceAlignment &alignment, seeta::FaceIdentification &face_recognizer)
{
	const std::string path_cropimages = base_dir + "data_iov/crop_image/";
	clock_t start, finish;
	int width = 200;
	int height = 200;
	cv::Mat dst(height * 5, width * 4, CV_8UC3);

	/**********************************************/
	cv::Mat src;
	cv::cvtColor(src_img, src, CV_BGR2GRAY);

	seeta::ImageData img_data;
	img_data.data = src.data;
	img_data.width = src.cols;
	img_data.height = src.rows;
	img_data.num_channels = 1;

	std::vector<seeta::FaceInfo> faces = detector.Detect(img_data);
	if (faces.size() == 0) {
		//fprintf(stderr, "don't detect face\n");
		return -1;
	}

	// Detect 5 facial landmarks: two eye centers, nose tip and two mouth corners
	std::vector<seeta::FacialLandmark> landmard(5);
	alignment.PointDetectLandmarks(img_data, faces[0], &landmard[0]);

	// landmards.push_back(landmard);
	/**********************************************/

	if (face_recognizer.crop_channels() != src_img.channels()) {
		//fprintf(stderr, "channels dismatch: %d, %d\n", face_recognizer.crop_channels(), src_img.channels());
		return -1;
	}

	// ImageData store data of an image without memory alignment.
	seeta::ImageData src_img_data(src_img.cols, src_img.rows, src_img.channels());
	src_img_data.data = src_img.data;

	// Create a image to store crop face.

	seeta::ImageData dst_img_data(dst_img.cols, dst_img.rows, dst_img.channels());
	dst_img_data.data = dst_img.data;

	// Crop Face
	face_recognizer.CropFace(src_img_data, &landmard[0], dst_img_data);
	return 0;
}

void getFeature(cv::Mat &img, float *feat, seeta::FaceIdentification &face_recognizer) {
	seeta::ImageData dst_img_data(img.cols, img.rows, img.channels());
	dst_img_data.data = img.data;

	// Extract feature
	face_recognizer.ExtractFeature(dst_img_data, feat);
}

int loadfacelib(string libfile, vector<driverfeat> &dfeat){
	FILE *fp;
	fp = fopen(libfile.c_str(), "r");
	if(fp == NULL){
		printf("file not exist!");
	}
	driverfeat tmp;
	while(!feof(fp)){
		if(fscanf(fp, "%ld", &tmp.id) == EOF)
			break;
		for(int i = 0 ; i < feat_size ; i++){
			fscanf(fp, ",%f", &tmp.feat[i]);
		}
		dfeat.push_back(tmp);
	}
	fclose(fp);
	return 1;
}

int updatelib(string libfile, driverfeat df){
	FILE *fp;
	fp = fopen(libfile.c_str(), "a");

	if(fp == NULL){
		printf("file not exist!\n");
	}

	fprintf(fp, "%lu", df.id);
	for(int i = 0; i < feat_size; i++){
		fprintf(fp, ",%f", df.feat[i]);
	}
	fprintf(fp, "\n");
	fclose(fp);
	return 1;
}

long faceRecognize(cv::Mat &img, string libfile){
	vector<driverfeat> dfeat;

	loadfacelib(libfile, dfeat);		

	seeta::FaceDetection detector(seeta_fd_frontal.c_str());
        seeta::FaceAlignment alignment(seeta_fa.c_str());
        seeta::FaceIdentification face_recognizer(seeta_fr.c_str());
        detector.SetMinFaceSize(20);
        detector.SetMaxFaceSize(200);
        detector.SetScoreThresh(2.f);
        detector.SetImagePyramidScaleFactor(0.8f);
        detector.SetWindowStep(4, 4); 

	int width = 200;
        int height = 200;
	
	cv::Mat dst_img(face_recognizer.crop_height(), face_recognizer.crop_width(), CV_8UC(face_recognizer.crop_channels()));
	float srcfeat[feat_size];
	
	if(getCropFace(img, dst_img, detector, alignment, face_recognizer) != -1){	
		getFeature(dst_img, srcfeat, face_recognizer);
	
		float sim = 0;
		float maxsim = 0;
	        int maxid = -1;	
	
		for(int i = 0; i < dfeat.size(); i++){
			sim = face_recognizer.CalcSimilarity(srcfeat, dfeat[i].feat);
			if (maxsim < sim) {
	                        maxsim = sim;
	                        maxid = dfeat[i].id;
	                }
		}
		if(maxsim > sim_threshold){
			return maxid;	//return match face id
		}
	}
	else 
		return -2;	// face not detect	
	return -1;	// face not found
}

long faceRecognizeWithFile(const string filepath, const string libfile){
	cv::Mat img = cv::imread(filepath, 1);
	if(img.data == nullptr){
		printf("image file not found!\n");
		return -1;
	}
	return faceRecognize(img, libfile);
}

int createfacelib(const string path_rawimages, const string path_cropimages, const string libfile)
{
	//const std::string path_cropimages = base_dir + "data_iov/crop_image/";
	//const std::string path_rawimages = base_dir + "data_iov/raw_image/";

	seeta::FaceDetection detector(seeta_fd_frontal.c_str());
	seeta::FaceAlignment alignment(seeta_fa.c_str());
	seeta::FaceIdentification face_recognizer(seeta_fr.c_str());
	cout<< "start program" <<endl;	
	detector.SetMinFaceSize(20);
	detector.SetMaxFaceSize(200);
	detector.SetScoreThresh(2.f);
	detector.SetImagePyramidScaleFactor(0.8f);
	detector.SetWindowStep(4, 4);

	int width = 200;
	int height = 200;

	cv::Mat dst_img(face_recognizer.crop_height(), face_recognizer.crop_width(), CV_8UC(face_recognizer.crop_channels()));

	// extract feature
	feat_size = face_recognizer.feature_size();
	printf("feat_size: %d\n", feat_size);

	if (feat_size != 2048) {
		fprintf(stderr, "feature size mismatch: %d\n", feat_size);
		return -1;
	}

	struct dirent* ent = NULL;  
    	DIR *pDir;  
    	pDir = opendir(path_rawimages.c_str());
  
    	if (pDir == NULL) {  
    	    return 0;  
    	}  

	std::string image;
	cv::Mat src_img;

	driverfeat dfeat;

    	while (NULL != (ent = readdir(pDir))) {  
		if (ent->d_type == 8) {  
			//file  
			//printf("%s\n", ent->d_name); 
			int _pos = string(ent->d_name).find_first_of('.');
			dfeat.id = stoul(string(ent->d_name, _pos));
			image = path_rawimages + "/" + string(ent->d_name);
			cout << image << " id:"<< dfeat.id <<endl;
			src_img = cv::imread(image, 1);
			if (src_img.data == nullptr) {
	                	fprintf(stderr, "Load image error: %s\n", image.c_str());
	                	continue;
       			}

			if(getCropFace(src_img, dst_img, detector, alignment, face_recognizer) == -1){
				printf("face not found!\n");
				continue;
			}
			cv::imwrite(path_cropimages + "/" + "crop_" + string(ent->d_name), dst_img);
			getFeature(dst_img, dfeat.feat, face_recognizer);
			updatelib(libfile, dfeat);
		} 
    	}

	return 0;
}

std::string get_image_name(std::string name)
{
	std::string name_ = name;
	int pos = name_.find_last_of("/");
	return name_.erase(0, pos + 1);
}

//}

int main(){
/*	char *basedir = getenv("IOVPATH");

        // initial logger
        try{
                string log_path = string(basedir) + string("/raspi_image.log");
                cout << log_path << endl;
                auto my_logger = spd::basic_logger_mt("faceRecognize_logger", log_path.c_str());
                my_logger->info("face recognize");

        }
        catch (const spd::spdlog_ex& ex){
                cout << "Log init failed: " << ex.what() << std::endl;
                return -1; 
        }
*/
	createfacelib(base_dir + "data_iov/raw_image/", base_dir + "data_iov/crop_image/", "facelib.csv");
/*
	vector<driverfeat> df;
	loadfacelib("facelib.csv", df);
	for(int i = 0; i < df.size(); i++){
		printf("id: %ld, feat: %f %f %f %f %f\n", df[i].id, df[i].feat[0], df[i].feat[1], df[i].feat[2], df[i].feat[3], df[i].feat[4]);
	}
*/
/*	cv::Mat img = cv::imread(base_dir + "data_iov/raw_image/" + "0.jpg", 1);
	cout <<	faceRecognize(img, "facelib.csv") << endl;
	return 0;

*/
}
