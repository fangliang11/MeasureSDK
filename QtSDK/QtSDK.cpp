#include "QtSDK.h"

QtSDK::QtSDK(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
}

void QtSDK::onButton()
{
	if (calib == NULL) {
		calib = new Calibration(this);
		calib->show();
	}
	else calib->show();
}

void QtSDK::closeButton()
{
	calib->close();
}