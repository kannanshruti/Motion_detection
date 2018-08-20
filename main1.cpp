#include "motion_detection.h"
#include "stdafx.h"

using namespace std;
using namespace cv;

int main()
{
	Mat img1 = imread("./Images/missa_1.tif", 0);
	Mat img2 = imread("./Images/missa_50.tif", 0);
	float theta = 1;
	float sigma_s = 1.22;
	float T = 2;

	motion_detection md(theta, sigma_s, T);

	// Absolute difference
	Mat abs_diff = md.abs_difference(img1, img2);
	namedWindow("Absolute difference", WINDOW_AUTOSIZE);
	imshow("Absolute difference", abs_diff);

	// Fixed threshold hypothesis
	Mat res_fixed = md.fixed_threshold(img1, img2);
	namedWindow("Fixed threshold", WINDOW_AUTOSIZE);
	imshow("Fixed threshold", res_fixed);

	// Variable threshold - 1st order MRF
	Mat res_var1 = md.variable_threshold1(img1, img2);
	namedWindow("Variable threshold- MRF1", WINDOW_AUTOSIZE);
	imshow("Variable threshold- MRF1", res_var1);

	// Variable threshold - 2nd order MRF
	Mat res_var2 = md.variable_threshold2(img1, img2);
	namedWindow("Variable threshold- MRF2", WINDOW_AUTOSIZE);
	imshow("Variable threshold- MRF2", res_var2);

	waitKey(0);
	return 0;
}