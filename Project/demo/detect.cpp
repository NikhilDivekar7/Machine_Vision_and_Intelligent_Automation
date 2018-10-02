#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <pthread.h>
#include <semaphore.h>

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

Mat lane_detect(Mat& frame);

Mat detectAndDraw( Mat& img,
                   CascadeClassifier& cascade, CascadeClassifier& nestedCascade,
                   double scale);

void * first_thread_func(void * data);

void * second_thread_func(void * data);

void * third_thread_func(void * data);

String cascadeName = "../../data/haarcascades/haarcascade_frontalface_alt.xml";
String cascadeName2;
String nestedCascadeName = "../../data/haarcascades/haarcascade_eye_tree_eyeglasses.xml";
Mat gray, hsv_img, gauss, dst,can_edge, mask_img, y_frame, w_frame, yw_frame, new_frame,hls_img,b_frame, and_frame;
Point close, close2, close3, close4;
double close_x1, close_y1, close_x2, close_y2 = 0;
double close_x3, close_y3, close_x4, close_y4 = 0;

CvCapture* capture = 0;
Mat frame, frameCopy, image;
const String scaleOpt = "--scale=";
size_t scaleOptLen = scaleOpt.length();
const String cascadeOpt = "--cascade=";
size_t cascadeOptLen = cascadeOpt.length();
const String nestedCascadeOpt = "--nested-cascade";
size_t nestedCascadeOptLen = nestedCascadeOpt.length();
String inputName;
CascadeClassifier cascade, cascade2, nestedCascade;
double scale = 1;

pthread_t thread1;
pthread_t thread2;
pthread_t thread3;

cpu_set_t cpuset1;
cpu_set_t cpuset2;
cpu_set_t cpuset3;
cpu_set_t cpuset4;

sem_t sem1;
sem_t sem2;
pthread_mutex_t pmutex;
pthread_mutex_t pmutex2;
pthread_mutex_t pmutex3;
int check = 0;

