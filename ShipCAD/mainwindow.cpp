#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "viewport.h"
#include "shipcadmodel.h"
#include "controller.h"

using namespace ShipCAD;
using namespace std;

MainWindow::MainWindow(Controller* c, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _controller(c)
{
    ui->setupUi(this);
    addDefaultViewports();

    // connect
    connect(ui->actionOpen, SIGNAL(triggered()), _controller, SLOT(loadFile()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addDefaultViewports()
{
    ShipCADModel* model = _controller->getModel();
    int row = 0;
    int col = 0;
    for (int i=0; i<4; i++) {
        // make the viewport
        Viewport* vp = new Viewport();
        vp->setSurface(model->getSurface());
        // put it in window container
        QWidget* container = QWidget::createWindowContainer(vp);
        container->setMinimumSize(320,200);
        // put it in display area
        ui->displayLayout->addWidget(container, row, col++);
        if (col == 2) {
            col = 0;
            row++;
        }
        _viewports.push_back(make_pair(container, vp));
    }
}
