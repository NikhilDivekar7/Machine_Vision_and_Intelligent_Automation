#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

using namespace cv;

#define nanosec 1000000000

Mat src, src_gray;
Mat dst, detected_edges;
Mat grad; 
int edgeThresh = 1;
int lowThreshold;
int const max_lowThreshold = 100;
int ratio = 3;
int kernel_size = 3;
char* window_name = "Canny/Sobel";
int scale = 1;
int delta = 0;
int ddepth = CV_16S;
int c;

struct timespec t1, t2, t3, start, end;
double diff;
double array_of_time[200];
double total;
double max_val;
double total_jitter;
double avg;
int timelimit;

int i = 0;


/*Function to analyze average frame time, worst case time and jitters*/
void analysis()
{
	/*Frame execution time*/
	diff = (double) ((t2.tv_sec * nanosec + t2.tv_nsec) - (t1.tv_sec * nanosec + t1.tv_nsec));

	clock_gettime(CLOCK_REALTIME, &end);

	/* To keep track of one minute*/
	timelimit = (double) ((end.tv_sec ) - (start.tv_sec ));	
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
		avg = total/i;
		/*Find average frame execution time*/
		printf("Average time: %lf nsecs \n", avg);

		/*Find average frame rate*/
		printf("Average FPS: %lf \n", nanosec/avg);
		printf("Worst time: %lf nsecs \n", max_val);

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
/**
 * @function CannyThreshold
 * @brief Trackbar callback - Canny thresholds input with a ratio 1:3
 */
void CannyThreshold(int, void*)
{
  /// Reduce noise with a kernel 3x3
  blur( src_gray, detected_edges, Size(3,3) );

  /// Canny detector
  Canny( detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size );

  /// Using Canny's output as a mask, we display our result
  dst = Scalar::all(0);

  src.copyTo( dst, detected_edges);
  imshow( window_name, dst );
}

/** @function main */
int main( int argc, char** argv )
{
	printf("Initializing things \n");
	printf("Default: Canny. \n");
	printf("Press S or s for Sobel \n");
	printf("Press C or c for Canny \n");

	CvCapture* capture = cvCreateCameraCapture(0);
	IplImage* frame;
	int canny = 1;
	int sobel = 0;

	clock_gettime(CLOCK_REALTIME, &start);
  	
while(1)
{
	clock_gettime(CLOCK_REALTIME, &t1);
	src=cvQueryFrame(capture);
	clock_gettime(CLOCK_REALTIME, &t2);
	if(canny == 1)
	{
		if( !src.data )
  		{ 
			return -1; 
		}

  		/// Create a matrix of the same type and size as src (for dst)
  		dst.create( src.size(), src.type() );

  		/// Convert the image to grayscale
  		cvtColor( src, src_gray, CV_BGR2GRAY );

  		/// Create a window
  		namedWindow( window_name, CV_WINDOW_AUTOSIZE );

  		/// Create a Trackbar for user to enter threshold
  		createTrackbar( "Min Threshold:", window_name, &lowThreshold, max_lowThreshold, CannyThreshold );

  		/// Show the image
  		CannyThreshold(0, 0);

  		/// Wait until user exit program by pressing a key
  		char b = waitKey(33);

		/*Execute Sobel for 's' or 'S'*/
  		if(b == 83 || b == 115)	
		{
			canny = 0;
			sobel = 1;
		}
		if(b == 27) break;
	}
	else if(sobel == 1) 
	{
  		if( !src.data )
    		{ 
			return -1; 
		}

  		GaussianBlur( src, src, Size(3,3), 0, 0, BORDER_DEFAULT );
	
 		/// Convert it to gray
  		cvtColor( src, src_gray, CV_RGB2GRAY );

  		/// Create window
  		namedWindow( window_name, CV_WINDOW_AUTOSIZE );

  		/// Generate grad_x and grad_y
  		Mat grad_x, grad_y;
  		Mat abs_grad_x, abs_grad_y;
 
  		/// Gradient X
  		//Scharr( src_gray, grad_x, ddepth, 1, 0, scale, delta, BORDER_DEFAULT );
  		Sobel( src_gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );   
  		convertScaleAbs( grad_x, abs_grad_x );

  		/// Gradient Y 
  		//Scharr( src_gray, grad_y, ddepth, 0, 1, scale, delta, BORDER_DEFAULT );
  		Sobel( src_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT );   
  		convertScaleAbs( grad_y, abs_grad_y );

  		/// Total Gradient (approximate)
  		addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad );

  		imshow( window_name, grad );
  		char b = waitKey(33);

		/*Execute Canny for 'c' or 'C'*/
  		if(b == 67 || b == 99)
		{
			canny = 1;
			sobel = 0;
		}
		if(b == 27) break;
	}
	analysis();
}
return 0;
}