int main( int argc, const char** argv )
{
    //help();

    for( int i = 1; i < argc; i++ )
    {
        cout << "Processing " << i << " " <<  argv[i] << endl;
        if( check == 0 && cascadeOpt.compare( 0, cascadeOptLen, argv[i], cascadeOptLen ) == 0 )
        {
            cascadeName.assign( argv[i] + cascadeOptLen );
            cout << "  from which we have cascadeName= " << cascadeName << endl;
	    check = 1;
        }
	if( check == 1 && cascadeOpt.compare( 0, cascadeOptLen, argv[i], cascadeOptLen ) == 0 )
        {
            cascadeName2.assign( argv[i] + cascadeOptLen );
            cout << "  from which we have cascadeName2= " << cascadeName2 << endl;
        }
        /*else if( nestedCascadeOpt.compare( 0, nestedCascadeOptLen, argv[i], nestedCascadeOptLen ) == 0 )
        {
            if( argv[i][nestedCascadeOpt.length()] == '=' )
                nestedCascadeName.assign( argv[i] + nestedCascadeOpt.length() + 1 );
            if( !nestedCascade.load( nestedCascadeName ) )
                cerr << "WARNING: Could not load classifier cascade for nested objects" << endl;
        }
        else if( scaleOpt.compare( 0, scaleOptLen, argv[i], scaleOptLen ) == 0 )
        {
            if( !sscanf( argv[i] + scaleOpt.length(), "%lf", &scale ) || scale < 1 )
                scale = 1;
            cout << " from which we read scale = " << scale << endl;
        }*/
        else if( argv[i][0] == '-' )
        {
            cerr << "WARNING: Unknown option %s" << argv[i] << endl;
        }
        else
            inputName.assign( argv[i] );
    }

    if( !cascade.load( cascadeName ) )
    {
        cerr << "ERROR: Could not load classifier cascade" << endl;
        cerr << "Usage: facedetect [--cascade=<cascade_path>]\n"
            "   [--nested-cascade[=nested_cascade_path]]\n"
            "   [--scale[=<image scale>\n"
            "   [filename|camera_index]\n" << endl ;
        return -1;
    }
    if( !cascade2.load( cascadeName2 ) )
    {
        cerr << "ERROR: Could not load classifier cascade" << endl;
        cerr << "Usage: facedetect [--cascade=<cascade_path>]\n"
            "   [--nested-cascade[=nested_cascade_path]]\n"
            "   [--scale[=<image scale>\n"
            "   [filename|camera_index]\n" << endl ;
        return -1;
    }

    if( inputName.empty() || (isdigit(inputName.c_str()[0]) && inputName.c_str()[1] == '\0') )
    {
        capture = cvCaptureFromCAM( inputName.empty() ? 0 : inputName.c_str()[0] - '0' );
        int c = inputName.empty() ? 0 : inputName.c_str()[0] - '0' ;
        if(!capture) cout << "Capture from CAM " <<  c << " didn't work" << endl;
    }
    else if( inputName.size() )
    {
        image = imread( inputName, 1 );
        if( image.empty() )
        {
            capture = cvCaptureFromAVI( inputName.c_str() );
            if(!capture) cout << "Capture from AVI didn't work" << endl;
        }
    }
    else
    {
        image = imread( "lena.jpg", 1 );
        if(image.empty()) cout << "Couldn't read lena.jpg" << endl;
    }
    cout << "before named window" << endl;
    cvNamedWindow( "result", 1 );
    cout << "after" << endl;

    if( capture )
    {
        cout << "In capture ..." << endl;
	pthread_create(&thread1, NULL, &first_thread_func, NULL);
	pthread_create(&thread2, NULL, &second_thread_func, NULL);
	pthread_create(&thread3, NULL, &third_thread_func, NULL);

	CPU_SET(0, &cpuset1);
	CPU_SET(1, &cpuset2);
	CPU_SET(2, &cpuset3);
	CPU_SET(3, &cpuset4);

	pthread_setaffinity_np(thread1, sizeof(cpuset1), &cpuset1);
	pthread_setaffinity_np(thread2, sizeof(cpuset2), &cpuset2);
	pthread_setaffinity_np(thread3, sizeof(cpuset3), &cpuset3);
        /*for(;;)
        {
            IplImage* iplImg = cvQueryFrame( capture );
            frame = iplImg;
            if( frame.empty() )
                break;
            if( iplImg->origin == IPL_ORIGIN_TL )
                frame.copyTo( frameCopy );
            else
                flip( frame, frameCopy, 0 );

	    
            detectAndDraw( frameCopy, cascade, nestedCascade, scale );
	    lane_detect(frameCopy);

            if( waitKey( 10 ) >= 0 )
                goto _cleanup_;
        }

        waitKey(0);

_cleanup_:
        cvReleaseCapture( &capture );*/
    }
    else
    {
        cout << "In image read" << endl;
        if( !image.empty() )
        {
	  cout << "Before detectanddraw" << endl;
            detectAndDraw( image, cascade, nestedCascade, scale );
	    detectAndDraw( image, cascade2, nestedCascade, scale );
	    lane_detect(image);
	    cout << "after" << endl;
            waitKey(0);
        }
        else if( !inputName.empty() )
        {
            /* assume it is a text file containing the
            list of the image filenames to be processed - one per line */
            FILE* f = fopen( inputName.c_str(), "rt" );
            if( f )
            {
                char buf[1000+1];
                while( fgets( buf, 1000, f ) )
                {
                    int len = (int)strlen(buf), c;
                    while( len > 0 && isspace(buf[len-1]) )
                        len--;
                    buf[len] = '\0';
                    cout << "file " << buf << endl;
                    image = imread( buf, 1 );
                    if( !image.empty() )
                    {
                        detectAndDraw( image, cascade, nestedCascade, scale );
                        c = waitKey(0);
                        if( c == 27 || c == 'q' || c == 'Q' )
                            break;
                    }
                    else
                    {
                    	cerr << "Aw snap, couldn't read image " << buf << endl;
                    }
                }
                fclose(f);
            }
        }
    }
    pthread_exit(&thread1);
    pthread_exit(&thread2);
    pthread_exit(&thread3);
    cvDestroyWindow("result");

    return 0;
}

