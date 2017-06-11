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

#include <QFile>
#include <QDataStream>
#include <iostream>
#include <stdexcept>
#include <climits>

#include "filebuffer.h"
#include "utility.h"
#include "backgroundimage.h"
#include "exception.h"

using namespace std;
using namespace ShipCAD;

// structure used to convert values from/to bytes
union convert_type_t {
    unsigned char d[4];
    qint32 ival;
    quint32 uval;
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

void FileBuffer::loadFromFile(QFile &file)
{
    _pos = 0;
    _data.clear();
	if (!file.open(QIODevice::ReadOnly))
		throw FileReadError("unable to open file for reading");
	_data.reserve(file.size());
	QDataStream in(&file);
    quint8 byte;
    while (!in.atEnd()) {
        in >> byte;
        _data.push_back(byte);
    }
    file.close();
    cout << "Read " << _data.size() << " bytes from '" << file.fileName().toStdString() << "'" << endl;
}

void FileBuffer::saveToFile(QFile& file)
{
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
		throw FileSaveError("can't open file for write");
    QDataStream out(&file);
    for (size_t i=0; i<_data.size(); ++i)
        out << _data[i];
    file.close();
    cout << "Wrote " << _data.size() << " bytes to '" << file.fileName().toStdString() << "'" << endl;
}

void FileBuffer::load(JPEGImage& img)
{
    load(img.width);
    load(img.height);
    load(img.size);
    if (img.size > INT_MAX || (img.size + _pos) > _data.size())
        throw runtime_error("jpeg image size too large");
    img.data.reserve(img.size);
    img.data.insert(img.data.begin(), _data.begin()+_pos, _data.begin()+_pos+img.size);
    _pos += img.size;
}

void FileBuffer::add(const JPEGImage& img)
{
    add(img.width);
    add(img.height);
    add(img.size);
    _data.insert(_data.end(), img.data.begin(), img.data.end());
    _pos += img.size;
}

void FileBuffer::load(quint8& val)
{
    //cout << "pos:" << _pos << endl;
	val = _data[_pos++];
}

void FileBuffer::add(quint8 val)
{
    _data.push_back(val);
}

void FileBuffer::load(bool& val)
{
    //cout << "pos:" << _pos << endl;
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
    //cout << "pos:" << _pos << endl;
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

void FileBuffer::load(qint32& val)
{
    //cout << "pos:" << _pos << endl;
    convert_type_t ct;
    for (int i=0; _pos<_data.size() && i<4; ++i,++_pos)
        ct.d[i] = _data[_pos];
    val = ct.ival;
}

void FileBuffer::add(qint32 val)
{
    convert_type_t ct;
    ct.ival = val;
    for (int i=0; i<4; ++i)
        _data.push_back(ct.d[i]);
}

void FileBuffer::load(quint32& val)
{
	convert_type_t ct;
    //cout << "pos:" << _pos << endl;
    for (int i=0; _pos<_data.size() && i<4; ++i,++_pos)
        ct.d[i] = _data[_pos];
    val = ct.uval;
}

void FileBuffer::add(quint32 val)
{
    convert_type_t ct;
    ct.uval = val;
    for (int i=0; i<4; ++i)
        _data.push_back(ct.d[i]);
}

#ifndef _WIN32
void FileBuffer::add(size_t val)
{
	if (val > ULONG_MAX)
		throw range_error("integer overflow");
    convert_type_t ct;
    ct.uval = val;
    for (int i=0; i<4; ++i)
        _data.push_back(ct.d[i]);
}
#endif

void FileBuffer::add(const QColor& c)
{
	_data.push_back(static_cast<quint8>(c.red()));
	_data.push_back(static_cast<quint8>(c.green()));
	_data.push_back(static_cast<quint8>(c.blue()));
	_data.push_back(static_cast<quint8>(c.alpha()));
}

void FileBuffer::load(QColor& val)
{
    //cout << "pos:" << _pos << endl;
	quint8 r = _data[_pos++];
	quint8 g = _data[_pos++];
	quint8 b = _data[_pos++];
	quint8 a = _data[_pos++];
    val = QColor(r, g, b, a);
}

void FileBuffer::load(QVector3D& val)
{
    //cout << "pos:" << _pos << endl;
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
    //cout << "pos:" << _pos << endl;
    convert_type_t ct;
    for (int i=0; _pos<_data.size() && i<4; ++i,++_pos)
        ct.d[i] = _data[_pos];
    //cout << "len:" << ct.ival << endl;
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

void FileBuffer::add(const char* str)
{
    // convert string to utf8
    QString s1(str);
    QByteArray s = s1.toUtf8();
    convert_type_t ct;
    ct.ival = s.length();
    for (int i=0; i<4; ++i)
        _data.push_back(ct.d[i]);
    for (int i=0; i<s.length(); ++i)
        _data.push_back(s[i]);
}

void FileBuffer::load(Plane& val)
{
    //cout << "pos:" << _pos << endl;
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

void FileBuffer::load(DelftSeriesResistance* buf)
{
    //cout << "pos:" << _pos << " sz:" << sizeof(DelftSeriesResistance) << endl;
    char * vbuf = (char *)buf;
	for (size_t i=0; i<sizeof(DelftSeriesResistance); i++)
		vbuf[i] = _data[_pos++];
}

void FileBuffer::add(const DelftSeriesResistance* buf)
{
	const char * vbuf = (const char *)&buf;
	for (size_t i=0; i<sizeof(DelftSeriesResistance); i++)
		_data.push_back(vbuf[i]);
}

void FileBuffer::load(KAPERResistance* buf)
{
    //cout << "pos:" << _pos << " sz:" << sizeof(KAPERResistance) << endl;
    char * vbuf = (char *)buf;
	for (size_t i=0; i<sizeof(KAPERResistance); i++)
		vbuf[i] = _data[_pos++];
}

void FileBuffer::add(const KAPERResistance* buf)
{
	const char * vbuf = (const char *)&buf;
	for (size_t i=0; i<sizeof(KAPERResistance); i++)
		_data.push_back(vbuf[i]);
}
