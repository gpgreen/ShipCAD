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
#include <QErrorMessage>

#include "controller.h"
#include "shipcadmodel.h"
#include "utility.h"
#include "undoobject.h"
#include "subdivpoint.h"
#include "subdivedge.h"
#include "viewport.h"
#include "viewportview.h"

using namespace ShipCAD;
using namespace std;

Controller::Controller(ShipCADModel* model)
        : _model(model)
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
                cout << "non control point selected" << endl;
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

UndoObject* Controller::createRedoObject()
{
	version_t version = _model->getFileVersion();
    bool preview = _model->getProjectSettings().isSavePreview();
	// temporarily set to the latest fileversion so no data will be lost
	UndoObject* rd = new UndoObject(_model,
                                    _model->getFilename(),
									_model->getEditMode(),
                                    _model->isFileChanged(),
                                    _model->isFilenameSet(),
									true);
	try {
		_model->setFileVersion(k_current_version);
		_model->getProjectSettings().setSavePreview(false);
		_model->saveBinary(rd->getUndoData());
		rd->accept();
		emit updateUndoData();
	} catch(...) {
		// restore the original version
		_model->setFileVersion(version);
		_model->getProjectSettings().setSavePreview(preview);
		delete rd;
		rd = 0;
	}
	return rd;
}

