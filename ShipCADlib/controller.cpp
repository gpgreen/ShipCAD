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
#include <QColorDialog>

#include "controller.h"
#include "shipcadmodel.h"
#include "utility.h"
#include "undoobject.h"
#include "subdivpoint.h"
#include "subdivedge.h"
#include "viewport.h"
#include "viewportview.h"
#include "subdivlayer.h"
#include "subdivface.h"

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

ChooseColorDialogData::ChooseColorDialogData(const QString& title, const QColor& initial)
    : accepted(false), title(title), initial(initial), options(QColorDialog::ColorDialogOptions())
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
                getModel()->setActiveControlPoint(0);
                emit showControlPointDialog(false);
                // is this an edge?
                SubdivisionControlEdge* edge = dynamic_cast<SubdivisionControlEdge*>(filtered[0]);
                if (edge != 0 ) {
                    edge->setSelected(true);
                    scene_changed = true;
                    cout << "control edge selected" << endl;
                }
                // is this a face?
                SubdivisionControlFace* face = dynamic_cast<SubdivisionControlFace*>(filtered[0]);
                if (face != 0) {
                    face->setSelected(true);
                    scene_changed = true;
                    cout << "control face selected" << endl;
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
        emit modifiedModel();
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

// FreeShipUnit.pas:4632
void Controller::addCurve()
{
	cout << "Controller::addCurve" << endl;
    vector<SubdivisionControlEdge*> edges;

    SubdivisionSurface* surf = getModel()->getSurface();
    
    set<SubdivisionControlEdge*>& list = surf->getSelControlEdgeCollection();
    for (set<SubdivisionControlEdge*>::iterator i=list.begin(); i!=list.end(); ++i) {
        if ((*i)->getCurve() == 0)
            edges.push_back(*i);
    }
    if (edges.size() == 0)
        return;
    // msg 0072
    getModel()->createUndo(tr("new controlcurve"), true);
    surf->addControlCurves(edges);
    list.clear();
    if (!getModel()->getVisibility().isShowControlCurves())
        getModel()->getVisibility().setShowControlCurves(true);
    emit modifiedModel();
}

// FreeShipUnit.pas:4697
void Controller::collapseEdges()
{
    cout << "Controller::collapseEdges" << endl;
    size_t n = 0;
    // msg 0073
    UndoObject* uo = getModel()->createUndo(tr("edge collapse"), false);
    set<SubdivisionControlEdge*>& list = getModel()->getSurface()->getSelControlEdgeCollection();
    for (set<SubdivisionControlEdge*>::iterator i=list.begin(); i!=list.end(); ++i) {
        if ((*i)->numberOfFaces() > 1) {
            getModel()->getSurface()->collapseEdge(*i);
            n++;
        }
    }
    list.clear();
    if (n > 0) {
        uo->accept();
        getModel()->setBuild(false);
        getModel()->setFileChanged(true);
        emit modifiedModel();
    }
    else {
        delete uo;
        emit changeSelectedItems();
    }
}

// FreeShipUnit.pas:4724
void Controller::connectEdges()
{
    cout << "Controller::connectEdges" << endl;
    if (getModel()->getSurface()->numberOfSelectedControlPoints() < 2)
        return;
    // msg 0074
    UndoObject* uo = getModel()->createUndo(tr("new edge"), false);
    if (getModel()->getSurface()->edgeConnect()) {
        uo->accept();
        getModel()->setBuild(false);
        getModel()->setFileChanged(true);
        emit modifiedModel();
    } else {
        // msg 0202
        displayWarningDialog(tr("Edge already exists"));
        delete uo;
    }
}

// FreeShipUnit.pas:4742
void Controller::creaseEdges()
{
    cout << "Controller::creaseEdges" << endl;
    // msg 0075
    getModel()->createUndo(tr("set crease edges"), true);
    set<SubdivisionControlEdge*>& list = getModel()->getSurface()->getSelControlEdgeCollection();
    for (set<SubdivisionControlEdge*>::iterator i=list.begin(); i!=list.end(); ++i) {
        (*i)->setCrease(!(*i)->isCrease());
    }
    getModel()->setBuild(false);
    getModel()->setFileChanged(true);
    emit modifiedModel();
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
    set<SubdivisionControlEdge*>& list = getModel()->getSurface()->getSelControlEdgeCollection();
    // only boundary edges are allowed
    vector<SubdivisionControlEdge*> actinglist;
    for (set<SubdivisionControlEdge*>::iterator i=list.begin(); i!=list.end(); ++i) {
        if ((*i)->numberOfFaces() == 1)
            actinglist.push_back(*i);
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
        emit modifiedModel();
    } else {
        emit changeSelectedItems();
        // msg 0077
        emit displayWarningDialog(tr("Only boundary edges can be extruded!"));
        delete uo;
    }
}

// FreeShipUnit.pas:4808
void Controller::splitEdges()
{
    cout << "Controller::splitEdges" << endl;
    // msg 0078
    UndoObject* uo = getModel()->createUndo(tr("edge split"), false);
    SubdivisionControlPoint* last = 0;
    set<SubdivisionControlEdge*>& list = getModel()->getSurface()->getSelControlEdgeCollection();
    size_t n = 0;
    for (set<SubdivisionControlEdge*>::iterator i=list.begin(); i!=list.end(); ++i) {
        const QVector3D& p1 = (*i)->startPoint()->getCoordinate();
        const QVector3D& p2 = (*i)->endPoint()->getCoordinate();
        SubdivisionControlPoint* point = (*i)->insertControlPoint((p1 + p2)/2);
        if (point != 0) {
            point->setSelected(true);
            last = point;
            n++;
        }
    }
    list.clear();
    if (last != 0) {
        getModel()->setActiveControlPoint(last);
        emit showControlPointDialog(true);
        emit updateControlPointValue(last);
    }
    if (n > 0) {
        uo->accept();
        getModel()->setBuild(false);
        getModel()->setFileChanged(true);
        emit modifiedModel();
    }
    else {
        delete uo;
        emit changeSelectedItems();
    }
}

void Controller::assembleFace()
{
	// TODO
}

void Controller::deleteNegativeFaces()
{
	// TODO
}

// FreeShipUnit.pas:4948
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
    OrderedPointMap& cplist =
        getModel()->getSurface()->getSelControlPointCollection();

    if (cplist.size() < 3) {
        // msg 0095 error
        emit displayWarningDialog(tr("You need to select at least 3 controlpoints to create a new controlface"));
        return;
    }
    // msg 0094
    UndoObject* uo = getModel()->createUndo(tr("new face"), false);
    // remember the number of faces, edges and points
    // assemble all points in a temporary list
    vector<SubdivisionControlPoint*> tmp;
    for (size_t i=0; i<cplist.size(); i++)
        tmp.push_back(cplist.get(i));
    // deselect the controlpoints
    cplist.clear();
    // add the new face
    SubdivisionControlFace* face =
        getModel()->getSurface()->addControlFace(tmp, true, getModel()->getActiveLayer());
    if (face != 0) {
        uo->accept();
        getModel()->setBuild(false);
        getModel()->setFileChanged(true);
        emit modifiedModel();
    } else {
        delete uo;
        emit changeSelectedItems();
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
            emit modifiedModel();
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
    emit modifiedModel();
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

void Controller::addIntersection()
{
	// TODO
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
    clearUndo();
    getModel()->newModel(fuMetric, 15.0, 6.0, 1.0, 10, 8);
    emit modelLoaded();
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
    OrderedPointMap& pmap = getModel()->getSurface()->getSelControlPointCollection();
    for (OrderedPointMap::iterator i=pmap.begin(); i!=pmap.end(); ++i) {
        SubdivisionControlPoint* pt = *i;
        if (!pt->isLocked() && pt->numberOfEdges() == 2) {
            getModel()->getSurface()->collapseControlPoint(pt);
            n++;
        }
    }
    if (n > 0) {
        uo->accept();
        getModel()->setBuild(false);
        getModel()->setFileChanged(true);
        emit modifiedModel();
    }
    else {
        delete uo;
        emit changeSelectedItems();
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
        emit modifiedModel();
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
            emit modifiedModel();
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
        OrderedPointMap& ptmap = surf->getSelControlPointCollection();
        for (OrderedPointMap::iterator i=ptmap.begin(); i!=ptmap.end(); ++i) {
            (*i)->setLocked(true);
        }
        getModel()->setFileChanged(true);
        emit modifiedModel();
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
    emit modifiedModel();
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
    emit modifiedModel();
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
    OrderedPointMap& ptset = getModel()->getSurface()->getSelControlPointCollection();
    for (size_t i=1; i<ptset.size()-1; i++) {
        if (ptset.get(i)->isLocked())
            nlocked++;
    }
    // number of locked points must be smaller than selected control points
    if (nlocked < ptset.size() - 2) {
        SubdivisionControlPoint* p1 = ptset.get(static_cast<size_t>(0));
        SubdivisionControlPoint* p2 = ptset.get(ptset.size()-1);
        // msg 0171
        UndoObject* uo = getModel()->createUndo(tr("align points"), false);
        size_t nchanged = 0;
        for (size_t i=1; i<ptset.size()-1; i++) {
            SubdivisionControlPoint* point = ptset.get(i);
            if (point->isLocked())
                continue;
            QVector3D p = PointProjectToLine(p1->getCoordinate(),
                                             p2->getCoordinate(),
                                             point->getCoordinate());
            if (p.distanceToPoint(point->getCoordinate()) > 1E-5) {
                point->setCoordinate(p);
                nchanged++;
            }
        }
        if (nchanged > 0) {
            uo->accept();
            getModel()->setBuild(false);
            getModel()->setFileChanged(true);
            emit modifiedModel();
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
    OrderedPointMap& ptset = getModel()->getSurface()->getSelControlPointCollection();
    for (OrderedPointMap::iterator i=ptset.begin(); i!=ptset.end(); ++i) {
        (*i)->setLocked(false);
    }
    getModel()->setFileChanged(true);
    emit modifiedModel();
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
    emit modifiedModel();
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
    emit modifiedModel();
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
    // msg 0175
    getModel()->createUndo(tr("delete"), true);
    getModel()->deleteSelected();
    getModel()->setActiveControlPoint(0);
    getModel()->setBuild(false);
    getModel()->setFileChanged(true);
    emit showControlPointDialog(false);
    emit modifiedModel();
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

void
Controller::showControlNet(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if (vis.isShowControlNet() != val) {
        vis.setShowControlNet(val);
		emit modifiedModel();
        cout << "surface control net visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showInteriorEdges(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if (vis.isShowInteriorEdges() != val) {
        vis.setShowInteriorEdges(val);
		emit modifiedModel();
        cout << "surface interior edges visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showGrid(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if (vis.isShowGrid() != val) {
        vis.setShowGrid(val);
		emit modifiedModel();
        cout << "grid visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showControlCurves(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if (vis.isShowControlCurves() != val) {
        vis.setShowControlCurves(val);
		emit modifiedModel();
        cout << "surface control curves visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showCurvature(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if (vis.isShowCurvature() != val) {
        vis.setShowCurvature(val);
		emit modifiedModel();
        cout << "surface curvature visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showNormals(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if (vis.isShowNormals() != val) {
        vis.setShowNormals(val);
		emit modifiedModel();
        cout << "surface normals visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showBothSides(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if ((vis.getModelView() == mvBoth && !val) || (vis.getModelView() == mvPort && val)) {
        vis.setModelView(val ? mvBoth : mvPort);
		emit modifiedModel();
        cout << "show both sides: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showMarkers(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if (vis.isShowMarkers() != val) {
        vis.setShowMarkers(val);
		emit modifiedModel();
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
		emit modifiedModel();
        cout << "show stations: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showButtocks(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if (vis.isShowButtocks() != val) {
        vis.setShowButtocks(val);
		emit modifiedModel();
        cout << "show buttocks: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showWaterlines(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if (vis.isShowWaterlines() != val) {
        vis.setShowWaterlines(val);
		emit modifiedModel();
        cout << "show waterlines: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showDiagonals(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if (vis.isShowDiagonals() != val) {
        vis.setShowDiagonals(val);
		emit modifiedModel();
        cout << "show diagonals: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showHydroData(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if (vis.isShowHydrostaticData() != val) {
        vis.setShowHydrostaticData(val);
		emit modifiedModel();
        cout << "show hydro features: " << (val ? 'y' : 'n') << endl;
    }
}

void
Controller::showFlowlines(bool val)
{
    Visibility& vis = getModel()->getVisibility();
    if (vis.isShowFlowlines() != val) {
        vis.setShowFlowlines(val);
		emit modifiedModel();
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

// Main.pas:952
void Controller::setActiveLayerColor()
{
	cout << "Controller::setActiveLayerColor" << endl;
    ChooseColorDialogData data(tr("Choose color for Layer"),
                               getModel()->getSurface()->getActiveLayer()->getColor());
    emit exeChooseColorDialog(data);
    if (data.accepted) {
        getModel()->createUndo(tr("change active layer color"), true);
        getModel()->getSurface()->getActiveLayer()->setColor(data.chosen);
        getModel()->setBuild(false);
        getModel()->setFileChanged(true);
        emit modifiedModel();
    }
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
            emit modifiedModel();
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
            emit modifiedModel();
        }
    }
}
