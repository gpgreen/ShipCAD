#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <vector>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

namespace ShipCAD {
class Viewport;
class Controller;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(ShipCAD::Controller* c, QWidget *parent = 0);
    ~MainWindow();

protected:
    void addDefaultViewports();

private:
    Ui::MainWindow *ui;
    ShipCAD::Controller* _controller;
    std::vector<std::pair<QWidget*, ShipCAD::Viewport*> > _viewports;
};

#endif // MAINWINDOW_H
