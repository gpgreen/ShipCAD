/*#############################################################################################}
  {    This code is distributed as part of the FREE!ship project. FREE!ship is an               }
  {    open source surface-modelling program based on subdivision surfaces and intended for     }
  {    designing ships.                                                                         }
  {                                                                                             }
  {    Copyright © 2005, by Martijn van Engeland                                                }
  {    e-mail                  : Info@FREEship.org                                              }
  {    FREE!ship project page  : https://sourceforge.net/projects/freeship                      }
  {    FREE!ship homepage      : www.FREEship.org                                               }
  {                                                                                             }
  {    This program is free software; you can redistribute it and/or modify it under            }
  {    the terms of the GNU General Public License as published by the                          }
  {    Free Software Foundation; either version 2 of the License, or (at your option)           }
  {    any later version.                                                                       }
  {                                                                                             }
  {    This program is distributed in the hope that it will be useful, but WITHOUT ANY          }
  {    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A          }
  {    PARTICULAR PURPOSE. See the GNU General Public License for more details.                 }
  {                                                                                             }
  {    You should have received a copy of the GNU General Public License along with             }
  {    this program; if not, write to the Free Software Foundation, Inc.,                       }
  {    59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                                    }
  {                                                                                             }
  {#############################################################################################*/

#ifndef GEOMETRY_H_
#define GEOMETRY_H_

#include <vector>
#include <QtCore>
#include <QtGui>
#include "openglwindow.h"

namespace ShipCADGeometry {

class FileBuffer;
class Entity;

//////////////////////////////////////////////////////////////////////////////////////

class Plane : public QObject
{
    Q_OBJECT
public:
    explicit Plane();
    explicit Plane(float a, float b, float c, float d);
    ~Plane() {}

    QPair<QVector3D, QVector3D> vertex_normal() const;

private:

    float _vars[4];

};

//////////////////////////////////////////////////////////////////////////////////////

class IntersectionData : public QObject
{
    Q_OBJECT
public:

    explicit IntersectionData();
    ~IntersectionData() {}

private:

    int _number_of_intersections;
    std::vector<QVector3D> _points;
    std::vector<float> _parameters;

};

//////////////////////////////////////////////////////////////////////////////////////

class Viewport : public OpenGLWindow
{
    Q_OBJECT
public:

    explicit Viewport();
    ~Viewport();

    enum ViewportMode {vmWireFrame, vmShade, vmShadeGauss, vmShadeDevelopable, vmShadeZebra};

    void initialize();
    void render();

    enum ViewportMode getViewportMode() const;
    void setViewportMode(enum ViewportMode mode);

    void add(Entity* entity);

private:

    GLuint loadShader(GLenum type, const char *source);

    enum ViewportMode _mode;

    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_matrixUniform;

    QOpenGLShaderProgram *m_program;
    int m_frame;

    std::vector<Entity*> _entities;
};

//////////////////////////////////////////////////////////////////////////////////////

class Entity : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool Build READ getBuild WRITE setBuild)
    Q_PROPERTY(QColor Color MEMBER _color)
    Q_PROPERTY(QVector3D Min READ getMin)
    Q_PROPERTY(QVector3D Max READ getMax)
    Q_PROPERTY(int PenWidth MEMBER _pen_width)
    Q_PROPERTY(Qt::PenStyle penstyle MEMBER _pen_style)

public:

    explicit Entity();
    // copy constructor
    //Entity(const Entity& entity);
    // assignment operator
    //Entity& operator=(const Entity& entity);
    virtual ~Entity() {}

    virtual void clear();
    virtual void extents(QVector3D& min, QVector3D& max);
    virtual void draw(Viewport& vp) = 0;
    virtual void rebuild() = 0;

    virtual QVector3D getMin();
    virtual QVector3D getMax();
    bool getBuild() const;
    
protected:

    virtual void setBuild(bool val);

protected:

    bool _build;
    QVector3D _min;
    QVector3D _max;
    int _pen_width;
    QColor _color;
    Qt::PenStyle _pen_style;


};

//////////////////////////////////////////////////////////////////////////////////////

class Spline : public Entity
{
    Q_OBJECT
    Q_PROPERTY(bool ShowCurvature MEMBER _show_curvature)
    Q_PROPERTY(bool ShowPoints MEMBER _show_points)
    Q_PROPERTY(QColor CurvatureColor MEMBER _curvature_color)
    Q_PROPERTY(float CurvatureScale MEMBER _curvature_scale)

public:

    explicit Spline();
    // copy constructor
    //Spline(const Spline& spline);
    // assignment operator
    //Spline& operator=(const Spline& spline);
    virtual ~Spline();
    
    // altering
    void add(const QVector3D& p);
    void delete_point(int index);
    void insert(int index, const QVector3D& p);
    void insert_spline(int index, bool invert, bool duplicate_point, const Spline& source);
    void invert_direction();
    bool simplify(float criterium);
    virtual void clear();
    virtual void rebuild();

    // geometry ops
    float coord_length(float t1, float t2);
    float chord_length_approximation(float percentage);
    float curvature(float parameter, QVector3D& normal);
    QVector3D first_derive(float parameter);
    QVector3D second_derive(float parameter);
    bool intersect_plane(const Plane& plane, IntersectionData& output) const;
    QVector3D value(float parameter);

    // persistence
    virtual void load_binary(FileBuffer& source);
    virtual void save_binary(FileBuffer& destination);
    void save_to_dxf(std::vector<QString>& strings, QString layername, bool sendmirror);

    // drawing
    int distance_to_cursor(int x, int y, Viewport& vp) const;
    virtual void draw(Viewport& vp);

    // getters/setters

    float getParameter(int index) const;

    QVector3D getPoint(int index) const;
    void setPoint(int index, const QVector3D& p);

    int getFragments() const;
    void setFragments(int val);

    bool getKnuckle(int index) const;
    void setKnuckle(int index, bool val);

protected:

    void setBuild(bool val);

    // methods used in simplify
    float weight(int index) const;
    int find_next_point(std::vector<float>& weights) const;

protected:

    int _nopoints;
    int _fragments;
    bool _show_curvature;
    bool _show_points;
    float _curvature_scale;
    float _total_length;
    QColor _curvature_color;
    std::vector<QVector3D> _points;
    std::vector<bool> _knuckles;
    std::vector<float> _parameters;
    std::vector<QVector3D> _derivatives;
};

//////////////////////////////////////////////////////////////////////////////////////

class FileBuffer : public QObject
{
    Q_OBJECT
public:

    explicit FileBuffer();
    ~FileBuffer() {}

private:

};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif

