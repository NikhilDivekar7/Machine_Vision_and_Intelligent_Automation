#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <string>
#include <stdio.h>

#include <iostream>
using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	string name;
	int count=0;
	char key;
	VideoCapture cap(0);

	while(1){
		Mat src;
		if(!cap.read(src)){
			cout<<"Error in reading frame"<<endl;
			break;
		}
		Mat gray, binary, mfblur;

		cvtColor(src, gray, CV_BGR2GRAY);

		// Use 70 negative for Moose, 150 positive for hand
		// 
		// To improve, compute a histogram here and set threshold to first peak
		//
		// For now, histogram analysis was done with GIMP
		//
		threshold(gray, binary, 150, 255, CV_THRESH_BINARY);
		binary = 255 - binary;

		// show bitmap of source image and wait for input to next step
//		imshow("binary", binary);
		//	waitKey();

		// To remove median filter, just replace blurr value with 1
		medianBlur(binary, mfblur, 1);

		// show median blur filter of source image and wait for input to next step
		//imshow("mfblur", mfblur);
		//waitKey();

		// This section of code was adapted from the following post, which was
		// based in turn on the Wikipedia description of a morphological skeleton
		//
		// http://felix.abecassis.me/2011/09/opencv-morphological-skeleton/
		//
		Mat skel(mfblur.size(), CV_8UC1, Scalar(0));
		Mat temp;
		Mat eroded;
		Mat element = getStructuringElement(MORPH_CROSS, Size(3, 3));
		bool done;
		int iterations=0;

		do
		{
			erode(mfblur, eroded, element);
			dilate(eroded, temp, element);
			subtract(mfblur, temp, temp);
			bitwise_or(skel, temp, skel);
			eroded.copyTo(mfblur);

			done = (countNonZero(mfblur) == 0);
			iterations++;

		} while (!done && (iterations <25));

		cout << "iterations=" << iterations << endl;

		imshow("skeleton", skel);
		
		char filename[80];
		sprintf(filename, "image%d.jpg", count);
		count ++;
		imwrite(filename, skel);

		key=waitKey(1);
		if(key==27){
			cout<<"Exiting the program since user pressed the Escape key"<<endl;
			break;
		}
		//	waitKey();
	}
	//	return 0;
}
