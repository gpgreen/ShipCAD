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

#include <iostream>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>

#include "controller.h"
#include "shipcadmodel.h"
#include "utility.h"
#include "undoobject.h"
#include "subdivpoint.h"
#include "subdivedge.h"
#include "viewport.h"
#include "viewportview.h"
#include "subdivlayer.h"

using namespace ShipCAD;
using namespace std;

InsertPlaneDialogData::InsertPlaneDialogData()
    : accepted(false),
      addControlCurveSelected(false),
      planeSelected(transverse),
      distance(0.0), min(ZERO), max(ZERO)
{
    // does nothing
}

IntersectLayersDialogData::IntersectLayersDialogData()
    : accepted(false), layer1(0), layer2(0)
{
    // does nothing
}

ExtrudeEdgeDialogData::ExtrudeEdgeDialogData()
    : accepted(false), vector(ZERO)
{
    // does nothing
}

Controller::Controller(ShipCADModel* model)
    : _model(model), _point_first_moved(false)
{
    // initialize settings
    QCoreApplication::setOrganizationName("bit-builder");
    QCoreApplication::setOrganizationDomain("bit-builder.com");
    QCoreApplication::setApplicationName("ShipCAD");
	
    // setup signals and slots
    connect(_model, SIGNAL(onFileChanged()), SLOT(modelFileChanged()));
    connect(_model, SIGNAL(onUpdateGeometryInfo()), SIGNAL(modelGeometryChanged()));
    connect(_model, SIGNAL(changeActiveLayer()), SIGNAL(changeActiveLayer()));
    connect(_model, SIGNAL(changedLayerData()), SIGNAL(changedLayerData()));
    connect(_model, SIGNAL(onUpdateVisibilityInfo()), SIGNAL(onUpdateVisibilityInfo()));
    connect(_model, SIGNAL(undoDataChanged()), SIGNAL(updateUndoData()));
}

Controller::~Controller()
{
    // does nothing
}

bool Controller::shootPickRay(Viewport& vp, const PickRay& ray)
{
    bool scene_changed = false;
    vector<SubdivisionBase*> filtered;
    multimap<float, SubdivisionBase*> picks = getModel()->getSurface()->shootPickRay(vp, ray);
    if (picks.size() > 0) {
        multimap<float, SubdivisionBase*>::iterator it, itlow;
        itlow = picks.lower_bound(0);
        float last = 0;
        bool first = true;
        for (it=itlow; it!=picks.end(); ++it) {
            float s = (*it).first;
            // check to see if next element is farther away than first, since this is multimap, we
            // can several at the same distance from the ray origin
            if (!first) {
                if (s > last)
                    break;
            } else
                first = false;
            filtered.push_back((*it).second);
            last = s;
        }
    }
    if (filtered.size() > 0) {
        if (filtered.size() >= 1 && !ray.multi_sel)
            getModel()->getSurface()->clearSelection();
        if (filtered.size() == 1) {
            // is this a point?
            SubdivisionControlPoint* cp = dynamic_cast<SubdivisionControlPoint*>(filtered[0]);
            if (cp != 0) {
                cp->setSelected(true);
                getModel()->setActiveControlPoint(cp);
                scene_changed = true;
                cout << "control point selected" << endl;
                emit showControlPointDialog(true);
                emit updateControlPointValue(cp);
            } else {
                SubdivisionControlEdge* edge = dynamic_cast<SubdivisionControlEdge*>(filtered[0]);
                if (edge != 0 ) {
                    edge->setSelected(true);
                    scene_changed = true;
                    cout << "control edge selected" << endl;
                }
            }
        } else {
            // need to discriminate against these elements
            throw runtime_error("multiple elements picked");
        }
    }
    if (scene_changed) {
        // TODO file changed, undo
        getModel()->setFileChanged(true);
        emit changeSelectedItems();
        emit modelGeometryChanged();
    }
    return scene_changed;
}

void Controller::deleteBackgroundImage()
{
	// TODO
}

void Controller::openBackgroundImage()
{
	// TODO
}

void Controller::addCurve()
{
	// TODO
}

void Controller::collapseEdges()
{
	// TODO
}

void Controller::connectEdges()
{
	// TODO
}

void Controller::creaseEdges()
{
	// TODO
}

