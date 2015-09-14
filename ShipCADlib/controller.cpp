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

#include <QCoreApplication>
#include <QFileDialog>
#include <QSettings>

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

const QStringList& Controller::getRecentFiles() const
{
	return _recent_files;
}

void Controller::addRecentFiles(const QString& filename)
{
	bool already_present = false;
	QString basename(ChangeFileExt(filename, ""));
    for (int i=0; i<_recent_files.size(); i++) {
		if (QString::compare(basename, _recent_files[i], Qt::CaseInsensitive) == 0) {
			already_present = true;
			_recent_files.removeAt(i);
			_recent_files.push_front(filename);
			break;
		}
	}
	if (!already_present) {
		if (_recent_files.size() == 10) {
			_recent_files.pop_back();
		}
		_recent_files.push_front(filename);
	}
    emit changeRecentFiles();
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
        if (filtered.size() == 1) {
            // is this a point?
            SubdivisionControlPoint* cp = dynamic_cast<SubdivisionControlPoint*>(filtered[0]);
            if (cp != 0) {
                cp->setSelected(true);
                scene_changed = true;
                cout << "control point selected" << endl;
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
	bool preview = _model->getProjectSettings().getSavePreview();
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
	// TODO
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

void Controller::loadFile()
{
    cout << "loadFile" << endl;
    // get last directory
    QSettings settings;
    QString lastdir;
    if (settings.contains("file/opendir")) {
        lastdir = settings.value("file/opendir").toString();
    }
    // get the filename
    QString filename = QFileDialog::getOpenFileName(0, tr("Open File"), lastdir);
    if (filename.length() == 0)
        return;
    QFileInfo fi(filename);
    QString filepath = fi.filePath();
    settings.setValue("file/opendir", filepath);
    addRecentFiles(filename);
    FileBuffer source;
    source.loadFromFile(filename);
    _model->loadBinary(source);
    _model->getSurface()->clearSelection();
    // TODO clear selected flowlines
    // TODO clear selected markers
    _model->setFilenameSet(true);
    // stop asking for file version
    _model->setFileChanged(false);
    emit modelLoaded();
}

void Controller::loadFile(const QString& filename)
{
	// TODO
}

void Controller::saveFile()
{
	// TODO
}

void Controller::saveAsFile()
{
	// TODO
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

bool Controller::newModel()
{
    return 0;
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
    if (vis.isShowHydroData() != val) {
        vis.setShowHydroData(val);
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
            emit updateControlPointValue(ap);
            emit modelGeometryChanged();
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
