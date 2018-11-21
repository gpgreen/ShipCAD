/*##############################################################################################
 *    ShipCAD                                                                                  *
 *    Copyright 2018, by Greg Green <ggreen@bit-builder.com>                                   *
 *    Original Copyright header below                                                          *
 *                                                                                             *
 *    This code is distributed as part of the FREE!ship project. FREE!ship is an               *
 *    open source surface-modelling program based on subdivision surfaces and intended for     *
 *    designing ships.                                                                         *
 *                                                                                             *
 *    Copyright © 2005, by Martijn van Engeland                                                *
 *    e-mail                  : Info@FREEship.org                                              *
 *    FREE!ship project page  : https://sourceforge.net/projects/freeship                      *
 *    FREE!ship homepage      : www.FREEship.org                                               *
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

#ifndef IGES_H_
#define IGES_H_

#include <iosfwd>
#include <QObject>
#include <QString>
#include <QStringList>
#include "shipcadlib.h"
#include "grid.h"

namespace ShipCAD {

// forward class definitions
class NURBSurface;
class ShipCADModel;
class SubdivisionPoint;
    
//////////////////////////////////////////////////////////////////////////////////////

class IGES : public QObject
{
    Q_OBJECT

public:

    explicit IGES(ShipCADModel* model, bool minimize_faces, bool send_triangles);
    virtual ~IGES() {}

    void addEntity128(NURBSurface& nurb, size_t color_index);
    size_t addEntity314(QColor col);
    
    // altering
    virtual void clear();

    // getters/setters
    const QString& getFileCreatedBy() const
        { return _file_created_by;}
    const QString& getFileName() const
        { return _file_name;}
    const QString& getSystemID() const
        { return _system_id; }
    unit_type_t getIGESUnits() const
        { return _iges_units; }

    // output
    void saveToFile(const QString& filename);
    
    void dump(std::ostream& os) const;

protected:

    void processParameterData(const QString& str, QStringList& param_data);
    void processGrid(ShipCAD::Grid<ShipCAD::SubdivisionPoint*>& grid, size_t color_index);
    
protected:

    QStringList _start_section;
    QStringList _global_section;
    QStringList _directory_section;
    QStringList _parameter_section;
    QStringList _terminate_section;
    size_t _num_surfaces;
    unit_type_t _iges_units;
    float _max_coordinate;
    QString _system_id;
    QString _file_created_by;
    QString _file_name;
    ShipCADModel* _model;
    bool _minimize_faces;
    bool _send_triangles;
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

std::ostream& operator << (std::ostream& os, const ShipCAD::IGES& iges);

#endif