// FreeShipUnit.pas:4753
void Controller::extrudeEdges()
{
    cout << "Controller::extrudeEdges" << endl;
    ExtrudeEdgeDialogData data;
    emit exeExtrudeEdgeDialog(data);
    if (!data.accepted)
        return;
    // msg 0076
    UndoObject* uo = getModel()->createUndo(tr("edge extrusion"), false);
    // assemble edges in a list
    vector<SubdivisionControlEdge*>& list = getModel()->getSurface()->getSelControlEdgeCollection();
    // only boundary edges are allowed
    vector<SubdivisionControlEdge*> actinglist;
    for (size_t i=0; i<list.size(); i++) {
        if (list[i]->numberOfFaces() == 1)
            actinglist.push_back(list[i]);
    }
    // clear selected edges
    list.clear();
    if (actinglist.size() > 0) {
        getModel()->getSurface()->extrudeEdges(actinglist, data.vector);
        // new edges are returned in the list, select them
        for (size_t i=0; i<actinglist.size(); i++) {
            actinglist[i]->setSelected(true);
        }
        uo->accept();
        getModel()->setBuild(false);
        getModel()->setFileChanged(true);
        emit modelGeometryChanged();
    } else {
        // msg 0077
        emit displayWarningDialog(tr("Only boundary edges can be extruded!"));
        delete uo;
    }
}

// FreeShipUnit.pas:4808
void Controller::splitEdges()
{
	// TODO
}

void Controller::assembleFace()
{
	// TODO
}

void Controller::deleteNegativeFaces()
{
	// TODO
}

void Controller::flipFaces()
{
	// TODO
}

void Controller::mirrorPlaneFace()
{
	// TODO
}

// FreeShipUnit.pas:5393
void Controller::newFace()
{
    if (getModel()->getSurface()->numberOfSelectedControlPoints() <= 2) {
        // msg 0095 error
        emit displayWarningDialog(tr("You need to select at least 3 controlpoints to create a new controlface"));
        return;
    }
    // msg 0094
    UndoObject* uo = getModel()->createUndo(tr("new face"), false);
    // remember the number of faces, edges and points
    // assemble all points in a temporary list
    vector<SubdivisionControlPoint*> tmp;
    for (size_t i=0; i<getModel()->getSurface()->numberOfSelectedControlPoints(); i++)
        tmp.push_back(getModel()->getSurface()->getSelectedControlPoint(i));
    // deselect the controlpoints
    for (size_t i=getModel()->getSurface()->numberOfSelectedControlPoints(); i>0; i--)
        getModel()->getSurface()->getSelectedControlPoint(i-1)->setSelected(false);
    // add the new face
    SubdivisionControlFace* face = getModel()->getSurface()->addControlFace(tmp, true, getModel()->getActiveLayer());
    if (face != 0) {
        uo->accept();
        getModel()->setBuild(false);
        getModel()->setFileChanged(true);
        emit modelGeometryChanged();
    } else {
        delete uo;
    }
}

void Controller::rotateFaces()
{
	// TODO
}

void Controller::scaleFaces()
{
	// TODO
}

void Controller::moveFaces()
{
	// TODO
}

void Controller::exportFileArchimedes()
{
	// TODO
}

void Controller::exportCoordinates()
{
	// TODO
}

void Controller::export2DPolylinesDXF()
{
	// TODO
}

void Controller::export3DPolylinesDXF()
{
	// TODO
}

void Controller::exportFacesDXF()
{
	// TODO
}

void Controller::exportFEF()
{
	// TODO
}

void Controller::exportGHS()
{
	// TODO
}

void Controller::exportPart()
{
	// TODO
}

void Controller::exportMichlet()
{
	// TODO
}

void Controller::exportIGES()
{

}

void Controller::importMichletWaves()
{
	// TODO
}

void Controller::exportObj()
{
	// TODO
}

void Controller::exportOffsets()
{
	// TODO
}

void Controller::exportSTL()
{
	// TODO
}

void Controller::importCarene()
{
	// TODO
}

void Controller::importChines()
{
	// TODO
}

void Controller::importFEF()
{
	// TODO
}

void Controller::importHull()
{
	// TODO
}

void Controller::importPart()
{
	// TODO
}

void Controller::importPolycad()
{
	// TODO
}

void Controller::importSurface()
{
	// TODO
}

void Controller::importVRML()
{
	// TODO
}

