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

size_t UndoObject::getMemory()
{
    return sizeof(this) + _undo_text.size() + _filename.size() + _undo_data.size();
}

// TODO this should be in the ship model object, not here
void UndoObject::accept()
{
    if (_owner->undoCount() > 0) {
        UndoObject* last = _owner->getUndoObject(_owner->undoCount() - 1);
        if (last->isTempRedoObj()) {
            _owner->deleteUndoObject(last);
            delete last;
        }
    }
    // delete all undo objects after the current one
    for (size_t i=_owner->undoCount(); i>_owner->undoPosition()+1; i--) {
        UndoObject* last = _owner->getUndoObject(i-1);
        _owner->deleteUndoObject(last);
        delete last;
    }
    _owner->addUndoObject(this);
    _owner->setUndoPosition(_owner->undoCount());
    // remove objects from the front of the list until memory is within limits or we have 2 items
    while (_owner->undoCount() > 2 &&
           (_owner->getUndoMemory() / (1024*1024)) > _owner->getPreferences().getMaxUndoMemory()) {
        UndoObject* first = _owner->getUndoObject(0);
        _owner->deleteUndoObject(first);
        delete first;
        _owner->setUndoPosition(_owner->undoPosition()-1);
        _owner->setPrevUndoPosition(_owner->prevUndoPosition()-1);
    }
}

// TODO this should be in the ship model object, not here
void UndoObject::restore()
{
    _owner->loadBinary(_undo_data);
	_owner->setFileChanged(_file_changed);
	_owner->setFilename(_filename);
	_owner->setEditMode(_edit_mode);
	_owner->setFilenameSet(_filename_set);
	_owner->redraw();
}
