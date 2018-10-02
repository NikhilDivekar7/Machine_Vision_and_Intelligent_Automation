#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>


#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/contrib/contrib.hpp"

using namespace cv;
using namespace std;

// Default resolution is 360p
#define VRES_ROWS (360)
#define HRES_COLS (640)
#define BORDER_WIDTH (4)

#define ESC_KEY (27)

// Buffer for highest resolution visualization possible
unsigned char imagebuffer[1440*2560*3]; // 1440 rows, 2560 cols/row, 3 channel

int main(int argc, char **argv)
{
    int hres = HRES_COLS;
    int vres = VRES_ROWS;
    int top, bottom, left, right;
    Mat basicimage(vres, hres, CV_8UC3, imagebuffer);
    Mat basicimage_border(vres, hres, CV_8UC3, imagebuffer);
    int frameCnt=0;
    Scalar value;

    

    // interactive computer vision loop 
    namedWindow("Profile Visualization", CV_WINDOW_AUTOSIZE);

    // read in default image
    if(vres == 360)
        basicimage = imread("Cactus360p.jpg", CV_LOAD_IMAGE_COLOR);
    else if(vres == 720)
        basicimage = imread("Cactus720p.jpg", CV_LOAD_IMAGE_COLOR);
    else if(vres == 1080)
        basicimage = imread("Cactus1080p.jpg", CV_LOAD_IMAGE_COLOR);
    else if(vres == 1440)
        basicimage = imread("Cactus1440p.jpg", CV_LOAD_IMAGE_COLOR);

    resize(basicimage, basicimage, Size(), 0.5, 0.5, INTER_LINEAR);			//resize image 180x320
    vres = vres/2;		//after resizing
    hres = hres/2;

    printf("hres=%d, vres=%d\n", hres, vres);

    line(basicimage, Point(hres/2,(vres/2-10)), Point(hres/2,(vres/2+10)), Scalar(0,255,255), 1, 8,0); //crosshair vertical

    line(basicimage, Point((hres/2-10), vres/2), Point((hres/2+10),vres/2), Scalar(0,255,255), 1, 8,0); 	//crosshair horizontal

    top = bottom = right = left = BORDER_WIDTH;	//border width

    value = Scalar(0,0,255);

    copyMakeBorder(basicimage, basicimage, top, bottom, left, right, BORDER_CONSTANT, value);		//Border plotting

    if(!basicimage.data)  // Check for invalid input
    {
        printf("Could not open or find the refernece starting image\n");
        exit(-1);
    }

    // Create an IplImage mapping onto the basicimage Mat object
    //
    IplImage basicimageImg = basicimage;


    // Interactive LOOP
    //
    while(1)
    {
        frameCnt++;

        // Write a zero value into the image buffer
        //
        basicimageImg.imageData[frameCnt] = (unsigned char)0;

        imshow("Profile Visualization", basicimage);  

        imwrite("output.jpg", basicimage);	//save image as output image

        // set pace on shared memory sampling in msecs
        char c = cvWaitKey(10);

        if(c == ESC_KEY)
        {
            exit(1);
        }

    }
 
    return 1;

}