void Controller::loadFile(const QString& filename)
{
    QFile loadfile(filename);
    FileBuffer source;
    try {
        source.loadFromFile(loadfile);
        getModel()->loadBinary(source);
        getModel()->getSurface()->clearSelection();
        // TODO clear selected flowlines
        // TODO clear selected markers
        getModel()->setFilename(filename);
        // stop asking for file version
    } catch(...) {
        // what do we do?
    }
    clearUndo();
    getModel()->setFileChanged(false);
    emit modelLoaded();
}

void Controller::saveFile()
{
    if (getModel()->isFilenameSet()) {
        const QString& filename = getModel()->getFilename();
        cout << "saveFile:" << filename.toStdString() << endl;
        // save to temporary file
        QFile tmp(ChangeFileExt(filename, ".tmp"));
        cout << "tmp:" << tmp.fileName().toStdString() << endl;
        if (tmp.exists())
            throw runtime_error("tmp file already exists");
        try {
            FileBuffer dest;
            getModel()->saveBinary(dest);
            dest.saveToFile(tmp);
            // remove backup if it exists
            QFile backup(ChangeFileExt(filename, ".Bak"));
            cout << "backup:" << backup.fileName().toStdString() << endl;
            if (backup.exists() && !backup.remove())
                throw runtime_error("unable to remove backup file");
            cout << "removed backup\n";
            // rename existing to .bak
            QFile existing(filename);
            cout << "existing:" << existing.fileName().toStdString() << endl;
            if (existing.exists() && !existing.rename(backup.fileName()))
                throw runtime_error("unable to rename original to backup file");
            cout << "renamed existing to backup\n";
            // rename saved tmp
            if (!tmp.rename(filename))
                throw runtime_error("unable to rename temporary to original");
            cout << "renamed tmp to original\n";
            // all done with saving file
            emit changedModel();
        } catch(...) {
            if (tmp.exists())
                tmp.remove();
            throw;
        }
    } else {
        throw runtime_error("no filename for save");
    }
}

void Controller::saveFileAs(const QString& filename)
{
    getModel()->setFilename(filename);
    saveFile();
}

void Controller::addFlowline(const QVector2D& source, viewport_type_t view)
{
	// TODO
}

void Controller::calculateHydrostatics()
{
	// TODO
}

void Controller::crossCurvesHydrostatics()
{
	// TODO
}

void Controller::hydrostaticsDialog()
{
	// TODO
}

void Controller::importFrames()
{
	// TODO
}

Intersection* Controller::addIntersection(intersection_type_t ty, float distance)
{
	// TODO
    return 0;
}

void Controller::addIntersectionToList(Intersection* inter)
{
	// TODO
}

void Controller::intersectionDialog()
{
	// TODO
}

void Controller::autoGroupLayer()
{
	// TODO
}

void Controller::developLayers()
{
	// TODO
}

void Controller::layerDialog()
{
	// TODO
}

void Controller::deleteEmptyLayers()
{
	// TODO
}

SubdivisionLayer* Controller::newLayer()
{
	// TODO
    return 0;
}

void Controller::addMarker(Marker* marker)
{
	// TODO
}

void Controller::deleteMarkers()
{
	// TODO
}

void Controller::importMarkers()
{
	// TODO
}

void Controller::checkModel()
{
	// TODO
}