void * first_thread_func(void * data)
{
	for(;;)
        {
	    pthread_mutex_lock(&pmutex);
            IplImage* iplImg = cvQueryFrame( capture );
            frame = iplImg;
	    pthread_mutex_unlock(&pmutex);
            if( frame.empty() )
                break;
            if( iplImg->origin == IPL_ORIGIN_TL )
                frame.copyTo( frameCopy );
            else
                flip( frame, frameCopy, 0 );

	    
            frameCopy = detectAndDraw( frameCopy, cascade, nestedCascade, scale );
	    //lane_detect(frameCopy);

	    	cv::imshow( "result", frameCopy );
	    sem_post(&sem1);

            if( waitKey( 10 ) >= 0 )
                goto _cleanup_;
        }

        waitKey(0);

_cleanup_:
        cvReleaseCapture( &capture );
}

void * second_thread_func(void * data)
{
	for(;;)
        {
	    /*pthread_mutex_lock(&pmutex);
            IplImage* iplImg = cvQueryFrame( capture );
            frame = iplImg;
	    pthread_mutex_unlock(&pmutex);
            if( frame.empty() )
                break;
            if( iplImg->origin == IPL_ORIGIN_TL )
                frame.copyTo( frameCopy );
            else
                flip( frame, frameCopy, 0 );*/

	    
            //detectAndDraw( frameCopy, cascade, nestedCascade, scale );

	    sem_wait(&sem1);
	    pthread_mutex_lock(&pmutex2);
	    frameCopy = lane_detect(frameCopy);
	    pthread_mutex_unlock(&pmutex2);
	    sem_post(&sem2);

            if( waitKey( 10 ) >= 0 )
                goto _cleanup_;
        }

        waitKey(0);

_cleanup_:
        cvReleaseCapture( &capture );
}

void * third_thread_func(void * data)
{
	for(;;)
        {
	    /*pthread_mutex_lock(&pmutex);
            IplImage* iplImg = cvQueryFrame( capture );
            frame = iplImg;
	    pthread_mutex_unlock(&pmutex);
            if( frame.empty() )
                break;
            if( iplImg->origin == IPL_ORIGIN_TL )
                frame.copyTo( frameCopy );
            else
                flip( frame, frameCopy, 0 );*/

	    
            //detectAndDraw( frameCopy, cascade, nestedCascade, scale );

	    sem_wait(&sem2);
	    pthread_mutex_lock(&pmutex3);
	    detectAndDraw( frameCopy, cascade2, nestedCascade, scale );
	    pthread_mutex_unlock(&pmutex3);
            if( waitKey( 10 ) >= 0 )
                goto _cleanup_;
        }

        waitKey(0);

_cleanup_:
        cvReleaseCapture( &capture );
}

