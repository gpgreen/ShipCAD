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

#include <cmath>
#include <algorithm>
#include "hydrostaticcalc.h"
#include "shipcadmodel.h"
#include "projsettings.h"
#include "utility.h"
#include "plane.h"
#include "shipcadlib.h"
#include "subdivlayer.h"
#include "subdivface.h"
#include "subdivpoint.h"

using namespace std;
using namespace ShipCAD;

void HydrostaticsData::clear()
{
	model_min = model_max = wl_min = wl_max = sub_min = sub_max = ZERO;
    waterline_plane = Plane();
	absolute_draft = volume = displacement = 0;
	center_of_buoyancy = ZERO;
	lcb_perc = length_waterline = beam_waterline = block_coefficient = wetted_surface = 0;
	leak = ZERO;
	mainframe_area = 0;
	mainframe_cog = ZERO;
	mainframe_coeff = waterplane_area = 0;
	waterplane_cog = ZERO;
	waterplane_entrance_angle = waterplane_coeff = 0;
    waterplane_mom_inertia = ZERO2;
	km_transverse = km_longitudinal = lateral_area = 0;
	lateral_cog = ZERO;
	prism_coefficient = vert_prism_coefficient = 0;
	sac.clear();
}

void CrosscurvesData::clear()
{
	waterline_plane = Plane();
	absolute_draft = volume = displacement = 0;
    center_of_buoyancy = ZERO;
	kn_sin_phi = 0;
}

HydrostaticCalc::HydrostaticCalc(ShipCADModel* owner)
	: _owner(owner), _heeling_angle(0.0), _trim(0.0),
      _draft(0.0), _calculated(false), _hydrostatic_type(fhShort),
      _mainframe(new Intersection(owner))
{
    // does nothing
}

HydrostaticCalc::~HydrostaticCalc()
{
	delete _mainframe;
}

HydrostaticCalc* HydrostaticCalc::construct(ShipCADModel* owner)
{
	HydrostaticCalc* hc = new HydrostaticCalc(owner);
	return hc;
}
	
void HydrostaticCalc::clear()
{
	_calculated = false;
	_hydrostatic_type = fhShort;
	_heeling_angle = 0.0;
	_trim = 0.0;
	_draft = 0.0;
	_data.clear();
    _mainframe->clear();
	_calculations.push_back(hcAll);
}

QString HydrostaticCalc::getErrorString() const
{
	const QString spc(" ");
	QString result("");
	if (hasError(feNothingSubmerged))
		result.append(HydrostaticCalc::tr("There is no submersion")+"\n");
    if (hasError(feMakingWater))
		result.append(HydrostaticCalc::tr("The ship is making water at")+spc
                      +QString("%1").arg(_data.leak.x(),0,'f',3)+", "
                      +QString("%1").arg(_data.leak.y(),0,'f',3)+", "
                      +QString("%1").arg(_data.leak.z(),0,'f',3)+"\n");
    return result;
}

float HydrostaticCalc::getTrimAngle() const
{
	float result = RadToDeg(atan((-_trim * cos(DegToRad(_heeling_angle)))
								 / _owner->getProjectSettings().getLength()));
	return result;
}

Plane HydrostaticCalc::getWlPlane() const
{
	float lowest_value = _owner->findLowestHydrostaticsPoint();
	QVector3D p1(0.0, 0.0, lowest_value + (_draft - 0.5 * _trim));
	QVector3D p2(_owner->getProjectSettings().getLength(), 0.0,
				 lowest_value + (_draft + 0.5 * _trim));
	QVector3D p3(_owner->getProjectSettings().getLength(),
				 cos(DegToRad(-_heeling_angle)),
                 lowest_value + (_draft + 0.5 * _trim) - sin(DegToRad(-_heeling_angle)));
	return Plane(p1, p2, p3);
}

void HydrostaticCalc::setCalculated(bool calc)
{
	_calculated = calc;
	if (!_calculated) {
		_errors.clear();
		_data.clear();
        _mainframe->clear();
	}
}

void HydrostaticCalc::setDraft(float draft)
{
	if (draft != _draft) {
		_draft = draft;
		setCalculated(false);
	}
}

void HydrostaticCalc::setHeelingAngle(float angle)
{
	if (angle != _heeling_angle) {
		_heeling_angle = angle;
		setCalculated(false);
	}
}

void HydrostaticCalc::setHydrostaticType(hydrostatic_type_t ty)
{
	if (ty != _hydrostatic_type) {
		_hydrostatic_type = ty;
		setCalculated(false);
	}
}

void HydrostaticCalc::setTrim(float trim)
{
	if (trim != _trim) {
		_trim = trim;
		setCalculated(false);
	}
}

bool HydrostaticCalc::hasError(hydrostatics_error_t error) const
{
    return find(_errors.begin(), _errors.end(), error) != _errors.end();
}

bool HydrostaticCalc::hasCalculation(hydrostatics_calc_t ty) const
{
    return find(_calculations.begin(), _calculations.end(), ty) != _calculations.end();
}

