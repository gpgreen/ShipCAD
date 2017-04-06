/*##############################################################################################
 *    ShipCAD																				   *
 *    Copyright 2015, by Greg Green <ggreen@bit-builder.com>								   *
 *    Original Copyright header below														   *
 *																							   *
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

#include "undoobject.h"
#include "shipcadmodel.h"

using namespace ShipCAD;
using namespace std;

UndoObject::UndoObject(ShipCADModel* owner, const QString& filename,
                       edit_mode_t mode, bool file_changed, bool filename_set,
                       bool is_temp_redo_obj)
    : _owner(owner), _file_changed(file_changed), _filename_set(filename_set),
      _filename(filename), _edit_mode(mode), _time(QTime::currentTime()),
      _is_temp_redo_obj(is_temp_redo_obj)
{
    // does nothing
}

// FreeShipUnit.pas:1011
size_t UndoObject::getMemory()
{
    return sizeof(this) + _undo_text.size() + _filename.size() + _undo_data.size();
}

// FreeShipUnit.pas:1030
void UndoObject::accept()
{
    _owner->acceptUndo(this);
}

// FreeShipUnit.pas:1092
void UndoObject::restore()
{
    _owner->loadBinary(_undo_data);
	_owner->setFileChanged(_file_changed);
	_owner->setFilename(_filename);
	_owner->setEditMode(_edit_mode);
	_owner->setFilenameSet(_filename_set);
}
