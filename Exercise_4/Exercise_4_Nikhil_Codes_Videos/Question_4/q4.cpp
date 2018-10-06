#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <string>
#include <stdio.h>

#include <iostream>
using namespace cv;
using namespace std;

int main()
{
	VideoCapture cap(0);		//capture video
	
	Mat original, previous, delta, grayscale, binary_map;
	
	cap >> previous;			//store prev frame

	int count = 0;
	
	while(1)
	{
		cap >> original;		//current frame
		
		if(original.empty())
		{
			cout<<" Can not open camera\n";
			return -1;
		}
		
		//imshow("source",original);
		
		absdiff(previous,original,delta);	//difference in frames
		
		//imshow("delta",delta);
		
		cvtColor(delta, grayscale, CV_BGR2GRAY);
		
		//imshow("grayscalemap", grayscale);
		
		threshold(grayscale, binary_map,30,255,CV_THRESH_BINARY);
		
		imshow("threshold", binary_map);	
		
		int i,j;

		int N,S;
		int p1, p2, p3, p4, p5, p6, p7, p8, p9, nz_pixel;	//pixels
 		int transition;
		
		for(i = 1; i < binary_map.rows-1; i++)
		{
			for(j = 1; j < binary_map.cols-1; j++)
			{
				N=S=0;
				
				//get scaled values of pixels
				p1 = binary_map.at<ushort>(i,j)/65535;
				p2 = binary_map.at<ushort>(i-1,j)/65535;
				p3 = binary_map.at<ushort>(i-1,j+1)/65535;
				p4 = binary_map.at<ushort>(i,j+1)/65535;
				p5 = binary_map.at<ushort>(i+1,j+1)/65535;
				p6 = binary_map.at<ushort>(i+1,j)/65535;
				p7 = binary_map.at<ushort>(i+1,j-1)/65535;
				p8 = binary_map.at<ushort>(i,j-1)/65535;
				p9 = binary_map.at<ushort>(i-1,j-1)/65535;

				N = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;

				if(p2 == 0 && p3 == 1)
				S++;

				if(p3 == 0 && p4 == 1)
				S++;
		
				if(p4 == 0 && p5 == 1)
				S++;

				if(p5 == 0 && p6 == 1)
				S++;

				if(p6 == 0 && p7 == 1)
				S++;

				if(p7 == 0 && p8 == 1)
				S++;

				if(p8 == 0 && p9 == 1)
				S++;

				if(p9 == 0 && p2 == 1)
				S++;
				
				//Condition for iteration 1
				if( p1!=0 && N>=2 && N<=6 && S==1 && (p2*p4*p6)==0 && (p4*p6*p8==0) )
				{
					binary_map.at<ushort>(i,j) = 0;
				}
			}
		}

		for(i = 1; i < binary_map.rows-1; i++)
		{
			for(j = 1; j < binary_map.cols-1; j++)
			{
				N=S=0;

				p1 = binary_map.at<ushort>(i,j)/65535;
				p2 = binary_map.at<ushort>(i-1,j)/65535;
				p3 = binary_map.at<ushort>(i-1,j+1)/65535;
				p4 = binary_map.at<ushort>(i,j+1)/65535;
				p5 = binary_map.at<ushort>(i+1,j+1)/65535;
				p6 = binary_map.at<ushort>(i+1,j)/65535;
				p7 = binary_map.at<ushort>(i+1,j-1)/65535;
				p8 = binary_map.at<ushort>(i,j-1)/65535;
				p9 = binary_map.at<ushort>(i-1,j-1)/65535;

				N = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;

				if(p2 == 0 && p3 == 1)
				S++;

				if(p3 == 0 && p4 == 1)
				S++;
		
				if(p4 == 0 && p5 == 1)
				S++;

				if(p5 == 0 && p6 == 1)
				S++;

				if(p6 == 0 && p7 == 1)
				S++;

				if(p7 == 0 && p8 == 1)
				S++;

				if(p8 == 0 && p9 == 1)
				S++;

				if(p9 == 0 && p2 == 1)
				S++;

				//Conditions for iteration 2
				if( p1!=0 && N>=2 && N<=6 && S==1 && (p2*p4*p8)==0 && (p2*p6*p8==0) )
				{
					binary_map.at<ushort>(i,j) = 0;
				}
			}
		}
		
		//show thinned frame
		imshow("thining", binary_map);
		char filename[80];

		//save frames
		sprintf(filename, "Frame%d.jpg", count);
		count ++;
		imwrite(filename, binary_map);
		
		cap >> previous;
		
		//waitKey();
		
		char c = cvWaitKey(33);
 		if( c == 27 ) break;
 
	}
}