void HydrostaticCalc::addData(QStringList& strings, hydrostatics_mode_t mode, QChar sep)
{
    const QString spc4 = "    ";
	const QString vb = " | ";
    LayerProperties total;

    unit_type_t u = _owner->getProjectSettings().getUnits();
    if (!isCalculated())
        calculate();
    if (_errors.size() == 0) {
        if (mode == fhSingleCalculation) {
            addHeader(strings);
            strings.push_back(HydrostaticCalc::tr("Volume properties")+':');
            strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Displaced volume"),38)+" : "+sep+MakeLength(_data.volume,-1,12)+sep+VolStr(u));
            strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Displacement"),38)+" : "+sep+MakeLength(_data.displacement,-1,12)+sep+WeightStr(u));
            if (_owner->getProjectSettings().getHydrostaticCoefficients() == fcActualData) {
                strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Total length of submerged body"),38)+" : "+sep+MakeLength(_data.sub_max.x()-_data.sub_min.x(),3,12)+sep+LengthStr(u));
                strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Total beam of submerged body"),38)+" : "+sep+MakeLength(_data.sub_max.y()-_data.sub_min.y(),3,12)+sep+LengthStr(u));
            }
            strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Block coefficient"),38)+" : "+sep+MakeLength(_data.block_coefficient,4,12));
            strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Prismatic coefficient"),38)+" : "+sep+MakeLength(_data.prism_coefficient,4,12));
            strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Vert. prismatic coefficient"),38)+" : "+sep+MakeLength(_data.vert_prism_coefficient,4,12));
            strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Wetted surface area"),38)+" : "+sep+MakeLength(_data.wetted_surface,-1,12)+sep+AreaStr(u));
            strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Longitudinal center of buoyancy"),38)+" : "+sep+MakeLength(_data.center_of_buoyancy.x(),-1,12)+sep+LengthStr(u));
            strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Longitudinal center of buoyancy"),38)+" : "+sep+MakeLength(_data.lcb_perc,3,12)+sep+"[%]");
            strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Vertical center of buoyancy"),38)+" : "+sep+MakeLength(_data.center_of_buoyancy.z(),-1,12)+sep+LengthStr(u));
            strings.push_back(HydrostaticCalc::tr("Midship properties")+":");
            strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Midship section area"),38)+" : "+sep+MakeLength(_data.mainframe_area,-1,12)+sep+AreaStr(u));
            strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Midship coefficient"),38)+" : "+sep+MakeLength(_data.mainframe_coeff,4,12));
            strings.push_back(HydrostaticCalc::tr("Waterplane properties")+":");
            strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Length on waterline"),38)+" : "+sep+MakeLength(_data.length_waterline,-1,12)+sep+LengthStr(u));
            strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Beam on waterline"),38)+" : "+sep+MakeLength(_data.beam_waterline,-1,12)+sep+LengthStr(u));
            strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Waterplane area"),38)+" : "+sep+MakeLength(_data.waterplane_area,-1,12)+sep+AreaStr(u));
            strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Waterplane coefficient"),38)+" : "+sep+MakeLength(_data.waterplane_coeff,4,12));
            strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Waterplane center of floatation"),38)+" : "+sep+MakeLength(_data.waterplane_cog.x(),-1,12)+sep+LengthStr(u));
            strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Entrance angle"),38)+" : "+sep+MakeLength(_data.waterplane_entrance_angle,-1,12)+sep+"[degr.]");
            strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Transverse moment of inertia"),38)+" : "+sep+MakeLength(_data.waterplane_mom_inertia.x(),-1,12)+sep+InertiaStr(u));
            strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Longitudinal moment of inertia"),38)+" : "+sep+MakeLength(_data.waterplane_mom_inertia.y(),-1,12)+sep+InertiaStr(u));
            strings.push_back(HydrostaticCalc::tr("Initial stability")+":");
            strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Transverse metacentric height"),38)+" : "+sep+MakeLength(_data.km_transverse,-1,12)+sep+LengthStr(u));
            strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Longitudinal metacentric height"),38)+" : "+sep+MakeLength(_data.km_longitudinal,-1,12)+sep+LengthStr(u));
            strings.push_back(HydrostaticCalc::tr("Lateral plane")+":");
            strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Lateral area"),38)+" : "+sep+MakeLength(_data.lateral_area,-1,12)+sep+AreaStr(u));
            strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Longitudinal center of effort"),38)+" : "+sep+MakeLength(_data.lateral_cog.x(),-1,12)+sep+LengthStr(u));
            strings.push_back(spc4+MakeLength(HydrostaticCalc::tr("Vertical center of effort"),38)+" : "+sep+MakeLength(_data.lateral_cog.z(),-1,12)+sep+LengthStr(u));
        } else {
            if (strings.size() == 0) {
                addHeader(strings);
                strings.push_back(" |  Draft |   Trim  |   Lwl   |   Bwl  |   Vol   |  Displ  |   LCB   |   VCB  |   CB   |   AM   |   CM   |   AW   |   CW   |   CP   |    S   |  KMtrv | KMlong | ");
                if (u == fuImperial)
                    strings.push_back(" |  [ft]  |   [ft]  |   [ft]  |  [ft]  |  [ft3]  |   [t]   |   [ft]  |  [ft]  |        |  [ft2] |        |  [ft2] |        |        |  [ft2] |  [ft]  |  [ft]  | ");
                else
                    strings.push_back(" |  [m]   |   [m]   |   [m]   |   [m]  |   [m3]  |   [t]   |   [m]   |   [m]  |        |  [m2]  |        |  [m2]  |        |        |  [m2]  |   [m]  |   [m]  | ");
                strings.push_back(" |--------+---------+---------+--------+---------+---------+---------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------| ");
            }
            strings.push_back(vb+MakeLength(_draft,3,6)+vb
							  +MakeLength(_trim,3,7)+vb
							  +MakeLength(_data.length_waterline,-1,7)+vb
							  +MakeLength(_data.beam_waterline,-1,6)+vb
							  +MakeLength(_data.volume,-1,7)+vb
							  +MakeLength(_data.displacement,-1,7)+vb
							  +MakeLength(_data.center_of_buoyancy.x(),-1,7)+vb
							  +MakeLength(_data.center_of_buoyancy.z(),-1,6)+vb
							  +MakeLength(_data.block_coefficient,4,6)+vb
							  +MakeLength(_data.mainframe_area,-1,6)+vb
							  +MakeLength(_data.mainframe_coeff,4,6)+vb
							  +MakeLength(_data.waterplane_area,-1,6)+vb
							  +MakeLength(_data.waterplane_coeff,4,6)+vb
							  +MakeLength(_data.prism_coefficient,4,6)+vb
							  +MakeLength(_data.wetted_surface,-1,6)+vb
							  +MakeLength(_data.km_transverse,-1,6)+vb
							  +MakeLength(_data.km_longitudinal,-1,6)+vb);
        }
    } else {
        strings.push_back(getErrorString());
    }
    SubdivisionSurface* surf = _owner->getSurface();
    if (mode == fhSingleCalculation) {
		// check if any layers are present to show
		size_t n = 0;
        for (size_t i=0; i<surf->numberOfLayers(); i++)
            if (surf->getLayer(i)->numberOfFaces() > 0)
				n++;
		if (n > 0) {
			strings.push_back("");
			strings.push_back("");
			strings.push_back(HydrostaticCalc::tr("The following layer properties are calculated for both sides of the ship")+":");
            strings.push_back("|"+MakeLength("",10)
							  +MakeLength(HydrostaticCalc::tr("Layer"),14)+vb
							  +MakeLength(HydrostaticCalc::tr("Area"),6)+vb
							  +MakeLength(HydrostaticCalc::tr("Thickness"),9)+vb
							  +MakeLength(HydrostaticCalc::tr("Weight"),8)+vb
							  +" COG X  |  COG Y  |  COG Z  |");
            strings.push_back("| "+MakeLength("",23)
							  +vb+MakeLength(AreaStr(u),6)
							  +vb+MakeLength("",9)
							  +vb+MakeLength(WeightStr(u),8)
							  +vb+MakeLength(LengthStr(u),7)
							  +vb+MakeLength(LengthStr(u),7)
							  +vb+MakeLength(LengthStr(u),7)+" |");
			strings.push_back("|-------------------------|--------|-----------|----------|---------|---------|---------|");
            for (size_t i=0; i<surf->numberOfLayers(); i++) {
                LayerProperties p = surf->getLayer(i)->getSurfaceProperties();
				if (u == fuImperial)
					p.weight /= (12*2240);
				else
					p.weight /= 1000;
                QString str(surf->getLayer(i)->getName());
                if (str.length() > 23)
                    str.truncate(23);
                strings.push_back("| "+MakeLength(str, 23)
                                  +vb+MakeLength(p.surface_area,-1,6)
                                  +vb+MakeLength(surf->getLayer(i)->getThickness(),3,9)
                                  +vb+MakeLength(p.weight,3,8)
                                  +vb+MakeLength(p.surface_center_of_gravity.x(),3,7)
                                  +vb+MakeLength(p.surface_center_of_gravity.y(),3,7)
                                  +vb+MakeLength(p.surface_center_of_gravity.z(),3,7) + " |");
                total.surface_area += p.surface_area;
                total.surface_center_of_gravity += p.weight * p.surface_center_of_gravity;
			}
            if (n > 1) {
                // if more than 1 layer added, the show the properties of all layers together
                strings.push_back("|-------------------------|--------|-----------|----------|---------|---------|---------|");
                if (total.weight != 0) {
                    total.surface_center_of_gravity += total.surface_center_of_gravity / total.weight;
                }
                const QString spc3("   ");
                strings.push_back("  "+MakeLength(HydrostaticCalc::tr("Total"),23)
                                  +spc3+MakeLength(total.surface_area,-1,6)
                                  +spc3+MakeLength("",9)
                                  +spc3+MakeLength(total.weight,3,8)
                                  +spc3+MakeLength(total.surface_center_of_gravity.x(),3,7)
                                  +spc3+MakeLength(total.surface_center_of_gravity.y(),3,7)
                                  +spc3+MakeLength(total.surface_center_of_gravity.z(),3,7));
                // add sectional area data
                if (_data.sac.size() > 0) {
                    strings.push_back("");
                    strings.push_back("");
                    strings.push_back(HydrostaticCalc::tr("Sectional areas")+":");
                    strings.push_back("");
                    strings.push_back(vb+MakeLength(HydrostaticCalc::tr("Location"),9)
                                      +vb+MakeLength(HydrostaticCalc::tr("Area"),8)+vb);
                    if (u == fuImperial)
                        strings.push_back(" |    [ft]   |   [ft2]  |");
                    else
                        strings.push_back(" |    [m]    |    [m2]  |");
                    strings.push_back(" |-----------+----------|");
                    for (size_t i=0; i<_data.sac.size(); i++) {
                        float position = _data.sac[i].x();
                        strings.push_back(vb+MakeLength(position,3,9)
                                          +vb+MakeLength(_data.sac[i].y(),3,8)
                                          +vb);
                    }
                    strings.push_back(" |-----------+----------|");
                }
            }
		}
        addFooter(strings, mode);
    }
}

