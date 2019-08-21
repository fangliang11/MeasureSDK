#include "QtSDK.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QtSDK w;
	w.show();
	return a.exec();
}
