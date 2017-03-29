#ifndef INTERSECTLAYERSDIALOG_H
#define INTERSECTLAYERSDIALOG_H

#include <QDialog>

namespace Ui {
class IntersectLayersDialog;
}

namespace ShipCAD {
struct IntersectLayersDialogData;
}

class IntersectLayersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IntersectLayersDialog(QWidget *parent = 0);
    ~IntersectLayersDialog();

    void initialize(ShipCAD::IntersectLayersDialogData& data);
    void retrieve(ShipCAD::IntersectLayersDialogData& data);

public slots:

    void layer1Changed();

protected:
    /*! \brief read stored settings for dialog
     */
    void readSettings();
    /*! \brief save settings for dialog
     */
    void saveSettings();
    virtual void closeEvent(QCloseEvent* event);

private:
    Ui::IntersectLayersDialog *ui;
};

#endif // INTERSECTLAYERSDIALOG_H
