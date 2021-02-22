// ch.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include "Define.h"
//#include "Opencv.h"


#define PIXEL_PER_GRID 48/一个格子占50像素

using namespace std;
using namespace cv;

bool opencvRun = false;

//保存新的全局棋盘（红子和黑子信息都保存）
unsigned char chessBoardNew[10][9] = { {   B_CAR, B_HORSE, B_ELEPHANT, B_BISHOP,  B_KING, B_BISHOP, B_ELEPHANT, B_HORSE,   B_CAR },
									   { NOCHESS, NOCHESS,    NOCHESS,  NOCHESS, NOCHESS,  NOCHESS,    NOCHESS, NOCHESS, NOCHESS },
									   { NOCHESS, B_CANON,    NOCHESS,  NOCHESS, NOCHESS,  NOCHESS,    NOCHESS, B_CANON, NOCHESS },
									   {  B_PAWN, NOCHESS,     B_PAWN,  NOCHESS,  B_PAWN,  NOCHESS,     B_PAWN, NOCHESS,  B_PAWN },
									   { NOCHESS, NOCHESS,    NOCHESS,  NOCHESS, NOCHESS,  NOCHESS,    NOCHESS, NOCHESS, NOCHESS },
	//------------------楚河-----------------------------------汉界------------------//
	{ NOCHESS, NOCHESS,    NOCHESS,  NOCHESS, NOCHESS,  NOCHESS,    NOCHESS, NOCHESS, NOCHESS },
	{  R_PAWN, NOCHESS,     R_PAWN,  NOCHESS,  R_PAWN,  NOCHESS,     R_PAWN, NOCHESS,  R_PAWN },
	{ NOCHESS, R_CANON,    NOCHESS,  NOCHESS, NOCHESS,  NOCHESS,    NOCHESS, R_CANON, NOCHESS },
	{ NOCHESS, NOCHESS,    NOCHESS,  NOCHESS, NOCHESS,  NOCHESS,    NOCHESS, NOCHESS, NOCHESS },
	{   R_CAR, R_HORSE, R_ELEPHANT, R_BISHOP,  R_KING, R_BISHOP, R_ELEPHANT, R_HORSE,   R_CAR } };
unsigned char chessBoardLast[10][9];//保存上次全局棋盘

unsigned char chessBoardRTScan[10][9];//保存opencv实时扫描的棋盘，要判断是否正确后才能使用

//保存新的玩家棋盘（只保存用户棋子信息）
unsigned char userChessBoardNew[10][9] = { { NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS },
											{ NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS },
											{ NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS },
											{ NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS },
											{ NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS },
	//------------------楚河-----------------------------------汉界------------------//
	{ NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS },
	{  R_PAWN, NOCHESS,  R_PAWN, NOCHESS,  R_PAWN, NOCHESS,  R_PAWN, NOCHESS,  R_PAWN },
	{ NOCHESS, R_CANON, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, R_CANON, NOCHESS },
	{ NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS, NOCHESS },
	{   R_CAR, R_HORSE, R_ELEPHANT, R_BISHOP, R_KING, R_BISHOP, R_ELEPHANT, R_HORSE, R_CAR } };
unsigned char userChessBoardLast[10][9];////保存上次玩家棋盘

