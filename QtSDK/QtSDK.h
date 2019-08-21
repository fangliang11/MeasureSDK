#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtSDK.h"
#include "Calibration.h"

class QtSDK : public QMainWindow
{
	Q_OBJECT

public:
	QtSDK(QWidget *parent = Q_NULLPTR);
	
private:
	Ui::QtSDKClass ui;
	Calibration *calib = NULL;

public slots:
	void onButton();
	void closeButton();
};
