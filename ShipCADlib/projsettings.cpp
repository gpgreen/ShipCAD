/*##############################################################################################
 *    ShipCAD																				   *
 *    Copyright 2015, by Greg Green <ggreen@bit-builder.com>								   *
 *    Original Copyright header below														   *
 *																							   *
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

#include "shipcadlib.h"
#include "projsettings.h"
#include "utility.h"
#include "shipcad.h"
#include "hydrostaticcalc.h"
#include "subdivlayer.h"
#include "subdivsurface.h"
#include "filebuffer.h"

using namespace std;
using namespace ShipCAD;

//////////////////////////////////////////////////////////////////////////////////////

ProjectSettings::ProjectSettings(ShipCADModel* owner)
	: _owner(owner)
{
	clear();
}

ProjectSettings::~ProjectSettings()
{
}

static void hc_set(HydrostaticCalc* elem)
{
    elem->setCalculated(false);
}

void ProjectSettings::setHydrostaticCoefficients(hydrostatic_coeff_t coeff)
{
	if (coeff != _hydrostatic_coefficients) {
		_hydrostatic_coefficients = coeff;
		_owner->setFileChanged(true);
        _owner->getHydrostaticCalculations().apply(hc_set);
	}
}

void ProjectSettings::setDisableModelCheck(bool val)
{
	if (val != _disable_model_check) {
		_disable_model_check = val;
		_owner->setFileChanged(true);
	}
}

float ProjectSettings::getMainframeLocation()
{
	if (_use_default_mainframe_location)
		return _length / 2;
	else
		return _mainframe_location;
}

void ProjectSettings::setAppendageCoefficient(float coeff)
{
    if (fabs(coeff - _appendage_coefficient) > 1e-5) {
        _appendage_coefficient = coeff;
		_owner->setFileChanged(true);
	}
}

void ProjectSettings::setBeam(float beam)
{
	if (fabs(beam - _beam) > 1e-5) {
		_beam = beam;
		_main_particulars_has_been_set = true;
		_owner->setFileChanged(true);
	}
}

void ProjectSettings::setDraft(float draft)
{
	if (fabs(draft - _draft) > 1e-5) {
		_draft = draft;
		_main_particulars_has_been_set = true;
		_owner->setFileChanged(true);
	}
}

void ProjectSettings::setLength(float length)
{
	if (fabs(length - _length) > 1e-5) {
		_length = length;
		_main_particulars_has_been_set = true;
		_owner->setFileChanged(true);
	}
}

void ProjectSettings::setMainframeLocation(float loc)
{
	if (fabs(loc - _mainframe_location) > 1e-5) {
		_mainframe_location = loc;
		_owner->setFileChanged(true);
	}
}

void ProjectSettings::setWaterDensity(float val)
{
	if (fabs(val - _water_density) > 1e-6) {
		_water_density = val;
		_owner->setFileChanged(true);
	}
}

void ProjectSettings::setSavePreview(bool val)
{
	if (val != _save_preview) {
		_save_preview = val;
		_owner->setFileChanged(true);
	}
}

void ProjectSettings::setStartDraft(float val)
{
	if (fabs(val - _start_draft) > 1e-5) {
		_start_draft = val;
		_owner->setFileChanged(true);
	}
}

void ProjectSettings::setTrim(float val)
{
	if (fabs(val - _trim) > 1e-5) {
		_trim = val;
		_owner->setFileChanged(true);
	}
}

void ProjectSettings::setEndDraft(float val)
{
	if (fabs(val - _end_draft) > 1e-5) {
		_end_draft = val;
		_owner->setFileChanged(true);
	}
}

void ProjectSettings::setDraftStep(float val)
{
	if (fabs(val - _draft_step) > 1e-5) {
		_draft_step = val;
		_owner->setFileChanged(true);
	}
}

void ProjectSettings::setUseDefaultMainframeLocation(bool use)
{
	if (use != _use_default_mainframe_location) {
		_use_default_mainframe_location = use;
		_owner->setFileChanged(true);
	}
}

void ProjectSettings::setName(const QString& name)
{
	if (_name != name) {
		_name = name;
		_owner->setFileChanged(true);
	}
}

void ProjectSettings::setDesigner(const QString& designer)
{
	if (_designer != designer) {
		_designer = designer;
		_owner->setFileChanged(true);
	}
}

void ProjectSettings::setComment(const QString& comment)
{
	if (_comment != comment) {
		_comment = comment;
		_owner->setFileChanged(true);
	}
}

void ProjectSettings::setFileCreatedBy(const QString& createdby)
{
	if (_file_created_by != createdby) {
		_file_created_by = createdby;
		_owner->setFileChanged(true);
	}
}

void ProjectSettings::setShadeUnderwaterShip(bool set)
{
	if (_shade_underwater_ship != set) {
		_shade_underwater_ship = set;
		_owner->setFileChanged(true);
	}
}

void ProjectSettings::setSimplifyIntersections(bool set)
{
	if (_simplify_intersections != set) {
		_simplify_intersections = set;
		// TODO: station,buttock,waterline,diagonal,calcs
	}
}

void ProjectSettings::setUnderWaterColor(QColor& col)
{
	if (_underwater_color != col) {
		_underwater_color = col;
		_owner->setFileChanged(true);
	}
}

void ProjectSettings::setUnits(unit_type_t unit)
{
	double unit_conversion_factor;
	QVector3D scale_vector;
	float weight_factor;
	float thickness_factor;
 	
	if (unit == _units)
		return;
	_units = unit;
	if (_units == fuImperial) {
		unit_conversion_factor = 1 / 0.3048;
        weight_factor = kWeightConversionFactor;
		thickness_factor = 1 / 25.4;
	} else {
		unit_conversion_factor = 0.3048;
        weight_factor = 1 / kWeightConversionFactor;
		thickness_factor = 25.4;
	}
	_water_density *= weight_factor;
    for (size_t i=0; i<_owner->getSurface()->numberOfLayers(); i++) {
		SubdivisionLayer *layer = _owner->getSurface()->getLayer(i);
		layer->setMaterialDensity(layer->getMaterialDensity() * weight_factor);
		layer->setThickness(layer->getThickness() * thickness_factor);
	}
    scale_vector.setX(unit_conversion_factor);
    scale_vector.setY(unit_conversion_factor);
    scale_vector.setZ(unit_conversion_factor);
	// TODO: owner->edit->model_scale
}

void ProjectSettings::clear()
{
	_name = "";
	_designer = "";
	_comment = "";
	_file_created_by = "";
	_length = 1.0;
	_beam = 1.0;
	_draft = 1.0;
	_water_density = 1.025;		// 1025 kg/m3 for seawater
	_appendage_coefficient = 1.0; // default 1.0, typical values for ships to compensate for appendices and shellplate = 1.0005
	_main_particulars_has_been_set = false;
	_shade_underwater_ship = true;
    // TODO:
//	_underwater_color = _owner->getPreferences()->getUnderwaterColor();
	_units = fuMetric;
    _use_default_mainframe_location = true;
	_mainframe_location = 0.0;
	_disable_model_check = false;
	_save_preview = true;
	_hydrostatic_coefficients = fcActualData;
	_simplify_intersections = true;
	_start_draft = 0.0;
	_end_draft = 1.0;
	_draft_step = 0.1;
	_trim = 0.0;
	_displacements.clear();
	_min_displacement = 0.0;
	_max_displacement = 1.0;
	_displ_increment = 0.1;
	_angles.clear();
	_angles.push_back(0.0);
	_angles.push_back(2.0);
	_angles.push_back(5.0);
	_angles.push_back(10.0);
	_angles.push_back(15.0);
	_angles.push_back(20.0);
	_angles.push_back(30.0);
	_angles.push_back(40.0);
	_angles.push_back(50.0);
	_angles.push_back(60.0);
	_stab_trims.clear();
	_stab_trims.push_back(0.0);
	_free_trim = true;
	_fvcg = 1.0;
}

void ProjectSettings::loadBinary(FileBuffer& source, QImage* img)
{
	clear();
    source.load(_name);
    source.load(_designer);
    source.load(_length);
    source.load(_beam);
    source.load(_draft);
    source.load(_main_particulars_has_been_set);
    source.load(_water_density);
    source.load(_appendage_coefficient);
    source.load(_shade_underwater_ship);
    source.load(_underwater_color);
    int n;
    source.load(n);
    _units = static_cast<unit_type_t>(n);
    source.load(_use_default_mainframe_location);
    source.load(_mainframe_location);
    source.load(_disable_model_check);
    source.load(_comment);
    source.load(_file_created_by);
    _save_preview = true;
    if (_owner->getFileVersion() >= fv210) {
        source.load(n);
        _hydrostatic_coefficients = static_cast<hydrostatic_coeff_t>(n);
        source.load(_save_preview);
        if (_save_preview) {
            // TODO load image
        }
        if (_owner->getFileVersion() >= fv230) {
            source.load(_simplify_intersections);
        }
        if (_owner->getFileVersion() >= fv250) {
            source.load(_start_draft);
            source.load(_end_draft);
            source.load(_draft_step);
            source.load(_trim);
            source.load(n);
            for (int i=0; i<n; i++) {
                float f;
                source.load(f);
                _displacements.push_back(f);
            }
            source.load(_min_displacement);
            source.load(_max_displacement);
            source.load(_displ_increment);
            source.load(_use_displ_increments);
            source.load(n);
            for (int i=0; i<n; i++) {
                float f;
                source.load(f);
                _angles.push_back(f);
            }
            source.load(n);
            for (int i=0; i<n; i++) {
                float f;
                source.load(f);
                _stab_trims.push_back(f);
            }
            source.load(_free_trim);
            source.load(_fvcg);
        }
    }
}

void ProjectSettings::saveBinary(FileBuffer& dest)
{
	// TODO:
}

void ProjectSettings::dump(ostream& os) const
{
	// TODO: dump this
}

ostream& operator << (ostream& os, const ShipCAD::ProjectSettings& settings)
{
    settings.dump(os);
    return os;
}