void Controller::newModel()
{
/*
      CreateUndoObject(Userstring(157),True);
      Cols:=FreeNewModelDialog.NCols-1;
      Rows:=FreeNewModelDialog.NRows-1;
      L:=FreeNewModelDialog.Length;
      B:=FreeNewModelDialog.Breadth;
      D:=FreeNewModelDialog.Draft;
      // station 0, stern
      Default[0,0]:=Point3D(0.00000,0.00000,1.56754);
      Default[0,1]:=Point3D(0.00000,0.05280,1.59170);
      Default[0,2]:=Point3D(0.00000,0.22171,1.77284);
      Default[0,3]:=Point3D(0.00000,0.28506,2.64108);
      Default[0,4]:=Point3D(0.00000,0.29135,3.48932);
      // station 1
      Default[1,0]:=Point3D(0.20880,0.00000,0.49656);
      Default[1,1]:=Point3D(0.20881,0.18796,0.53622);
      Default[1,2]:=Point3D(0.20880,0.33700,0.97840);
      Default[1,3]:=Point3D(0.20880,0.45607,2.05422);
      Default[1,4]:=Point3D(0.20882,0.47184,3.44280);
      // station 2
      Default[2,0]:=Point3D(0.41765,0.00000,0.00000);
      Default[2,1]:=Point3D(0.41765,0.23565,0.07524);
      Default[2,2]:=Point3D(0.41765,0.41555,0.67735);
      Default[2,3]:=Point3D(0.41765,0.49421,1.91004);
      Default[2,4]:=Point3D(0.41737,0.51468,3.45474);
      // station 3
      Default[3,0]:=Point3D(0.58471,0.00000,0.00000);
      Default[3,1]:=Point3D(0.58472,0.24072,0.02507);
      Default[3,2]:=Point3D(0.58472,0.39528,0.71080);
      Default[3,3]:=Point3D(0.58488,0.45356,2.04881);
      Default[3,4]:=Point3D(0.58472,0.46756,3.54662);
      // station 4
      Default[4,0]:=Point3D(0.75179,0.00000,0.28284);
      Default[4,1]:=Point3D(0.75178,0.13715,0.44098);
      Default[4,2]:=Point3D(0.75179,0.20950,0.87760);
      Default[4,3]:=Point3D(0.75179,0.30538,2.38232);
      Default[4,4]:=Point3D(0.75177,0.34473,3.67786);
      // station 5
      Default[5,0]:=Point3D(0.90672,0.00000,0.81860);
      Default[5,1]:=Point3D(0.90681,0.01887,0.98650);
      Default[5,2]:=Point3D(0.90658,0.04671,1.29873);
      Default[5,3]:=Point3D(0.90637,0.11195,2.83107);
      Default[5,4]:=Point3D(0.90672,0.14523,3.81697);
      // station 6 , stem
      Default[6,0]:=Point3D(0.91580,0.00000,0.85643);
      Default[6,1]:=Point3D(0.92562,0.00000,1.17444);
      Default[6,2]:=Point3D(0.93387,0.00000,1.44618);
      Default[6,3]:=Point3D(0.97668,0.00000,3.03482);
      Default[6,4]:=Point3D(1.00000,0.00000,3.91366);
      Owner.Clear;

      Owner.ProjectSettings.ProjectUnits:=TFreeUnitType(FreeNewModelDialog.ComboBox1.ItemIndex);
      Owner.ProjectSettings.ProjectLength:=L;
      Owner.ProjectSettings.ProjectBeam:=B;
      Owner.ProjectSettings.ProjectDraft:=D;

      TrvSplines:=TFasterList.Create;
      StemPoint:=nil;
      // First create tmp. splines in transverse direction
      for I:=0 to 6 do
      begin
         Spline1:=TFreeSpline.Create;
         for J:=0 to 4 do
         begin
            P:=Default[I,J];
            P.X:=P.X*L;
            P.Y:=P.Y*B;
            P.Z:=P.Z*D;
            Spline1.Add(P);
         end;
         TrvSplines.Add(Spline1);
      end;
      // now create tmp. splines in longitudinal direction
      Setlength(Pts,Rows+1);
      for I:=0 to rows do
      begin
         Setlength(Pts[I],Cols+1);
         Spline2:=TFreeSpline.Create;
         for j:=0 to TrvSplines.Count-1 do
         begin
            Spline1:=TrvSplines[J];
            P:=Spline1.Value(I/Rows);
            Spline2.Add(P);
         end;
         // now calculate all points on the longitudinal spline and send it to the surface
         for J:=0 to Cols do
         begin
            P:=Spline2.Value(J/Cols);
            Pts[I,J]:=TFreeSubdivisionControlPoint.Create(Owner.Surface);
            Owner.Surface.AddControlPoint(Pts[I,J]);
            Pts[I,J].Coordinate:=P;
            if (I=0) and (J=Cols) then
            begin
               StemPoint:=Pts[I,J];
            end;
         end;
         Spline2.Destroy;
      end;
      // Destroy tmp splines
      for I:=1 to TrvSplines.Count do
      begin
         Spline1:=TrvSplines[I-1];
         Spline1.Destroy;
      end;
      TrvSplines.Clear;
      // finally create the controlfaces over the newly calculated points
      for I:=1 to Rows do
      begin
         for J:=1 to cols do
         begin
            TrvSplines.Clear;
            Trvsplines.Add(Pts[I,J-1]);
            Trvsplines.Add(Pts[I,J]);
            Trvsplines.Add(Pts[I-1,J]);
            Trvsplines.Add(Pts[I-1,J-1]);
            Owner.Surface.AddControlFace(Trvsplines,True);
         end;
      end;
      Owner.Precision:=fpMedium;
      Owner.Surface.Initialize(1,1,1);
      // Collapse stempoint to mage the grid irregular in order to demonstrate subdivision-surface capabilities
      if StemPoint<>nil then if StemPoint.VertexType=svCorner then stempoint.VertexType:=svCrease;
      Owner.Build:=False;

      // Add 21 stations
      for I:=0 to 20 do Intersection_Add(fiStation,I/20*(Owner.Surface.Max.X-Owner.Surface.Min.X));
      // Add 7 buttocks
      for I:=1 to 6 do Intersection_Add(fiButtock,I/7*(Owner.Surface.Max.Y-Owner.Surface.Min.Y));
      // Add 11 waterlines
      for I:=0 to 10 do Intersection_Add(fiWaterline,I/10*(Owner.Surface.Max.Z-Owner.Surface.Min.Z));

      Owner.draw;
      Owner.FileChanged:=True;
      if Assigned(Owner.OnUpdateGeometryInfo) then Owner.OnUpdateGeometryInfo(self);
      Result:=true;
      TrvSplines.Destroy;
   end;
   FreeNewModelDialog.Destroy;
*/    
}

