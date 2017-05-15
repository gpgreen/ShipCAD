/*##############################################################################################
 *    ShipCAD																				   *
 *    Copyright 2017, by Greg Green <ggreen@bit-builder.com>								   *
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

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <vector>
#include <QDialog>
#include <QSignalMapper>
#include "dialogdata.h"
#include "colorview.h"

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent = 0);
    ~PreferencesDialog();

    void initialize(ShipCAD::PreferencesDialogData* data);

signals:
    
    void exeChooseColorDialog(ShipCAD::ChooseColorDialogData& data);
    void reset();
                                                                   
public slots:

    void colorClicked(int id);
    
protected:

    /*! \brief initialize color structures
     */
    void initMembers();
    
    /*! \brief read stored settings for dialog
     */
    void readSettings();
    /*! \brief save settings for dialog
     */
    void saveSettings();
    /*! \brief called before dialog is closed
     */
    virtual void closeEvent(QCloseEvent* event);

private:
    Ui::PreferencesDialog *ui;
    QSignalMapper* _mapper;
    std::vector<std::pair<QFrame*, ColorView*> > _color_views;
    ShipCAD::PreferencesDialogData* _data;
};

#endif // PREFERENCESDIALOG_H
