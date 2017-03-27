#ifndef INSERTPLANEPOINTSDIALOG_H
#define INSERTPLANEPOINTSDIALOG_H

#include <QDialog>

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
    bool transversePlane() const;
    bool horizontalPlane() const;
    bool verticalPlane() const;
    bool addControlCurveSelected() const;
                                             
public slots:

    void setExtents(float min, float max);

protected:

    void readSettings();
    void saveSettings();
    virtual void closeEvent(QCloseEvent* event);
    
private:

    Ui::InsertPlanePointsDialog *ui;
};

#endif // INSERTPLANEPOINTSDIALOG_H
