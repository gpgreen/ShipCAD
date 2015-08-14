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
#include "hydrostaticcalc.h"
#include "shipcadmodel.h"
#include "projsettings.h"
#include "utility.h"
#include "plane.h"

using namespace std;
using namespace ShipCAD;

HydrostaticCalc::HydrostaticCalc(ShipCADModel* owner)
	: _owner(owner), _heeling_angle(0.0), _trim(0.0),
	  _draft(0.0), _calculated(false), _hydrostatic_type(),
	  _mainframe(0)
{
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
	_mainframe.clear();
	_calculations.push_back(hcAll);
}

QString HydrostaticCalc::getErrorString()
{
	// TODO
}

float HydrostaticCalc::getTrimAngle()
{
	float result = RadToDeg(atan((-_trim * cos(DegToRad(_heeling_angle)))
								 / _owner->getProjectSettings().getLength()));
	return result;
}

Plane HydrostaticCalc::getWlPlane()
{
	float lowest_value = _owner->findLowestHydrostaticsPoint();
	QVector3D p1(0.0, 0.0, lowest_value + (_draft - 0.5 * _trim));
	QVector3D p2(_owner->getProjectSettings().getLength(), 0.0,
				 lowest_value + (_draft + 0.5 * _trim));
	QVector3D p3(_owner->getProjectSettings().getLength(),
				 cos(DegToRad(-_heeling_angle)),
				 lowest_value + (_draft + 0.5 * _trim) - sin(DegToRad(_heeling_angle)));
	return Plane(p1, p2, p3);
}

void HydrostaticCalc::setCalculated(bool calc)
{
	_calculated = calc;
	if (!_calculated) {
		_errors.clear();
		_data.clear();
		_mainframe.clear();
	}
}

void HydrostaticCalc::setDraft(float draft)
{
	if (draft != _draft) {
		_draft = draft;
		setCalculated(false);
	}
}

void HydrostaticCalc::addError(HydrostaticError error)
{
	_errors.push_back(error);
}

void HydrostaticCalc::setHeelingAngle(float angle)
{
	if (angle != _heeling_angle) {
		_heeling_angle = angle;
		setCalculated(false);
	}
}

void HydrostaticCalc::setHydrostaticType(HydrostaticType ty)
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

void HydrostaticCalc::addData(QStringList& strings, HydrostaticsMode mode, QChar separator)
{
	// TODO
}

void HydrostaticCalc::addHeader(QStringList& strings)
{
	// TODO
}

void HydrostaticCalc::addFooter(QStringList& strings)
{
	// TODO
}

static float Interpolate(float x, float x1, float y1, float x2, float y2)
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

static QVector3D RotatePointBack(QVector3D& p, float CosTrim, float SinTrim,
								 float CosHeel, float SinHeel)
{
	return QVector3d(p.x() * CosTrim - p.z() * SinTrim,
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

static Plane CalculateWaterlinePlane(float desired_draft, MinMaxData& mmd)
{
	QVector3D p(mmd.lowest_point.x() + desired_draft * mmd.plane_normal.x(),
				mmd.lowest_point.y() + desired_draft * mmd.plane_normal.y(),
				mmd.lowest_point.z() + desired_draft * mmd.plane_normal.z());
	return Plane(p, mmd.plane_normal);
}

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
	for (size_t i=0; i<owner->getNumberOfLayers(); i++) {
		if (owner->getLayer(i)->useInHydrostatics()) {
			SubdivisionLayer* layer = owner->getLayer(i);
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

struct TrimData
{
	float trim;
	float lcb;
};

struct DraftData
{
	float draft;
	float displ;
};

bool HydrostaticCalc::balance(float displacement, bool freetotrim,
							  CrosscurvesData* output)
{
	int max_iterations = 25;
	float max_error = 5e-4;
	float max_trim_error = 1e-4;
	int trim_iteration;
	int displ_iteration;
	float cos_heel;
	float sin_heel;
	float cos_trim;
	float sin_trim;
	float error, trim_error;
	float prev_error;
	Plane wlplane;
	MinMaxData mmd;
	DraftData min_draft;
	DraftData max_draft;
	DraftData curr_draft;
	_data.clear();
	mmd.calculated = false;
	output->clear();

	if (_displacement == 0)
		return true;
	result = false;
	error = 0;
	trim_iteration = 0;
	trim_error = 0;
	do {
		trim_iteration++;
		cos_heel = cos(DegToRad(-_heeling_angle));
		sin_heel = sin(DegToRad(-_heeling_angle));
		cos_trim = cos(DegToRad(_trim_angle));
		sin_trim = sin(DegToRad(_trim_angle));
		if (!mmd.calculated)
			CalculateMinMaxData(mmd, _owner, wlplane, cos_trim, sin_trim,
								cos_heel, sin_heel);
		min_draft.draft = 0;
		min_draft.displ = 0;
		max_draft.draft = mmd.max_draft;
		wlplane = CalculateWaterlinePlane(max_draft.draft, mmd);
		calculateVolume(wlplane);
		max_draft.displ = _data.displacement;
		if (_displacement > 1.005 * max_draft.displ)
			addError(feNotEnoughBuoyancy);
		else {
			displ_iteration = 0;
			prev_error = 0;
			do {
				displ_iteration++;
				curr_draft.draft = Interpolate(_displacement,
											   min_draft.displ,
											   min_draft.draft,
											   max_draft.displ,
											   max_draft.draft);
				wlplane = CalculateWaterlinePlane(curr_draft.draft, mmd);
				calculateVolume(wlplane);
				curr_draft.displ = _data.displacement;
				if (_displacment < 0.1)
					error = fabs(_displacement - curr_draft.displ);
				else
					error = fabs((_displacement - curr_draft.displ) / _displacment);
				if (error > max_error) {
					if (curr_draft.displ < _displacement)
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
		output->waterline_plane = _data.waterline_plane;
		output->absolute_draft = _data.absolute_draft;
		output->volume = _data.volume;
		output->displacement = _data.displacement;
		output->center_of_buoyancy = _data.center_of_buoyancy;
		if (fabs(_heeling_angle) < 1e-5)
			output->center_of_buoyancy.setY(0.0);
	}
	return result;
}

void HydrostaticCalc::calculate()
{
}

void HydrostaticCalc::calculateVolume(Plane& waterline_plane)
{
}

void HydrostaticCalc::showData(HydrostaticsMode mode)
{
}

										

