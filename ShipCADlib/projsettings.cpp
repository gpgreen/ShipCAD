/*##############################################################################################
 *    ShipCAD										       *
 *    Copyright 2015, by Greg Green <ggreen@bit-builder.com>				       *
 *    Original Copyright header below							       *
 *											       *
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
#include "shipcadmodel.h"
#include "hydrostaticcalc.h"
#include "subdivlayer.h"
#include "subdivsurface.h"
#include "filebuffer.h"
#include "backgroundimage.h"

using namespace std;
using namespace ShipCAD;

//////////////////////////////////////////////////////////////////////////////////////

ProjectSettings::ProjectSettings(ShipCADModel* owner)
    : _owner(owner), _preview_img(nullptr)
{
    clear();
}

ProjectSettings::~ProjectSettings()
{
    delete _preview_img;
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

float ProjectSettings::getMainframeLocation() const
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

void ProjectSettings::setDefaultMainframeLocation(bool use)
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

// TODO: does changing this invalidate something
void ProjectSettings::setMinDisplacement(float val)
{
    if (fabs(val - _min_displacement) >= 1e-5) {
        _min_displacement = val;
        _owner->setFileChanged(true);
    }
}

// TODO: does changing this invalidate something
void ProjectSettings::setMaxDisplacement(float val)
{
    if (fabs(val - _max_displacement) >= 1e-5) {
        _max_displacement = val;
        _owner->setFileChanged(true);
    }
}

// TODO: does changing this invalidate something
void ProjectSettings::setUseDisplacementIncrements(bool set)
{
    if (_use_displ_increments != set) {
        _use_displ_increments = set;
        _owner->setFileChanged(true);
    }
}

// TODO: does changing this invalidate something
void ProjectSettings::setDisplacementInc(float val)
{
    if (fabs(val - _displ_increment) >= 1e-5) {
        _displ_increment = val;
        _owner->setFileChanged(true);
    }
}

// TODO: does changing this invalidate something
void ProjectSettings::setFreeTrim(bool set)
{
    if (_free_trim != set) {
        _free_trim = set;
        _owner->setFileChanged(true);
    }
}

// TODO: does changing this invalidate something
void ProjectSettings::setFVCG(float val)
{
    if (fabs(val - _fvcg) >= 1e-5) {
        _fvcg = val;
        _owner->setFileChanged(true);
    }
}


void ProjectSettings::setSimplifyIntersections(bool set)
{
    if (_simplify_intersections != set) {
        _simplify_intersections = set;
        IntersectionVectorIterator i;
        for (i=_owner->getStations().begin(); i!=_owner->getStations().end(); ++i)
            (*i)->setBuild(false);
        for (i=_owner->getButtocks().begin(); i!=_owner->getButtocks().end(); ++i)
            (*i)->setBuild(false);
        for (i=_owner->getWaterlines().begin(); i!=_owner->getWaterlines().end(); ++i)
            (*i)->setBuild(false);
        for (i=_owner->getDiagonals().begin(); i!=_owner->getDiagonals().end(); ++i)
            (*i)->setBuild(false);
        HydrostaticCalcVectorIterator j;
        for (j=_owner->getHydrostaticCalculations().begin();
             j!=_owner->getHydrostaticCalculations().end(); ++j)
            (*j)->setCalculated(false);
    }
}

void ProjectSettings::setUnderWaterColor(QColor col)
{
    if (_underwater_color != col) {
        _underwater_color = col;
        _owner->setFileChanged(true);
    }
}

void ProjectSettings::changeUnitsInSettingsOnly(unit_type_t unit)
{
    double unit_conversion_factor;
    float weight_factor;

    if (unit == _units)
        return;
    _units = unit;
    if (_units == fuImperial) {
        unit_conversion_factor = 1 / 0.3048;
        weight_factor = kWeightConversionFactor;
    } else {
        unit_conversion_factor = 0.3048;
        weight_factor = 1 / kWeightConversionFactor;
    }
    _water_density *= weight_factor;
    setLength(abs(getLength() * unit_conversion_factor));
    setBeam(abs(getBeam() * unit_conversion_factor));
    setDraft(abs(getDraft() * unit_conversion_factor));
    if (!useDefaultMainframeLocation())
        setMainframeLocation(getMainframeLocation() * unit_conversion_factor);
}

bool ProjectSettings::setUnits(unit_type_t unit)
{
    double unit_conversion_factor;
    float weight_factor;
    float thickness_factor;
 	
    if (unit == _units)
        return false;
    _units = unit;
    if (_units == fuImperial) {
        unit_conversion_factor = 1 / 0.3048;
        weight_factor = kWeightConversionFactor;
        thickness_factor = 1 / 25.4f;
    } else {
        unit_conversion_factor = 0.3048;
        weight_factor = 1 / kWeightConversionFactor;
        thickness_factor = 25.4f;
    }
    _water_density *= weight_factor;
    for (size_t i=0; i<_owner->getSurface()->numberOfLayers(); i++) {
        SubdivisionLayer *layer = _owner->getSurface()->getLayer(i);
        layer->setMaterialDensity(layer->getMaterialDensity() * weight_factor);
        layer->setThickness(layer->getThickness() * thickness_factor);
    }
    QVector3D scale_vector(unit_conversion_factor,
                           unit_conversion_factor,
                           unit_conversion_factor);
    _owner->scaleModel(scale_vector, true, true); // scale and redraw
    return true;
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
    _water_density = 1.025f;		// 1025 kg/m3 for seawater
    _appendage_coefficient = 1.0; // default 1.0, typical values for ships to compensate for appendices and shellplate = 1.0005
    _main_particulars_has_been_set = false;
    _shade_underwater_ship = true;
    _underwater_color = _owner->getPreferences().getUnderwaterColor();
    _units = fuMetric;
    _use_default_mainframe_location = true;
    _mainframe_location = 0.0;
    _disable_model_check = false;
    _save_preview = false;
    delete _preview_img;
    _preview_img = nullptr;
    _hydrostatic_coefficients = fcActualData;
    _simplify_intersections = true;
    _use_displ_increments = true;
    _start_draft = 0.0;
    _end_draft = 1.0;
    _draft_step = 0.1f;
    _trim = 0.0;
    _displacements.clear();
    _min_displacement = 0.0;
    _max_displacement = 1.0;
    _displ_increment = 0.1f;
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

// FreeShipUnit.pas:11311
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
    quint32 n;
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
            _preview_img = new JPEGImage();
            source.load(*_preview_img);
            if (img != nullptr)
                *img = CreateFromJPEG(_preview_img);
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
            for (size_t i=0; i<n; i++) {
                float f;
                source.load(f);
                _displacements.push_back(f);
            }
            source.load(_min_displacement);
            source.load(_max_displacement);
            source.load(_displ_increment);
            source.load(_use_displ_increments);
            source.load(n);
            _angles.clear();
            for (size_t i=0; i<n; i++) {
                float f;
                source.load(f);
                _angles.push_back(f);
            }
            source.load(n);
            _stab_trims.clear();
            for (size_t i=0; i<n; i++) {
                float f;
                source.load(f);
                _stab_trims.push_back(f);
            }
            source.load(_free_trim);
            source.load(_fvcg);
        }
    }
}

// FreeShipUnit.pas:11379
void ProjectSettings::saveBinary(FileBuffer& dest)
{
    if (_owner->getFileVersion() >= fv120) {
        dest.add(_name);
        dest.add(_designer);
        dest.add(_length);
        dest.add(_beam);
        dest.add(_draft);
        dest.add(_main_particulars_has_been_set);
        dest.add(_water_density);
        dest.add(_appendage_coefficient);
        dest.add(_shade_underwater_ship);
        dest.add(_underwater_color);
        dest.add(static_cast<quint32>(_units));
        if (_owner->getFileVersion() >= fv160) {
            dest.add(_use_default_mainframe_location);
            dest.add(_mainframe_location);
            if (_owner->getFileVersion() >= fv165) {
                dest.add(_disable_model_check);
            }
        }
        dest.add(_comment);
        dest.add(_file_created_by);
        if (_owner->getFileVersion() >= fv210) {
            dest.add(static_cast<quint32>(_hydrostatic_coefficients));
            dest.add(_save_preview);
            if (_save_preview) {
                if (_preview_img == nullptr)
                    throw logic_error("trying to save preview image that doesn't exist");
                dest.add(*_preview_img);
            }
            if (_owner->getFileVersion() >= fv230) {
                dest.add(_simplify_intersections);
            }
            if (_owner->getFileVersion() >= fv250) {
                dest.add(_start_draft);
                dest.add(_end_draft);
                dest.add(_draft_step);
                dest.add(_trim);
                dest.add(static_cast<quint32>(_displacements.size()));
                for (size_t i=0; i<_displacements.size(); i++)
                    dest.add(_displacements[i]);
                dest.add(_min_displacement);
                dest.add(_max_displacement);
                dest.add(_displ_increment);
                dest.add(_use_displ_increments);
                dest.add(static_cast<quint32>(_angles.size()));
                for (size_t i=0; i<_angles.size(); i++)
                    dest.add(_angles[i]);
                dest.add(static_cast<quint32>(_stab_trims.size()));
                for (size_t i=0; i<_stab_trims.size(); i++)
                    dest.add(_stab_trims[i]);
                dest.add(_free_trim);
                dest.add(_fvcg);
            }
        }
    }
}

void ProjectSettings::copy_from_dialog(ProjectSettings& dialog_state)
{
    _name = dialog_state._name;
    _designer = dialog_state._designer;
    _comment = dialog_state._comment;
    _file_created_by = dialog_state._file_created_by;
    _shade_underwater_ship = dialog_state._shade_underwater_ship;
    _save_preview = dialog_state._save_preview;
    _simplify_intersections = dialog_state._simplify_intersections;
    _length = dialog_state._length;
    _beam = dialog_state._beam;
    _draft = dialog_state._draft;
    _mainframe_location = dialog_state._mainframe_location;
    _use_default_mainframe_location = dialog_state._use_default_mainframe_location;
    _water_density = dialog_state._water_density;
    _appendage_coefficient = dialog_state._appendage_coefficient;
    _hydrostatic_coefficients = dialog_state._hydrostatic_coefficients;
    _disable_model_check = dialog_state._disable_model_check;
}

void ProjectSettings::copy_to_dialog(ProjectSettings& original)
{
    _name = original._name;
    _designer = original._designer;
    _comment = original._comment;
    _file_created_by = original._file_created_by;
    _units = original._units;
    _shade_underwater_ship = original._shade_underwater_ship;
    _save_preview = original._save_preview;
    _simplify_intersections = original._simplify_intersections;
    _length = original._length;
    _beam = original._beam;
    _draft = original._draft;
    _mainframe_location = original._mainframe_location;
    _use_default_mainframe_location = original._use_default_mainframe_location;
    _water_density = original._water_density;
    _appendage_coefficient = original._appendage_coefficient;
    _hydrostatic_coefficients = original._hydrostatic_coefficients;
    _disable_model_check = original._disable_model_check;
}

void ProjectSettings::dump(ostream& os) const
{
    os << "ProjectSettings";
}

ostream& operator << (ostream& os, const ShipCAD::ProjectSettings& settings)
{
    settings.dump(os);
    return os;
}
