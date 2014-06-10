#include "filebuffer.h"

using namespace std;
using namespace ShipCADGeometry;

// structure used to convert values from/to bytes
union convert_type_t {
    unsigned char d[8];
    int ival;
    size_t uval;
    float fval;
};

FileBuffer::FileBuffer()
    : _fb(0), _pos(0)
{
    // does nothing
}

FileBuffer::~FileBuffer()
{
    delete _fb;
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
    // BUGBUG: convert to utf8 before saving
    convert_type_t ct;
    for (int i=0; _pos<_data.size() && i<4; ++i,++_pos)
        ct.d[i] = _data[_pos];
    char *buf = new char [ct.ival];
    for (int i=0; _pos<_data.size() && i<ct.ival; ++i,++_pos)
        buf[i] = _data[_pos];
    val = QString(buf);
    delete [] buf;
}

void FileBuffer::add(const QString& val)
{
    convert_type_t ct;
    ct.ival = val.length();
    for (int i=0; i<4; ++i)
        _data.push_back(ct.d[i]);
    QByteArray ba = val.toUtf8();
    for (int i=0; i<ba.length(); ++i)
        _data.push_back(ba[i]);
}
