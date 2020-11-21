#include "Utilities.h"
#include <iostream>
#include <fstream>
#include <list>
#include <experimental/filesystem> // C++-standard header file name
#include <filesystem> // Microsoft-specific implementation header file name
using namespace std::experimental::filesystem::v1;
using namespace std;

#define NUMBER_OF_POSTBOXES 6
int PostboxLocations[NUMBER_OF_POSTBOXES][8] = {
	{ 26, 113, 106, 113, 13, 133, 107, 134 },
	{ 119, 115, 199, 115, 119, 135, 210, 136 },
	{ 30, 218, 108, 218, 18, 255, 109, 254 },
	{ 119, 217, 194, 217, 118, 253, 207, 253 },
	{ 32, 317, 106, 315, 22, 365, 108, 363 },
	{ 119, 315, 191, 314, 118, 362, 202, 361 } };
#define POSTBOX_TOP_LEFT_COLUMN 0
#define POSTBOX_TOP_LEFT_ROW 1
#define POSTBOX_TOP_RIGHT_COLUMN 2
#define POSTBOX_TOP_RIGHT_ROW 3
#define POSTBOX_BOTTOM_LEFT_COLUMN 4
#define POSTBOX_BOTTOM_LEFT_ROW 5
#define POSTBOX_BOTTOM_RIGHT_COLUMN 6
#define POSTBOX_BOTTOM_RIGHT_ROW 7


void MyApplication(VideoCapture& video)
{
	if (video.isOpened())
	{
		bool post_in_box[6];
		bool obscured;

		Mat current_frame, thresholded_image, current_frame_gray, first_frame;
		Mat croppedImage, greyImage, thresholdImage, labeledImage;
		video.set(cv::CAP_PROP_POS_FRAMES, 0);
		video >> current_frame;
		first_frame = current_frame;
		Mat final = Mat::zeros(current_frame.size(), CV_8UC3);

		Mat mask = Mat::zeros(current_frame.size(), CV_8UC1);
		for (int i = 0; i < NUMBER_OF_POSTBOXES; i++) {
			mask(Rect(PostboxLocations[i][POSTBOX_TOP_LEFT_COLUMN], PostboxLocations[i][POSTBOX_TOP_LEFT_ROW],
				PostboxLocations[i][POSTBOX_BOTTOM_RIGHT_COLUMN] - PostboxLocations[i][POSTBOX_TOP_LEFT_COLUMN], PostboxLocations[i][POSTBOX_BOTTOM_RIGHT_ROW] - PostboxLocations[i][POSTBOX_TOP_LEFT_ROW])) = 255;
		}

		bitwise_not(mask, mask);
		double frame_rate = video.get(cv::CAP_PROP_FPS);
		Mat hsv;
		cvtColor(first_frame, hsv, COLOR_BGR2HSV);
		Mat first_frame_hist;
		int hbins = 30, sbins = 32;
		int histSize[] = { hbins, sbins };
		// hue varies from 0 to 179, see cvtColor
		float hranges[] = { 0, 180 };
		// saturation varies from 0 (black-gray-white) to
		// 255 (pure spectrum color)
		float sranges[] = { 0, 256 };
		const float* ranges[] = { hranges, sranges };
		int channels[] = { 0, 1 };
		//calcHist(&hsv, {0,1}, Mat(), hist, histSize, ranges, true, false);
		calcHist(&hsv, 1, channels, mask, // do not use mask
			first_frame_hist, 2, histSize, ranges,
			true, // the histogram is uniform
			false);

		double time_between_frames = 1000.0 / frame_rate;
		//Timestamper* timer = new Timestamper();
		//int frame_count = 0;

		while ((!current_frame.empty())) {
			//cvtColor(current_frame, current_frame_gray, COLOR_BGR2GRAY);
			//threshold(current_frame_gray, thresholded_image, 90, 255, THRESH_BINARY);

			cvtColor(current_frame, hsv, COLOR_BGR2HSV);

			obscured = false;
			Mat hist;
			calcHist(&hsv, 1, channels, mask, // do not use mask
				hist, 2, histSize, ranges,
				true, // the histogram is uniform
				false);

			double hist_correlation = compareHist(first_frame_hist,
				hist,
				HISTCMP_CORREL
			);

			for (int i = 0; i < NUMBER_OF_POSTBOXES; i++) {
				post_in_box[i] = true;

				Mat ROI(current_frame, Rect(PostboxLocations[i][POSTBOX_TOP_LEFT_COLUMN], PostboxLocations[i][POSTBOX_TOP_LEFT_ROW],
					PostboxLocations[i][POSTBOX_BOTTOM_RIGHT_COLUMN] - PostboxLocations[i][POSTBOX_TOP_LEFT_COLUMN], PostboxLocations[i][POSTBOX_BOTTOM_RIGHT_ROW] - PostboxLocations[i][POSTBOX_TOP_LEFT_ROW]));

				ROI.copyTo(croppedImage);

				cvtColor(croppedImage, greyImage, COLOR_BGR2GRAY);
				//threshold(greyImage, thresholded_image, 90, 255, THRESH_BINARY);
				GaussianBlur(greyImage, greyImage, Size(3, 3), 0, 0, BORDER_DEFAULT);
				threshold(greyImage, thresholdImage, 0, 255, THRESH_BINARY | THRESH_OTSU);
				// Copy the data into new matrix


				int nRegions = connectedComponents(thresholdImage, labeledImage, 8);
				printf("%i:\t%i\n", i, nRegions);
				printf("post in %i:  %i\n", i, post_in_box[i]);

				if (nRegions == 6 || nRegions == 7) {
					post_in_box[i] = false;
				}

			}


			char obscure_str[100] = { 0 };
			sprintf_s(obscure_str, "Correlation:  %f", hist_correlation);
			putText(current_frame, obscure_str, Point(0,20), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(255, 255, 0));
			if (hist_correlation < 0.8) {
				putText(current_frame, "OBSCURED", Point(0, 10), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255));
			}

			for (int i = 0; i < NUMBER_OF_POSTBOXES; i++) {

				char restult_str[100] = { 0 };
				sprintf_s(restult_str, "post in %i:  %i", i, post_in_box[i]);
				putText(current_frame, restult_str, Point(PostboxLocations[i][0], PostboxLocations[i][1]), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(255, 255, 0));

			}

			bitwise_and(current_frame, current_frame, final, mask);
			imshow("Mask", final);
			imshow("Original", current_frame);
			//imshow("Threshold", thresholded_image);
			//imshow("Otsu", otsu_thresholded_image);
			video >> current_frame;

			char c = (char)waitKey(time_between_frames);
			if (c == 27) break;

		}

		video.release();

		cv::destroyAllWindows();
	}
}


