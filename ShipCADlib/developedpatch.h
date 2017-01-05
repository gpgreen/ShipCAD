/*###############################################################################################
 *    ShipCAD																					*
 *    Copyright 2017, by Greg Green <ggreen@bit-builder.com>									*
 *    Original Copyright header below															*
 *																								*
 *    This code is distributed as part of the FREE!ship project. FREE!ship is an                *
 *    open source surface-modelling program based on subdivision surfaces and intended for      *
 *    designing ships.                                                                          *
 *                                                                                              *
 *    Copyright © 2005, by Martijn van Engeland                                                 *
 *    e-mail                  : Info@FREEship.org                                               *
 *    FREE!ship project page  : https://sourceforge.net/projects/freeship                       *
 *    FREE!ship homepage      : www.FREEship.org                                                *
 *                                                                                              *
 *    This program is free software; you can redistribute it and/or modify it under             *
 *    the terms of the GNU General Public License as published by the                           *
 *    Free Software Foundation; either version 2 of the License, or (at your option)            *
 *    any later version.                                                                        *
 *                                                                                              *
 *    This program is distributed in the hope that it will be useful, but WITHOUT ANY           *
 *    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A           *
 *    PARTICULAR PURPOSE. See the GNU General Public License for more details.                  *
 *                                                                                              *
 *    You should have received a copy of the GNU General Public License along with              *
 *    this program; if not, write to the Free Software Foundation, Inc.,                        *
 *    59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                                     *
 *                                                                                              *
 *##############################################################################################*/

#ifndef DEVELOPEDPATCH_H_
#define DEVELOPEDPATCH_H_

#include <QObject>
#include <QVector2D>
#include <QVector3D>
#include <QString>
#include <vector>
#include <iosfwd>
#include "plane.h"
#include "spline.h"

namespace ShipCAD {

class Viewport;
class LineShader;
class SubdivisionLayer;
class SubdivisionFace;
class SubdivisionControlFace;
class SubdivisionEdge;
class SubdivisionPoint;
    
//////////////////////////////////////////////////////////////////////////////////////

/*! \brief class to store SubdivisionPoint to unrolled point mapping
 */
struct PatchPoints
{
    QVector2D pt2D;
    SubdivisionPoint* pt;
    bool processed;
};

typedef std::vector<PatchPoints>::iterator patchpt_iter;
typedef std::vector<PatchPoints>::const_iterator const_patchpt_iter;

enum PolygonOrientation {poCCW, poCW};
    
/*! \brief base class for all non-surface drawable elements
 */
class DevelopedPatch : public QObject
{
    Q_OBJECT

public:

    explicit DevelopedPatch(SubdivisionLayer* layer);
    virtual ~DevelopedPatch() {clear();}

    void assign(DevelopedPatch* org, bool mirror);
    
    virtual void clear();
    virtual void extents(QVector3D& min, QVector3D& max);
    virtual void draw(Viewport& vp, LineShader* lineshader);

    SubdivisionLayer* getOwner()
        {return _owner;}

    void intersectPlane(Plane& plane, QColor color);
    QVector3D convertTo3D(QVector2D p);
    void saveToDXF(QStringList& strings);
    void saveToTextFile(QStringList& strings);
    void unroll(std::vector<SubdivisionControlFace*> controlfaces);
    
    // getters/setters
    double totalAreaError()
        {return _totalAreaError;}
    double maxAreaError()
        {return _maxAreaError;}
    double maxError();
    QVector2D midPoint();
    double minError();
    const QString& name()
        {return _name;}
    void setName(const QString& nm)
        {_name = nm;}
    int numberOfIterations()
        {return _numIterations;}
    QVector3D getPoint(size_t index);
    QVector3D getMirrorPoint(size_t index);