void HydrostaticCalc::addHeader(QStringList& strings)
{
	const QString sep(" : ");
	const QString spc(" ");

    ProjectSettings& ps = _owner->getProjectSettings();
	unit_type_t u = ps.getUnits();
	
	strings.push_back(MakeLength(HydrostaticCalc::tr("Project"),21)+sep
					  +ps.getName());
	strings.push_back(MakeLength(HydrostaticCalc::tr("Designer"),21)+sep
					  +ps.getDesigner());
	if (ps.getFileCreatedBy().length() != 0)
		strings.push_back(MakeLength(HydrostaticCalc::tr("Created by"),21)+sep
						  +ps.getFileCreatedBy());
	if (ps.getComment().length() != 0)
		strings.push_back(MakeLength(HydrostaticCalc::tr("Comment"),21)+sep
						  +ps.getComment());
	strings.push_back(MakeLength(HydrostaticCalc::tr("Filename"),21)
					  +sep+_owner->getFilename());
	strings.push_back("");
	strings.push_back(MakeLength(HydrostaticCalc::tr("Design length"),21)+sep
					  +MakeLength(ps.getLength(),-1,10)+spc+LengthStr(u));
	strings.push_back(MakeLength(HydrostaticCalc::tr("Length over all"),21)+sep
					  +MakeLength(_data.model_max.x() - _data.model_min.x(),-1,10)
					  +spc+LengthStr(u));
	strings.push_back(MakeLength(HydrostaticCalc::tr("Design beam"),21)+sep
					  +MakeLength(ps.getBeam(),-1,10)+spc+LengthStr(u));
	strings.push_back(MakeLength(HydrostaticCalc::tr("Beam over all"),21)+sep
					  +MakeLength(_data.model_max.y(),-1,10)
					  +spc+LengthStr(u));
	strings.push_back(MakeLength(HydrostaticCalc::tr("Design draft"),21)+sep
					  +MakeLength(ps.getDraft(),-1,10)+spc+LengthStr(u));
	strings.push_back(MakeLength(HydrostaticCalc::tr("Midship location"),21)+sep
					  +MakeLength(ps.getMainframeLocation(),-1,10)+spc+LengthStr(u));
	strings.push_back(MakeLength(HydrostaticCalc::tr("Water density"),21)+sep
					  +MakeLength(ps.getWaterDensity(),3,10)+spc+DensityStr(u));
	strings.push_back(MakeLength(HydrostaticCalc::tr("Appendage coefficient"),21)+sep
					  +MakeLength(ps.getAppendageCoefficient(),4,10));
}