void Controller::lackenbyModelTransformation()
{
	// TODO
}

void Controller::scaleModel()
{
	// TODO
}

// FreeShipUnit.pas:10086
void Controller::collapsePoint()
{
    // msg 0160
    UndoObject* uo = getModel()->createUndo(tr("collapse point"), false);
    int n = 0;
    for (size_t i=getModel()->getSurface()->numberOfSelectedControlPoints(); i>0; i--) {
        SubdivisionControlPoint* pt = getModel()->getSurface()->getSelectedControlPoint(i-1);
        if (!pt->isLocked() && pt->numberOfEdges() == 2) {
            getModel()->getSurface()->collapseControlPoint(pt);
            n++;
        }
    }
    if (n > 0) {
        uo->accept();
        getModel()->setBuild(false);
        getModel()->setFileChanged(true);
        emit modelGeometryChanged();
    }
    else {
        delete uo;
    }
}

void Controller::removeUnusedPoint()
{
	// TODO
}

// FreeShipUnit.pas:10141
void Controller::insertPlane()
{
    cout << "Controller::insertPlane" << endl;
    QVector3D min, max;
    getModel()->extents(min, max);
    InsertPlaneDialogData data;
    data.min = min;
    data.max = max;
    emit exeInsertPlanePointsDialog(data);
    if (!data.accepted)
        return;
    // msg 0163
    UndoObject* uo = getModel()->createUndo(tr("insert plane intersections"), false);
    size_t n = getModel()->getSurface()->numberOfControlPoints();
    Plane p;
    switch(data.planeSelected) {
    case transverse:
        p = Plane(1, 0, 0, -data.distance);
        break;
    case horizontal:
        p = Plane(0, 0, 1, -data.distance);
        break;
    case vertical:
        p = Plane(0, 1, 0, -data.distance);
        break;
    default:
        delete uo;
        return;
    }
    getModel()->getSurface()->insertPlane(p, data.addControlCurveSelected);
    if (n < getModel()->getSurface()->numberOfControlPoints()) {
        uo->accept();
        getModel()->setBuild(false);
        getModel()->setFileChanged(true);
        emit modelGeometryChanged();
    }
    else {
        delete uo;
    }
}

// FreeShipUnit.pas:10170
void Controller::intersectLayerPoint()
{
    IntersectLayersDialogData data;
    for (size_t i=0; i<getModel()->getSurface()->numberOfLayers(); i++) {
        if (getModel()->getSurface()->getLayer(i)->numberOfFaces() > 0)
            data.layers.push_back(getModel()->getSurface()->getLayer(i));
    }
    if (data.layers.size() <= 1) {
        // msg 0166
        emit displayWarningDialog(tr("At least two layers are needed to perform this operation."));
        return;
    }
    emit exeIntersectLayersDialog(data);
    if (data.accepted) {
        SubdivisionLayer* layer1 = data.layers[data.layer1];
        SubdivisionLayer* layer2 = data.layers[data.layer2];
        // msg 0164
        UndoObject* uo = getModel()->createUndo(tr("layer intersection"), false);
        if (layer1->calculateIntersectionPoints(layer2)) {
            uo->accept();
            getModel()->setBuild(false);
            getModel()->setFileChanged(true);
            emit modelGeometryChanged();
        }
        else {
            delete uo;
            // msg 0165
            emit displayErrorDialog(tr("No intersection points found."));
        }
    }
}

