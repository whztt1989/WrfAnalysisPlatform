#include "wrf_analysis_platform.h"
#include <QtGui/QApplication>
#include <QDebug>

class MyApplication : public QApplication 
{
public:
	MyApplication(int& argc, char** argv): QApplication(argc, argv) { }
	virtual ~MyApplication() { }

	// reimplemented from QApplication so we can throw exceptions in slots
	virtual bool notify(QObject* receiver, QEvent* event) {
		try {
			return QApplication::notify(receiver, event);
		} catch(std::exception& e) {
			  qCritical() << "Exception thrown:" << e.what();
		}
		return false;
	}
};

int main(int argc, char *argv[])
{
	MyApplication a(argc, argv);
	WrfAnalysisPlatform w;
	w.show();

	return a.exec();
}