void HydrostaticCalc::addFooter(QStringList& strings, hydrostatics_mode_t mode)
{
    ProjectSettings& ps = _owner->getProjectSettings();
	strings.push_back(HydrostaticCalc::tr("NOTE 1: Draft (and all other vertical heights) is measured above the lowest point of the hull!")
                      +" (Z= "+QString("%1").arg(_data.model_min.z(),0,'f',3)
                      +")");
	if (ps.getHydrostaticCoefficients() == fcActualData)
		strings.push_back(HydrostaticCalc::tr("NOTE 2: All calculated coefficients based on actual dimensions of submerged body."));
	else
		strings.push_back(HydrostaticCalc::tr("NOTE 2: All calculated coefficients based on project length), draft and beam."));
	strings.push_back("");
	if (mode == fhMultipleCalculations) {
		strings.push_back("Lwl    : "+HydrostaticCalc::tr("Length on waterline"));
		strings.push_back("Bwl    : "+HydrostaticCalc::tr("Beam on waterline"));
		strings.push_back("Volume : "+HydrostaticCalc::tr("Displaced volume"));
		strings.push_back("Displ. : "+HydrostaticCalc::tr("Displacement"));
		strings.push_back("LCB    : "+HydrostaticCalc::tr("Longitudinal center of buoyancy")
						  +", "+HydrostaticCalc::tr("measured from the aft perpendicular at X=0.0"));
		strings.push_back("VCB    : "+HydrostaticCalc::tr("Vertical center of buoyancy")
						  +", "+HydrostaticCalc::tr("measured from the lowest point of the hull"));
		strings.push_back("Cb     : "+HydrostaticCalc::tr("Block coefficient"));
		strings.push_back("Am     : "+HydrostaticCalc::tr("Midship section area"));
		strings.push_back("Cm     : "+HydrostaticCalc::tr("Midship coefficient"));
		strings.push_back("Aw     : "+HydrostaticCalc::tr("Waterplane area"));
		strings.push_back("Cw     : "+HydrostaticCalc::tr("Waterplane coefficient"));
		strings.push_back("LCF    : "+HydrostaticCalc::tr("Waterplane center of floatation"));
		strings.push_back("CP     : "+HydrostaticCalc::tr("Prismatic coefficient"));
		strings.push_back("S      : "+HydrostaticCalc::tr("Wetted surface area"));
		strings.push_back("KMt    : "+HydrostaticCalc::tr("Transverse metacentric height"));
		strings.push_back("KMl    : "+HydrostaticCalc::tr("Longitudinal metacentric height"));
	}
}

// used in balance
static float DisplInterpolate(float x, float x1, float y1, float x2, float y2)
{
	float result;
	
	if (fabs(x2 - x1) < 1e-3)
		result = 0.5 * (y1 + y2);
	else
		result = y1 + (y2 - y1) * (x - x1) / (x2 - x1);
	if (result < y1 || result > y2)
		result = 0.5 * (y1 + y2);
	return result;
}

// used in balance
static QVector3D RotatePointBack(const QVector3D& p, float CosTrim, float SinTrim,
								 float CosHeel, float SinHeel)
{
    return QVector3D(p.x() * CosTrim - p.z() * SinTrim,
					 p.y() * CosHeel + p.x() * SinTrim * SinHeel
					 + p.z() * CosTrim * SinHeel,
					 -p.y() * SinHeel + p.x() * SinTrim * CosHeel
					 + p.z() * CosTrim * CosHeel);
}

struct MinMaxData 
{
	QVector3D lowest_point;
	QVector3D lowest_leak;
	QVector3D plane_normal;
	float max_draft;
	float lowest_z;
	bool calculated;
};

// used in balance
static Plane CalculateWaterlinePlane(float desired_draft, MinMaxData& mmd)
{
    QVector3D p = mmd.lowest_point + desired_draft * mmd.plane_normal;
	return Plane(p, mmd.plane_normal);
}

