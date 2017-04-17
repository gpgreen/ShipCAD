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
