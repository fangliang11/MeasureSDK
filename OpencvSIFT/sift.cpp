#include "opencv2/opencv.hpp"  
#include"opencv2/xfeatures2d.hpp"

//#if _DEBUG
//#pragma comment(lib,"opencv_world341d.lib")
//#else
//#pragma comment(lib,"opencv_world341.lib")
//#endif // _DEBUG

using namespace cv;
using namespace std;

int main() {
	Mat img = imread("house.jpg", CV_LOAD_IMAGE_COLOR);
	Ptr<Feature2D> sift = xfeatures2d::SIFT::create();
	vector<KeyPoint> keypoints;
	Mat descriptors;

	sift->detectAndCompute(img, noArray(), keypoints, descriptors);
	drawKeypoints(img, keypoints, descriptors, Scalar(0, 255, 255));
	imshow("Result", descriptors);
	waitKey(0);

	return 0;
}
