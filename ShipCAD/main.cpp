#include "mainwindow.h"
#include <QApplication>
#include "controller.h"
#include "shipcadmodel.h"

using namespace ShipCAD;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ShipCADModel* model = new ShipCADModel();
    Controller c(model);

    MainWindow w;
    w.show();

    return a.exec();
}
