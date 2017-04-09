#ifndef ROTATEDIALOG_H
#define ROTATEDIALOG_H

#include <QDialog>
#include "dialogdata.h"

namespace Ui {
class RotateDialog;
}

class RotateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RotateDialog(QWidget *parent = 0);
    ~RotateDialog();

    void initialize(ShipCAD::RotateDialogData& data);
    void retrieve(ShipCAD::RotateDialogData& data);

protected:

    void readSettings();
    void saveSettings();
    virtual void closeEvent(QCloseEvent* event);
    
private:
    Ui::RotateDialog *ui;
};

#endif // ROTATEDIALOG_H
