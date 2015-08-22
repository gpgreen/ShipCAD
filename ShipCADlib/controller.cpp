/*##############################################################################################
 *    ShipCAD
 *    Copyright 2015, by Greg Green <ggreen@bit-builder.com>
 *    Original Copyright header below
 *
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

#include "controller.h"
#include "shipcadmodel.h"
#include "utility.h"
#include "undoobject.h"

using namespace ShipCAD;
using namespace std;

Controller::Controller(ShipCADModel* model)
        : _model(model)
{
    // setup signals and slots
    connect(_model, SIGNAL(onFileChanged()), SLOT(modelFileChanged()));
    connect(_model, SIGNAL(onUpdateGeometryInfo()), SLOT(modelGeometryChanged()));
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
    return 0;
}

void Controller::addCurve()
{
}

void Controller::collapseEdges()
{
}

void Controller::connectEdges()
{
}

void Controller::creaseEdges()
{
}

void Controller::extrudeEdges()
{
}

void Controller::splitEdges()
{
}

void Controller::assembleFace()
{
}

void Controller::deleteNegativeFaces()
{
}

void Controller::flipFaces()
{
}

void Controller::mirrorPlaneFace()
{
}

void Controller::newFace()
{
}

void Controller::rotateFaces()
{
}

void Controller::scaleFaces()
{
}

void Controller::moveFaces()
{
}

void Controller::exportFileArchimedes()
{
}

void Controller::exportCoordinates()
{
}

void Controller::export2DPolylinesDXF()
{
}

void Controller::export3DPolylinesDXF()
{
}

void Controller::exportFacesDXF()
{
}

void Controller::exportFEF()
{
}

void Controller::exportGHS()
{
}

void Controller::exportPart()
{
}

void Controller::exportMichlet()
{
}

void Controller::importMichletWaves()
{
}

void Controller::exportObj()
{
}

void Controller::exportOffsets()
{
}

void Controller::exportSTL()
{
}

void Controller::importCarene()
{
}

void Controller::importChines()
{
}

void Controller::importFEF()
{
}

void Controller::importHull()
{
}

void Controller::importPart()
{
}

void Controller::importPolycad()
{
}

void Controller::importSurface()
{
}

void Controller::importVRML()
{
}

void Controller::loadFile()
{
}

void Controller::loadFile(const QString& filename)
{
}

void Controller::saveFile()
{
}

void Controller::saveAsFile()
{
}

void Controller::addFlowline(const QVector2D& source, viewport_type_t view)
{
}

void Controller::calculateHydrostatics(float draft, float heel_angle, float trim)
{
}

void Controller::crossCurvesHydrostatics()
{
}

void Controller::hydrostaticsDialog()
{
}

void Controller::importFrames()
{
}

Intersection* Controller::addIntersection(intersection_type_t ty, float distance)
{
    return 0;
}

void Controller::addIntersectionToList(Intersection* inter)
{
}

void Controller::intersectionDialog()
{
}

void Controller::autoGroupLayer()
{
}

void Controller::developLayers()
{
}

void Controller::layerDialog()
{
}

void Controller::deleteEmptyLayers(bool quiet)
{
}

SubdivisionLayer* Controller::newLayer()
{
    return 0;
}

void Controller::addMarker(Marker* marker)
{
}

void Controller::deleteMarkers()
{
}

void Controller::importMarkers()
{
}

void Controller::checkModel(bool showResult)
{
}

bool Controller::newModel()
{
    return 0;
}

void Controller::lackenbyModelTransformation()
{
}

void Controller::scaleModel(QVector3D scale_vector, bool overrideLock, bool adjustMarkers)
{
}

void Controller::collapsePoint()
{
}

void Controller::removeUnusedPoint()
{
}

void Controller::insertPlane()
{
}

void Controller::intersectLayerPoint()
{
}

void Controller::lockPoints()
{
}

SubdivisionControlPoint* Controller::newPoint()
{
    return 0;
}

void Controller::projectStraightLinePoint()
{
}

void Controller::unlockPoints()
{
}

void Controller::unlockAllPoints()
{
}

bool Controller::proceedWhenLockedPoints()
{
    return false;
}

void Controller::redo()
{
}

void Controller::delftResistance()
{
}

void Controller::kaperResistance()
{
}

void Controller::clearSelections()
{
}

void Controller::deleteSelections()
{
}

void Controller::selectAll()
{
}

void Controller::undo()
{
}

void Controller::clearUndo()
{
}

void Controller::showHistoryUndo()
{
}

void Controller::modelFileChanged()
{
    emit changedModel();
}

void Controller::modelGeometryChanged()
{
}
