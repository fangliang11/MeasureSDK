/* This is sample from the OpenCV book. The copyright notice is below */

/* *************** License:**************************
   Oct. 3, 2008
   Right to use this code in any way you want without warranty, support or any guarantee of it working.

   BOOK: It would be nice if you cited it:
   Learning OpenCV: Computer Vision with the OpenCV Library
	 by Gary Bradski and Adrian Kaehler
	 Published by O'Reilly Media, October 3, 2008

   AVAILABLE AT:
	 http://www.amazon.com/Learning-OpenCV-Computer-Vision-Library/dp/0596516134
	 Or: http://oreilly.com/catalog/9780596516130/
	 ISBN-10: 0596516134 or: ISBN-13: 978-0596516130

   OPENCV WEBSITES:
	 Homepage:      http://opencv.org
	 Online docs:   http://docs.opencv.org
	 Q&A forum:     http://answers.opencv.org
	 Issue tracker: http://code.opencv.org
	 GitHub:        https://github.com/opencv/opencv/
   ************************************************** */
#pragma warning(disable : 4996)

#include "opencv2/calib3d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

using namespace cv;
using namespace std;

static int print_help()
{
	cout <<
		" Given a list of chessboard images, the number of corners (nx, ny)\n"
		" on the chessboards, and a flag: useCalibrated for \n"
		"   calibrated (0) or\n"
		"   uncalibrated \n"
		"     (1: use cvStereoCalibrate(), 2: compute fundamental\n"
		"         matrix separately) stereo. \n"
		" Calibrate the cameras and display the\n"
		" rectified results along with the computed disparity images.   \n" << endl;
	cout << "Usage:\n ./stereo_calib -w=<board_width default=9> -h=<board_height default=6> -s=<square_size default=1.0> <image list XML/YML file default=../data/stereo_calib.xml>\n" << endl;
	return 0;
}