    float rotation()
        {return _rotation;}
    void setRotation(float val);
    QVector2D translation()
        {return _translation;}
    void setTranslation(QVector2D val)
        {_translation = val;}
    bool isMirror()
        {return _mirror;}
    void setMirror(bool set)
        {_mirror = set;}
    bool mirrorOnScreen()
        {return _mirrorOnScreen;}
    void setMirrorOnScreen(bool set);
    bool isShadeSubmerged()
        {return _shadeSubmerged;}
    void setShadeSubmerged(bool set)
        {_shadeSubmerged = set;}
    bool showBoundingBox()
        {return _showBoundingBox;}
    void setShowBoundingBox(bool set)
        {_showBoundingBox = set;}
    bool showButtocks()
        {return _showButtocks;}
    void setShowButtocks(bool set)
        {_showButtocks = set;}
    bool showDiagonals()
        {return _showDiagonals;}
    void setShowDiagonals(bool set)
        {_showDiagonals = set;}
    bool showDimensions()
        {return _showDimensions;}
    void setShowDimensions(bool set)
        {_showDimensions = set;}
    bool showErrorEdges()
        {return _showErrorEdges;}
    void setShowErrorEdges(bool set)
        {_showErrorEdges = set;}
    bool showInteriorEdges()
        {return _showInteriorEdges;}
    void setShowInteriorEdges(bool set)
        {_showInteriorEdges = set;}
    bool showPartName()
        {return _showPartName;}
    void setShowPartName(bool set)
        {_showPartName = set;}
    bool showSolid()
        {return _showSolid;}
    void setShowSolid(bool set)
        {_showSolid = set;}
    bool showStations()
        {return _showStations;}
    void setShowStations(bool set)
        {_showStations = set;}
    bool showWaterlines()
        {return _showWaterlines;}
    void setShowWaterlines(bool set)
        {_showWaterlines = set;}
    
    // output
    void dump(std::ostream& os) const;

protected:

    // used in saveToDXF
    void exportSpline(QStringList& strings, Spline* spline, const QString& layername);

    // used in unroll

    // Computes the crossproduct of three points
    // Returns whether their internal angle is clockwise or counter-clockwise.
    PolygonOrientation crossproduct(const QVector2D& p1, const QVector2D& p2,
                                    const QVector2D& p3);
    
    // Calculates the third point of a triangle when the length of its
    // three sides and two coordinates are known
    QVector2D calculateTriangle(double a, double b, double c,
                                const QVector2D& p1, const QVector2D& p2);
    QVector2D calculateTriangle2(double a, double b, double c,
                                 const QVector2D& p1, const QVector2D& p2);
    void unroll2D(SubdivisionFace* face, bool& firstface, bool& error,
                  PolygonOrientation& orientation);
    void processTriangle(SubdivisionPoint* p1,
                         SubdivisionPoint* p2,
                         SubdivisionPoint* p3,
                         patchpt_iter& ind1,
                         patchpt_iter& ind2,
                         patchpt_iter& ind3,
                         bool& first,
                         bool& error,
                         PolygonOrientation& orientation);
    double triangleArea(const QVector2D& p1, const QVector2D& p2, const QVector2D& p3);
    void processFaces(SubdivisionFace* seedface, double& maxerror,
                      std::vector<SubdivisionFace*>::iterator& error_index,
                      std::vector<SubdivisionFace*>& faces);
    
protected:

    SubdivisionLayer* _owner;
    QString _name;
    DevelopedPatch* _connectedMirror;
    std::vector<PatchPoints> _points;
    std::vector<SubdivisionEdge*> _edges;
    std::vector<SubdivisionEdge*> _boundaryEdges;
    std::vector<double> _edgeErrors;
    std::vector<SubdivisionFace*> _donelist;
    SplineVector _stations;
    SplineVector _waterlines;
    SplineVector _buttocks;
    SplineVector _diagonals;
    std::vector<SubdivisionPoint*> _corners;
    
    bool _visible;
    bool _mirror;
    bool _showSolid;
    bool _showPartName;
    bool _showBoundingBox;
    bool _showInteriorEdges;
    bool _showStations;
    bool _showButtocks;
    bool _showDiagonals;
    bool _showWaterlines;
    bool _showErrorEdges;
    bool _showDimensions;
    bool _mirrorOnScreen;
    bool _shadeSubmerged;
    
    int _numIterations;
    float _rotation;
    Plane _mirrorPlane;
    QVector2D _min2D;
    QVector2D _max2D;
    QVector2D _translation;
    double _maxAreaError;
    double _totalAreaError;
    float _xgrid;
    float _ygrid;
    float _cos;
    float _sin;
    // _units;
};

};				/* end namespace */

std::ostream& operator << (std::ostream& os, const ShipCAD::DevelopedPatch& patch);

//////////////////////////////////////////////////////////////////////////////////////

#endif

