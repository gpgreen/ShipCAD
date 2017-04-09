#ifndef CHOOSELAYERDIALOG_H
#define CHOOSELAYERDIALOG_H

#include <QDialog>
#include "dialogdata.h"

namespace Ui {
class ChooseLayerDialog;
}

class ChooseLayerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChooseLayerDialog(QWidget *parent = 0);
    ~ChooseLayerDialog();

    void initialize(ShipCAD::ChooseLayerDialogData& data);
    void retrieve(ShipCAD::ChooseLayerDialogData& data);

signals:

    void layerSelected(ShipCAD::SubdivisionLayer*);
    void layerDeselected(ShipCAD::SubdivisionLayer*);
    void layerUpdate(ShipCAD::ChooseLayerDialogData*);

public slots:

    void listItemChanged(QStandardItem *item);
    void includePointClicked();

protected:
    /*! \brief read stored settings for dialog
     */
    void readSettings();
    /*! \brief save settings for dialog
     */
    void saveSettings();
    virtual void closeEvent(QCloseEvent* event);

private:
    Ui::ChooseLayerDialog *ui;
    QStandardItemModel* _listModel;
    ShipCAD::ChooseLayerDialogData* _data;
};

#endif // CHOOSELAYERDIALOG_H
