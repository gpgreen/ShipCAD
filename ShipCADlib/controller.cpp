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

bool Controller::shootPickRay(Viewport& vp, const PickRay& ray)
{
    bool element_sel = false;
    SubdivisionBase* pick = getSurface()->shootPickRay(vp, ray);
    if (pick != nullptr) {
        if (!ray.multi_sel)
            getSurface()->clearSelection();
        // is this a point?
        SubdivisionControlPoint* cp = dynamic_cast<SubdivisionControlPoint*>(pick);
        if (cp != nullptr) {
            cp->setSelected(true);
            getModel()->setActiveControlPoint(cp);
            element_sel = true;
            cout << "control point selected" << endl;
            emit showControlPointDialog(true);
            emit updateControlPointValue(cp);
        } else {
            getModel()->setActiveControlPoint(nullptr);
            emit showControlPointDialog(false);
            // is this an edge?
            SubdivisionControlEdge* edge = dynamic_cast<SubdivisionControlEdge*>(pick);
            if (edge != nullptr) {
                edge->setSelected(true);
                element_sel = true;
                cout << "control edge selected" << endl;
            }
            // is this a face?
            SubdivisionControlFace* face = dynamic_cast<SubdivisionControlFace*>(pick);
            if (!element_sel && face != nullptr) {
                face->setSelected(true);
                element_sel = true;
                cout << "control face selected" << endl;
            }
        }
    }
    if (element_sel) {
        getModel()->setFileChanged(true);
        emit modifiedModel();
    }
    return element_sel;
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

    SubdivisionSurface* surf = getSurface();
    
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
    showControlCurves(true);
    emit modifiedModel();
}

