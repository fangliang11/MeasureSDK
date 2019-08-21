#include "Calibration.h"

Calibration::Calibration(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
	hide();
}

Calibration::~Calibration()
{
}