static void
StereoCalib(const vector<string>& imagelist, Size boardSize, float squareSize, bool displayCorners = false, bool useCalibrated = false, bool showRectified = false)
{
	if (imagelist.size() % 2 != 0)
	{
		//如果图像数目不是偶数则报错
		cout << "Error: the image list contains odd (non-even) number of elements\n";
		return;
	}

	const int maxScale = 2;    //缩放倍数
	// ARRAY AND VECTOR STORAGE:

	vector<vector<Point2f> > imagePoints[2];  //二维vector：存储所有图像中的角点，左相机图像序列为imagePoints[0]，右相机为imagePoints[1]
	vector<vector<Point3f> > objectPoints;    //存储所有角点的目标坐标
	Size imageSize;    //输入图像的尺寸

	int i, j, k, nimages = (int)imagelist.size() / 2;  //标定板图像的数量

	imagePoints[0].resize(nimages);
	imagePoints[1].resize(nimages);
	vector<string> goodImageList;    //合格的图像的名称

	for (i = j = 0; i < nimages; i++)
	{
		for (k = 0; k < 2; k++)
		{
			const string& filename = imagelist[i * 2 + k];
			Mat img = imread(filename, 0);  //读取图像（选择读入的方式灰度）
			if (img.empty())
				break;
			if (imageSize == Size())
				imageSize = img.size();
			else if (img.size() != imageSize)
			{
				//左右相机图片大小不同
				cout << "The image " << filename << " has the size different from the first image size. Skipping the pair\n";
				break;
			}
			bool found = false;
			vector<Point2f>& corners = imagePoints[k][j];   //存储单幅图片中的角点
			for (int scale = 1; scale <= maxScale; scale++)
			{
				Mat timg;
				if (scale == 1)
					timg = img;
				else
					resize(img, timg, Size(), scale, scale, INTER_LINEAR_EXACT);

				//检测图片中的角点
				found = findChessboardCorners(timg, boardSize, corners,
					CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE);
				if (found)
				{
					if (scale > 1)
					{
						Mat cornersMat(corners);
						cornersMat *= 1. / scale;
					}
					break;
				}
			}
			//显示角点
			if (displayCorners)
			{
				cout << filename << endl;
				Mat cimg, cimg1;
				cvtColor(img, cimg, COLOR_GRAY2BGR);
				drawChessboardCorners(cimg, boardSize, corners, found);   //绘制棋盘角点
				double sf = 1280. / MAX(img.rows, img.cols);
				resize(cimg, cimg1, Size(), sf, sf, INTER_LINEAR_EXACT);
				imshow("corners", cimg1);
				//imwrite("corner", cimg1);
				char c = (char)waitKey(500);
				if (c == 27 || c == 'q' || c == 'Q') //Allow ESC to quit
					exit(-1);
			}
			else
				putchar('.');  //显示点  .
			if (!found)
				break;
			//找到亚像素角点，提高标定精度
			cornerSubPix(img, corners, Size(11, 11), Size(-1, -1),
				TermCriteria(TermCriteria::COUNT + TermCriteria::EPS,
					30, 0.01));
		}
		if (k == 2)
		{
			goodImageList.push_back(imagelist[i * 2]);
			goodImageList.push_back(imagelist[i * 2 + 1]);
			j++;
		}
	}
	cout << j << " pairs have been successfully detected.\n";
	nimages = j;
	if (nimages < 2)
	{
		cout << "Error: too little pairs to run the calibration\n";
		return;
	}

	imagePoints[0].resize(nimages);
	imagePoints[1].resize(nimages);
	objectPoints.resize(nimages);

	for (i = 0; i < nimages; i++)
	{
		for (j = 0; j < boardSize.height; j++)
			for (k = 0; k < boardSize.width; k++)
				//根据格子尺寸计算角点本来应该存在的坐标
				objectPoints[i].push_back(Point3f(k*squareSize, j*squareSize, 0)); //存储计算得到的角点
	}

	cout << "Running stereo calibration ...\n";

	Mat cameraMatrix[2], distCoeffs[2];  //相机矩阵，畸变矩阵

	////********************************************************//////////////////
	cout << "First,calibration camera left...\n";

	vector<Mat> rvecs_left, tvecs_left;
	double rms_left = calibrateCamera(objectPoints, imagePoints[0], imageSize, cameraMatrix[0],
		distCoeffs[0], rvecs_left, tvecs_left, CALIB_FIX_K3 + CALIB_FIX_K4 + CALIB_FIX_K5);

	cout << "Second,calibration camera right...\n";

	vector<Mat> rvecs_right, tvecs_right;
	double rms_right = calibrateCamera(objectPoints, imagePoints[1], imageSize, cameraMatrix[1],
		distCoeffs[1], rvecs_right, tvecs_right, CALIB_FIX_K3 + CALIB_FIX_K4 + CALIB_FIX_K5);
	cout << "RMS error reported by calibrateCamera :left  right :" << rms_left << "    " << rms_right << endl;

	////********************************************************////////////////
	//调用initCameraMatrix2D函数获得相机初始内参
	//cameraMatrix[0] = initCameraMatrix2D(objectPoints, imagePoints[0], imageSize, 0);
	//cameraMatrix[1] = initCameraMatrix2D(objectPoints, imagePoints[1], imageSize, 0);
	Mat R, T, E, F;    //旋转矩阵，平移矩阵，本征矩阵，基本矩阵

	//立体标定函数
	//R-输出型，第一和第二个摄像机之间的旋转矩阵
	//T - 输出型，第一和第二个摄像机之间的平移矩阵
	//E - 输出型，基本矩阵
	//F - 输出型，基础矩阵
	//§ CV_CALIB_FIX_INTRINSIC 内参矩阵和畸变矩阵不变，所以只有 R, T, E , 和 F矩阵被估计出来
	//§ CV_CALIB_USE_INTRINSIC_GUESS 内参矩阵和畸变矩阵初始值由用户提供，并在迭代中进行优化
	//§ CV_CALIB_FIX_PRINCIPAL_POINT 在优化过程中确定主点。
	//§ CV_CALIB_FIX_FOCAL_LENGTH迭代中不改变焦距 .
	//§ CV_CALIB_FIX_ASPECT_RATIO保持 fx和 fy比值相同.
	//§ CV_CALIB_SAME_FOCAL_LENGTH强制保持两个摄像机的焦距相同 .
	//§ CV_CALIB_ZERO_TANGENT_DIST设置每个相机切向畸变系数为零并且设为固定值。
	//§ CV_CALIB_FIX_K1, ..., CV_CALIB_FIX_K6在优化中不改变相应的径向畸变系数.如果设置CV_CALIB_USE_INTRINSIC_GUESS, 使用distCoeffs矩阵提供的系数。否则将其置零.
	//§ CV_CALIB_RATIONAL_MODEL能够输出系数k4，k5，k6。如果FLAG没有被设置, 该函数计算并只返回5畸变系数。
	double rms = stereoCalibrate(objectPoints, imagePoints[0], imagePoints[1],
		cameraMatrix[0], distCoeffs[0],
		cameraMatrix[1], distCoeffs[1],
		imageSize, R, T, E, F,
		CALIB_FIX_INTRINSIC +
		CALIB_FIX_ASPECT_RATIO +
		CALIB_USE_INTRINSIC_GUESS +
		CALIB_SAME_FOCAL_LENGTH,
		TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 100, 1e-5));

	cout << "done with RMS error=" << rms << endl; //均方根误差

	// CALIBRATION QUALITY CHECK
	// because the output fundamental matrix implicitly
	// includes all the output information,
	// we can check the quality of calibration using the
	// epipolar geometry constraint: m2^t*F*m1=0
	//计算极线误差
	double err = 0;
	int npoints = 0;
	vector<Vec3f> lines[2];
	for (i = 0; i < nimages; i++)
	{
		int npt = (int)imagePoints[0][i].size();  //角点数量
		Mat imgpt[2];
		for (k = 0; k < 2; k++)
		{
			imgpt[k] = Mat(imagePoints[k][i]);
			undistortPoints(imgpt[k], imgpt[k], cameraMatrix[k], distCoeffs[k], Mat(), cameraMatrix[k]);  //矫正角点坐标
			computeCorrespondEpilines(imgpt[k], k + 1, F, lines[k]); //为一幅图像中的点计算其在另一幅图像中对应的对极线
		}
		for (j = 0; j < npt; j++)
		{
			double errij = fabs(imagePoints[0][i][j].x*lines[1][j][0] +
				imagePoints[0][i][j].y*lines[1][j][1] + lines[1][j][2]) +
				fabs(imagePoints[1][i][j].x*lines[0][j][0] +
					imagePoints[1][i][j].y*lines[0][j][1] + lines[0][j][2]);
			err += errij;
		}
		npoints += npt;
	}
	cout << "average epipolar err = " << err / npoints << endl;  //平均极线误差

	// save intrinsic parameters
	FileStorage fs("intrinsics_opencv_data.yml", FileStorage::WRITE); //保存相机内参
	if (fs.isOpened())
	{
		fs << "M1" << cameraMatrix[0] << "D1" << distCoeffs[0] <<
			"M2" << cameraMatrix[1] << "D2" << distCoeffs[1];
		fs.release();
	}
	else
		cout << "Error: can not save the intrinsic parameters\n";

	Mat R1, R2, P1, P2, Q;
	Rect validRoi[2];  //有效区域

	/*
	立体校正的时候需要两幅图像共面并且行对准 以使得立体匹配更加的可靠
	使得两幅图像共面的方法就是把两个摄像头的图像投影到一个公共成像面上，这样每幅图像从本图像平面投影到公共图像平面都需要一个旋转矩阵R
	stereoRectify 这个函数计算的就是从图像平面投影到公共成像平面的旋转矩阵Rl,Rr。 Rl,Rr即为左右相机平面行对准的校正旋转矩阵。
	左相机经过Rl旋转，右相机经过Rr旋转之后，两幅图像就已经共面并且行对准了。
	其中Pl,Pr为两个相机的投影矩阵，其作用是将3D点的坐标转换到图像的2D点的坐标:P*[X Y Z 1]' =[x y w]
	Q矩阵为重投影矩阵，即矩阵Q可以把2维平面(图像平面)上的点投影到3维空间的点:Q*[x y d 1] = [X Y Z W]。其中d为左右两幅图像的时差
	*/
	//立体矫正函数
	//R- 输入矩阵 stereoCalibrate() 求得的R矩阵
	//T - 输入矩阵 stereoCalibrate() 求得的T矩阵
	//R1 - 输出矩阵，第一个摄像机的校正变换矩阵（旋转变换）
	//R2 - 输出矩阵，第二个摄像机的校正变换矩阵（旋转矩阵）
	//P1 - 输出矩阵，第一个摄像机在新坐标系下的投影矩阵
	//P2 - 输出矩阵，第二个摄像机在想坐标系下的投影矩阵
	//Q - 4 * 4的深度差异映射矩阵

	//立体矫正
	stereoRectify(cameraMatrix[0], distCoeffs[0],
		cameraMatrix[1], distCoeffs[1],
		imageSize, R, T, R1, R2, P1, P2, Q,
		CALIB_ZERO_DISPARITY, 1, imageSize, &validRoi[0], &validRoi[1]);

	fs.open("extrinsics_opencv_data.yml", FileStorage::WRITE); //保存相机外参
	if (fs.isOpened())
	{
		fs << "R" << R << "T" << T << "R1" << R1 << "R2" << R2 << "P1" << P1 << "P2" << P2 << "Q" << Q;
		fs.release();
	}
	else
		cout << "Error: can not save the extrinsic parameters\n";

	// OpenCV can handle left-right
	// or up-down camera arrangements
	//处理左右或者上下问题: 计算是水平还是垂直显示
	bool isVerticalStereo = fabs(P2.at<double>(1, 3)) > fabs(P2.at<double>(0, 3));

	// COMPUTE AND DISPLAY RECTIFICATION
	//计算并显示矫正后的图像
	if (!showRectified)
		return;
	//左相机第一映射remap[0][0]，左相机第二映射remap[0][1]
	//右相机第一映射remap[1][0]，右相机第二映射remap[1][1]
	Mat rmap[2][2];
	// IF BY CALIBRATED (BOUGUET'S METHOD)
	//选择用 bouguet 的矫正算法
	if (useCalibrated)
	{
		// we already computed everything
	}

	// OR ELSE HARTLEY'S METHOD
	//选择用 hartley 的矫正算法
	else
		// use intrinsic parameters of each camera, but
		// compute the rectification transformation directly
		// from the fundamental matrix
	{
		vector<Point2f> allimgpt[2];
		for (k = 0; k < 2; k++)
		{
			for (i = 0; i < nimages; i++)
				std::copy(imagePoints[k][i].begin(), imagePoints[k][i].end(), back_inserter(allimgpt[k]));
		}
		F = findFundamentalMat(Mat(allimgpt[0]), Mat(allimgpt[1]), FM_8POINT, 0, 0);
		Mat H1, H2;
		stereoRectifyUncalibrated(Mat(allimgpt[0]), Mat(allimgpt[1]), F, imageSize, H1, H2, 3); //立体矫正

		R1 = cameraMatrix[0].inv()*H1*cameraMatrix[0];
		R2 = cameraMatrix[1].inv()*H2*cameraMatrix[1];
		P1 = cameraMatrix[0];
		P2 = cameraMatrix[1];
	}

	/*
	根据stereoRectify 计算出来的R 和 P 来计算图像的映射表 mapx,mapy
	mapx,mapy这两个映射表接下来可以给remap()函数调用，来校正图像，使得两幅图像共面并且行对准
	ininUndistortRectifyMap()的参数newCameraMatrix就是校正后的摄像机矩阵。在openCV里面，校正后的计算机矩阵Mrect是跟投影矩阵P一起返回的。
	所以我们在这里传入投影矩阵P，此函数可以从投影矩阵P中读出校正后的摄像机矩阵
	*/
	//undistort()函数就是initundistortrectifymap()函数和remap()函数的简单组合
	//Precompute maps for cv::remap()
	//为remap函数计算 map 映射表
	//相机矩阵、畸变系数、修正变换矩阵、新的相机矩阵、未畸变的图像尺寸、第一个输出映射的类型、第一个输出映射、第二个输出映射
	initUndistortRectifyMap(cameraMatrix[0], distCoeffs[0], R1, P1, imageSize, CV_16SC2, rmap[0][0], rmap[0][1]);    //左相机
	initUndistortRectifyMap(cameraMatrix[1], distCoeffs[1], R2, P2, imageSize, CV_16SC2, rmap[1][0], rmap[1][1]);    //右相机

	//裁剪
	Mat canvas;  //画布
	double sf;  //缩放因子
	int w, h;
	if (!isVerticalStereo)
	{
		sf = 600. / MAX(imageSize.width, imageSize.height);
		w = cvRound(imageSize.width*sf);
		h = cvRound(imageSize.height*sf);
		//sf = 1;
		//w = 1280;
		//h = 960;
		canvas.create(h, w * 2, CV_8UC3);
	}
	else
	{
		sf = 300. / MAX(imageSize.width, imageSize.height);
		w = cvRound(imageSize.width*sf);
		h = cvRound(imageSize.height*sf);
		canvas.create(h * 2, w, CV_8UC3);
	}

	for (i = 0; i < nimages; i++)
	{
		for (k = 0; k < 2; k++)
		{
			Mat img = imread(goodImageList[i * 2 + k], 0), rimg, cimg;

			//使用映射表进行矫正
			//输入图像、目标图像、第一个映射、第二个映射、插值（双线性插值）
			remap(img, rimg, rmap[k][0], rmap[k][1], INTER_LINEAR);  //重映射
			cvtColor(rimg, cimg, COLOR_GRAY2BGR);
			Mat canvasPart = !isVerticalStereo ? canvas(Rect(w*k, 0, w, h)) : canvas(Rect(0, h*k, w, h));

			//改变图像大小：输入图像、目标图像、输出图像大小、width方向缩放比例、hight方向缩放比例、插值（区域插值）
			resize(cimg, canvasPart, canvasPart.size(), 0, 0, INTER_AREA);
			if (useCalibrated)
			{
				Rect vroi(cvRound(validRoi[k].x*sf), cvRound(validRoi[k].y*sf),
					cvRound(validRoi[k].width*sf), cvRound(validRoi[k].height*sf));
				rectangle(canvasPart, vroi, Scalar(255, 0, 0), 3, 8);  //矩形边框
			}
		}

		if (!isVerticalStereo)
			for (j = 0; j < canvas.rows; j += 16)
				line(canvas, Point(0, j), Point(canvas.cols, j), Scalar(0, 255, 0), 1, 8);   //画布上画线
		else
			for (j = 0; j < canvas.cols; j += 16)
				line(canvas, Point(j, 0), Point(j, canvas.rows), Scalar(0, 255, 0), 1, 8);
		imshow("rectified", canvas);
		//imwrite("rectify.bmp", canvas);

		char c = (char)waitKey();
		if (c == 27 || c == 'q' || c == 'Q')
			break;
	}
}