int redChessNum = 0;//棋局上剩余红色棋子数量
int blackChessNum = 0;//棋局上剩余黑色棋子数量
char userMovePath[4];//保存玩家走棋路线，在电脑上走棋
void findTrianglePlus(Mat& img, Mat& result) {
	Mat temp;
	img.copyTo(temp);
	Point2f srcPoints[4];//四个角点的位置
	Point2f dstPoints[4];//变换后的坐标位置
	//交点的位置 左上 右上 右下 左下
	dstPoints[0] = Point2f(0, 0);
	dstPoints[1] = Point2f(520, 0);
	dstPoints[2] = Point2f(520, 470);
	dstPoints[3] = Point2f(0, 470);
	srcPoints[0] = Point2f(355, 261);
	srcPoints[1] = Point2f(964, 258);
	srcPoints[2] = Point2f(909, 719);
	srcPoints[3] = Point2f(406, 719);
	Mat transMat = getPerspectiveTransform(srcPoints, dstPoints);
	warpPerspective(img, temp, transMat, Size(520, 470));
	result = temp;
}
void findTriangle(Mat& img, Mat& result) {
	Mat temp;
	img.copyTo(temp);
	Mat gaussImage;
	//高斯滤波
	GaussianBlur(img, gaussImage, Size(5, 5), 0);

	Mat grayImage;
	cvtColor(gaussImage, grayImage, CV_BGR2GRAY);
	//adaptiveThreshold(grayImage, grayImage, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 15, 6);
	imwrite("1.jpg", grayImage);
	Mat cannyImage;
	Canny(grayImage, cannyImage, 60, 180, 3);

	Mat element = getStructuringElement(MORPH_RECT, Size(15, 15));
	dilate(cannyImage, cannyImage, element);
	erode(cannyImage, cannyImage, element);

	vector< vector< Point> > contours;
	findContours(cannyImage,
		contours, // a vector of contours
		CV_RETR_EXTERNAL, // 提取外部轮廓
		CV_CHAIN_APPROX_NONE); // all pixels of each contours
//	result = grayImg;

	int max = 0;
	int index = 0;
	//这是根据最大面积来进行轮廓的筛选
	/*
	vector<Point> maxArea;
	for (int i = 0; i < contours.size(); i++)
	{
		vector<Point> p;
		p = contours[i];
		printf("%d\n", p.size());
		if (p.size() > max)
		{
			max = p.size();

			maxArea = p;
			index = i;
		}
	}
	*/
	//根据面积来进行区域分割
	vector<Point> maxArea;
	vector<Point> maxAreaTemp;
	for (int i = 0; i < contours.size(); i++)
	{
		vector<Point> p;
		p = contours[i];
		if (p.size() > 1500 &&p.size()<3000)
		{
			max = p.size();
			maxArea = p;
			index = i;
		}
	}


	/*绘制最大区域
	for (size_t i = 0; i < maxArea.size(); i++)
	{
		circle(temp, maxArea[i], 1, Scalar(240, 255, 25));
	}
	imshow("temp", temp);
	*/
	//多边形拟合凸包
	vector<Point> hull;
	vector<Point> approx;
	convexHull(maxArea, hull);
	double epsilon = 0.02 * arcLength(maxArea, true);
	approxPolyDP(hull, approx, epsilon, true);
	for (int i = 0; i < approx.size(); i++) {
		circle(temp, approx[i], 2, Scalar(255, 255, 0),5);
	}
	imwrite("tmp.jpg", temp);

	//仿形映射
	Point2f srcPoints[4];//四个角点的位置
	Point2f dstPoints[4];//变换后的坐标位置
	//交点的位置 左上 右上 右下 左下
	dstPoints[0] = Point2f(0, 0);
	dstPoints[1] = Point2f(520, 0);
	dstPoints[2] = Point2f(520,470);
	dstPoints[3] = Point2f(0, 470);
	bool sorted = false;
	int n = 4;
	while (!sorted) {
		sorted = true;
		for (int i = 1; i < n; i++) {
			if (approx[i - 1].x > approx[i].x) {
				swap(approx[i - 1], approx[i]);
				sorted = false;
			}
		}
		n--;
	}
	if (approx[0].y < approx[1].y) {
		srcPoints[0] = approx[0];
		srcPoints[3] = approx[1];
	}
	else {
		srcPoints[0] = approx[1];
		srcPoints[3] = approx[0];
	}

	if (approx[2].y < approx[3].y) {
		srcPoints[1] = approx[2];
		srcPoints[2] = approx[3];
	}
	else {
		srcPoints[1] = approx[3];
		srcPoints[2] = approx[2];
	}
	
	for (int i = 0; i < 4; ++i) {
		printf("%f,%f\n", srcPoints[i].x, srcPoints[i].y);
	}
	
	//srcPoints[0] = Point2f(356,206);
	//srcPoints[1] = Point2f(980, 184);
	//srcPoints[2] = Point2f(932, 665);
	//srcPoints[3] = Point2f(423, 670);
	Mat transMat = getPerspectiveTransform(srcPoints, dstPoints);
	warpPerspective(img, temp, transMat,Size(520,470));
	result = temp;
}


