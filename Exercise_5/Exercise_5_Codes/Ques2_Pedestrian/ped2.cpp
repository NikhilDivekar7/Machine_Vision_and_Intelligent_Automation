//Reference: http://www.magicandlove.com/blog/2011/12/04/people-detection-sample-from-opencv/

#include <iostream>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdio.h>
#include <string.h>

using namespace std;
using namespace cv;

int show = 0;
int store = 0;
int number_people;
int Frame = 1;
 
int main (int argc, const char * argv[])
{
    int i = 0;
    while(i < argc)
    {
	if(string(argv[i]) == "--show")
	{
		show = 1;
	}
	if(string(argv[i]) == "--store")
	{
		store = 1;
	}
	i++;
    }

    string file_name;
    file_name = string(argv[1]);

    FILE * fptr;
    fptr = fopen("log.txt", "w");
    if(fptr != NULL)
	printf("Error in file opening");

    VideoCapture cap(file_name);
    cap.set(CV_CAP_PROP_FRAME_WIDTH, 320);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, 240);  

    int frame_width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    int frame_height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

    VideoWriter output_vid("output_video.avi", CV_FOURCC('M', 'J', 'P', 'G'), 10, Size(frame_width, frame_height));
    if (!cap.isOpened())
        return -1;
 
    Mat img;
    //const char* filename = argc >= 2 ? argv[1] : "pic1.jpg";
    //img = imread(filename, CV_LOAD_IMAGE_COLOR);
    HOGDescriptor hog;
    hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
 
    //namedWindow("video capture", CV_WINDOW_AUTOSIZE);
    while(1)
    {
	fptr = fopen("log.txt", "a");
        cap >> img;
        if (!img.data)
	{
            printf("Error");
	}

	if(show == 1)
	imshow("Input", img);

	if(show == 1)
	imshow("Resized input", img);
 
        vector<Rect> people_found, people_marked;
        hog.detectMultiScale(img, people_found, 0, Size(8,8), Size(32,32), 1.05, 2);
 
        size_t i, j;
        for (i=0; i<people_found.size(); i++)
        {
            Rect r = people_found[i];
            for (j=0; j< people_found.size(); j++)
                if (j!=i && (r & people_found[j])==r)
                    break;
            if (j==people_found.size())
                people_marked.push_back(r);
        }
	//if(show == 1)
	{
        	for (i=0; i<people_marked.size(); i++)
		{
		    Rect r = people_marked[i];
		    r.x += cvRound(r.width*0.1);
		    r.width = cvRound(r.width*0.8);
		    r.y += cvRound(r.height*0.06);
		    r.height = cvRound(r.height*0.9);

		    rectangle(img, r.tl(), r.br(), cv::Scalar(0,255,0), 2);

		    Point center = (r.br() + r.tl())* 0.5;

		    printf("Pedestrian detected at x = %d and y = %d \n", center.x, center.y);
		    fprintf(fptr, "Frame number: %d \n", Frame);

		    fprintf(fptr, "Pedestrian detected at x = %d and y = %d \n", center.x, center.y);
		}
	}
        imshow("Output", img);
	
	number_people = people_marked.size();
	cout << "Number of people in this frame: " << people_marked.size() << endl;
	fprintf(fptr, "Number of people in the frame: %d", number_people);

	printf("\n");
	fprintf(fptr, "%s", "\n");

	if(store == 1)
	{
	output_vid << img;
	}

	Frame++;

        if (waitKey(20) >= 0)
            break;
	fclose(fptr);
	//waitKey();
    }
    return 0;
}
