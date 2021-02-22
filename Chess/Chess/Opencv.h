#pragma once
#ifndef opencv_h_
#define opencv_h_

#include <opencv2/opencv.hpp>
#include "pch.h"


void opencv_test();
void findTriangle(cv::Mat& img,cv:: Mat& result);
extern char userMovePath[4];
extern unsigned char chessBoardNew[10][9];
extern bool opencvRun;
extern int redChessNum;
extern int blackChessNum;;

#endif