// used in balance
static void CalculateMinMaxData(MinMaxData& mmd, ShipCADModel* owner,
								Plane& wlplane, float CosTrim, float SinTrim,
								float CosHeel, float SinHeel)
{
	// zero out min max data
	mmd.lowest_point = ZERO;
	mmd.lowest_leak = ZERO;
	mmd.plane_normal = ZERO;
	mmd.max_draft = mmd.lowest_z = 0.0;
	mmd.calculated = false;
	
	QVector3D p1 = RotatePointBack(ZERO, CosTrim, SinTrim, CosHeel, SinHeel);
	QVector3D p2 = RotatePointBack(QVector3D(1.0, 0.0, 0.0),
								   CosTrim, SinTrim, CosHeel, SinHeel);
	QVector3D p3 = RotatePointBack(QVector3D(1.0, 1.0, 0.0),
								   CosTrim, SinTrim, CosHeel, SinHeel);
	// the following plane has the right orientation for the given trim
	// and angle of heel, however the correct distance from the lowest point
	// on the hull has to be calculated
	mmd.plane_normal = UnifiedNormal(p1, p2, p3);
	wlplane = Plane(p1, p2, p3);
	bool first = true;
	bool firstleak = true;
	float min = 0;
	float max = 0;
	float distance;
    SubdivisionSurface* surf = owner->getSurface();
    for (size_t i=0; i<surf->numberOfLayers(); i++) {
        if (surf->getLayer(i)->useInHydrostatics()) {
            SubdivisionLayer* layer = surf->getLayer(i);
			for (size_t j=0; j<layer->numberOfFaces(); j++) {
				SubdivisionControlFace* face = layer->getFace(j);
				for (size_t k=0; k<face->numberOfChildren(); k++) {
					SubdivisionFace* child = face->getChild(k);
					for (size_t l=0; l<child->numberOfPoints(); l++) {
						QVector3D p = child->getPoint(l)->getCoordinate();
						distance = wlplane.a() * p1.x() + wlplane.b() * p1.y()
							+ wlplane.c() * p1.z() + wlplane.d();
						if (first) {
							min = distance;
							max = min;
							mmd.lowest_point = p1;
							mmd.lowest_z = p1.z();
						} else {
							if (distance < min) {
								min = distance;
								mmd.lowest_point = p1;
							} else if (distance > max) {
								max = distance;
							}
							if (p1.z() < mmd.lowest_z)
								mmd.lowest_z = p1.z();
						}
						p1.setY(-p1.y());
						distance = wlplane.a() * p1.x() + wlplane.b() * p1.y()
							+ wlplane.c() * p1.z() + wlplane.d();
						if (distance < min) {
							min = distance;
							mmd.lowest_point = p1;
						} else if (distance > max)
							max = distance;
						// check if this point is a leak point
						if (fabs(p.y()) > 1e-4 && child->getPoint(l)->isBoundaryVertex()) {
							p1 = p;
							if (firstleak) {
								firstleak = false;
								mmd.lowest_leak = p1;
							} else {
								distance = wlplane.a() * p1.x()
									+ wlplane.b() * p1.y()
									+ wlplane.c() * p1.z() + wlplane.d();
								float tmp = wlplane.a() * mmd.lowest_leak.x()
									+ wlplane.b() * mmd.lowest_leak.y()
									+ wlplane.c() * mmd.lowest_leak.z()
									+ wlplane.d();
								if (distance < tmp)
									mmd.lowest_leak = p1;
							}
							p1.setY(-p1.y());
							distance = wlplane.a() * p1.x()
								+ wlplane.b() * p1.y()
								+ wlplane.c() * p1.z() + wlplane.d();
							float tmp = wlplane.a() * mmd.lowest_leak.x()
								+ wlplane.b() * mmd.lowest_leak.y()
								+ wlplane.c() * mmd.lowest_leak.z()
								+ wlplane.d();
							if (distance < tmp)
								mmd.lowest_leak = p1;
						}
					}
				}
			}
		}
	}
	if (!firstleak) {
		// leak points have been found, check if this restricts the max draft
		distance = wlplane.a() * mmd.lowest_leak.x()
			+ wlplane.b() * mmd.lowest_leak.y()
			+ wlplane.c() * mmd.lowest_leak.z()
			+ wlplane.d();
		if (distance < max)
			max = distance;
		mmd.max_draft = max - min - 1e-4;
	}
}

struct DraftData
{
	float draft;
	float displ;
};

bool HydrostaticCalc::balance(float displacement, bool freetotrim,
                              CrosscurvesData& output)
{
    if (displacement == 0)
        return true;

    if (!_owner->isBuild())
        _owner->rebuildModel(true);

    bool result = false;
	int max_iterations = 25;
    float max_error = 5e-4f;
    float max_trim_error = 1e-4f;
    int trim_iteration = 0;
	int displ_iteration;
	float cos_heel;
	float sin_heel;
	float cos_trim;
	float sin_trim;
    float error = 0;
    float trim_error = 0;
    float error_difference = 0;
	float prev_error;
	Plane wlplane;
	MinMaxData mmd;
	DraftData min_draft;
	DraftData max_draft;
	DraftData curr_draft;
	_data.clear();
	mmd.calculated = false;
    output.clear();

    do {
		trim_iteration++;
		cos_heel = cos(DegToRad(-_heeling_angle));
		sin_heel = sin(DegToRad(-_heeling_angle));
        cos_trim = cos(DegToRad(getTrimAngle()));
        sin_trim = sin(DegToRad(getTrimAngle()));
		if (!mmd.calculated)
			CalculateMinMaxData(mmd, _owner, wlplane, cos_trim, sin_trim,
								cos_heel, sin_heel);
		min_draft.draft = 0;
		min_draft.displ = 0;
		max_draft.draft = mmd.max_draft;
		wlplane = CalculateWaterlinePlane(max_draft.draft, mmd);
		calculateVolume(wlplane);
		max_draft.displ = _data.displacement;
        if (displacement > 1.005 * max_draft.displ)
			addError(feNotEnoughBuoyancy);
		else {
			displ_iteration = 0;
			prev_error = 0;
			do {
				displ_iteration++;
                curr_draft.draft = DisplInterpolate(displacement,
													min_draft.displ,
													min_draft.draft,
													max_draft.displ,
													max_draft.draft);
				wlplane = CalculateWaterlinePlane(curr_draft.draft, mmd);
				calculateVolume(wlplane);
				curr_draft.displ = _data.displacement;
                if (displacement < 0.1)
                    error = fabs(displacement - curr_draft.displ);
				else
                    error = fabs((displacement - curr_draft.displ) / displacement);
				if (error > max_error) {
                    if (curr_draft.displ < displacement)
						min_draft = curr_draft;
					else
						max_draft = curr_draft;
				}
				error_difference = fabs(error - prev_error);
				prev_error = error;
			} while ((error >= max_error)
					 && (displ_iteration <= max_iterations)
					 && (error_difference >= 1e-5));
		}
		if (freetotrim) {
			// TODO
		} else
			trim_error = 0;
	} while ((trim_iteration <= max_iterations)
			 && (trim_error <= max_trim_error));
	result = (trim_iteration > max_iterations) && (error <= max_error)
		&& (error_difference < 1e-5);
	if (result) {
        output.waterline_plane = _data.waterline_plane;
        output.absolute_draft = _data.absolute_draft;
        output.volume = _data.volume;
        output.displacement = _data.displacement;
        output.center_of_buoyancy = _data.center_of_buoyancy;
		if (fabs(_heeling_angle) < 1e-5)
            output.center_of_buoyancy.setY(0.0);
	}
	return result;
}

