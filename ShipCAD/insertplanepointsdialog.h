#ifndef INSERTPLANEPOINTSDIALOG_H
#define INSERTPLANEPOINTSDIALOG_H

#include <QDialog>
#include "shipcadlib.h"

namespace Ui {
class InsertPlanePointsDialog;
}

class InsertPlanePointsDialog : public QDialog
{
    Q_OBJECT

public:

    explicit InsertPlanePointsDialog(QWidget *parent = 0);
    ~InsertPlanePointsDialog();

    QString distanceValue() const;
    ShipCAD::plane_selected_t whichPlane() const;
    void setPlaneSelected(ShipCAD::plane_selected_t pln);
    bool addControlCurveSelected() const;
                                             
public slots:

    void planeChanged();
    void setExtents(const QVector3D& min, const QVector3D& max);

protected:

    void readSettings();
    void saveSettings();
    virtual void closeEvent(QCloseEvent* event);
    
private:

    Ui::InsertPlanePointsDialog *ui;
    QVector3D _min;
    QVector3D _max;
};

#endif // INSERTPLANEPOINTSDIALOG_H
