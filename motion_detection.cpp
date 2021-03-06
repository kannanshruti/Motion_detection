/*
   @file    motion_detection.cpp
   @author  Shruti Kannan

   @brief Motion detection

   @section DESCRIPTION
	Accepts 2 consecutive frames from a video and returns
	an image representing the movement between the input 
	frames.

   @section EXPLANATION
	Assuming a Gaussian distribution for both stationary and moving pixels,
	the Binary hypothesis test simplifies to :
	psi[x0] ^ 2 >< (2 * sig_s ^ 2) * (ln(theta * sig_m / sig_s) + (Qs - Qm) / T)
	(Assumption: theta = 1, sig_m / sig_s = 5, T->inf)
	1. Fixed threshold hypothesis: T -> inf
	2. Variable threshold MRF order 1: 1st order neighbourhood for Qs and Qm
	3. Variable threshold MRF order 2: 2nd order neighbourhood for Qs and Qm
*/

#include "motion_detection.h"
#include "stdafx.h"

typedef unsigned char uchar;
using namespace std;
using namespace cv;

motion_detection::motion_detection(float theta, float sigma_s, float T) {
	this -> theta = theta;
	this -> sigma_s = sigma_s;
	this -> T = T;
}

Mat motion_detection::abs_difference(Mat img1, Mat img2) {
	/*
	Description: Computes the absolute difference between 2 frames. 
	@param "img1": Frame at time t_k
	@param "img2": Frame at time t_k+1
	@return "result": Image with motion detected areas marked in non-black
	*/
	if (img1.size != img2.size) throw invalid_argument("Images should be of same size");
	Mat result = Mat::zeros(Size(img1.cols, img1.rows), CV_8UC1);
	absdiff(img2, img1, result);
	return result;
}

Mat motion_detection::fixed_threshold(Mat img1, Mat img2) {
	/*	
	Description: Computes the fixed threshold hypothesis for 2 frames
	@param "img1": Frame at time t_k
	@param "img2": Frame at time t_k+1
	@return "result": Image with motion detected areas containing white pixels
	*/
	double psi, sigma_m, threshold;
	sigma_m = 5 * sigma_s;	
	if (img1.size != img2.size) throw invalid_argument("Images should be of same size");
	Mat result = Mat::zeros(Size(img1.cols, img1.rows), CV_8UC1);
	result = abs_difference(img1, img2);
	threshold = 2.0 * pow(sigma_s, 2) * (log(theta * sigma_m / sigma_s));
	cout << "Threshold: " << threshold << "\n";
	for (int i = 0; i < img1.rows; i++) {
		for (int j = 0; j < img1.cols; j++) {
			psi = pow(result.at<uchar>(i, j),2);
			if (psi > threshold) result.at<uchar>(i, j) = 255;
			else result.at<uchar>(i, j) = 0;
		}
	}
	return result;
}

neighbours motion_detection::get_neighbour_count(Mat img, int row, int col, int num_neighbours) {
	/*
	Description: Computes the number of 1st or 2nd order neighbours for an image. A neighbour is
		classified as 'Static' or 'Moving' depending on whether it is background (defined by intensity
		0) or not.
	@param "img": Image whose neighbour status is to be determined
	@param "row", "col": Coordinate in "img" whose #neighbours are to be computed
	@param "num_neighbours": Order of neighbours ("4" or "8")
	@return "n1": Member of struct 'neighbours', holding the count values for "Qs" (number of static
		neighbours) and "Qm" (number of moving neighbours) 
	*/
	neighbours n1;
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			if (i == 0 && j == 0) continue;
			if (row + i < 0 || row + i >= img.rows || col + j < 0 || col + j >= img.cols)
				continue;
			if (num_neighbours == 4 && abs(i+j) == 1) {
				if (img.at<uchar>(row+i, col+j) != 0) n1.Qm += 1;
				else n1.Qs += 1;
			}
			if (num_neighbours == 8) {
				if (img.at<uchar>(row+i, col+j) != 0) n1.Qm += 1;
				else n1.Qs += 1;
			}
		}
	}
	return n1;
}

Mat motion_detection::variable_threshold1(Mat img1, Mat img2, int iterations) {
	/*
	Description: Computes motion detection using an adaptive threshold (MRF of order1, 4 neighbours)
	@param "img1": Frame at time t_k
	@param "img2": Frame at time t_k+1
	@param "iterations": Number of iterations over which the threshold is to be run. Default is 5.
	@return "result": Image with motion detected areas containing white pixels
	*/
	double psi=0, sigma_m=0, thresh=0;
	int qs_qm = 0;
	sigma_m = 5 * sigma_s;
	
	if (img1.size != img2.size) throw invalid_argument("Images should be of same size");
	Mat result = Mat::zeros(Size(img1.cols, img1.rows), CV_8UC1);
	Mat abs_diff = 	abs_difference(img1, img2);
	result = abs_diff.clone();
	for (int iter = 0; iter < iterations; iter++) {
		for (int i = 0; i < img1.rows; i++) {
			for (int j = 0; j < img1.cols; j++) {
				neighbours n1 = get_neighbour_count(result, i, j, 4);
				thresh = 2.0 * pow(sigma_s, 2) * (log(theta * sigma_m / sigma_s) + ((n1.Qs - n1.Qm) / T));
				psi = pow(abs_diff.at<uchar>(i, j), 2);
				if (psi > thresh) result.at<uchar>(i, j) = 255;
				else result.at<uchar>(i, j) = 0;				
			}
		}
	}
	return result;
}

Mat motion_detection::variable_threshold2(Mat img1, Mat img2, int iterations) {
	/*
	Description: Computes motion detection using an adaptive threshold (MRF of order2, 8 neighbours)
	@param "img1": Frame at time t_k
	@param "img2": Frame at time t_k+1
	@param "iterations": Number of iterations over which the threshold is to be run. Default is 5.
	@return "result": Image with motion detected areas containing white pixels
	*/
	double psi = 0, sigma_m = 0, thresh = 0;
	int qs_qm = 0;
	sigma_m = 5 * sigma_s;

	if (img1.size != img2.size) throw invalid_argument("Images should be of same size");
	Mat result = Mat::zeros(Size(img1.cols, img1.rows), CV_8UC1);
	Mat abs_diff = abs_difference(img1, img2);
	result = abs_diff.clone();
	for (int iter = 0; iter < iterations; iter++) {
		for (int i = 0; i < img1.rows; i++) {
			for (int j = 0; j < img1.cols; j++) {
				neighbours n1 = get_neighbour_count(result, i, j, 8);
				thresh = 2.0 * pow(sigma_s, 2) * (log(theta * sigma_m / sigma_s) + ((n1.Qs - n1.Qm) / T));
				psi = pow(abs_diff.at<uchar>(i, j), 2);
				if (psi > thresh) result.at<uchar>(i, j) = 255;
				else result.at<uchar>(i, j) = 0;
			}
		}
	}
	return result;
}

/*
NOTES/ LEARNING:
1. Size Mat::size() const The method returns a matrix size: (cols, rows) and not (rows, cols)
2. Mat::at() if a Mat is defined as CV_8UC1 or something, type in 'at' should be unsigned char and
	not unsigned int. Should be same, otherwise it will throw an error.
3. The default value for an member function's argument can either go in declaration or definition but not both.
*/