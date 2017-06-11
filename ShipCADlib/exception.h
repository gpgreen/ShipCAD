/*##############################################################################################
 *    ShipCAD
 *    Copyright 2015, by Greg Green <ggreen@bit-builder.com>
 *    Original Copyright header below
 *
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

#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <stdexcept>
#include <string>
#include <QString>

namespace ShipCAD {

    class ParseError : public std::runtime_error
    {
    public:
        ParseError(size_t linenum, const QString& info)
            : std::runtime_error(""), _lineno(linenum), _info(info) {}

        const QString& info() const {return _info;}
        size_t lineno() const {return _lineno;}

    private:
        size_t _lineno;
        QString _info;
    };

    class FileSaveError : public std::runtime_error
    {
    public:
      FileSaveError(const QString& info)
	: std::runtime_error(""), _info(info) {}

      const QString& info() const {return _info;}

    private:
      QString _info;
    };

    class FileReadError : public std::runtime_error
    {
    public:
      FileReadError(const QString& info)
	: std::runtime_error(""), _info(info) {}

      const QString& info() const {return _info;}

    private:
      QString _info;
    };

};

#endif // EXCEPTION_H
