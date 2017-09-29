cflag = -std=c++11 -O3 -L /home/pi/Documents/3000fps/3000fps -I /home/pi/Documents/3000fps/3000fps -I /usr/include/python2.7/ -I /usr/local/include -lopencv_imgcodecs  -lopencv_imgproc -L /usr/local/lib -lopencv_core -L ~/data_iov/lib/ -lviplnet -lseeta_fa_lib -lseeta_facedet_lib -I /home/pi/SeetaFaceEngine/FaceDetection/include -I /home/pi/SeetaFaceEngine/FaceAlignment/include -I /home/pi/SeetaFaceEngine/FaceIdentification/include  -lpython2.7 -lopencv_highgui -lopencv_videoio -pthread -flto -lopencv_ml -lopencv_calib3d -lopencv_objdetect -I . -L .

fatiguefile = /home/pi/Documents/3000fps/3000fps/fatigueDetect.cpp /home/pi/Documents/3000fps/3000fps/Utils.cpp /home/pi/Documents/3000fps/3000fps/Facedetect.cpp  /home/pi/Documents/3000fps/3000fps/LBFRegressor.cpp  /home/pi/Documents/3000fps/3000fps/RandomForest.cpp  /home/pi/Documents/3000fps/3000fps/Tree.cpp  /home/pi/Documents/3000fps/3000fps/liblinear/linear.cpp  /home/pi/Documents/3000fps/3000fps/liblinear/tron.cpp  /home/pi/Documents/3000fps/3000fps/liblinear/blas/daxpy.c  /home/pi/Documents/3000fps/3000fps/liblinear/blas/ddot.c

faceRecognize: faceRecognize.cpp
	g++ -o  faceRecognize faceRecognize.cpp $(cflag)

dynamiclib: faceRecognize.cpp
	g++ -shared -o libfaceRecognize.so faceRecognize.cpp $(cflag)

drivermonitor: drivermonitor.cpp 
	g++ -o drivermonitor drivermonitor.cpp $(cflag) -lfaceRecognize -lfatigueDetect

crosslinedetect: cross_line_detect.cpp
	g++ -o cross_line_detect cross_line_detect.cpp -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc -lopencv_videoio