/*! \brief calculate the volume of underwater body
 */
struct VolumeCalc
{
    QVector3D new_origin;
    QVector3D keel;
    float CosTrim;
    float SinTrim;
    float CosHeel;
    float SinHeel;
    bool first_submerged_point;
    bool first_point;
    HydrostaticCalc* hydro_calc;
    HydrostaticsData& data;

	VolumeCalc(const Plane& wl, HydrostaticCalc* hc)
		: first_submerged_point(true), first_point(true),
		  hydro_calc(hc), data(hc->getData())
		{
			data.waterline_plane = wl;
			CosHeel = cos(DegToRad(-hc->getHeelingAngle()));
			SinHeel = sin(DegToRad(-hc->getHeelingAngle()));
			CosTrim = cos(DegToRad(-hc->getTrimAngle()));
			SinTrim = sin(DegToRad(-hc->getTrimAngle()));
            keel = QVector3D(0, 0, hc->getOwner()->findLowestHydrostaticsPoint());
            // in order to calculate the volume enclosed by the underwatership correctly
            // the origin(0,0,0) is projected onto the waterline plane
			new_origin = data.waterline_plane.projectPointOnPlane(ZERO);
			data.absolute_draft = 1000;
		}
	
    // rotate a point at heel=0 and trim=0 position to given trim and heel
    QVector3D RotatePoint(QVector3D p)
    {
        p.setZ(p.z() - keel.z());
        return QVector3D(p.x() * CosTrim + p.y() * SinHeel * SinTrim + p.z() * CosHeel * SinTrim,
                         p.y() * CosHeel - p.z() * SinHeel,
                         -p.x() * SinTrim + p.y() * SinHeel * CosTrim + p.z() * CosHeel * CosTrim);
    }

    void CheckSubmergedBody(QVector3D p, float side)
    {
        p = RotatePoint(p);
        if (first_submerged_point) {
            data.sub_min = p;
            data.sub_max = p;
            first_submerged_point = false;
        } else {
            MinMax(p, data.sub_min, data.sub_max);
        }
        if (side > -1e-5 && side < 1e-5) {
            // point is exactly on waterplane
            if (first_point) {
                // calculate waterline properties
                data.wl_min = p;
                data.wl_max = p;
                first_point = false;
            } else {
                MinMax(p, data.wl_min, data.wl_max);
            }
        }
    }

    void ProcessTriangle(QVector3D p1, QVector3D p2, QVector3D p3)
    {
        QVector3D volume_moment;
        // reposition points with respect to the new projected origin
        p1 -= new_origin;
        p2 -= new_origin;
        p3 -= new_origin;
        QVector3D center = (p1 + p2 + p3) / 3.0f;
        float volume = QVector3D::dotProduct(p1, QVector3D::crossProduct(p2, p3)) / 6.0f;
        if (volume != 0) {
            volume_moment = .75 * volume * center;
            data.volume += volume;
            data.center_of_buoyancy += volume_moment;
        }
        float ax = 0.5 * ((p1.y() - p2.y()) * (p1.z() + p2.z()) + (p2.y() - p3.y()) * (p2.z() + p3.z())
                          + (p3.y() - p1.y()) * (p3.z() + p1.z()));
        float ay = 0.5 * ((p1.z() - p2.z()) * (p1.x() + p2.x()) + (p2.z() - p3.z()) * (p2.x() + p3.x())
                          + (p3.z() - p1.z()) * (p3.x() + p1.x()));
        float az = 0.5 * ((p1.x() - p2.x()) * (p1.y() + p2.y()) + (p2.x() - p3.x()) * (p2.y() + p3.y())
                          + (p3.x() - p1.x()) * (p3.y() + p1.y()));
        data.wetted_surface += sqrt(ax * ax + ay * ay + az * az);
    }