void opencv_test() {
	VideoCapture capture(1);

	Mat showImg;//最终显示的图像
	Mat sourceImg;//原始图像
	Mat rotateImg;//旋转及镜像调整后的图像
	Mat medianFilterImg;//中值滤波图像
	Mat grayImg;//灰度图
	Mat erodeImg;//腐蚀图像
	Mat element;//腐蚀膨胀结构元
	Mat erzhihua(40, 40, CV_8UC3);//40*40分辨率，保存分割后的棋子图像，用于分析每个棋子

	vector<Vec3f> pcircles;//保存霍夫圆检测到的结果
	int redChessCount = 0;//统计红色棋子数量
	int blackChessCount = 0;//统计黑色棋子数量
	int pixelNum = 0;//像素数量累计变量
	int pixelCount = 0;//像素值累计变量

	char st[10];//用于存放帧率的字符串
	double t;//计时变量，计算帧率

	capture.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
	capture.set(CV_CAP_PROP_FPS, 30);//帧数
	//打印摄像头参数
	printf("width = %.2f\n", capture.get(CV_CAP_PROP_FRAME_WIDTH));
	printf("height = %.2f\n", capture.get(CV_CAP_PROP_FRAME_HEIGHT));
	printf("fbs = %.2f\n", capture.get(CV_CAP_PROP_FPS));
	printf("brightness = %.2f\n", capture.get(CV_CAP_PROP_BRIGHTNESS));
	printf("contrast = %.2f\n", capture.get(CV_CAP_PROP_CONTRAST));
	printf("saturation = %.2f\n", capture.get(CV_CAP_PROP_SATURATION));
	printf("hue = %.2f\n", capture.get(CV_CAP_PROP_HUE));
	printf("exposure = %.2f\n", capture.get(CV_CAP_PROP_EXPOSURE));

	while (true)
	{
		t = (double)cv::getTickCount();//记录开始时间

		capture >> sourceImg;//读一帧原始图像

		//对原始图像进行仿形变换

		findTrianglePlus(sourceImg, sourceImg);
		Rect ROI(0, 0, 470, 520);//感兴趣区域，去除摄像头视野中不需要检测的范围
		Rect board0(40,40, 390, 436);//棋盘最外圈矩形框
		transpose(sourceImg, sourceImg);//镜像

		flip(sourceImg, sourceImg, 0);//旋转

		sourceImg(ROI).copyTo(rotateImg);//划定寻找象棋ROI区域，去除多余视野

		cvtColor(rotateImg, grayImg, CV_BGR2GRAY);//RGB转灰度图

		//threshold(grayImg, grayImg, 128, 255, CV_THRESH_BINARY);//二值化

		medianBlur(grayImg, medianFilterImg, 3);//中值滤波，核3
		element = getStructuringElement(MORPH_RECT, Size(2, 2), Point(1, 1));
		//dilate(grayImg, frame, element);//膨胀
		//erode(medianFilterImg, erodeImg, element);//腐蚀

		memset(chessBoardRTScan, NOCHESS, sizeof(chessBoardRTScan));//将实时扫描棋盘清空，准备接受新的棋盘数据

		//输入，输出，方法（类型）-HOUGH_GRADIENT，dp(dp=1时表示霍夫空间与输入图像空间的大小一致，dp=2时霍夫空间是输入图像空间的一半，以此类推)，最短距离-可以分辨是两个圆否 则认为是同心圆 ,边缘检测时使用Canny算子的高阈值，中心点累加器阈值—候选圆心（霍夫空间内累加和大于该阈值的点就对应于圆心），检测到圆的最小半径，检测到圆的的最大半径
		//
		HoughCircles(medianFilterImg, pcircles, CV_HOUGH_GRADIENT, 1, 20, 25, 20, 15, 25);
		bool circleFlag = (pcircles.size() > 0);
	
		if (circleFlag){
			for (int c = 0; c < pcircles.size(); c++)//遍历所有识别到的棋子
			{
				Point center((int)(pcircles[c][0] + 0.5), (int)(pcircles[c][1] + 0.5));//提取圆心坐标，四舍五入
				int radius = (int)(pcircles[c][2] + 0.5);//提取半径，四舍五入
				Rect rec(center.x - 20, center.y - 20, 40, 40);//定义一个框住棋子的方框，40*40大小

				//cv::circle(rotateImg, center, radius, Scalar(0, 255, 0), 2);

				if (!(ROI.contains(rec.tl()) && ROI.contains(rec.br()))) continue;//如果棋子过于靠近边缘，rec提取的局域图像将会超出范围，忽略这个棋子

				//////将图像转换为HSV空间分析
				//Mat src_hsv;
				//Mat V;
				//cvtColor(rotateImg  (rec), src_hsv, CV_BGR2HSV);
				//vector<Mat> hsvSplit;
				//split(src_hsv, hsvSplit);
				//equalizeHist(hsvSplit[2], hsvSplit[2]);
				//merge(hsvSplit, src_hsv);
				//inRange(src_hsv, Scalar(0, 46, 50), Scalar(180, 255,190), V); //Threshold the image
				//V.copyTo(erzhihua);//将灰度图中单个棋子的局部图复制到erzhihua进行分析

				grayImg(rec).copyTo(erzhihua);//将灰度图中单个棋子的局部图复制到erzhihua进行分析

				pixelNum = pixelCount = 0;//像素值累计变量

				for (int i = 0; i < 40; i++)//分析霍夫圆检测到的圆心周围40*40正方形区域像素
				{
					for (int j = 0; j < 40; j++)
					{
						if (i + j < 12 || i + j>68 || i > j + 28 || j > i + 28) continue;//去掉9倒角像素,逼近圆形

						if (erzhihua.at<uchar>(i, j) > 100)//100为设定的静态二值化阈值，待修改为动态二值化********************************************
							erzhihua.at<uchar>(i, j) = 255;//白色
						else
						{
							pixelNum++;
							pixelCount += rotateImg.at<Vec3b>(rec.y + i, rec.x + j)[2];
							erzhihua.at<uchar>(i, j) = 0;//黑色
						}
					}
				}

				//	cv::imshow("【局域二值化】", erzhihua);

				if (pixelNum == 0) continue;//没有检测到棋子图案，一般是有干扰物遮挡，继续下一个棋子
				if (pixelCount / pixelNum > 100)//80为设定的静态二值化阈值，待修改为动态二值化********************************************************
					chessBoardRTScan[(center.y - board0.y + (PIXEL_PER_GRID >> 1)) / PIXEL_PER_GRID] \
					[(center.x - board0.x + (PIXEL_PER_GRID >> 1)) / PIXEL_PER_GRID] = R_CHESS;//红色暂时一律认为是红帅
				else
					chessBoardRTScan[(center.y - board0.y + (PIXEL_PER_GRID >> 1)) / PIXEL_PER_GRID] \
					[(center.x - board0.x + (PIXEL_PER_GRID >> 1)) / PIXEL_PER_GRID] = B_CHESS;//黑色暂时一律认为是黑将

				cv::circle(rotateImg, center, 4, IsRed(chessBoardRTScan[(center.y - board0.y + (PIXEL_PER_GRID >> 1)) / PIXEL_PER_GRID][(center.x - board0.x + (PIXEL_PER_GRID >> 1)) / PIXEL_PER_GRID]) ? \
					Scalar(0, 255, 0) : Scalar(255, 255, 255), -1, LINE_8, 0);//在不同棋子上标记不同颜色的点，黑色标白点，红色标绿点
			}
	}
		if (opencvRun&&circleFlag)
		{
			redChessCount = 0;//红棋计数变量清零，准备计数
			for (int i = 0; i < 10; i++)//统计摄像头扫描的红色（玩家）棋子数量
			{
				for (int j = 0; j < 9; j++)
					if (IsRed(chessBoardRTScan[i][j])) redChessCount++;
			}
			printf("%d", redChessCount);

			if (redChessCount == redChessNum)//玩家棋子数量无误
			{
				memset(userChessBoardNew, NOCHESS, sizeof(userChessBoardNew));
				memset(userChessBoardLast, NOCHESS, sizeof(userChessBoardLast));
				//注意此处chessBoardRTScan为实时棋盘，即玩家走棋后的全局棋盘，chessBoardNew为上次棋盘，即电脑走棋后等待玩家走棋时的全局棋盘
				//userChessBoardNew为实时玩家棋盘，即玩家走棋后的玩家棋盘，userChessBoardLast为电脑走棋后等待玩家走棋时的玩家棋盘
				for (int i = 0; i < 10; i++)
				{
					for (int j = 0; j < 9; j++)
					{
						if (IsRed(chessBoardRTScan[i][j]))
							userChessBoardNew[i][j] = chessBoardRTScan[i][j];//从摄像头扫描结果中只将玩家棋子复制到玩家新棋盘
						if (IsRed(chessBoardNew[i][j]))
							userChessBoardLast[i][j] = chessBoardNew[i][j];//将电脑走棋后的玩家棋盘复制到玩家上次棋盘
					}
				}

				for (int i = 0; i < 10; i++)
				{
					for (int j = 0; j < 9; j++)
					{
						if (IsRed(userChessBoardNew[i][j]) && (userChessBoardLast[i][j] == NOCHESS))//红棋走子终点
						{
							userMovePath[2] = j;
							userMovePath[3] = i;
							if (IsBlack(chessBoardNew[i][j]))
							{
								blackChessNum--;//红棋吃子，否则正常走子
								printf("吃子终点(%d,%d)\n", j, i);
							}
							else
								printf("走子终点(%d,%d)\n", j, i);
						}
						else if (IsRed(userChessBoardLast[i][j]) && (userChessBoardNew[i][j] == NOCHESS))//红棋走子起点
						{
							userMovePath[0] = j;
							userMovePath[1] = i;
							printf("走子起点(%d,%d)\n", j, i);
						}
					}
				}

				opencvRun = false;//标志位关闭
			}
		}

		t = ((double)getTickCount() - t) / getTickFrequency();
		sprintf_s(st, "FPS:%.2f", 1.0f / t);
		cv::putText(rotateImg, st, Point(5, 20), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255));
		//帧率显示↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑

		cv::rectangle(rotateImg, board0, Scalar(255, 255, 0), 1, LINE_8, 0);//画出棋盘区域

		cv::rectangle(rotateImg, ROI, Scalar(255, 0, 255), 1, LINE_8, 0);//画出棋盘区域

		//resize(rotateImg, showImg, Size(370, 440), 0, 0, CV_INTER_LINEAR);
		rotateImg.copyTo(showImg);
		int width = rotateImg.cols ;
		int height = rotateImg.rows ;



		cv::resize(rotateImg, rotateImg, Size(width, height), 0, 0, CV_INTER_LINEAR);
		cv::imshow("OpenCV", showImg);
		if (waitKey(10) == 27) break;
	}
}


int calDistance(Point a, Point b) {
	return  sqrt((a.x - b.x)* (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

