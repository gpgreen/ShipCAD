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

#include <QFile>
#include <QDataStream>
#include <iostream>
#include <stdexcept>

#include "filebuffer.h"
#include "utility.h"

using namespace std;
using namespace ShipCAD;

// structure used to convert values from/to bytes
union convert_type_t {
    unsigned char d[4];
    int ival;
    size_t uval;
    float fval;
};

FileBuffer::FileBuffer()
    : _pos(0), _file_version(k_current_version)
{
    // does nothing
}

void FileBuffer::reset()
{
    _pos = 0;
}

void FileBuffer::setVersion(version_t v)
{
    _file_version = v;
}

void FileBuffer::loadFromFile(const QString &filename)
{
    _pos = 0;
    _data.clear();
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly))
		return;
	QDataStream in(&file);
    quint8 byte;
    while (!in.atEnd()) {
        in >> byte;
        _data.push_back(byte);
    }
    file.close();
    cout << "Read " << _data.size() << " bytes from '" << filename.toStdString() << "'" << endl;
}

void FileBuffer::saveToFile(const QString &filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
		return;
	QDataStream out(&file);
    for (size_t i=0; i<_data.size(); ++i)
        out << _data[i];
    file.close();
    cout << "Wrote " << _data.size() << " bytes to '" << filename.toStdString() << "'" << endl;
}

void FileBuffer::load(JPEGImage& img)
{
    load(img.width);
    load(img.height);
    load(img.size);
	for (size_t i=0; i<img.size; i++, _pos++)
        img.data.push_back(_data[_pos]);
}

void FileBuffer::add(const JPEGImage& img)
{
	throw runtime_error("not implemented");
}
	
void FileBuffer::load(quint8& val)
{
	cout << "pos:" << _pos << endl;
	val = _data[_pos++];
}

void FileBuffer::add(quint8 val)
{
    _data.push_back(val);
}

void FileBuffer::load(bool& val)
{
	cout << "pos:" << _pos << endl;
    convert_type_t ct;
    ct.d[0] = _data[_pos++];
    val = (ct.d[0] == 0) ? false : true;
}

void FileBuffer::add(bool val)
{
    _data.push_back((val ? 0x1 : 0x0));
}

void FileBuffer::load(float& val)
{
	cout << "pos:" << _pos << endl;
    convert_type_t ct;
    for (int i=0; _pos<_data.size() && i<4; ++i,++_pos)
        ct.d[i] = _data[_pos];
    val = ct.fval;
}

void FileBuffer::add(float val)
{
    convert_type_t ct;
    ct.fval = val;
    for (int i=0; i<4; ++i)
        _data.push_back(ct.d[i]);
}

void FileBuffer::load(int& val)
{
	cout << "pos:" << _pos << endl;
    convert_type_t ct;
    for (int i=0; _pos<_data.size() && i<4; ++i,++_pos)
        ct.d[i] = _data[_pos];
    val = ct.ival;
}

void FileBuffer::add(const QColor& c)
{
	_data.push_back(static_cast<quint8>(c.red()));
	_data.push_back(static_cast<quint8>(c.green()));
	_data.push_back(static_cast<quint8>(c.blue()));
	_data.push_back(static_cast<quint8>(c.alpha()));
}

void FileBuffer::load(QColor& val)
{
	cout << "pos:" << _pos << endl;
	quint8 r = _data[_pos++];
	quint8 g = _data[_pos++];
	quint8 b = _data[_pos++];
	quint8 a = _data[_pos++];
    val = QColor(r, g, b, a);
}

void FileBuffer::add(int val)
{
    convert_type_t ct;
    ct.ival = val;
    for (int i=0; i<4; ++i)
        _data.push_back(ct.d[i]);
}

void FileBuffer::load(size_t& val)
{
	cout << "pos:" << _pos << endl;
	quint8 d[4];
    for (int i=0; _pos<_data.size() && i<4; ++i,++_pos)
        d[i] = _data[_pos];
    val = *reinterpret_cast<size_t*>(&d);
}

void FileBuffer::add(size_t val)
{
    convert_type_t ct;
    ct.uval = val;
    for (int i=0; i<4; ++i)
        _data.push_back(ct.d[i]);
}

void FileBuffer::load(QVector3D& val)
{
	cout << "pos:" << _pos << endl;
    convert_type_t ct;
    for (int i=0; _pos<_data.size() && i<4; ++i,++_pos)
        ct.d[i] = _data[_pos];
    val.setX(ct.fval);
    for (int i=0; _pos<_data.size() && i<4; ++i,++_pos)
        ct.d[i] = _data[_pos];
    val.setY(ct.fval);
    for (int i=0; _pos<_data.size() && i<4; ++i,++_pos)
        ct.d[i] = _data[_pos];
    val.setZ(ct.fval);
}

void FileBuffer::add(const QVector3D& val)
{
    convert_type_t ct;
    ct.fval = val.x();
    for (int i=0; i<4; ++i)
        _data.push_back(ct.d[i]);
    ct.fval = val.y();
    for (int i=0; i<4; ++i)
        _data.push_back(ct.d[i]);
    ct.fval = val.z();
    for (int i=0; i<4; ++i)
        _data.push_back(ct.d[i]);
}

void FileBuffer::load(QString& val)
{
	cout << "pos:" << _pos << endl;
    convert_type_t ct;
    for (int i=0; _pos<_data.size() && i<4; ++i,++_pos)
        ct.d[i] = _data[_pos];
	cout << "len:" << ct.ival << endl;
    char *buf = new char [ct.ival];
    for (int i=0; _pos<_data.size() && i<ct.ival; ++i,++_pos)
        buf[i] = _data[_pos];
    // convert char buffer (which was stored as utf8), to unicode
    val = QString::fromUtf8(buf, ct.ival);
    delete [] buf;
}

void FileBuffer::add(const QString& val)
{
    // convert string to utf8
    QByteArray s = val.toUtf8();
    convert_type_t ct;
    ct.ival = s.length();
    for (int i=0; i<4; ++i)
        _data.push_back(ct.d[i]);
    for (int i=0; i<s.length(); ++i)
        _data.push_back(s[i]);
}

void FileBuffer::load(Plane& val)
{
	cout << "pos:" << _pos << endl;
    convert_type_t ct;
    for (int i=0; _pos<_data.size() && i<4; ++i,++_pos)
        ct.d[i] = _data[_pos];
    val.setA(ct.fval);
    for (int i=0; _pos<_data.size() && i<4; ++i,++_pos)
        ct.d[i] = _data[_pos];
    val.setB(ct.fval);
    for (int i=0; _pos<_data.size() && i<4; ++i,++_pos)
        ct.d[i] = _data[_pos];
    val.setC(ct.fval);
    for (int i=0; _pos<_data.size() && i<4; ++i,++_pos)
        ct.d[i] = _data[_pos];
    val.setD(ct.fval);
}

void FileBuffer::add(const Plane& val)
{
    convert_type_t ct;
    ct.fval = val.a();
    for (int i=0; i<4; ++i)
        _data.push_back(ct.d[i]);
    ct.fval = val.b();
    for (int i=0; i<4; ++i)
        _data.push_back(ct.d[i]);
    ct.fval = val.c();
    for (int i=0; i<4; ++i)
        _data.push_back(ct.d[i]);
    ct.fval = val.d();
    for (int i=0; i<4; ++i)
        _data.push_back(ct.d[i]);
}
