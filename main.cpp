#include "widget.h"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    QCoreApplication::setOrganizationName("leo-ai-vision");
    QCoreApplication::setApplicationName("EnvironmentMonitor");

    Widget window;
    window.show();
    return application.exec();
}
