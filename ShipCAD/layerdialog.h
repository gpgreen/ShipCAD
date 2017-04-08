#ifndef LAYERDIALOG_H
#define LAYERDIALOG_H

#include <QDialog>
#include "dialogdata.h"

namespace Ui {
class LayerDialog;
}

class LayerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LayerDialog(QWidget *parent = 0);
    ~LayerDialog();

    void initialize(ShipCAD::LayerDialogData& data);
    void retrieve(ShipCAD::LayerDialogData& data);

protected:
    /*! \brief read stored settings for dialog
     */
    void readSettings();
    /*! \brief save settings for dialog
     */
    void saveSettings();
    virtual void closeEvent(QCloseEvent* event);

private:
    Ui::LayerDialog *ui;
};

#endif // LAYERDIALOG_H
