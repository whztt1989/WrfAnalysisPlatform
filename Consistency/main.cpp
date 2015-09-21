#include "consistency_vis.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	ConsistencyVis w;
	w.show();
	return a.exec();
}
