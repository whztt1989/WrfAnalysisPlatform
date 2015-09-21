#include "wrf_visualization_platform.h"
#include "glyph_test_window.h"
#include <QtGui/QApplication>
#include <QtCore/QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	QTextCodec::setCodecForLocale(QTextCodec::codecForName("en_US.UTF-8"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("en_US.UTF-8"));
	QTextCodec::setCodecForTr(QTextCodec::codecForName("en_US.UTF-8"));

    //WrfVisualizationPlatform w;
	GlyphTestWindow w;
    QFont font;
    font.setFamily("aria");
    font.setPixelSize(9);

    w.show();

    return a.exec();
}