    void run() {
		QVector3D p, p1, p2, p3;
		bool submerged = false;
        float side1, side2, parameter;
        vector<QVector3D> points;
        for (size_t i=0; i<hydro_calc->getOwner()->getSurface()->numberOfLayers(); i++) {
            SubdivisionLayer* layer = hydro_calc->getOwner()->getSurface()->getLayer(i);
            if (!layer->useInHydrostatics()) continue;
            for (size_t j=0; j<layer->numberOfFaces(); j++) {
                SubdivisionControlFace* face = layer->getFace(j);
                for (size_t k=0; k<face->numberOfChildren(); k++) {
                    SubdivisionFace* child = face->getChild(k);
                    // calculate the portside of the model
                    points.clear();
                    p1 = child->getPoint(child->numberOfPoints()-1)->getCoordinate();
                    // calculate on which side of the waterplane this point is
                    side1 = data.waterline_plane.distance(p1);
                    for (size_t l=0; l<child->numberOfPoints(); l++) {
                        p2 = child->getPoint(l)->getCoordinate();
                        side2 = data.waterline_plane.distance(p2);
                        if ((side1 < -1e-5 && side2 > 1e-5) || (side1 > 1e-5 && side2 < -1e-5)) {
                            // the current linesegment between p1-p2 intersects the waterline plane
                            if (side1 == side2)
                                parameter = .5 * (side1 + side2);
                            else
                                parameter = -side1 / (side2 - side1);
                            p = p1 + parameter * (p2 - p1);
                            CheckSubmergedBody(p, 0);
                            points.push_back(p);
                        }
                        if (side2 <= 1e-5) {
                            if (side2 < data.absolute_draft)
                                data.absolute_draft = side2;
                            // p2 lies also on or under the waterlineplane
                            if (side2 < -1e-5) {
                                // point is submerged, check if the model is making water
                                if (child->getPoint(l)->isBoundaryVertex() && fabs(child->getPoint(l)->getCoordinate().y()) > 1e-4) {
                                    if (!hydro_calc->hasError(feMakingWater)) {
                                        hydro_calc->addError(feMakingWater);
                                        data.leak = child->getPoint(l)->getCoordinate();
                                    }
                                }
                            }
                            CheckSubmergedBody(p2, side2);
                            points.push_back(p2);
                        }
                        p1 = p2;
                        side1 = side2;
                    } // end of child face point loop

                    // calculate volume aft of this face
                    if (points.size() > 2)
                        submerged = true;
                    for (size_t l=3; l<=points.size(); l++)
                        ProcessTriangle(points[0], points[l-2], points[l-1]);
                    if (layer->isSymmetric()) {
                        // calculate the starboard side of the model
                        points.clear();
                        p1 = child->getPoint(child->numberOfPoints()-1)->getCoordinate();
                        p1.setY(-p1.y());
                        // calculate on which side of the waterplane this point is
                        side1 = data.waterline_plane.distance(p1);
                        for (size_t l=0; l<child->numberOfPoints(); l++) {
                            p2 = child->getPoint(l)->getCoordinate();
                            p2.setY(-p2.y());
                            side2 = data.waterline_plane.distance(p2);
                            if ((side1 < -1e-5 && side2 > 1e-5) || (side1 > 1e-5 && side2 < -1e-5)) {
                                // the current linesegment between p1-p2 intersects the waterline plane
                                if (side1 == side2)
                                    parameter = .5 * (side1 + side2);
                                else
                                    parameter = -side1 / (side2 - side1);
                                p = p1 + parameter * (p2 - p1);
                                CheckSubmergedBody(p, 0);
                                points.push_back(p);
                            }
                            if (side2 <= 1e-5) {
                                if (side2 < data.absolute_draft)
                                    data.absolute_draft = side2;
                                // p2 lies also on or under the waterlineplane
                                if (side2 < -1e-5) {
                                    // point is submerged, check if the model is making water
                                    if (child->getPoint(l)->isBoundaryVertex() && fabs(child->getPoint(l)->getCoordinate().y()) > 1e-4) {
                                        if (!hydro_calc->hasError(feMakingWater)) {
                                            hydro_calc->addError(feMakingWater);
                                            data.leak = child->getPoint(l)->getCoordinate();
                                        }
                                    }
                                }
                                CheckSubmergedBody(p2, side2);
                                points.push_back(p2);
                            }
                            p1 = p2;
                            side1 = side2;
                        } // end of face point loop
                        // calculate volume aft of this face
                        for (size_t l=3; l<=points.size(); l++)
                            ProcessTriangle(points[0], points[l-1], points[l-2]);
                    } // end of symmetric face if
                } // end of face loop
            } // end of subdiv control face loop
        } // end of layers

        data.absolute_draft = -data.absolute_draft;
        if (first_point) {
            // no intersection with the watersurface found, the ship is either
            // not submerged or totally submerged
            if (!submerged) {
                hydro_calc->addError(feNothingSubmerged);
                data.absolute_draft = 0;
            }
        }
        if (hydro_calc->hasError(feMakingWater)) {
            data.volume = 0;
            data.center_of_buoyancy = ZERO;
        }

		ProjectSettings& ps = hydro_calc->getOwner()->getProjectSettings();

		data.displacement = VolumeToDisplacement(data.volume,
												 ps.getWaterDensity(),
												 ps.getAppendageCoefficient(),
												 ps.getUnits());
		data.length_waterline = data.wl_max.x() - data.wl_min.x();
		data.beam_waterline = data.wl_max.y() - data.wl_min.y();
        if (data.volume != 0) {
            // translate center of buoyancy back to the original origin
            data.center_of_buoyancy = new_origin + data.center_of_buoyancy / data.volume;
            data.center_of_buoyancy = RotatePoint(data.center_of_buoyancy);
            if (data.length_waterline != 0) {
                data.lcb_perc = 100 * (data.center_of_buoyancy.x() - ps.getMainframeLocation()) / data.length_waterline;
            }
            // apply appendage coeff
            data.volume *= ps.getAppendageCoefficient();
        }
    }
};

// used in calculate
struct StationAreaCalculation
{
    HydrostaticsData& data;
    ShipCADModel* owner;
    StationAreaCalculation(HydrostaticsData& d, ShipCADModel* o)
        : data(d), owner(o) {}
    void operator() (Intersection* intersect)
    {
        QVector3D tmp3d;
        QVector2D tmp2d;
        float area;
        Intersection frame(owner, intersect->getIntersectionType(), intersect->getPlane(), true);
        frame.calculateArea(data.waterline_plane, &area, &tmp3d, &tmp2d);
        if (area != 0)
            data.sac.push_back(QVector2D(-frame.getPlane().d(), area));
    }
};