// FreeShipUnit.pas:10207
void Controller::lockPoints()
{
    cout << "Controller::lockPoints" << endl;
    SubdivisionSurface* surf = getModel()->getSurface();
    if (surf->numberOfSelectedLockedPoints() < surf->numberOfSelectedControlPoints()) {
        // msg 0167
        getModel()->createUndo(tr("lock points"), true);
        for (size_t i=0; i<surf->numberOfSelectedControlPoints(); i++) {
            surf->getSelectedControlPoint(i)->setLocked(true);
        }
        getModel()->setFileChanged(true);
        emit modelGeometryChanged();
    }
}

void Controller::newPoint()
{
    SubdivisionControlPoint* pt = SubdivisionControlPoint::construct(getModel()->getSurface());
    pt->setCoordinate(ZERO);
    getModel()->getSurface()->addControlPoint(pt);
    getModel()->getSurface()->setSelectedControlPoint(pt);
    getModel()->setActiveControlPoint(pt);
    getModel()->setFileChanged(true);
    emit showControlPointDialog(true);
    emit updateControlPointValue(pt);
    emit modelGeometryChanged();
}

// FreeShipUnit.pas:13645
void Controller::movePoint(QVector3D changedCoords)
{
    cout << "Controller::movePoint" << endl;
    if (getModel()->getSurface()->numberOfSelectedControlPoints() != 1)
        throw runtime_error("moving multiple points at once");
    SubdivisionControlPoint* pt = getModel()->getActiveControlPoint();
    if (pt->isLocked()) {
        // msg 0191, warning
        return;
    }
    if (!_point_first_moved) {
        // if we just started to move the point, create an undo
        _point_first_moved = true;
        // msg 0190
        getModel()->createUndo(tr("point move"), true);
    }
    QVector3D updated = pt->getCoordinate();
    if (changedCoords.x() != 0.0)
        updated.setX(changedCoords.x());
    if (changedCoords.y() != 0.0)
        updated.setY(changedCoords.y());
    if (changedCoords.z() != 0.0)
        updated.setZ(changedCoords.z());
    pt->setCoordinate(updated);
    getModel()->setFileChanged(true);
    emit updateControlPointValue(pt);
    emit modelGeometryChanged();
}

void Controller::stopMovePoint()
{
    _point_first_moved = false;
}

// FreeShipUnit:10356
void Controller::projectStraightLinePoint()
{
    SubdivisionSurface* surf = getModel()->getSurface();
    cout << "Controller::projectStraightLinePoint" << endl;
    if (surf->numberOfSelectedControlPoints() < 3) {
        // msg 0xxx
        emit displayWarningDialog(tr("At least 3 points must be selected to align"));
        return;
    }
    // determine if the number of points to be moved doesn't contain locked controlpoints only
    // however the first and last points (determining the line segment) are allowed to be locked
    size_t nlocked = 0;
    for (size_t i=1; i<surf->numberOfSelectedControlPoints()-1; i++) {
        if (surf->getSelectedControlPoint(i)->isLocked())
            nlocked++;
    }
    // number of locked points must be smaller than selected control points
    if (nlocked < surf->numberOfSelectedControlPoints() - 2) {
        SubdivisionControlPoint* p1 = surf->getSelectedControlPoint(0);
        SubdivisionControlPoint* p2 = surf->getSelectedControlPoint(surf->numberOfSelectedControlPoints()-1);
        // msg 0171
        UndoObject* uo = getModel()->createUndo(tr("align points"), false);
        size_t nchanged = 0;
        for (size_t i=1; i<surf->numberOfSelectedControlPoints()-1; i++) {
            SubdivisionControlPoint* point = surf->getSelectedControlPoint(i);
            if (point->isLocked())
                continue;
            QVector3D p = PointProjectToLine(p1->getCoordinate(), p2->getCoordinate(), point->getCoordinate());
            if (p.distanceToPoint(point->getCoordinate()) > 1E-5) {
                point->setCoordinate(p);
                nchanged++;
            }
        }
        if (nchanged > 0) {
            uo->accept();
            getModel()->setBuild(false);
            getModel()->setFileChanged(true);
            emit modelGeometryChanged();
        } else {
            delete uo;
        }
    } else {
        // msg 0172
        emit displayErrorDialog(tr("All of the selected points are locked and cannot be moved"));
    }
}

