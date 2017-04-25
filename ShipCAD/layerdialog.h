/*##############################################################################################
 *    ShipCAD																				   *
 *    Copyright 2015, by Greg Green <ggreen@bit-builder.com>								   *
 *                                                                                             *
 *    This program is free software; you can redistribute it and/or modify it under            *
 *    the terms of the GNU General Public License as published by the                          *
 *    Free Software Foundation; either version 2 of the License, or (at your option)           *
 *    any later version.                                                                       *
 *                                                                                             *
 *    This program is distributed in the hope that it will be useful, but WITHOUT ANY          *
 *    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A          *
 *    PARTICULAR PURPOSE. See the GNU General Public License for more details.                 *
 *                                                                                             *
 *    You should have received a copy of the GNU General Public License along with             *
 *    this program; if not, write to the Free Software Foundation, Inc.,                       *
 *    59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                                    *
 *                                                                                             *
 *#############################################################################################*/

#ifndef LAYERDIALOG_H
#define LAYERDIALOG_H

#include <QDialog>
#include <QToolButton>
#include "dialogdata.h"
#include "colorview.h"

namespace Ui {
class LayerDialog;
}

class LayerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LayerDialog(QWidget *parent = 0);
    ~LayerDialog();

    void initialize(ShipCAD::LayerDialogData* data, bool delete_data,
                    ShipCAD::unit_type_t units);
    ShipCAD::LayerDialogData* retrieve() {return _data;}

signals:

    void activeLayerChanged(int index);
    void exeChooseColorDialog(ShipCAD::ChooseColorDialogData& data);
    void layerColorChanged(const QColor& color);
    void newLayer();
    void deleteEmptyLayer();
    void reorderLayerList(ShipCAD::LayerDialogData* data);

public slots:

    void nameChanged(const QString& nm);
    void weightChanged();
    void thicknessChanged();
    void alphaChanged();
    void listRowChanged(int item);
    void hydroBoxChanged(int state);
    void symmBoxChanged(int state);
    void curveBoxChanged(int state);
    void devBoxChanged(int state);
    void showLinesBoxChanged(int state);
    void selectColor();
    void moveUp();
    void moveDown();
    /*! \brief update state of ui after change
     */
    void updateState();
    
protected:
    /*! \brief read stored settings for dialog
     */
    void readSettings();
    /*! \brief save settings for dialog
     */
    void saveSettings();
    /*! \brief create tool buttons in dialog
     */
    void createToolButtons();
    /*! \brief called before dialog is closed
     */
    virtual void closeEvent(QCloseEvent* event);
    /*! \brief update the layer list widget from layer list
     */
    void updateLayerList();

private:
    size_t _current;
    Ui::LayerDialog *ui;
    ShipCAD::LayerDialogData* _data;
    QToolButton* _newToolButton;
    QToolButton* _removeEmptyToolButton;
    QToolButton* _moveUpToolButton;
    QToolButton* _moveDownToolButton;
    QAction* _colorAction;
    QAction* _newLayerAction;
    QAction* _deleteEmptyAction;
    QAction* _moveUpAction;
    QAction* _moveDownAction;
    ColorView* _colorView;
    QString _areastr;
    QString _weightstr;
    QString _lengthstr;
};

#endif // LAYERDIALOG_H