Mat lane_detect(Mat& frame)
{

	cvtColor(frame, gray, CV_BGR2GRAY);
	cvtColor(frame, hsv_img, CV_BGR2HSV);
	cvtColor(frame, hls_img, CV_BGR2HLS);
	inRange(gray, 120, 255, w_frame);
	inRange(hls_img, Scalar(0, 63, 48), Scalar(62, 74, 57), y_frame);
        bitwise_or(y_frame, w_frame, yw_frame);
        bitwise_and(gray, yw_frame, and_frame);
	//cv::imshow( "result", yw_frame );

		int rows = frame.rows;
		int rows2 = gray.rows;
		int columns = frame.cols;
		int columns2 = gray.cols;	 

		GaussianBlur(and_frame, gauss, Size(1, 1), 0, 0 );
		Canny(gauss,can_edge, 20, 60, 3);

		Mat cropped;
 		cropped = can_edge(Rect(can_edge.cols/4,3*can_edge.rows/5,2*can_edge.cols/5,can_edge.rows/5));

		Point third = Point(0, 3*rows/5+ 25);
	 	Point fourth = Point(columns, 3*rows/5+ 25);

		Point fifth = Point(0, 7*rows/10);
	 	Point sixth = Point(columns, 7*rows/10);
		
		vector<Vec4i> lines;
		HoughLinesP(cropped, lines, 1, CV_PI/180, 40, 20, 60);

		Point cent = Point(columns/2 - 15, 3*rows/5 + 25);
		Point min_left, min_right;
	 	int left_detected = 0;
		int right_detected = 0;
		
		double test_x1 = 0;
		double test_x2 = columns;
		double old_test_x1;
		double old_test_x2;

		double test_x3 = 0;
		double test_x4 = columns;
		double old_test_x3;
		double old_test_x4;

		for( size_t i = 0; i < lines.size(); i++ )
		{
		    Vec4i l = lines[i];
		    Point first = Point((l[0]+can_edge.cols/3), (l[1]+3*can_edge.rows/5));
		    Point second = Point((l[2]+can_edge.cols/3), (l[3]+3*can_edge.rows/5));

		    double slope = (second.y - first.y) / (double)(second.x - first.x);

		    if(abs(first.y - second.y) < 40)
		    {}
	 	    else
		    {
			    double a1 = second.y - first.y;
			    double b1 = first.x - second.x;
			    double c1 = a1 * (first.x) + b1 * (first.y);

			    double a2 = fourth.y - third.y;
			    double b2 = third.x - fourth.x;
			    double c2 = a2 * (third.x) + b2 * (third.y);

			    double d = a1*b2 - a2*b1;
			    double x1,y1;

			    if(d != 0)
			    {
				x1 = (b2 * c1 - b1 * c2)/d;
				y1 = (a1 * c2 - a2 * c1)/d;
			    }

			    if(x1 > 0 && x1 < 700)
			    {
				if(x1 > test_x1)
				{
					test_x1 = x1;
					close_x1 = x1;
					close_y1 = y1;
					line(frame, Point(x1- 100, y1), Point(x1 - 100, y1 +10), Scalar(0,0,0), 3, CV_AA);
					line(frame, Point((l[0]+can_edge.cols/4), (l[1]+3*can_edge.rows/5)), Point((l[2]+can_edge.cols/4), (l[3]+3*can_edge.rows/5)), Scalar(0,0,255), 3, CV_AA);
				}
			    }

			    if(x1 > 700 && x1 < columns)
			    {
				if(x1 < test_x2)
				{
					test_x2 = x1;
					close_x2 = x1;
					close_y2 = y1;
					line(frame, Point(x1- 100, y1), Point(x1 - 100, y1 +10), Scalar(0,0,0), 3, CV_AA);
					line(frame, Point((l[0]+can_edge.cols/4), (l[1]+3*can_edge.rows/5)), Point((l[2]+can_edge.cols/4), (l[3]+3*can_edge.rows/5)), Scalar(0,0,255), 3, CV_AA);
				}
			    }

			    old_test_x1 = test_x1;
			    old_test_x2 = test_x2;

			    double a3 = second.y - first.y;
			    double b3 = first.x - second.x;
			    double c3 = a3* (first.x) + b3 * (first.y);

			    double a4 = sixth.y - fifth.y;
			    double b4 = fifth.x - sixth.x;
			    double c4 = a4 * (fifth.x) + b4 * (fifth.y);

			    double d1 = a3*b4 - a4*b3;
			    double x2,y2;

			    if(d1 != 0)
			    {
				x2 = (b4 * c3 - b3 * c4)/d1;
				y2 = (a3 * c4 - a4 * c3)/d1;
			    }

			    if(x2 > 0 && x2 < 700)
			    {
				if(x2 > test_x3)
				{
					test_x3 = x2;
					close_x3 = x2;
					close_y3 = y2;
					line(frame, Point(x2- 100, y2), Point(x2 - 100, y2 +10), Scalar(0,0,0), 3, CV_AA);
					//line(frame, Point((l[0]+can_edge.cols/4), (l[1]+3*can_edge.rows/5)), Point((l[2]+can_edge.cols/4), (l[3]+3*can_edge.rows/5)), Scalar(0,0,255), 3, CV_AA);
				}
			    }

			    if(x2 > 700 && x2 < columns)
			    {
				if(x2 < test_x4)
				{
					test_x4 = x2;
					close_x4 = x2;
					close_y4 = y2;
					line(frame, Point(x2- 100, y2), Point(x2 - 100, y2 +10), Scalar(0,0,0), 3, CV_AA);
					//line(frame, Point((l[0]+can_edge.cols/4), (l[1]+3*can_edge.rows/5)), Point((l[2]+can_edge.cols/4), (l[3]+3*can_edge.rows/5)), Scalar(0,0,255), 3, CV_AA);
				}
			    }

			    old_test_x3 = test_x3;
			    old_test_x4 = test_x4;
			}
		
		}
		test_x1 = 0;
		close = Point(close_x1, close_y1);
		close.x = close.x - 100;
		circle(frame, close, 50, Scalar(0,0,255),0);

		test_x2 = 0;
		close2 = Point(close_x2, close_y2);
		close2.x = close2.x - 100;
		circle(frame, close2, 50, Scalar(0,0,255),0);

		test_x3 = 0;
		close3 = Point(close_x3, close_y3);
		close3.x = close3.x - 100;
		circle(frame, close3, 50, Scalar(0,0,255),0);

		test_x4 = 0;
		close4 = Point(close_x4, close_y4);
		close4.x = close4.x - 100;
		circle(frame, close4, 50, Scalar(0,0,255),0);

		Point midpoint;
		Point midpoint2;

		if(close.x != -100 && close2.x != -100)
			midpoint = Point((close.x + close2.x)/2, 3*rows/5+ 25);
		else
			midpoint = Point(0, 0);

		if(close3.x != -100 && close4.x != -100)
			midpoint2 = Point((close3.x + close4.x)/2, 3*rows/5+ 25);
		else if(close2.x == -100 && close4.x == -100)
		{
			midpoint = Point(close2.x - columns/7, 3*rows/5 + 15);
			midpoint2 = Point(close4.x - columns/7, 3*rows/5 + 15); 
		}
		else
			midpoint2 = Point(0, 0);
			

		/*if(midpoint.x > (columns/2))
			putText(frame,"Turn left", cvPoint(100,50),FONT_HERSHEY_COMPLEX_SMALL, 2 ,cvScalar(255, 0, 0));
		else if(midpoint.x > 0 && midpoint.x < (columns/2 - 20))
			putText(frame, "Turn right",cvPoint(900,50),FONT_HERSHEY_COMPLEX_SMALL, 2,cvScalar(255, 0, 0));
		else if(midpoint.x == 0 && midpoint.y == 0)
			putText(frame, "Stay on course",cvPoint(500,50),FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(255, 0, 0));
		else
			putText(frame, "Stay on course",cvPoint(500,50),FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(255, 0, 0));*/

		if(midpoint.x > midpoint2.x + 10 && midpoint.x < midpoint2.x + 20)
			putText(frame,"Turn right by 10 degree", cvPoint(800,50),FONT_HERSHEY_COMPLEX_SMALL, 2 ,cvScalar(255, 0, 0));
		else if(midpoint.x > midpoint2.x + 10 && midpoint.x < midpoint2.x + 30)
			putText(frame,"Turn right by 20 degree", cvPoint(800,50),FONT_HERSHEY_COMPLEX_SMALL, 2 ,cvScalar(255, 0, 0));
		else if(midpoint.x > midpoint2.x + 10 && midpoint.x < midpoint2.x + 40)
			putText(frame,"Turn right by 30 degree", cvPoint(800,50),FONT_HERSHEY_COMPLEX_SMALL, 2 ,cvScalar(255, 0, 0));
		else if(midpoint.x > midpoint2.x + 10 && midpoint.x < midpoint2.x + 50)
			putText(frame,"Turn right by 40 degree", cvPoint(800,50),FONT_HERSHEY_COMPLEX_SMALL, 2 ,cvScalar(255, 0, 0));
		else if(midpoint.x > midpoint2.x + 10 && midpoint.x < midpoint2.x + 60)
			putText(frame,"Turn right by 50 degree", cvPoint(800,50),FONT_HERSHEY_COMPLEX_SMALL, 2 ,cvScalar(255, 0, 0));
		else if(midpoint.x > midpoint2.x + 10 && midpoint.x < midpoint2.x + 70)
			putText(frame,"Turn right by 60 degree", cvPoint(800,50),FONT_HERSHEY_COMPLEX_SMALL, 2 ,cvScalar(255, 0, 0));
		else if(midpoint.x > 0 && midpoint.x < midpoint2.x - 10 && midpoint.x > midpoint2.x -20)
			putText(frame, "Turn left by 10 degree",cvPoint(100,50),FONT_HERSHEY_COMPLEX_SMALL, 2,cvScalar(255, 0, 0));
		else if(midpoint.x > 0 && midpoint.x < midpoint2.x - 10 && midpoint.x > midpoint2.x -30)
			putText(frame, "Turn left by 20 degree",cvPoint(100,50),FONT_HERSHEY_COMPLEX_SMALL, 2,cvScalar(255, 0, 0));
		else if(midpoint.x > 0 && midpoint.x < midpoint2.x - 10 && midpoint.x > midpoint2.x -40)
			putText(frame, "Turn left by 30 degree",cvPoint(100,50),FONT_HERSHEY_COMPLEX_SMALL, 2,cvScalar(255, 0, 0));
		else if(midpoint.x > 0 && midpoint.x < midpoint2.x - 10 && midpoint.x > midpoint2.x -50)
			putText(frame, "Turn left by 40 degree",cvPoint(100,50),FONT_HERSHEY_COMPLEX_SMALL, 2,cvScalar(255, 0, 0));
		else if(midpoint.x > 0 && midpoint.x < midpoint2.x - 10 && midpoint.x > midpoint2.x -60)
			putText(frame, "Turn left by 50 degree",cvPoint(100,50),FONT_HERSHEY_COMPLEX_SMALL, 2,cvScalar(255, 0, 0));
		else if(midpoint.x > 0 && midpoint.x < midpoint2.x - 10 && midpoint.x > midpoint2.x -70)
			putText(frame, "Turn left by 60 degree",cvPoint(100,50),FONT_HERSHEY_COMPLEX_SMALL, 2,cvScalar(255, 0, 0));
		else if(midpoint.x == 0 && midpoint.y == 0)
			putText(frame, "Stay on course",cvPoint(500,50),FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(255, 0, 0));
		else
			putText(frame, "Stay on course",cvPoint(500,50),FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(255, 0, 0));	

		//line(frame, Point(columns/2-10, 3*rows/5+ 20), Point(columns/2-10, 3*rows/5 + 30), Scalar(0, 255, 255), 2, CV_AA);
		line(frame, Point(0, 3*rows/5+ 25), Point(columns, 3*rows/5+ 25), Scalar(255,255,0), 1, CV_AA);
		line(frame, Point(0, 7*rows/10), Point(columns, 7*rows/10), Scalar(255,255,0), 1, CV_AA);

		if(close2.x == -100 || close2.y == 0)
		{
			line(frame, Point(close.x + rows/7, 3*rows/5+15), Point(close.x + rows/7, 3*rows/5+35), Scalar(255,120,255), 4, CV_AA);
		}
		else if(close.x == -100 || close.y == 0)
		{
			line(frame, Point(close.x - rows/7, 3*rows/5+15), Point(close.x - rows/7, 3*rows/5+35), Scalar(255,120,255), 4, CV_AA);	
		}
		else
		line(frame, Point(midpoint.x, 3*rows/5+15), Point(midpoint.x, 3*rows/5+35), Scalar(255,120,255), 4, CV_AA);

		if(close4.x == -100 || close4.y == 0)
		{
			line(frame, Point(close3.x + rows/7, 7*rows/10+5), Point(close3.x + rows/7, 7*rows/10+25), Scalar(255,120,255), 4, CV_AA);
		}
		else if(close3.x == -100 || close3.y == 0)
		{
			line(frame, Point(close3.x - rows/7, 7*rows/10+5), Point(close3.x - rows/7, 7*rows/10+25), Scalar(255,120,255), 4, CV_AA);	
		}
		else
		line(frame, Point(midpoint2.x, 7*rows/10 + 10), Point(midpoint2.x, 7*rows/10-10), Scalar(255,180,255), 4, CV_AA);
		
		
		//imshow("source", frame);

		close_x1 = close_y1 = close_x2 = close_y2 = 0;
		close_x3 = close_y3 = close_x4 = close_y4 = 0;
	 	return frame;
}

