#include "mainwindow.h"
#include <QApplication>

#include <gst/gst.h>

int main(int argc, char *argv[])
{
    gst_init( &argc,&argv);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
