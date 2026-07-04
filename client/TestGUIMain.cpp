#include <QApplication>
#include "TestWindow.h"


//this is a test run to test our backend integreted with a dummy frontend
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("AUC Networking Test");

    TestWindow w;
    w.show();

    return app.exec();
}