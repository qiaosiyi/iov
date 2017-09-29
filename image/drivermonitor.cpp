#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include "faceRecognize.h"
#include "spdlog/spdlog.h"
#include "fatigueDetect.h"
#include <memory>
#include <time.h>

using namespace std;
using namespace cv;
namespace spd = spdlog;

int fr_interval = 10;

string getID(){
	time_t t = time(0);
	char tmp[10];
	strftime( tmp, sizeof(tmp), "%m%d%H%M",localtime(&t));
	return string(tmp);
}

void *message_helper(void *command){
	int cnt = 5;
	int res = 1;
	printf("try to send pic\n");
	while(res != 0 && cnt--){
		res = system((char*)command);
	}
	return 0;
}

int main(int argc, char* argv[]){
	if(argc < 2)
		return 0;
	fr_interval = stoi(string(argv[1]));

	int capnum = 0;
	if(argc == 3)
		capnum = stoi(string(argv[2]));

	char *basedir = getenv("IOVPATH");
	string facedir = "/home/pi/data_iov/raw_image/";
	string fatiguedir = "/home/pi/data_iov/fatigue_image/";
	char *hostip = getenv("SERVERIP");
	// initial logger
	try{
		string log_path = string(basedir) + string("/raspi_image.log");
		cout << log_path << endl;
		auto my_logger = spd::basic_logger_mt("drivermonitor_logger", log_path.c_str());
		my_logger->info("Some log message");
	}
	catch (const spd::spdlog_ex& ex){
		cout << "Log init failed: " << ex.what() << std::endl;
		return -1;
	}
	//initial fatigue detect
	cv::CascadeClassifier fcc;
        fcc.load((modelPath+cascadeName).c_str());
    
        ReadGlobalParamFromFile(modelPath + "LBF.model");

        LBFRegressor regressor;
        regressor.Load(modelPath + "LBF.model");
    
        cv::Ptr<cv::ml::SVM> rightEyeStatusSVM=cv::ml::SVM::load<cv::ml::SVM>((modelPath+rightEyeStatusSVMModelFile).c_str());

        cv::HOGDescriptor eyeHog(cv::Size(48, 16), cv::Size(16, 16), cv::Size(8, 8), cv::Size(8, 8), 9); 
        const int eyeHogSize = eyeHog.getDescriptorSize();

        cv::Ptr<cv::ml::SVM> mouthChinStatusSVM = cv::ml::SVM::load<cv::ml::SVM>((modelPath+mouthChinStatusSVMModelFile).c_str());
        cv::HOGDescriptor mouthChinHog(cv::Size(48, 48), cv::Size(16, 16), cv::Size(8, 8), cv::Size(8, 8), 9); 
        const int mouthChinHogSize = mouthChinHog.getDescriptorSize();


	cv::VideoCapture cap(0);
        if (!cap.isOpened())
        {
                cout << "fail to open camera" << endl;
                return -1; 
        }

	cv:Mat frame;
        cap.set(CV_CAP_PROP_FRAME_WIDTH,360);
        cap.set(CV_CAP_PROP_FRAME_HEIGHT,480);
	time_t start, finish;
	time(&start);
	double inter = 0;
	int errtimes = 0;
	printf("start monitoring\n");
	while(true){
		cap >> frame;
		//cv::imshow("raw_stream", frame);
		//do something
		time(&finish);
		inter = difftime(finish, start);
		if(inter > fr_interval){
		// face recognize
			printf("face recognize!\n");
			int res = faceRecognize(frame, "facelib.csv");
			if(res == -1){
				printf("person not exist!\n");
				errtimes++;
				if(errtimes > 2){
					time(&start);
					errtimes = 0;
					//upload image
					printf("report event: person not exist!\n");
					string  _id = getID();
					string imgfile = facedir + _id + ".jpg";
					cv::imwrite(imgfile, frame);
					string command =  "python " + string(basedir) + "/rabbitMQ/send_message.py -m " + "\'{\"filepath\":\""+imgfile+"\",\"tag\":2}\'";
					pthread_t t;
					int ret = pthread_create(&t, NULL , message_helper, (void*)(command.c_str()));
					//int errorcode = system(command.c_str());
					//cout << command << endl << errorcode << endl;
				}
			}
			else if(res == -2){
				//face not detect
				printf("face not detect! try again!\n");
				string command = "python " + string(basedir) + "/rabbitMQ/send_message.py -m " + "\'{\"tag\":1,\"result\":\"face not detect\"}\'";
//python send_message.py -m '{"tag":1,"result":"hello world"}'
				int errorcode = system(command.c_str());
				//cout << command << endl << errorcode << endl;
			}
			else{
				printf("match %ld\n", res);
				string command =  "python " + string(basedir) + "/rabbitMQ/send_message.py -m " + "\'{\"did\":\""+to_string(res)+"\",\"tag\":3}\'";
				pthread_t t;
				int ret = pthread_create(&t, NULL , message_helper, (void*)(command.c_str()));
				time(&start);
			}
		}
		else{
		// fatigue detect
			//printf("fatigue detect! %f\n", inter);
			int res = fatiguepredict(frame, fcc, regressor, eyeHog, eyeHogSize, rightEyeStatusSVM, mouthChinStatusSVM, mouthChinHog, mouthChinHogSize);
			if(res != 0){
				printf("fatigue detect res : %d\n", res);
				string  _id = getID();
				string imgfile = fatiguedir + _id + ".jpg";
				cv::imwrite(imgfile, frame);
				string command =  "python " + string(basedir) + "/rabbitMQ/send_message.py -m " + "\'{\"filepath\":\""+imgfile+"\",\"tag\":4,\"fatigue\":"+ to_string(res) +"}\'";
				pthread_t t;
				int ret = pthread_create(&t, NULL , message_helper, (void*)(command.c_str()));
				//int errorcode = system(command.c_str());
				//cout << command << endl << errorcode << endl;				
			}
		
		}
		//if(cv::waitKey(30) > 0)
                //        break;
	}
	return 0;
}
