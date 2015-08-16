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

QString HydrostaticCalc::getErrorString()
{
	// TODO
    return "";
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

bool HydrostaticCalc::hasError(HydrostaticError error)
{
    return find(_errors.begin(), _errors.end(), error) != _errors.end();
}

bool HydrostaticCalc::hasCalculation(HydrostaticsCalculation ty)
{
    return find(_calculations.begin(), _calculations.end(), ty) != _calculations.end();
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
    for (size_t i=0; i<owner->numberOfLayers(); i++) {
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
    float error_difference;
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
			new_origin = data.waterline_plane.projectPointOnPlane(ZERO);
			data.absolute_draft = 1000;
		}
	
			
    QVector3D RotatePoint(QVector3D &p)
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

    void ProcessTriangle(QVector3D& p1, QVector3D& p2, QVector3D& p3)
    {
        QVector3D volume_moment;
        // reposition points with respect to the new projected origin
        p1 -= new_origin;
        p2 -= new_origin;
        p3 -= new_origin;
        QVector3D center((p1.x() + p2.x() + p3.x()) / 3,
                         (p1.y() + p2.y() + p3.y()) / 3,
                         (p1.z() + p2.z() + p3.z()) / 3);
        float volume = ((p1.z() * (p2.x() * p3.y() - p2.y() * p3.x()))
                        + (p1.y() * (p2.z() * p3.x() - p2.x() * p3.z()))
                        + (p1.x() * (p2.y() * p3.z() - p2.z() * p3.y()))) / 6;
        if (volume != 0) {
            volume_moment = .75 * center * volume;
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
        for (size_t i=0; i<hydro_calc->getOwner()->numberOfLayers(); i++) {
            SubdivisionLayer* layer = hydro_calc->getOwner()->getLayer(i);
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
                            points.push_back(p);
                            CheckSubmergedBody(p, 0);
                        }
                        if (side2 <= 1e-5) {
                            if (side2 < data.absolute_draft)
                                data.absolute_draft = side2;
                            // p2 lies also on or under the waterlineplane
                            points.push_back(p2);
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
                        }
                        p1 = p2;
                        side1 = side2;
                    } // end of child face point loop

                    // calculate volume aft of this face
                    if (points.size() > 2)
                        submerged = true;
                    for (size_t l=3; l<points.size(); l++)
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
                            side2 = data.waterline_plane.distance(p2);
                            if ((side1 < -1e-5 && side2 > 1e-5) || (side1 > 1e-5 && side2 < -1e-5)) {
                                // the current linesegment between p1-p2 intersects the waterline plane
                                if (side1 == side2)
                                    parameter = .5 * (side1 + side2);
                                else
                                    parameter = -side1 / (side2 - side1);
                                p = p1 + parameter * (p2 - p1);
                                points.push_back(p);
                                CheckSubmergedBody(p, 0);
                            }
                            if (side2 <= 1e-5) {
                                if (side2 < data.absolute_draft)
                                    data.absolute_draft = side2;
                                // p2 lies also on or under the waterlineplane
                                points.push_back(p2);
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
                            }
                            p1 = p2;
                            side1 = side2;
                        } // end of face point loop
                        // calculate volume aft of this face
                        for (size_t l=3; l<points.size(); l++)
                            ProcessTriangle(points[0], points[l-2], points[l-1]);
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
                data.lcb_perc = 100 * data.center_of_buoyancy.x() - ps.getMainframeLocation() / data.length_waterline;
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
    size_t index;
    StationAreaCalculation(HydrostaticsData& d, ShipCADModel* o)
        : data(d), owner(o), index(0) {}
    // each time this method is called, the station area is calculated
    // and the index is incremented so that area data goes into the
    // right place
    void operator() (Intersection* intersect)
    {
        QVector3D tmp3d;
        QVector2D tmp2d;
        Intersection *frame = new Intersection(owner, intersect->getIntersectionType(), intersect->getPlane(), true);
        float area;
        frame->calculateArea(data.waterline_plane, &area, &tmp3d, &tmp2d);
        delete frame;
        if (area != 0)
            data.sac.push_back(QVector2D(-frame->getPlane().d(), area));
    }
};

void HydrostaticCalc::calculate()
{
    QVector3D p, p1, p2, p3;
    bool first_point = true;
    float parameter;

    setCalculated(false);

    // calculate overall extents of the hull alone
    for (size_t i=0; i<_owner->numberOfLayers(); i++) {
        SubdivisionLayer* layer = _owner->getLayer(i);
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
        _mainframe->setPlane(Plane(1,0,0,ps.getMainframeLocation()));
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
    if (_data.volume > 0 && _errors.size() == 0 && (hasCalculation(hcWaterline) or hasCalculation(hcAll))) {
        Intersection* waterplane = new Intersection(_owner, fiWaterline, _data.waterline_plane, true);
        waterplane->rebuild();
        parameter = -1e6;
        _data.waterplane_entrance_angle = 0;
        SplineVector& wpsplines = waterplane->getSplines();
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
                p2 = spline->value(0.99);
                if (p1.x() - p2.x() != 0)
                    _data.waterplane_entrance_angle = RadToDeg(atan((p2.y() - p1.y()) / (p1.x() - p2.x())));
                else
                    _data.waterplane_entrance_angle = -_data.waterplane_entrance_angle;
            }
        }
        waterplane->calculateArea(_data.waterline_plane, &_data.waterplane_area, &_data.waterplane_cog, &_data.waterplane_mom_inertia);
        if (ps.getBeam() * ps.getLength() != 0)
            _data.waterplane_coeff = _data.waterplane_area / (ps.getBeam() * ps.getLength());
        delete waterplane;
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
        Intersection* lateralplane = new Intersection(_owner, fiButtock, Plane(0,1,0,0.001), true);
        lateralplane->calculateArea(_data.waterline_plane, &_data.lateral_area, &_data.lateral_cog, &tmpp2d);
        delete lateralplane;
    }

    // all done!
    setCalculated(true);
}


void HydrostaticCalc::calculateVolume(const Plane& waterline_plane)
{
    setCalculated(false);

	VolumeCalc vc(waterline_plane, this);
	vc.run();

	setCalculated(true);
}

void HydrostaticCalc::showData(HydrostaticsMode mode)
{
	// TODO
}

										

