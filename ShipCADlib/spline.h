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

#ifndef SPLINE_H_
#define SPLINE_H_

#include <vector>
#include <iosfwd>
#include <QObject>
#include <QVector3D>
#include <QColor>
#include <QString>
#include "entity.h"

namespace ShipCADGeometry {

class FileBuffer;
class Plane;
class IntersectionData;

//////////////////////////////////////////////////////////////////////////////////////

class Spline : public Entity
{
    Q_OBJECT
    Q_PROPERTY(size_t Fragments READ getFragments WRITE setFragments)
    Q_PROPERTY(bool ShowCurvature MEMBER _show_curvature)
    Q_PROPERTY(bool ShowPoints MEMBER _show_points)
    Q_PROPERTY(float CurvatureScale MEMBER _curvature_scale)
    Q_PROPERTY(QColor CurvatureColor MEMBER _curvature_color)

public:

    explicit Spline();
    virtual ~Spline();
    
    // altering
    void add(const QVector3D& p);
    void delete_point(size_t index);
    void insert(size_t index, const QVector3D& p);
    void insert_spline(size_t index, bool invert, bool duplicate_point, 
		       const Spline& source);
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
    bool intersect_plane(const Plane& plane, IntersectionData& output);
    QVector3D value(float parameter);

    // persistence
    virtual void load_binary(FileBuffer& source);
    virtual void save_binary(FileBuffer& destination);
    void save_to_dxf(std::vector<QString>& strings, QString layername, 
		     bool sendmirror);

    // drawing
    //int distance_to_cursor(int x, int y, Viewport& vp) const;
    virtual void draw(Viewport& vp);

    // getters/setters

    float getParameter(size_t index);
    QVector3D getPoint(size_t index);
    void setPoint(size_t index, const QVector3D& p);
    int getFragments();
    void setFragments(size_t val);
    bool getKnuckle(size_t index);
    void setKnuckle(size_t index, bool val);

    // output
    void dump(std::ostream& os) const;

protected:

    void setBuild(bool val);

    // methods used in simplify
    float weight(size_t index);
    std::vector<float>::iterator find_next_point(std::vector<float>& weights);

    // method used in intersect_plane
    void add_to_output(const QVector3D& p, float parameter, 
		       IntersectionData& output);

protected:

    size_t _nopoints;
    size_t _fragments;
    bool _show_curvature;
    bool _show_points;
    float _curvature_scale;
    QColor _curvature_color;
    std::vector<QVector3D> _points;
    std::vector<bool> _knuckles;

    // cached elements
    float _total_length;
    std::vector<float> _parameters;
    std::vector<QVector3D> _derivatives;
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

std::ostream& operator << (std::ostream& os, const ShipCADGeometry::Spline& spline);

#endif

