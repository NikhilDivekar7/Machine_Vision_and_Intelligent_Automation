/*
 *
 *  Example by Sam Siewert 
 *
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/time.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

#define nanosec 1000000000

struct timespec t1, t2, start, end;	

	double diff;
	double array_of_time[300];
	double total;
	double max_val;
	double total_jitter;
	double avg;
	int timelimit; 
	int i = 0;

/*Function to analyze average frame time, worst case time and jitters*/
void analysis()
{
	clock_gettime(CLOCK_REALTIME, &end);

	/* To keep track of one minute*/
	timelimit = (double) ((end.tv_sec ) - (start.tv_sec ));

	/*Frame execution time*/
	diff = (double) ((t2.tv_sec * nanosec + t2.tv_nsec) - (t1.tv_sec * nanosec + t1.tv_nsec));
	
	printf("Time taken for Iteration number %d: %lf nsecs \n",i,diff);	

	if(i == 0)
	{	
		max_val = diff;
	}

	if(timelimit < 60)
	{
		array_of_time[i] = diff;
		/*Finding worst frame execution time*/
		if(diff > max_val)
		{
			max_val = diff;
		}
		
		total += diff;
		i++;
	}
	else
	{
		/*Find average frame execution time*/
		avg = total/i;
		printf("Average time: %lf nsecs\n", avg);

		/*Find average frame rate*/
		printf("Average FPS: %lf \n", nanosec/avg);
		printf("Worst time: %lf nsecs\n", max_val);

		/*Find total jitter*/
		for(int j = 0; j < i; j++)
		{	
			if(array_of_time[j] > avg)
			{
				total_jitter += array_of_time[j] - avg;
			}
		}
		printf("Total jitter: %lf nsecs \n", total_jitter);
		total_jitter = 0;
		i = 0;
		total = 0;
		clock_gettime(CLOCK_REALTIME, &start);
	}

}

int main( int argc, char** argv )
{

    cvNamedWindow("Capture Example", CV_WINDOW_AUTOSIZE);
    CvCapture* capture = cvCreateCameraCapture(0);
    IplImage* frame;
	
    clock_gettime(CLOCK_REALTIME, &start);

    while(1)
    {
	
	clock_gettime(CLOCK_REALTIME, &t1);	
        frame=cvQueryFrame(capture);     
	clock_gettime(CLOCK_REALTIME, &t2);

	analysis();
        if(!frame) break;

        cvShowImage("Capture Example", frame);

        char c = cvWaitKey(33);
        if( c == 27 ) break;
    }

    cvReleaseCapture(&capture);
    cvDestroyWindow("Capture Example");
    
};
