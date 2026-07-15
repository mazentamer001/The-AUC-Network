#include <QApplication>
#include "ui/MainWindow.h"




int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("The Network");

    MainWindow w;
    w.show();

    return app.exec();
}