void HydrostaticCalc::calculate()
{
    QVector3D p, p1, p2, p3;
    bool first_point = true;
    float parameter;

    setCalculated(false);
    if (!_owner->isBuild())
        _owner->rebuildModel(true);

    // calculate overall extents of the hull alone
    for (size_t i=0; i<_owner->getSurface()->numberOfLayers(); i++) {
        SubdivisionLayer* layer = _owner->getSurface()->getLayer(i);
        if (layer->useInHydrostatics()) {
            for (size_t j=0; j<layer->numberOfFaces(); j++) {
                SubdivisionControlFace* face = layer->getFace(j);
                for (size_t k=0; k<face->numberOfChildren(); k++) {
                    SubdivisionFace* child = face->getChild(k);
                    for (size_t l=0; l<child->numberOfPoints(); l++) {
                        p2 = child->getPoint(l)->getCoordinate();
                        if (first_point) {
                            _data.model_min = p2;
                            _data.model_max = p2;
                            first_point = false;
                        } else {
                            MinMax(p2, _data.model_min, _data.model_max);
                        }
                    }
                }
            }
        }
    }

    // setup volume calculation using our waterline plane
	VolumeCalc vc(getWlPlane(), this);
	vc.run();
	
    ProjectSettings& ps = _owner->getProjectSettings();

    float submerged_length = _data.sub_max.x() - _data.sub_min.x();
    float submerged_width = _data.sub_max.y() - _data.sub_min.y();

    QVector2D tmpp2d;

    // calculate mainframe properties
    if (_data.volume > 0 && _errors.size() == 0 && (hasCalculation(hcMainframe) || hasCalculation(hcAll))) {
        _mainframe->setIntersectionType(fiStation);
        _mainframe->setUseHydrostaticsSurfacesOnly(true);
        _mainframe->setPlane(Plane(1,0,0,-ps.getMainframeLocation()));
        _mainframe->calculateArea(_data.waterline_plane, &_data.mainframe_area, &_data.mainframe_cog, &tmpp2d);
        _data.mainframe_cog.setZ(_data.mainframe_cog.z() - _data.model_min.z());
        if (ps.getHydrostaticCoefficients() == fcActualData) {
            if (submerged_width * _draft != 0.0) {
                _data.mainframe_coeff = _data.mainframe_area / (submerged_width * _draft);
            }
        } else if (ps.getBeam() * _draft != 0) {
            _data.mainframe_coeff = _data.mainframe_area / (ps.getBeam() * _draft);
        }
    }

    // calculate waterline properties
    if (_data.volume > 0 && _errors.size() == 0 && (hasCalculation(hcWaterline) || hasCalculation(hcAll))) {
        Intersection waterplane(_owner, fiWaterline, _data.waterline_plane, true);
        waterplane.rebuild();
        parameter = -1e6;
        _data.waterplane_entrance_angle = 0;
        SplineVector& wpsplines = waterplane.getSplines();
        for (size_t j=0; j<wpsplines.size(); j++) {
            Spline* spline = wpsplines.get(j);
            // rotate all points back to a horizontal plane
            for (size_t k=0; k<spline->numberOfPoints(); k++) {
                p1 = spline->getPoint(k);
                p2 = vc.RotatePoint(p1);
                spline->setPoint(k, p2);
            }
            if (spline->value(0).x() > spline->value(1).x())
                spline->invert_direction();
            p1 = spline->value(1);
            if (p1.x() > parameter) {
                parameter = p1.x();
                p2 = spline->value(0.99f);
                if (p1.x() - p2.x() != 0)
                    _data.waterplane_entrance_angle = RadToDeg(atan((p2.y() - p1.y()) / (p1.x() - p2.x())));
                else
                    _data.waterplane_entrance_angle = 90.0;
                if (p2.y() < p1.y())
                    _data.waterplane_entrance_angle = -_data.waterplane_entrance_angle;
            }
        }
        waterplane.calculateArea(_data.waterline_plane, &_data.waterplane_area,
                                 &_data.waterplane_cog, &_data.waterplane_mom_inertia);
        if (ps.getBeam() * ps.getLength() != 0)
            _data.waterplane_coeff = _data.waterplane_area / (ps.getBeam() * ps.getLength());
        _data.km_transverse = _data.center_of_buoyancy.z() + _data.waterplane_mom_inertia.x() / _data.volume;
        _data.km_longitudinal = _data.center_of_buoyancy.z() + _data.waterplane_mom_inertia.y() / _data.volume;
    }

    // calculate block and prismatic coefficients
    if (_draft != 0) {
        if (_data.waterplane_area * _draft != 0)
            _data.vert_prism_coefficient = _data.volume / (_data.waterplane_area * _draft);
        if (ps.getHydrostaticCoefficients() == fcActualData) {
            // block coefficient based on length and beam measured on waterline
            if (submerged_width * submerged_length * _draft != 0)
                _data.block_coefficient = _data.volume / (submerged_width * submerged_length * _draft);
            // prismatic coefficient based on length and beam measured on waterline
            if (_data.mainframe_area * submerged_length != 0)
                _data.prism_coefficient = _data.volume / (_data.mainframe_area * submerged_length);
        } else {
            // block coefficient based on length and beam from settings
            if (ps.getLength() * ps.getBeam() * _draft != 0)
                _data.block_coefficient = _data.volume / (ps.getLength() * ps.getBeam() * _draft);
            // prismatic coefficient based on length and beam from settings
            if (_data.mainframe_area * ps.getLength() != 0)
                _data.prism_coefficient = _data.volume / (_data.mainframe_area * ps.getLength());
        }
    }

    // calculate sectional areas
    if (hasCalculation(hcSAC) || hasCalculation(hcAll)) {
        if (_owner->getStations().size()) {
            StationAreaCalculation sac(_data, _owner);
            for_each(_owner->getStations().begin(), _owner->getStations().end(), sac);
        }
    }

    // calculate lateral area and center of gravity
    if (hasCalculation(hcLateralArea) || hasCalculation(hcAll)) {
        Intersection lateralplane(_owner, fiButtock, Plane(0,1,0,-0.001f), true);
        lateralplane.calculateArea(_data.waterline_plane, &_data.lateral_area, &_data.lateral_cog, &tmpp2d);
        _data.lateral_cog.setZ(_data.lateral_cog.z() - _data.model_min.z());
    }

    // all done!
    setCalculated(true);
}


void HydrostaticCalc::calculateVolume(const Plane& waterline_plane)
{
    setCalculated(false);

    if (!_owner->isBuild())
        _owner->rebuildModel(true);

	VolumeCalc vc(waterline_plane, this);
	vc.run();

	setCalculated(true);
}
