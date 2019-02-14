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
		//���ͼ����Ŀ����ż���򱨴�
		cout << "Error: the image list contains odd (non-even) number of elements\n";
		return;
	}

	const int maxScale = 2;    //���ű���
	// ARRAY AND VECTOR STORAGE:

	vector<vector<Point2f> > imagePoints[2];  //��άvector���洢����ͼ���еĽǵ㣬�����ͼ������ΪimagePoints[0]�������ΪimagePoints[1]
	vector<vector<Point3f> > objectPoints;    //�洢���нǵ��Ŀ������
	Size imageSize;    //����ͼ��ĳߴ�

	int i, j, k, nimages = (int)imagelist.size() / 2;  //�궨��ͼ�������

	imagePoints[0].resize(nimages);
	imagePoints[1].resize(nimages);
	vector<string> goodImageList;    //�ϸ��ͼ�������

	for (i = j = 0; i < nimages; i++)
	{
		for (k = 0; k < 2; k++)
		{
			const string& filename = imagelist[i * 2 + k];
			Mat img = imread(filename, 0);  //��ȡͼ��ѡ�����ķ�ʽ�Ҷȣ�
			if (img.empty())
				break;
			if (imageSize == Size())
				imageSize = img.size();
			else if (img.size() != imageSize)
			{
				//�������ͼƬ��С��ͬ
				cout << "The image " << filename << " has the size different from the first image size. Skipping the pair\n";
				break;
			}
			bool found = false;
			vector<Point2f>& corners = imagePoints[k][j];   //�洢����ͼƬ�еĽǵ�
			for (int scale = 1; scale <= maxScale; scale++)
			{
				Mat timg;
				if (scale == 1)
					timg = img;
				else
					resize(img, timg, Size(), scale, scale, INTER_LINEAR_EXACT);

				//���ͼƬ�еĽǵ�
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
			//��ʾ�ǵ�
			if (displayCorners)
			{
				cout << filename << endl;
				Mat cimg, cimg1;
				cvtColor(img, cimg, COLOR_GRAY2BGR);
				drawChessboardCorners(cimg, boardSize, corners, found);   //�������̽ǵ�
				double sf = 1280. / MAX(img.rows, img.cols);
				resize(cimg, cimg1, Size(), sf, sf, INTER_LINEAR_EXACT);
				imshow("corners", cimg1);
				//imwrite("corner", cimg1);
				char c = (char)waitKey(500);
				if (c == 27 || c == 'q' || c == 'Q') //Allow ESC to quit
					exit(-1);
			}
			else
				putchar('.');  //��ʾ��  .
			if (!found)
				break;
			//�ҵ������ؽǵ㣬��߱궨����
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
				//���ݸ��ӳߴ����ǵ㱾��Ӧ�ô��ڵ�����
				objectPoints[i].push_back(Point3f(k*squareSize, j*squareSize, 0)); //�洢����õ��Ľǵ�
	}

	cout << "Running stereo calibration ...\n";

	Mat cameraMatrix[2], distCoeffs[2];  //������󣬻������

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
	//����initCameraMatrix2D������������ʼ�ڲ�
	//cameraMatrix[0] = initCameraMatrix2D(objectPoints, imagePoints[0], imageSize, 0);
	//cameraMatrix[1] = initCameraMatrix2D(objectPoints, imagePoints[1], imageSize, 0);
	Mat R, T, E, F;    //��ת����ƽ�ƾ��󣬱������󣬻�������

	//����궨����
	//R-����ͣ���һ�͵ڶ��������֮�����ת����
	//T - ����ͣ���һ�͵ڶ��������֮���ƽ�ƾ���
	//E - ����ͣ���������
	//F - ����ͣ���������
	//�� CV_CALIB_FIX_INTRINSIC �ڲξ���ͻ�����󲻱䣬����ֻ�� R, T, E , �� F���󱻹��Ƴ���
	//�� CV_CALIB_USE_INTRINSIC_GUESS �ڲξ���ͻ�������ʼֵ���û��ṩ�����ڵ����н����Ż�
	//�� CV_CALIB_FIX_PRINCIPAL_POINT ���Ż�������ȷ�����㡣
	//�� CV_CALIB_FIX_FOCAL_LENGTH�����в��ı佹�� .
	//�� CV_CALIB_FIX_ASPECT_RATIO���� fx�� fy��ֵ��ͬ.
	//�� CV_CALIB_SAME_FOCAL_LENGTHǿ�Ʊ�������������Ľ�����ͬ .
	//�� CV_CALIB_ZERO_TANGENT_DIST����ÿ������������ϵ��Ϊ�㲢����Ϊ�̶�ֵ��
	//�� CV_CALIB_FIX_K1, ..., CV_CALIB_FIX_K6���Ż��в��ı���Ӧ�ľ������ϵ��.�������CV_CALIB_USE_INTRINSIC_GUESS, ʹ��distCoeffs�����ṩ��ϵ��������������.
	//�� CV_CALIB_RATIONAL_MODEL�ܹ����ϵ��k4��k5��k6�����FLAGû�б�����, �ú������㲢ֻ����5����ϵ����
	double rms = stereoCalibrate(objectPoints, imagePoints[0], imagePoints[1],
		cameraMatrix[0], distCoeffs[0],
		cameraMatrix[1], distCoeffs[1],
		imageSize, R, T, E, F,
		CALIB_FIX_INTRINSIC +
		CALIB_FIX_ASPECT_RATIO +
		CALIB_USE_INTRINSIC_GUESS +
		CALIB_SAME_FOCAL_LENGTH,
		TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 100, 1e-5));

	cout << "done with RMS error=" << rms << endl; //���������

	// CALIBRATION QUALITY CHECK
	// because the output fundamental matrix implicitly
	// includes all the output information,
	// we can check the quality of calibration using the
	// epipolar geometry constraint: m2^t*F*m1=0
	//���㼫�����
	double err = 0;
	int npoints = 0;
	vector<Vec3f> lines[2];
	for (i = 0; i < nimages; i++)
	{
		int npt = (int)imagePoints[0][i].size();  //�ǵ�����
		Mat imgpt[2];
		for (k = 0; k < 2; k++)
		{
			imgpt[k] = Mat(imagePoints[k][i]);
			undistortPoints(imgpt[k], imgpt[k], cameraMatrix[k], distCoeffs[k], Mat(), cameraMatrix[k]);  //�����ǵ�����
			computeCorrespondEpilines(imgpt[k], k + 1, F, lines[k]); //Ϊһ��ͼ���еĵ����������һ��ͼ���ж�Ӧ�ĶԼ���
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
	cout << "average epipolar err = " << err / npoints << endl;  //ƽ���������

	// save intrinsic parameters
	FileStorage fs("intrinsics_opencv_data.yml", FileStorage::WRITE); //��������ڲ�
	if (fs.isOpened())
	{
		fs << "M1" << cameraMatrix[0] << "D1" << distCoeffs[0] <<
			"M2" << cameraMatrix[1] << "D2" << distCoeffs[1];
		fs.release();
	}
	else
		cout << "Error: can not save the intrinsic parameters\n";

	Mat R1, R2, P1, P2, Q;
	Rect validRoi[2];  //��Ч����

	/*
	����У����ʱ����Ҫ����ͼ���沢���ж�׼ ��ʹ������ƥ����ӵĿɿ�
	ʹ������ͼ����ķ������ǰ���������ͷ��ͼ��ͶӰ��һ�������������ϣ�����ÿ��ͼ��ӱ�ͼ��ƽ��ͶӰ������ͼ��ƽ�涼��Ҫһ����ת����R
	stereoRectify �����������ľ��Ǵ�ͼ��ƽ��ͶӰ����������ƽ�����ת����Rl,Rr�� Rl,Rr��Ϊ�������ƽ���ж�׼��У����ת����
	���������Rl��ת�����������Rr��ת֮������ͼ����Ѿ����沢���ж�׼�ˡ�
	����Pl,PrΪ���������ͶӰ�����������ǽ�3D�������ת����ͼ���2D�������:P*[X Y Z 1]' =[x y w]
	Q����Ϊ��ͶӰ���󣬼�����Q���԰�2άƽ��(ͼ��ƽ��)�ϵĵ�ͶӰ��3ά�ռ�ĵ�:Q*[x y d 1] = [X Y Z W]������dΪ��������ͼ���ʱ��
	*/
	//�����������
	//R- ������� stereoCalibrate() ��õ�R����
	//T - ������� stereoCalibrate() ��õ�T����
	//R1 - ������󣬵�һ���������У���任������ת�任��
	//R2 - ������󣬵ڶ����������У���任������ת����
	//P1 - ������󣬵�һ���������������ϵ�µ�ͶӰ����
	//P2 - ������󣬵ڶ����������������ϵ�µ�ͶӰ����
	//Q - 4 * 4����Ȳ���ӳ�����

	//�������
	stereoRectify(cameraMatrix[0], distCoeffs[0],
		cameraMatrix[1], distCoeffs[1],
		imageSize, R, T, R1, R2, P1, P2, Q,
		CALIB_ZERO_DISPARITY, 1, imageSize, &validRoi[0], &validRoi[1]);

	fs.open("extrinsics_opencv_data.yml", FileStorage::WRITE); //����������
	if (fs.isOpened())
	{
		fs << "R" << R << "T" << T << "R1" << R1 << "R2" << R2 << "P1" << P1 << "P2" << P2 << "Q" << Q;
		fs.release();
	}
	else
		cout << "Error: can not save the extrinsic parameters\n";

	// OpenCV can handle left-right
	// or up-down camera arrangements
	//�������һ�����������: ������ˮƽ���Ǵ�ֱ��ʾ
	bool isVerticalStereo = fabs(P2.at<double>(1, 3)) > fabs(P2.at<double>(0, 3));

	// COMPUTE AND DISPLAY RECTIFICATION
	//���㲢��ʾ�������ͼ��
	if (!showRectified)
		return;
	//�������һӳ��remap[0][0]��������ڶ�ӳ��remap[0][1]
	//�������һӳ��remap[1][0]��������ڶ�ӳ��remap[1][1]
	Mat rmap[2][2];
	// IF BY CALIBRATED (BOUGUET'S METHOD)
	//ѡ���� bouguet �Ľ����㷨
	if (useCalibrated)
	{
		// we already computed everything
	}

	// OR ELSE HARTLEY'S METHOD
	//ѡ���� hartley �Ľ����㷨
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
		stereoRectifyUncalibrated(Mat(allimgpt[0]), Mat(allimgpt[1]), F, imageSize, H1, H2, 3); //�������

		R1 = cameraMatrix[0].inv()*H1*cameraMatrix[0];
		R2 = cameraMatrix[1].inv()*H2*cameraMatrix[1];
		P1 = cameraMatrix[0];
		P2 = cameraMatrix[1];
	}

	/*
	����stereoRectify ���������R �� P ������ͼ���ӳ��� mapx,mapy
	mapx,mapy������ӳ������������Ը�remap()�������ã���У��ͼ��ʹ������ͼ���沢���ж�׼
	ininUndistortRectifyMap()�Ĳ���newCameraMatrix����У����������������openCV���棬У����ļ��������Mrect�Ǹ�ͶӰ����Pһ�𷵻صġ�
	�������������ﴫ��ͶӰ����P���˺������Դ�ͶӰ����P�ж���У��������������
	*/
	//undistort()��������initundistortrectifymap()������remap()�����ļ����
	//Precompute maps for cv::remap()
	//Ϊremap�������� map ӳ���
	//������󡢻���ϵ���������任�����µ��������δ�����ͼ��ߴ硢��һ�����ӳ������͡���һ�����ӳ�䡢�ڶ������ӳ��
	initUndistortRectifyMap(cameraMatrix[0], distCoeffs[0], R1, P1, imageSize, CV_16SC2, rmap[0][0], rmap[0][1]);    //�����
	initUndistortRectifyMap(cameraMatrix[1], distCoeffs[1], R2, P2, imageSize, CV_16SC2, rmap[1][0], rmap[1][1]);    //�����

	//�ü�
	Mat canvas;  //����
	double sf;  //��������
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

			//ʹ��ӳ������н���
			//����ͼ��Ŀ��ͼ�񡢵�һ��ӳ�䡢�ڶ���ӳ�䡢��ֵ��˫���Բ�ֵ��
			remap(img, rimg, rmap[k][0], rmap[k][1], INTER_LINEAR);  //��ӳ��
			cvtColor(rimg, cimg, COLOR_GRAY2BGR);
			Mat canvasPart = !isVerticalStereo ? canvas(Rect(w*k, 0, w, h)) : canvas(Rect(0, h*k, w, h));

			//�ı�ͼ���С������ͼ��Ŀ��ͼ�����ͼ���С��width�������ű�����hight�������ű�������ֵ�������ֵ��
			resize(cimg, canvasPart, canvasPart.size(), 0, 0, INTER_AREA);
			if (useCalibrated)
			{
				Rect vroi(cvRound(validRoi[k].x*sf), cvRound(validRoi[k].y*sf),
					cvRound(validRoi[k].width*sf), cvRound(validRoi[k].height*sf));
				rectangle(canvasPart, vroi, Scalar(255, 0, 0), 3, 8);  //���α߿�
			}
		}

		if (!isVerticalStereo)
			for (j = 0; j < canvas.rows; j += 16)
				line(canvas, Point(0, j), Point(canvas.cols, j), Scalar(0, 255, 0), 1, 8);   //�����ϻ���
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

//��ȡͼ���б�
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
	showRectified = !parser.has("nr");  //�Ƿ���ʾ�������ͼ��
	imagelistfn = parser.get<string>("@input");    //ͼ���б��ļ���
	boardSize.width = parser.get<int>("w");     //������ĸ�������
	boardSize.height = parser.get<int>("h");    //�߷���ĸ�������
	float squareSize = parser.get<float>("s");  //���ӵ�ʵ�ʳߴ�
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

	//ִ�б궨����
	StereoCalib(imagelist, boardSize, squareSize, true, true, showRectified);


	return 0;
}