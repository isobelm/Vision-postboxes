/*
 * This code is provided as part of "A Practical Introduction to Computer Vision with OpenCV"
 * by Kenneth Dawson-Howe © Wiley & Sons Inc. 2014.  All rights reserved.
 */
#include "Utilities.h"
#include <iostream>
#include <fstream>
#include <list>

VideoCapture* postbox_video;


int main(int argc, const char** argv)
{
	Mat* image = new Mat();
	string filename_img("Media/home.jpg");
	*image = imread(filename_img, -1);
	if (image->empty())
	{
		cout << "Could not open " << filename_img << endl;
		return -1;
	}

	const char* video_file = "Media/PostboxesWithLines.avi";
	VideoCapture* postbox_video = new VideoCapture;
	string filename_vid(video_file);
	postbox_video->open(filename_vid);
	if (!postbox_video->isOpened())
	{
		cout << "Cannot open video file: " << filename_vid << endl;
		//			return -1;
	}

	//// Load Haar Cascade(s)
	//vector<CascadeClassifier> cascades;
	//char* cascade_files[] = {
	//	"haarcascades/haarcascade_frontalface_alt.xml" };
	//int number_of_cascades = sizeof(cascade_files) / sizeof(cascade_files[0]);
	//for (int cascade_file_no = 0; (cascade_file_no < number_of_cascades); cascade_file_no++)
	//{
	//	CascadeClassifier cascade;
	//	string filename(file_location);
	//	filename.append(cascade_files[cascade_file_no]);
	//	if (!cascade.load(filename))
	//	{
	//		cout << "Cannot load cascade file: " << filename << endl;
	//		return -1;
	//	}
	//	else cascades.push_back(cascade);
	//}

	int line_step = 13;
	Point location(7, 13);
	Scalar colour(0, 0, 0);
	Mat default_image = ComputeDefaultImage(*image);
	putText(default_image, "OpenCV demonstration system from:", location, FONT_HERSHEY_SIMPLEX, 0.4, colour);
	location.y += line_step * 3 / 2;
	putText(default_image, "m. My Application", location, FONT_HERSHEY_SIMPLEX, 0.4, colour);
	location.y += line_step;
	putText(default_image, "X. eXit", location, FONT_HERSHEY_SIMPLEX, 0.4, colour);
	//Mat imageROI;
	//imageROI = default_image(cv::Rect(0, 0, default_image.cols, 245));
	//addWeighted(imageROI, 2.5, imageROI, 0.0, 0.0, imageROI);

	int choice;
	do
	{
		imshow("Welcome", default_image);
		choice = cv::waitKey();
		cv::destroyAllWindows();
		switch (choice)
		{
		case 'm':
			MyApplication(*postbox_video);
			break;
		default:
			break;
		}
	} while ((choice != 'x') && (choice != 'X'));
}

