/*##############################################################################################
 *    ShipCAD                                                                                  *
 *    Copyright 2015, by Greg Green <ggreen@bit-builder.com>                                   *
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

#ifndef FILEBUFFER_H_
#define FILEBUFFER_H_

#include <vector>
#include <QFile>
#include <QObject>
#include <QVector3D>
#include <QColor>
#include <QString>

#include "version.h"
#include "plane.h"
#include "resistance.h"

namespace ShipCAD {

struct JPEGImage;

//////////////////////////////////////////////////////////////////////////////////////

/*! \brief in-memory buffer for a binary file (FREE!Ship format)
 */
class FileBuffer : public QObject
{
    Q_OBJECT
public:

    explicit FileBuffer();
    ~FileBuffer() {}

    size_t size() {return _data.size();}
    size_t pos() {return _pos;}
    
    // version
    version_t getVersion() {return _file_version;}
    void setVersion(version_t v);
	
    // save/restore/reset
    void loadFromFile(QFile& file);
    void saveToFile(QFile& file);
    void reset();

    // load/add

	void load(JPEGImage& img);
	void add(const JPEGImage& img);
	
	void load(quint8& val);
	void add(quint8 val);
	
    void load(bool& val);
    void add(bool val);

    void load(float& val);
    void add(float val);

    void load(qint32& val);
    void add(qint32 val);

    void load(quint32& val);
    void add(quint32 val);

#if defined(_WIN32) && !defined(_WIN64)
    // don't define a size_t add, taken care of by quint32 add
#else
	/*! \brief save value, (check that it fits in 32 bits)
	 *
	 * \param val integer to save
	 * \throws range_error if val is greater than 32bit unsigned
	 */
	void add(size_t val);
#endif
    void load(QVector3D& val);
    void add(const QVector3D& val);

    void load(QColor& val);
    void add(const QColor& val);

    void load(QString& val);
    void add(const QString& val);
    void add(const char* str);

    void load(Plane& val);
    void add(const Plane& val);

	void load(DelftSeriesResistance* buf);
	void add(const DelftSeriesResistance* buf);

	void load(KAPERResistance* buf);
	void add(const KAPERResistance* buf);

private:

    size_t _pos;           // current position in the data vector
    version_t _file_version;
    std::vector<quint8> _data;   // the data
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif

