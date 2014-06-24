#include <iostream>
#include <fstream>

#include "filebuffer.h"
#include "utility.h"

using namespace std;
using namespace ShipCADGeometry;
using namespace ShipCADUtility;

// structure used to convert values from/to bytes
union convert_type_t {
    unsigned char d[8];
    int ival;
    size_t uval;
    float fval;
};

FileBuffer::FileBuffer()
    : _pos(0)
{
    // does nothing
}

FileBuffer::~FileBuffer()
{
    _data.clear();
}

void FileBuffer::reset()
{
    _pos = 0;
}

void FileBuffer::loadFromFile(const QString &filename)
{
    _pos = 0;
    _data.clear();
    ifstream ifile(filename.toStdString());
    unsigned char byte;
    while (ifile) {
        ifile >> byte;
        _data.push_back(byte);
    }
    ifile.close();
    cout << "Read " << _data.size() << " bytes from '" << filename.toStdString() << "'" << endl;
}

void FileBuffer::saveToFile(const QString &filename)
{
    ofstream ofile(filename.toStdString());
    for (size_t i=0; i<_data.size() && ofile; ++i)
        ofile << _data[i];
    ofile.close();
    cout << "Wrote " << _data.size() << " bytes to '" << filename.toStdString() << "'" << endl;
}

void FileBuffer::load(version_t& val)
{
    convert_type_t ct;
    for (int i=0; _pos<_data.size() && i<4; ++i,++_pos)
        ct.d[i] = _data[_pos];
    val = static_cast<version_t>(ct.ival);
}

void FileBuffer::add(version_t val)
{
    _data.push_back(static_cast<version_t>(val));
}

void FileBuffer::load(bool& val)
{
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
    convert_type_t ct;
    for (int i=0; _pos<_data.size() && i<4; ++i,++_pos)
        ct.d[i] = _data[_pos];
    val = ct.ival;
}

void FileBuffer::add(const QColor& c)
{
    convert_type_t ct;
    ct.ival = FindDXFColorIndex(c);
    for (int i=0; i<4; ++i)
        _data.push_back(ct.d[i]);
}

void FileBuffer::load(QColor& val)
{
    convert_type_t ct;
    for (int i=0; _pos<_data.size() && i<4; ++i,++_pos)
        ct.d[i] = _data[_pos];
    int index = ct.ival;
    val = QColorFromDXFIndex(index);
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
    convert_type_t ct;
    for (int i=0; _pos<_data.size() && i<4; ++i,++_pos)
        ct.d[i] = _data[_pos];
    val = ct.uval;
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
    convert_type_t ct;
    for (int i=0; _pos<_data.size() && i<4; ++i,++_pos)
        ct.d[i] = _data[_pos];
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
