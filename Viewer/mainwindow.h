#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QActionGroup>

namespace Ui {
class MainWindow;
}

namespace ShipCADGeometry {
class Viewport;
class SubdivisionSurface;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setAnimating(bool animating);

    void setSurface(ShipCADGeometry::SubdivisionSurface* surface);

public slots:

    void openFile();

protected:

    virtual void resizeEvent(QResizeEvent *event);

protected slots:

    void wireFrame();
    void shade();
    void shadeCurvature();
    void shadeDevelopable();
    void shadeZebra();
    void showControlNet(bool val);
    void showInteriorEdges(bool val);
    void showControlCurves(bool val);
    void showCurvature(bool val);
    void showNormals(bool val);
    void drawMirror(bool val);
    void shadeUnderwater(bool val);

    void animationTimeout();

private:
    Ui::MainWindow *ui;
    ShipCADGeometry::Viewport* _vp;
    QActionGroup* _modeGroup;
    QTimer* _animation_timer;
};

#endif // MAINWINDOW_H
