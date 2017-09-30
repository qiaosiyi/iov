#include "fatigueDetect.h"

int main(){
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