Mat detectAndDraw( Mat& img,
                   CascadeClassifier& cascade, CascadeClassifier& nestedCascade,
                   double scale)
{
    static int count = 0;
    int i = 0;
    double t = 0;
    vector<Rect> faces;
    const static Scalar colors[] =  { CV_RGB(0,0,255),
        CV_RGB(0,128,255),
        CV_RGB(0,255,255),
        CV_RGB(0,255,0),
        CV_RGB(255,128,0),
        CV_RGB(255,255,0),
        CV_RGB(255,0,0),
        CV_RGB(255,0,255)} ;
    Mat gray, smallImg( cvRound (img.rows/scale), cvRound(img.cols/scale), CV_8UC1 );
    //resize( img, img, Size(), 0.4, 0.4, INTER_LINEAR );
    cvtColor( img, gray, CV_BGR2GRAY );

    resize( gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR );
    equalizeHist( smallImg, smallImg );

    t = (double)cvGetTickCount();
    cout << "bf detect multi" << endl;
    cascade.detectMultiScale( smallImg, faces,
        1.1,25, 3
        //|CV6HAAR_FIND_BIGGEST_OBJECT
        //|CV_HAAR_DO_ROUGH_SEARCH
        |CV_HAAR_SCALE_IMAGE
        ,
        Size(30, 30) );
    cout << "after" << endl;
    t = (double)cvGetTickCount() - t;
    printf( "detection time = %g ms\n", t/((double)cvGetTickFrequency()*1000.) );
    cout << "Count:" << count << endl;
    for( vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++, i++ )
    {
        Mat smallImgROI;
        vector<Rect> nestedObjects;
        Point center;
        Scalar color = colors[i%8];
        int radius;
        center.x = cvRound((r->x + r->width*0.5)*scale);
        center.y = cvRound((r->y + r->height*0.5)*scale);
        radius = cvRound((r->width + r->height)*0.25*scale);
	//cout << " Center is " << center.x << endl;
	//cout << " actual center of image is " <<   img.cols/2 << endl;
	//if(center.x > 2*img.cols/5 && center.y < 4*img.rows/5)
	{ 
		//if(color[0] == 255 && color[1] == 0 && color[2] == 0 && color[3] == 0)
		if(radius < 140)
		{
			circle( img, center, radius, color, 3, 8, 0 );
			count++;
			cout << color << endl;
			
		}
	}

	
        /*if( nestedCascade.empty())
            continue;*/
        /*smallImgROI = smallImg(*r);
        nestedCascade.detectMultiScale( smallImgROI, nestedObjects,
            1.1, 2, 0
            //|CV_HAAR_FIND_BIGGEST_OBJECT
            //|CV_HAAR_DO_ROUGH_SEARCH
            //|CV_HAAR_DO_CANNY_PRUNING
            |CV_HAAR_SCALE_IMAGE
            ,
            Size(30, 30) );*/
    }
    cv::imshow( "result", img );
    
    frameCopy = img;
    return img;
}
