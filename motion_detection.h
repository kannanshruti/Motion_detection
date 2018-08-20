#include "stdafx.h"
struct neighbours {
	int Qs = 0;
	int Qm = 0;
};

class motion_detection {
	/*	Description:	Implements techniques for motion detection between 2 frames
	Example:		motion_detection md(theta, sigma_s, T)
	md.abs_difference(frame1, frame2)
	md.fixed_threshold(frame1, frame2)
	md.variable_threshold1(frame1, frame2, iterations)
	md.variable_threshold2(frame1, frame2, iterations)
	*/
public:
	float theta, sigma_s, T;
	motion_detection(float theta, float sigma_s, float T);
	Mat abs_difference(Mat img1, Mat img2);
	Mat fixed_threshold(Mat img1, Mat img2);
	Mat variable_threshold1(Mat img1, Mat img2, int iterations = 5);
	Mat variable_threshold2(Mat img1, Mat img2, int iterations = 5);
	neighbours get_neighbour_count(Mat abs_diff, int row, int col, int num_neighbours);
};