// FreeShipUnit.pas:10221
void Controller::unlockPoints()
{
    if (getModel()->getSurface()->numberOfSelectedLockedPoints() == 0)
        return;
    // msg 0168
    getModel()->createUndo(tr("unlock points"), true);
    for (size_t i=0; i<getModel()->getSurface()->numberOfSelectedControlPoints(); i++) {
        getModel()->getSurface()->getSelectedControlPoint(i)->setLocked(false);
    }
    getModel()->setFileChanged(true);
    emit modelGeometryChanged();
}

// FreeShipUnit.pas:10235
void Controller::unlockAllPoints()
{
    if (getModel()->getSurface()->numberOfLockedPoints() == 0)
        return;
    // msg 0169
    getModel()->createUndo(tr("unlock all points"), true);
    for (size_t i=0; i<getModel()->getSurface()->numberOfControlPoints(); i++) {
        getModel()->getSurface()->getControlPoint(i)->setLocked(false);
    }
    getModel()->setFileChanged(true);
    // emit dialog msg 0170
    emit modelGeometryChanged();
}

// FreeShipUnit.pas:10251
bool Controller::proceedWhenLockedPoints()
{
	// TODO
    return false;
}

void Controller::delftResistance()
{
	// TODO
}

void Controller::kaperResistance()
{
	// TODO
}

// FreeShipUnit.pas:10402
void Controller::clearSelections()
{
    cout << "Controller::clearSelections" << endl;
    SubdivisionSurface* surf = getModel()->getSurface();
    size_t n = surf->numberOfSelectedControlPoints()
            + surf->numberOfSelectedControlEdges()
            + surf->numberOfSelectedControlFaces()
            + surf->numberOfSelectedControlCurves(); //BUGBUG: markers and flowlines
    if (n == 0)
        return;
    getModel()->clearSelectedItems();
    getModel()->setActiveControlPoint(0);
    // BUGBUG: clear markers and flowlines
    emit showControlPointDialog(false);
    emit modelGeometryChanged();
	emit changeSelectedItems();
}

// FreeShipUnit.pas:10411
void Controller::deleteSelections()
{
    cout << "Controller::deleteSelections" << endl;
    SubdivisionSurface* surf = getModel()->getSurface();
    size_t n = surf->numberOfSelectedControlPoints()
            + surf->numberOfSelectedControlEdges()
            + surf->numberOfSelectedControlFaces()
            + surf->numberOfSelectedControlCurves(); //BUGBUG: markers and flowlines
    if (n == 0)
        return;
    // TODO
    // msg 0175
    //getModel()->createUndo(tr("delete"), true);
    //getModel()->clearSelectedItems();
    //getModel()->setActiveControlPoint(0);
    //emit showControlPointDialog(false);
    //emit modelGeometryChanged();
    //emit changeSelectedItems();
}

void Controller::selectAll()
{
	// TODO
}

void Controller::undo()
{
    getModel()->undo();
}

void Controller::redo()
{
    getModel()->redo();
}

void Controller::clearUndo()
{
    getModel()->clearUndo();
}

void Controller::showHistoryUndo()
{
	// TODO
}

void Controller::modelFileChanged()
{
	// TODO
    emit changedModel();
}

