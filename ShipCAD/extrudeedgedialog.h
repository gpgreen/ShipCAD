#ifndef EXTRUDEEDGEDIALOG_H
#define EXTRUDEEDGEDIALOG_H

#include <QDialog>
#include "dialogdata.h"

namespace Ui {
    class ExtrudeEdgeDialog;
}

class ExtrudeEdgeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExtrudeEdgeDialog(QWidget *parent = 0);
    ~ExtrudeEdgeDialog();

    void initialize(ShipCAD::ExtrudeEdgeDialogData& data);
    void retrieve(ShipCAD::ExtrudeEdgeDialogData& data);

protected:
    /*! \brief read stored settings for dialog
     */
    void readSettings();
    /*! \brief save settings for dialog
     */
    void saveSettings();
    virtual void closeEvent(QCloseEvent* event);

private:
    Ui::ExtrudeEdgeDialog *ui;
};

#endif // EXTRUDEEDGEDIALOG_H