//读取图像列表
static bool readStringList(const string& filename, vector<string>& l)
{
	l.resize(0);
	FileStorage fs(filename, FileStorage::READ);
	if (!fs.isOpened())
		return false;
	FileNode n = fs.getFirstTopLevelNode();
	if (n.type() != FileNode::SEQ)
		return false;
	FileNodeIterator it = n.begin(), it_end = n.end();
	for (; it != it_end; ++it)
		l.push_back((string)*it);
	return true;
}

int main(int argc, char** argv)
{
	Size boardSize;
	string imagelistfn;
	bool showRectified;
	cv::CommandLineParser parser(argc, argv, "{w|14|}{h|13|}{s|1|}{nr||}{help||}{@input|F:\\ComputerVersion\\CalibrationImageData\\high2_data\\stereo_calib.xml|}");
	if (parser.has("help"))
		return print_help();
	showRectified = !parser.has("nr");  //是否显示矫正后的图像
	imagelistfn = parser.get<string>("@input");    //图像列表文件名
	boardSize.width = parser.get<int>("w");     //宽方向的格子数量
	boardSize.height = parser.get<int>("h");    //高方向的格子数量
	float squareSize = parser.get<float>("s");  //格子的实际尺寸
	if (!parser.check())
	{
		parser.printErrors();
		return 1;
	}
	vector<string> imagelist;
	bool ok = readStringList(imagelistfn, imagelist);
	if (!ok || imagelist.empty())
	{
		cout << "can not open " << imagelistfn << " or the string list is empty" << endl;
		return print_help();
	}

	//执行标定程序
	StereoCalib(imagelist, boardSize, squareSize, true, true, showRectified);


	return 0;
}
