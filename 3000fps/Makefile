cflag = -std=c++11 -I /usr/local/include/ -L /usr/local/lib -lopencv_core -lopencv_videoio -lopencv_ml -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc -lopencv_calib3d -lopencv_objdetect

FatigueDetect: fatigueDetect.cpp Utils.cpp Facedetect.cpp LBFRegressor.cpp  RandomForest.cpp Tree.cpp liblinear/linear.cpp liblinear/tron.cpp     liblinear/blas/daxpy.c liblinear/blas/ddot.c
	g++ -o FatigueDetect fatigueDetect.cpp Utils.cpp Facedetect.cpp LBFRegressor.cpp  RandomForest.cpp Tree.cpp liblinear/linear.cpp liblinear/tron.cpp liblinear/blas/daxpy.c liblinear/blas/ddot.c liblinear/blas/dnrm2.c liblinear/blas/dscal.c $(cflag)

dynamiclib: fatigueDetect.cpp Utils.cpp Facedetect.cpp LBFRegressor.cpp  RandomForest.cpp Tree.cpp liblinear/linear.cpp liblinear/tron.cpp     liblinear/blas/daxpy.c liblinear/blas/ddot.c 
	g++ -shared -o libfatigueDetect.so fatigueDetect.cpp Utils.cpp Facedetect.cpp LBFRegressor.cpp  RandomForest.cpp Tree.cpp liblinear/linear.cpp liblinear/tron.cpp     liblinear/blas/daxpy.c liblinear/blas/ddot.c liblinear/blas/dnrm2.c liblinear/blas/dscal.c $(cflag)

main: main.cpp Utils.cpp Facedetect.cpp LBFRegressor.cpp  RandomForest.cpp Tree.cpp liblinear/linear.cpp liblinear/tron.cpp     liblinear/blas/daxpy.c liblinear/blas/ddot.c
	g++ -o main main.cpp Utils.cpp Facedetect.cpp LBFRegressor.cpp  RandomForest.cpp Tree.cpp liblinear/linear.cpp liblinear/tron.cpp liblinear/blas/daxpy.c liblinear/blas/ddot.c liblinear/blas/dnrm2.c liblinear/blas/dscal.c $(cflag)