// FreeShipUnit.pas:4697
void Controller::collapseEdges()
{
    cout << "Controller::collapseEdges" << endl;
    size_t n = 0;
    // msg 0073
    UndoObject* uo = getModel()->createUndo(tr("edge collapse"), false);
    set<SubdivisionControlEdge*>& list = getSurface()->getSelControlEdgeCollection();
    for (set<SubdivisionControlEdge*>::iterator i=list.begin(); i!=list.end(); ++i) {
        if ((*i)->numberOfFaces() > 1) {
            getSurface()->collapseEdge(*i);
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
    if (getSurface()->numberOfSelectedControlPoints() < 2)
        return;
    // msg 0074
    UndoObject* uo = getModel()->createUndo(tr("new edge"), false);
    if (getSurface()->edgeConnect()) {
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
    set<SubdivisionControlEdge*>& list = getSurface()->getSelControlEdgeCollection();
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
    set<SubdivisionControlEdge*>& list = getSurface()->getSelControlEdgeCollection();
    // only boundary edges are allowed
    vector<SubdivisionControlEdge*> actinglist;
    for (set<SubdivisionControlEdge*>::iterator i=list.begin(); i!=list.end(); ++i) {
        if ((*i)->numberOfFaces() == 1)
            actinglist.push_back(*i);
    }
    // clear selected edges
    list.clear();
    if (actinglist.size() > 0) {
        getSurface()->extrudeEdges(actinglist, data.vector);
        // new edges are returned in the list, select them
        for (size_t i=0; i<actinglist.size(); i++) {
            actinglist[i]->setSelected(true);
        }
        uo->accept();
        getModel()->setBuild(false);
        getModel()->setFileChanged(true);
    } else {
        // msg 0077
        emit displayWarningDialog(tr("Only boundary edges can be extruded!"));
        delete uo;
    }
    emit modifiedModel();
}

// FreeShipUnit.pas:4808
void Controller::splitEdges()
{
    cout << "Controller::splitEdges" << endl;
    // msg 0078
    UndoObject* uo = getModel()->createUndo(tr("edge split"), false);
    SubdivisionControlPoint* last = 0;
    set<SubdivisionControlEdge*>& list = getSurface()->getSelControlEdgeCollection();
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
    cout << "Controller::flipFaces" << endl;
    // msg 0083
    getModel()->createUndo(tr("flip normals"), true);
    set<SubdivisionControlFace*>::iterator i =
        getSurface()->getSelControlFaceCollection().begin();
    for (; i!=getSurface()->getSelControlFaceCollection().end(); ++i)
        (*i)->flipNormal();
    getModel()->setBuild(false);
    getModel()->setFileChanged(true);
    emit modifiedModel();
}

// returns true if dialog accepted, faces or points will have been selected, or all deselected if
// dialog not accepted
bool Controller::showChooseLayerDialog(LayerSelectMode mode)
{
    // get current visibility values
    bool old_normals = getModel()->getVisibility().isShowNormals();
    bool old_edges = getModel()->getVisibility().isShowInteriorEdges();
    bool old_control_net = getModel()->getVisibility().isShowControlNet();

    // set new visibility values
    if (mode == fsFaces) {
        getModel()->getVisibility().setShowNormals(false);
        getModel()->getVisibility().setShowInteriorEdges(true);
    } else {
        getModel()->getVisibility().setShowNormals(false);
        getModel()->getVisibility().setShowInteriorEdges(false);
        getModel()->getVisibility().setShowControlNet(true);
    }

    // collect all visible layers
    vector<SubdivisionLayer*> layers;
    for (size_t i=0; i<getSurface()->numberOfLayers(); i++) {
        if (getSurface()->getLayer(i)->isVisible())
            layers.push_back(getSurface()->getLayer(i));
    }

    // the dialog
    ChooseLayerDialogData data(layers, mode);
    emit exeChooseLayerDialog(data);

    // recover
    getModel()->getVisibility().setShowNormals(old_normals);
    getModel()->getVisibility().setShowInteriorEdges(old_edges);
    getModel()->getVisibility().setShowControlNet(old_control_net);
    if (!data.accepted) {
        clearSelections();
        return false;
    }
    return true;
}

// FreeShipUnit.pas:4959
void Controller::mirrorPlaneFace()
{
    cout << "Controller::mirrorPlaneFace" << endl;
    vector<SubdivisionControlFace*> mirrorfaces;
    set<SubdivisionControlFace*>& cflist = getSurface()->getSelControlFaceCollection();

    // if nothing selected, use the Choose Layers dialog
    if (cflist.size() == 0 && !showChooseLayerDialog(fsFaces))
        return;

    // use all selected control faces
    mirrorfaces.insert(mirrorfaces.end(), cflist.begin(), cflist.end());
    if (mirrorfaces.size() == 0)
        return;
    MirrorDialogData data(false, transverse, 0.0);
    emit exeMirrorDialog(data);
    if (!data.accepted) {
        clearSelections();
        return;
    }
    // msg 0084
    getModel()->createUndo(tr("mirror"), true);
    Plane p(0,0,0,-data.distance);
    if (data.which_plane == transverse)
        p.setA(1.0);
    else if (data.which_plane == horizontal)
        p.setC(1.0);
    else
        p.setB(1.0);
    getSurface()->mirrorFaces(data.connect_points, p, mirrorfaces);
    getModel()->setBuild(false);
    getModel()->setFileChanged(true);
    emit modifiedModel();
}

// FreeShipUnit.pas:5393
void Controller::newFace()
{
    cout << "Controller::newFace" << endl;
    OrderedPointMap& cplist =
        getSurface()->getSelControlPointCollection();

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
        getSurface()->addControlFace(tmp, true, getSurface()->getActiveLayer());
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

// FreeShipUnit.pas:5102
void Controller::rotateFaces()
{
    cout << "Controller::rotateFaces" << endl;

    vector<SubdivisionControlPoint*> points;
    size_t nlocked;

    getSurface()->extractPointsFromSelection(points, nlocked);
    if (points.size() == 0 && !showChooseLayerDialog(fsPoints))
        return;

    getSurface()->extractPointsFromSelection(points, nlocked);
    if (points.size() == 0)
        return;
    bool proceed = true;
    if (nlocked > 0)
        proceed = proceedWhenLockedPoints();
    if (!proceed) {
        clearSelections();
        return;
    }
    
    // msg 0088
    RotateDialogData data(tr("Rotation vector"), "[Degr.]");
    emit exeRotateDialog(data);
    if (!data.accepted) {
        clearSelections();
        return;
    }
    // msg 0089
    getModel()->createUndo(tr("rotate"), true);
    double cosx = cos(DegToRad(data.rotation_vector.x()));
    double sinx = sin(DegToRad(data.rotation_vector.x()));
    double cosy = cos(DegToRad(data.rotation_vector.y()));
    double siny = sin(DegToRad(data.rotation_vector.y()));
    double cosz = cos(DegToRad(data.rotation_vector.z()));
    double sinz = sin(DegToRad(data.rotation_vector.z()));

    for (size_t i=0; i<points.size(); i++) {
        if (!points[i]->isLocked())
            points[i]->setCoordinate(RotateVector(points[i]->getCoordinate(), sinx, cosx, siny, cosy, sinz, cosz));
    }

    if (points.size() == getSurface()->numberOfControlPoints()
        && adjustMarkersDialog()) {
        // rotate the markers also
        MarkerVectorIterator i = getModel()->getMarkers().begin();
        for (; i!=getModel()->getMarkers().end(); ++i) {
            for (size_t j=0; j<(*i)->numberOfPoints(); j++) {
                (*i)->setPoint(j, RotateVector((*i)->getPoint(j),
                                               sinx, cosx,
                                               siny, cosy,
                                               sinz, cosz));
            }
        }
    }

    getModel()->setBuild(false);
    getModel()->setFileChanged(true);
    emit modifiedModel();
}

bool Controller::adjustMarkersDialog()
{
    if (getModel()->numberOfMarkers() == 0)
        return false;
    bool ok;
    // msg 0180
    emit displayQuestionDialog(
        tr("Do you want to adjust the markers too?"), ok);
    return ok;
}
    
// FreeShipUnit.pas:5195
void Controller::scaleFaces()
{
    cout << "Controller::scaleFaces" << endl;

    vector<SubdivisionControlPoint*> points;
    size_t nlocked;

    getSurface()->extractPointsFromSelection(points, nlocked);
    if (points.size() == 0 && !showChooseLayerDialog(fsPoints))
        return;

    getSurface()->extractPointsFromSelection(points, nlocked);
    if (points.size() == 0)
        return;
    bool proceed = true;
    if (nlocked > 0)
        proceed = proceedWhenLockedPoints();
    if (!proceed) {
        clearSelections();
        return;
    }

    // msg 0090
    RotateDialogData data(tr("Scale vector"), "");
    data.rotation_vector = QVector3D(1.0f, 1.0f, 1.0f);
    emit exeRotateDialog(data);
    if (!data.accepted) {
        clearSelections();
        return;
    }
    // msg 0091
    getModel()->createUndo(tr("scale"), true);
    if (points.size() == getSurface()->numberOfControlPoints()) {
        bool adjust_markers = adjustMarkersDialog();
        // scale the entire model
        getModel()->scaleModel(data.rotation_vector, false, adjust_markers);
    } else {
        // only a selected part of the model must be scaled
        for (size_t i=0; i<points.size(); i++) {
            if (!points[i]->isLocked())
                points[i]->setCoordinate(
                    data.rotation_vector * points[i]->getCoordinate());
        }
    }
    getModel()->setBuild(false);
    getModel()->setFileChanged(true);
    emit modifiedModel();
}

// FreeShipUnit.pas:5287
void Controller::moveFaces()
{
	cout << "Controller::moveFaces" << endl;
    vector<SubdivisionControlPoint*> points;
    size_t nlocked;

    getSurface()->extractPointsFromSelection(points, nlocked);
    if (points.size() == 0 && !showChooseLayerDialog(fsPoints))
        return;

    getSurface()->extractPointsFromSelection(points, nlocked);
    if (points.size() == 0)
        return;
    bool proceed = true;
    if (nlocked > 0)
        proceed = proceedWhenLockedPoints();
    if (!proceed) {
        clearSelections();
        return;
    }

    // msg 0092
    RotateDialogData data(tr("Translation vector"),
                          LengthStr(getModel()->getProjectSettings().getUnits()));
    
    emit exeRotateDialog(data);
    if (!data.accepted) {
        clearSelections();
        return;
    }
    // msg 0093
    getModel()->createUndo(tr("move"), true);
    bool adjust_markers = false;
    if (points.size() == getSurface()->numberOfControlPoints())
         adjust_markers = adjustMarkersDialog();
    getModel()->moveFaces(points, data.rotation_vector, adjust_markers);
    getModel()->setBuild(false);
    getModel()->setFileChanged(true);
    emit modifiedModel();
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
    cout << "Controller::loadFile" << endl;
    QFile loadfile(filename);
    FileBuffer source;
    try {
        source.loadFromFile(loadfile);
        getModel()->loadBinary(source);
        getSurface()->clearSelection();
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
    cout << "Controller::saveFile" << endl;
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
    cout << "Controller::saveFileAs" << endl;
    getModel()->setFilename(filename);
    saveFile();
    emit modifiedModel();
}

void Controller::addFlowline(const QVector2D& /*source*/, viewport_type_t /*view*/)
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

void Controller::addIntersectionToList(Intersection* /*inter*/)
{
	// TODO
}

void Controller::intersectionDialog()
{
	// TODO
}

// FreeShipUnit.pas:9128
void Controller::autoGroupLayer()
{
    cout << "Controller::autoGroupLayer" << endl;
    SubdivisionSurface* surf = getSurface();
    // msg 0139
    UndoObject* uo = getModel()->createUndo(tr("layer grouping"), false);
    if (surf->autoGroupFaces()) {
        surf->deleteEmptyLayers();
        surf->setActiveLayer(surf->getLayer(surf->numberOfLayers() - 1));
        uo->accept();
        getModel()->setFileChanged(true);
        emit modifiedModel();
        emit changeActiveLayer();
    } else
        delete uo;
}

void Controller::developLayers()
{
	// TODO
}

void Controller::layerDialogComplete(ShipCAD::LayerDialogData* data)
{
    cout << "Controller::layerDialogComplete" << endl;
    UndoObject* uo = getModel()->createUndo(tr("edit layers"), false);
    bool changed = false;
    vector<SubdivisionLayer*>::iterator itr = getSurface()->getLayers().begin();
    size_t count = 0;
    for (; itr!=getSurface()->getLayers().end(); ++itr) {
        if ((*itr)->setProperties(data->layers[count++]))
            changed = true;
    }
    if (changed) {
        uo->accept();
        getModel()->setFileChanged(true);
        emit modifiedModel();
        emit changeActiveLayer();
    } else
        delete uo;
}

// FreeShipUnit.pas:9316
void Controller::deleteEmptyLayers()
{
    cout << "Controller::deleteEmptyLayers" << endl;
    SubdivisionSurface* surf = getSurface();
    // msg 0140
    UndoObject* uo = getModel()->createUndo(tr("delete empty layers"), false);
    size_t n = surf->deleteEmptyLayers();
    if (surf->getActiveLayer() == 0)
        surf->setActiveLayer(surf->getLayer(surf->numberOfLayers() - 1));
    if (n > 0) {
        uo->accept();
        getModel()->setFileChanged(true);
        emit modifiedModel();
        emit changeActiveLayer();
        // msg 0141
        QString msg(tr("%1 empty layers deleted").arg(n));
        emit displayInfoDialog(msg);
    } else
        delete uo;
}

void Controller::reorderLayerList(LayerDialogData* data)
{
    cout << "Controller::reorderLayerList" << endl;
    getModel()->createUndo(tr("reorder layers"), true);
    vector<SubdivisionLayer*> reordered;
    for (size_t i=0; i<data->layers.size(); i++)
        reordered.push_back(const_cast<SubdivisionLayer*>(data->layers[i].data));
    getSurface()->getLayers().swap(reordered);
    getModel()->setFileChanged(true);
    emit modifiedModel();
    emit changeActiveLayer();
}

// FreeShipUnit.pas:9349
void Controller::newLayer()
{
    cout << "Controller::newLayer" << endl;
    // msg 0142
    getModel()->createUndo(tr("new layer"), true);
    SubdivisionLayer* layer = getSurface()->addNewLayer();
    layer->setColor(getModel()->getPreferences().getLayerColor());
    getModel()->setFileChanged(true);
    emit changeActiveLayer();
    emit modifiedModel();
}

// FreeShipUnit.pas:9365
void Controller::deleteMarkers()
{
    cout << "Controller::deleteMarkers" << endl;
    bool ok;
    // msg 0143
    emit displayQuestionDialog(tr("Are you sure you want to delete all the markers?"), ok);
    if (!ok)
        return;
    // msg 0144
    getModel()->createUndo(tr("delete markers"), true);
    getModel()->getMarkers().clear();
    getModel()->setBuild(false);
    getModel()->setFileChanged(true);
    emit modifiedModel();
}

// FreeShipUnit.pas:9380
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
    cout << "Controller::newModel" << endl;
    clearUndo();
    getModel()->newModel(fuMetric, 15.0, 6.0, 1.0, 10, 8);
    emit modelLoaded();
}

void Controller::lackenbyModelTransformation()
{
	// TODO
}

// FreeShipUnit.pas:10086
void Controller::collapsePoint()
{
    cout << "Controller::collapsePoint" << endl;
    // msg 0160
    UndoObject* uo = getModel()->createUndo(tr("collapse point"), false);
    int n = 0;
    OrderedPointMap& pmap = getSurface()->getSelControlPointCollection();
    for (OrderedPointMap::iterator i=pmap.begin(); i!=pmap.end(); ++i) {
        SubdivisionControlPoint* pt = *i;
        if (!pt->isLocked() && pt->numberOfEdges() == 2) {
            getSurface()->collapseControlPoint(pt);
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

// FreeShipUnit.pas:10113
void Controller::removeUnusedPoint()
{
    cout << "Controller::removeUnusedPoint()" << endl;
    size_t n = 0;
    // msg 0161
    UndoObject* uo = getModel()->createUndo(tr("remove unused points"), false);
    for (size_t i=getSurface()->numberOfControlPoints(); i>=1; i++) {
        SubdivisionControlPoint* pt = getSurface()->getControlPoint(i-1);
        if (pt->numberOfFaces() == 0) {
            getSurface()->deleteControlPoint(pt);
            n++;
        }
    }
    if (n > 0) {
        uo->accept();
        getModel()->setBuild(false);
        getModel()->setFileChanged(true);
        emit modifiedModel();
        // msg 0162
        QString msg("%1 %2");
        msg.arg(n).arg(tr("points removed."));
        emit displayInfoDialog(msg);
    }
    else
        delete uo;
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
    size_t n = getSurface()->numberOfControlPoints();
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
    getSurface()->insertPlane(p, data.addControlCurveSelected);
    if (n < getSurface()->numberOfControlPoints()) {
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
    cout << "Controller::intersectLayerPoint" << endl;
    IntersectLayersDialogData data;
    for (size_t i=0; i<getSurface()->numberOfLayers(); i++) {
        if (getSurface()->getLayer(i)->numberOfFaces() > 0)
            data.layers.push_back(getSurface()->getLayer(i));
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
    SubdivisionSurface* surf = getSurface();
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
    cout << "Controller::newPoint" << endl;
    SubdivisionControlPoint* pt = SubdivisionControlPoint::construct(getSurface());
    pt->setCoordinate(ZERO);
    getSurface()->addControlPoint(pt);
    getSurface()->setSelectedControlPoint(pt);
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
    if (getSurface()->numberOfSelectedControlPoints() != 1)
        throw runtime_error("moving multiple points at once");
    SubdivisionControlPoint* pt = getModel()->getActiveControlPoint();
    if (pt->isLocked()) {
        // msg 0191, warning
        emit displayWarningDialog(tr("Locked points can not be moved!"));
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
    SubdivisionSurface* surf = getSurface();
    cout << "Controller::projectStraightLinePoint" << endl;
    if (surf->numberOfSelectedControlPoints() < 3) {
        // msg 0xxx
        emit displayWarningDialog(tr("At least 3 points must be selected to align"));
        return;
    }
    // determine if the number of points to be moved doesn't contain locked controlpoints only
    // however the first and last points (determining the line segment) are allowed to be locked
    size_t nlocked = 0;
    OrderedPointMap& ptset = getSurface()->getSelControlPointCollection();
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
    cout << "Controller::unlockPoints" << endl;
    if (getSurface()->numberOfSelectedLockedPoints() == 0)
        return;
    // msg 0168
    getModel()->createUndo(tr("unlock points"), true);
    OrderedPointMap& ptset = getSurface()->getSelControlPointCollection();
    for (OrderedPointMap::iterator i=ptset.begin(); i!=ptset.end(); ++i) {
        (*i)->setLocked(false);
    }
    getModel()->setFileChanged(true);
    emit modifiedModel();
}

// FreeShipUnit.pas:10235
void Controller::unlockAllPoints()
{
    cout << "Controller::unlockAllPoints" << endl;
    if (getSurface()->numberOfLockedPoints() == 0)
        return;
    // msg 0169
    getModel()->createUndo(tr("unlock all points"), true);
    for (size_t i=0; i<getSurface()->numberOfControlPoints(); i++) {
        getSurface()->getControlPoint(i)->setLocked(false);
    }
    getModel()->setFileChanged(true);
    // emit dialog msg 0170
    emit modifiedModel();
}

// FreeShipUnit.pas:10251
bool Controller::proceedWhenLockedPoints()
{
    bool proceed = true;
    // show proceed dialog
    // msg 0086 + msg 0087
    QString msg("%1\n%2");
    msg.arg(tr("The selection contains locked points that will not be affected by this operation"))
        .arg(tr("Are you sure you want to continue?"));
    emit displayQuestionDialog(msg, proceed);
    return proceed;
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
    size_t n = getModel()->countSelectedItems();
    if (n == 0)
        return;
    getModel()->clearSelectedItems();
    getModel()->setActiveControlPoint(0);
    emit showControlPointDialog(false);
    emit modifiedModel();
}

// FreeShipUnit.pas:10411
void Controller::deleteSelections()
{
    cout << "Controller::deleteSelections" << endl;
    SubdivisionSurface* surf = getSurface();
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

// FreeShipUnit.pas:10438
void Controller::selectAll()
{
    cout << "Controller::selectAll" << endl;
    for (size_t i=0; i<getSurface()->numberOfLayers(); i++) {
        if (getSurface()->getLayer(i)->isVisible()) {
            for (size_t j=0; j<getSurface()->getLayer(i)->numberOfFaces(); j++)
                getSurface()->getLayer(i)->getFace(j)->setSelected(true);
        }
    }
    for (size_t i=0; i<getSurface()->numberOfControlEdges(); i++) {
        if (getSurface()->getControlEdge(i)->isVisible())
            getSurface()->getControlEdge(i)->setSelected(true);
    }
    for (size_t i=0; i<getSurface()->numberOfControlPoints(); i++) {
        if (getSurface()->getControlPoint(i)->isVisible())
            getSurface()->getControlPoint(i)->setSelected(true);
    }
    for (size_t i=0; i<getSurface()->numberOfControlCurves(); i++) {
        if (getSurface()->getControlCurve(i)->isVisible())
            getSurface()->getControlCurve(i)->setSelected(true);
    }
    for (size_t i=0; i<getModel()->numberOfMarkers(); i++) {
        if (getModel()->getMarkers().get(i)->isVisible())
            getModel()->getMarkers().get(i)->setSelected(true);
    }
    for (size_t i=0; i<getModel()->numberOfFlowlines(); i++) {
        if (getModel()->getFlowline(i)->isVisible())
            getModel()->getFlowline(i)->setSelected(true);
    }
    emit modifiedModel();
}

void Controller::undo()
{
    cout << "Controller::undo" << endl;
    getModel()->undo();
}

void Controller::redo()
{
    cout << "Controller::redo" << endl;
    getModel()->redo();
}

void Controller::clearUndo()
{
    cout << "Controller::clearUndo" << endl;
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
Controller::shadeUnderwater(bool /*val*/)
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
Controller::setPrecision(precision_t prec)
{
    cout << "Controller::setPrecision" << endl;
    if (prec != getModel()->getPrecision()) {
        getModel()->setPrecision(prec);
        getModel()->setFileChanged(true);
        emit modifiedModel();
    }
}

// Main.pas:925
void Controller::setActiveLayer(int layernum)
{
    cout << "Controller::setActiveLayer" << endl;
    size_t idx = to_size_t(layernum);
    if (idx > getSurface()->numberOfLayers())
        return;
    SubdivisionLayer* layer = getSurface()->getLayer(idx);
    if (getSurface()->numberOfSelectedControlFaces() == 0
            && layer != getSurface()->getActiveLayer()) {
        // change the active layer
        getSurface()->setActiveLayer(layer);
        cout << "active layer:" << layer->getName().toStdString() << endl;
    } else if(getSurface()->numberOfSelectedControlFaces() > 0) {
        // assign all selected controlfaces to the new layer
        set<SubdivisionControlFace*>::iterator i =
            getSurface()->getSelControlFaceCollection().begin();
        for (; i!=getSurface()->getSelControlFaceCollection().end(); ++i)
            (*i)->setLayer(layer);
        cout << getSurface()->numberOfSelectedControlFaces()
             << " faces changed to layer:"
             << layer->getName().toStdString() << endl;
    } else
        return;
    getModel()->setFileChanged(true);
    emit modifiedModel();
    emit changeActiveLayer();
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
                               getSurface()->getActiveLayer()->getColor());
    emit exeChooseColorDialog(data);
    if (data.accepted) {
        getModel()->createUndo(tr("change active layer color"), true);
        getSurface()->getActiveLayer()->setColor(data.chosen);
        getModel()->setBuild(false);
        getModel()->setFileChanged(true);
        emit modifiedModel();
    }
}

// Main.pas:952
void Controller::setActiveLayerColor(const QColor& color)
{
	cout << "Controller::setActiveLayerColor with color" << endl;
    getSurface()->getActiveLayer()->setColor(color);
    getModel()->setBuild(false);
    getModel()->setFileChanged(true);
    emit modifiedModel();
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

// called when a layer's faces are selected
void Controller::layerFacesSelected(SubdivisionLayer* layer)
{
    layerFacesSelection(layer, true);
}

// called when a layer's faces are de-selected
void Controller::layerFacesDeselected(SubdivisionLayer* layer)
{
    layerFacesSelection(layer, false);
}

void Controller::layerFacesSelection(SubdivisionLayer* layer, bool selected)
{
    vector<SubdivisionControlFace*>::const_iterator i = layer->faces_begin();
    for(; i!=layer->faces_end(); ++i) {
        if (selected)
            getSurface()->setSelectedControlFace(*i);
        else
            getSurface()->removeSelectedControlFace(*i);
    }
}

void Controller::layerSelectionUpdate(ChooseLayerDialogData* data)
{
    for (size_t i=0; i<getSurface()->numberOfControlPoints(); i++) {
        SubdivisionControlPoint* pt = getSurface()->getControlPoint(i);
        bool select = false;
        if (pt->numberOfFaces() > 0) {
            if (data->include_points) {
                // point must be included in the selection if AT LEAST 1 attached
                // face belongs to a selected layer
                for (size_t j=0; j<pt->numberOfFaces(); j++) {
                    SubdivisionControlFace* face = dynamic_cast<SubdivisionControlFace*>(pt->getFace(j));
                    SubdivisionLayer* layer = face->getLayer();
                    for (size_t k=0; k<data->layers.size(); k++) {
                        if (data->layers[k].first == layer && data->layers[k].second) {
                            select = true;
                            break;
                        }
                    }
                    if (select)
                        break;
                }
            } else {
                // point must be included in the selection only if ALL attached
                // faces belong to to selected layers
                select = true;
                for (size_t j=0; j<pt->numberOfFaces(); j++) {
                    SubdivisionControlFace* face = dynamic_cast<SubdivisionControlFace*>(pt->getFace(j));
                    SubdivisionLayer* layer = face->getLayer();
                    for (size_t k=0; k<data->layers.size(); k++) {
                        if (data->layers[k].first == layer && !data->layers[k].second) {
                            select = false;
                            break;
                        }
                    }
                    if (!select)
                        break;
                }
            }
            pt->setSelected(select);
        }
    }
}
