#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QActionGroup>

namespace Ui {
class MainWindow;
}

namespace ShipCADGeometry {
class Viewport;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setAnimating(bool animating);

protected slots:

    void wireFrame();
    void shade();
    void shadeCurvature();
    void shadeDevelopable();
    void shadeZebra();

private:
    Ui::MainWindow *ui;
    ShipCADGeometry::Viewport* _vp;
    QActionGroup* _modeGroup;
};

#endif // MAINWINDOW_H
