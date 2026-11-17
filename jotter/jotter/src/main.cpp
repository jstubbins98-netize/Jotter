#include "MainWindow.h"
#include <QApplication>
#include <QImageReader>

int main(int argc, char *argv[]) {
    QApplication application(argc, argv);
    application.setApplicationName("Jotter");
    application.setOrganizationName("Jotter");
    application.setApplicationVersion("1.0.0");
    application.setStyle("Fusion");
    QImageReader::setAllocationLimit(128);
    MainWindow window;
    window.show();
    return application.exec();
}
