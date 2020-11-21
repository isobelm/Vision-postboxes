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


void write_out_frame(ofstream& output, bool obscured, bool* post_in_box, int frame_count) {
	output << frame_count << ", ";
	if (obscured) {
		output << "View obscured";

	}
	else {
		char post_list[100] = { 0 };
		for (int i = 0; i < NUMBER_OF_POSTBOXES; i++) {
			if (post_in_box[i]) {
				char tmp[100] = { 0 };
				sprintf_s(tmp, " %i", i + 1);
				strcat_s(post_list, tmp);
			}
		}
		if (post_list[0] == 0) {
			output << "No post";
		}
		else {
			output << "Post in" << post_list;
		}
	}
	output << "\n";
}

bool check_for_post(Mat current_frame, int i) {
	Mat croppedImage, greyImage, thresholdImage, labeledImage;
	Mat ROI(current_frame, Rect(PostboxLocations[i][POSTBOX_TOP_LEFT_COLUMN], PostboxLocations[i][POSTBOX_TOP_LEFT_ROW],
		PostboxLocations[i][POSTBOX_BOTTOM_RIGHT_COLUMN] - PostboxLocations[i][POSTBOX_TOP_LEFT_COLUMN], PostboxLocations[i][POSTBOX_BOTTOM_RIGHT_ROW] - PostboxLocations[i][POSTBOX_TOP_LEFT_ROW]));

	ROI.copyTo(croppedImage);

	cvtColor(croppedImage, greyImage, COLOR_BGR2GRAY);
	GaussianBlur(greyImage, greyImage, Size(3, 3), 0, 0, BORDER_DEFAULT);
	threshold(greyImage, thresholdImage, 0, 255, THRESH_BINARY | THRESH_OTSU);

	int nRegions = connectedComponents(thresholdImage, labeledImage, 8);

	if (nRegions == 6 || nRegions == 7) {
		return false;
	}
	else
		return true;
}

Mat create_histogram(Mat frame, Mat mask) {
	Mat hsv;
	cvtColor(frame, hsv, COLOR_BGR2HSV);
	Mat hist;
	int hbins = 30, sbins = 32;
	int histSize[] = { hbins, sbins };
	float hranges[] = { 0, 180 };
	float sranges[] = { 0, 256 };
	const float* ranges[] = { hranges, sranges };
	int channels[] = { 0, 1 };
	calcHist(&hsv, 1, channels, mask,
		hist, 2, histSize, ranges, true, false);
	return hist;
}

void MyApplication(VideoCapture& video)
{
	if (video.isOpened())
	{
		bool post_in_box[6];
		bool obscured;
		ofstream output;
		output.open("output.txt");
		Mat current_frame, first_frame;
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
		
		Mat first_frame_hist = create_histogram(first_frame, mask);

		double time_between_frames = 1000.0 / frame_rate;
		int frame_count = 1;

		while ((!current_frame.empty())) {
			obscured = false;

			Mat hist = create_histogram(current_frame, mask);

			double hist_correlation = compareHist(first_frame_hist,
				hist,
				HISTCMP_CORREL
			);

			for (int i = 0; i < NUMBER_OF_POSTBOXES; i++) {
				post_in_box[i] = check_for_post(current_frame, i);
			}

			bitwise_and(current_frame, current_frame, final, mask);

			char obscure_str[100] = { 0 };
			sprintf_s(obscure_str, "Correlation:  %f", hist_correlation);
			putText(current_frame, obscure_str, Point(0, 20), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(255, 255, 0));
			if (hist_correlation < 0.85) {
				putText(current_frame, "OBSCURED", Point(0, 10), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255));
				obscured = true;
			}

			for (int i = 0; i < NUMBER_OF_POSTBOXES; i++) {

				char restult_str[100] = { 0 };
				sprintf_s(restult_str, "post in %i:  %i", i, post_in_box[i]);
				putText(current_frame, restult_str, Point(PostboxLocations[i][0], PostboxLocations[i][1]), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(255, 255, 0));

			}

			imshow("Mask", final);
			imshow("Original", current_frame);
			video >> current_frame;
			write_out_frame(output, obscured, post_in_box, frame_count);
			frame_count++;

			//char c = (char)waitKey(time_between_frames);
			char c = (char)waitKey(1);
			if (c == 27) break;

		}
		output.close();


		video.release();

		cv::destroyAllWindows();
	}
}


