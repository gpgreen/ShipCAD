#ifndef MIRRORDIALOG_H
#define MIRRORDIALOG_H

#include <QDialog>
#include "dialogdata.h"

namespace Ui {
class MirrorDialog;
}

class MirrorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MirrorDialog(QWidget *parent = 0);
    ~MirrorDialog();

    void initialize(ShipCAD::MirrorDialogData& data);
    void retrieve(ShipCAD::MirrorDialogData& data);

protected:

    void readSettings();
    void saveSettings();
    virtual void closeEvent(QCloseEvent* event);
    
private:
    Ui::MirrorDialog *ui;
};

#endif // MIRRORDIALOG_H