UndoObject* Controller::createUndoObject(QString& undotext, bool accept)
{
	// TODO
    return 0;
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

void Controller::extrudeEdges()
{
	// TODO
}

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

void Controller::newFace()
{
    if (getModel()->getSurface()->numberOfSelectedControlPoints() <= 2) {
        QErrorMessage* em = new QErrorMessage();
        em->showMessage(
            tr("You need to select at least 3 controlpoints to create a new controlface"));
        return;
    }
    // create undo object userstring 0094
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
        getModel()->setBuild(false);
        getModel()->setFileChanged(true);
        emit modelGeometryChanged();
    } else {
        // undo
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
    source.loadFromFile(loadfile);
    _model->loadBinary(source);
    _model->getSurface()->clearSelection();
    // TODO clear selected flowlines
    // TODO clear selected markers
    _model->setFilename(filename);
    // stop asking for file version
    _model->setFileChanged(false);
    emit modelLoaded();
}

void Controller::saveFile()
{
    if (_model->isFilenameSet()) {
        const QString& filename = _model->getFilename();
        cout << "saveFile:" << filename.toStdString() << endl;
        // save to temporary file
        QFile tmp(ChangeFileExt(filename, ".tmp"));
        cout << "tmp:" << tmp.fileName().toStdString() << endl;
        if (tmp.exists())
            throw runtime_error("tmp file already exists");
        try {
            FileBuffer dest;
            _model->saveBinary(dest);
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
    _model->setFilename(filename);
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

void Controller::collapsePoint()
{
	// TODO
}

void Controller::removeUnusedPoint()
{
	// TODO
}

void Controller::insertPlane()
{
	// TODO
}

void Controller::intersectLayerPoint()
{
	// TODO
}

void Controller::lockPoints()
{
	// TODO
}

void Controller::newPoint()
{
    SubdivisionControlPoint* pt = SubdivisionControlPoint::construct(_model->getSurface());
    pt->setCoordinate(ZERO);
    _model->getSurface()->addControlPoint(pt);
    _model->getSurface()->setSelectedControlPoint(pt);
    _model->setActiveControlPoint(pt);
    _model->setFileChanged(true);
    emit showControlPointDialog(true);
    emit updateControlPointValue(pt);
    emit modelGeometryChanged();
}

void Controller::movePoint(QVector3D changedCoords)
{
    cout << "Controller::movePoint" << endl;
    if (_model->getSurface()->numberOfSelectedControlPoints() != 1)
        throw runtime_error("moving multiple points at once");
    SubdivisionControlPoint* pt = _model->getActiveControlPoint();
    QVector3D updated = pt->getCoordinate();
    if (changedCoords.x() != 0.0)
        updated.setX(changedCoords.x());
    if (changedCoords.y() != 0.0)
        updated.setY(changedCoords.y());
    if (changedCoords.z() != 0.0)
        updated.setZ(changedCoords.z());
    pt->setCoordinate(updated);
    _model->setFileChanged(true);
    emit updateControlPointValue(pt);
    emit modelGeometryChanged();
}

void Controller::projectStraightLinePoint()
{
	// TODO
}

void Controller::unlockPoints()
{
	// TODO
}

void Controller::unlockAllPoints()
{
	// TODO
}

bool Controller::proceedWhenLockedPoints()
{
	// TODO
    return false;
}

void Controller::redo()
{
	// TODO
}

void Controller::delftResistance()
{
	// TODO
}

void Controller::kaperResistance()
{
	// TODO
}

void Controller::clearSelections()
{
    _model->clearSelectedItems();
    // TODO undo....
    emit modelGeometryChanged();
	emit changeSelectedItems();
}

void Controller::deleteSelections()
{
	// TODO
}

void Controller::selectAll()
{
	// TODO
}

void Controller::undo()
{
	// TODO
}

void Controller::clearUndo()
{
	// TODO
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
    Visibility& vis = _model->getVisibility();
    if (vis.isShowControlNet() != val) {
        vis.setShowControlNet(val);
		emit onUpdateVisibilityInfo();
        cout << "surface control net visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showInteriorEdges(bool val)
{
    Visibility& vis = _model->getVisibility();
    if (vis.isShowInteriorEdges() != val) {
        vis.setShowInteriorEdges(val);
		emit onUpdateVisibilityInfo();
        cout << "surface interior edges visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showGrid(bool val)
{
    Visibility& vis = _model->getVisibility();
    if (vis.isShowGrid() != val) {
        vis.setShowGrid(val);
		emit onUpdateVisibilityInfo();
        cout << "grid visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showControlCurves(bool val)
{
    Visibility& vis = _model->getVisibility();
    if (vis.isShowControlCurves() != val) {
        vis.setShowControlCurves(val);
		emit onUpdateVisibilityInfo();
        cout << "surface control curves visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showCurvature(bool val)
{
    Visibility& vis = _model->getVisibility();
    if (vis.isShowCurvature() != val) {
        vis.setShowCurvature(val);
		emit onUpdateVisibilityInfo();
        cout << "surface curvature visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showNormals(bool val)
{
    Visibility& vis = _model->getVisibility();
    if (vis.isShowNormals() != val) {
        vis.setShowNormals(val);
		emit onUpdateVisibilityInfo();
        cout << "surface normals visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showBothSides(bool val)
{
    Visibility& vis = _model->getVisibility();
    if ((vis.getModelView() == mvBoth && !val) || (vis.getModelView() == mvPort && val)) {
        vis.setModelView(val ? mvBoth : mvPort);
		emit onUpdateVisibilityInfo();
        cout << "show both sides: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showMarkers(bool val)
{
    Visibility& vis = _model->getVisibility();
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
    Visibility& vis = _model->getVisibility();
    if (vis.isShowStations() != val) {
        vis.setShowStations(val);
		emit onUpdateVisibilityInfo();
        cout << "show stations: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showButtocks(bool val)
{
    Visibility& vis = _model->getVisibility();
    if (vis.isShowButtocks() != val) {
        vis.setShowButtocks(val);
		emit onUpdateVisibilityInfo();
        cout << "show buttocks: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showWaterlines(bool val)
{
    Visibility& vis = _model->getVisibility();
    if (vis.isShowWaterlines() != val) {
        vis.setShowWaterlines(val);
		emit onUpdateVisibilityInfo();
        cout << "show waterlines: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showDiagonals(bool val)
{
    Visibility& vis = _model->getVisibility();
    if (vis.isShowDiagonals() != val) {
        vis.setShowDiagonals(val);
		emit onUpdateVisibilityInfo();
        cout << "show diagonals: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showHydroData(bool val)
{
    Visibility& vis = _model->getVisibility();
    if (vis.isShowHydrostaticData() != val) {
        vis.setShowHydrostaticData(val);
		emit onUpdateVisibilityInfo();
        cout << "show hydro features: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showFlowlines(bool val)
{
    Visibility& vis = _model->getVisibility();
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

void Controller::cornerPointSelected(bool sel)
{
    cout << "corner point selected:" << (sel ? 'y' : 'n') << endl;
    if (_model->getActiveControlPoint() != 0) {
        SubdivisionControlPoint* ap = _model->getActiveControlPoint();
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
            // create an undo object
            getModel()->setBuild(false);
            getModel()->setFileChanged(true);
            emit modelGeometryChanged();
            emit updateControlPointValue(ap);
        }
    }
}

void Controller::dialogUpdatedPointCoord(float x, float y, float z)
{
    cout << "point coord changed: (" << x << "," << y << "," << z << ")" << endl;
    if (_model->getActiveControlPoint() != 0) {
        SubdivisionControlPoint* ap = _model->getActiveControlPoint();
        QVector3D newcoord(x, y, z);
        if (ap->getCoordinate().distanceToPoint(newcoord) > 1e-4) {
            ap->setCoordinate(newcoord);
            // create an undo object
            emit modelGeometryChanged();
        }
    }
}