void
Controller::showControlNet(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if (vis.isShowControlNet() != val) {
        vis.setShowControlNet(val);
		emit onUpdateVisibilityInfo();
        cout << "surface control net visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showInteriorEdges(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if (vis.isShowInteriorEdges() != val) {
        vis.setShowInteriorEdges(val);
		emit onUpdateVisibilityInfo();
        cout << "surface interior edges visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showGrid(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if (vis.isShowGrid() != val) {
        vis.setShowGrid(val);
		emit onUpdateVisibilityInfo();
        cout << "grid visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showControlCurves(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if (vis.isShowControlCurves() != val) {
        vis.setShowControlCurves(val);
		emit onUpdateVisibilityInfo();
        cout << "surface control curves visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showCurvature(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if (vis.isShowCurvature() != val) {
        vis.setShowCurvature(val);
		emit onUpdateVisibilityInfo();
        cout << "surface curvature visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showNormals(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if (vis.isShowNormals() != val) {
        vis.setShowNormals(val);
		emit onUpdateVisibilityInfo();
        cout << "surface normals visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showBothSides(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if ((vis.getModelView() == mvBoth && !val) || (vis.getModelView() == mvPort && val)) {
        vis.setModelView(val ? mvBoth : mvPort);
		emit onUpdateVisibilityInfo();
        cout << "show both sides: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showMarkers(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if (vis.isShowMarkers() != val) {
        vis.setShowMarkers(val);
		emit onUpdateVisibilityInfo();
        cout << "show markers: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::shadeUnderwater(bool val)
{
	// TODO
}

void
Controller::showStations(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if (vis.isShowStations() != val) {
        vis.setShowStations(val);
		emit onUpdateVisibilityInfo();
        cout << "show stations: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showButtocks(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if (vis.isShowButtocks() != val) {
        vis.setShowButtocks(val);
		emit onUpdateVisibilityInfo();
        cout << "show buttocks: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showWaterlines(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if (vis.isShowWaterlines() != val) {
        vis.setShowWaterlines(val);
		emit onUpdateVisibilityInfo();
        cout << "show waterlines: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showDiagonals(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if (vis.isShowDiagonals() != val) {
        vis.setShowDiagonals(val);
		emit onUpdateVisibilityInfo();
        cout << "show diagonals: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showHydroData(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if (vis.isShowHydrostaticData() != val) {
        vis.setShowHydrostaticData(val);
		emit onUpdateVisibilityInfo();
        cout << "show hydro features: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showFlowlines(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if (vis.isShowFlowlines() != val) {
        vis.setShowFlowlines(val);
		emit onUpdateVisibilityInfo();
        cout << "show flowlines: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::addCylinder()
{
	// TODO
	cout << "add cylinder" << endl;
}

void
Controller::keelAndRudderWizard()
{
	// TODO
	cout << "keel and rudder wizard" << endl;
}

void
Controller::setActiveLayerColor()
{
	// TODO
	cout << "set active layer color" << endl;
}

// FreeControlPointForm.pas:320
void Controller::cornerPointSelected(bool sel)
{
    cout << "corner point selected:" << (sel ? 'y' : 'n') << endl;
    if (getModel()->getActiveControlPoint() != 0) {
        // msg 0213
        UndoObject* uo = getModel()->createUndo(tr("corner"), false);
        SubdivisionControlPoint* ap = getModel()->getActiveControlPoint();
        // count the number of crease edges connected to this point
        vertex_type_t oldtype = ap->getVertexType();
        if (ap->getVertexType() == svCorner) {
            size_t n = 0;
            for (size_t i=0; i<ap->numberOfEdges(); i++) {
                if (ap->getEdge(i)->isCrease())
                    n++;
            }
            // count the number of incident crease edges
            if (n == 0)
                ap->setVertexType(svRegular);
            else if (n == 1)
                ap->setVertexType(svDart);
            else if (n == 2)
                ap->setVertexType(svCrease);
        } else
            ap->setVertexType(svCorner);
        if (ap->getVertexType() != oldtype) {
            uo->accept();
            getModel()->setBuild(false);
            getModel()->setFileChanged(true);
            emit modelGeometryChanged();
            emit updateControlPointValue(ap);
        } else
            delete uo;
    }
}

void Controller::dialogUpdatedPointCoord(float x, float y, float z)
{
    cout << "point coord changed: (" << x << "," << y << "," << z << ")" << endl;
    if (getModel()->getActiveControlPoint() != 0) {
        SubdivisionControlPoint* ap = getModel()->getActiveControlPoint();
        QVector3D newcoord(x, y, z);
        if (ap->getCoordinate().distanceToPoint(newcoord) > 1e-4) {
            // msg 0190
            getModel()->createUndo(tr("point move"), true);
            ap->setCoordinate(newcoord);
            emit modelGeometryChanged();
        }
    }
}
