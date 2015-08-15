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

#ifndef HYDROSTATICCALC_H_
#define HYDROSTATICCALC_H_

#include <vector>
#include <algorithm>
#include <QtCore>
#include <QtGui>
#include "shipcadlib.h"
#include "plane.h"

namespace ShipCAD {

//////////////////////////////////////////////////////////////////////////////////////

class ShipCADModel;
class Plane;
class Intersection;
	
struct HydrostaticsData
{
	QVector3D model_min;
	QVector3D model_max;
	QVector3D wl_min;
	QVector3D wl_max;
	QVector3D sub_min;
	QVector3D sub_max;
	Plane waterline_plane;
	float absolute_draft;
	float volume;
	float displacement;
	QVector3D center_of_buoyancy;
	float lcb_perc;
	float length_waterline;
	float beam_waterline;
	float block_coefficient;
	float wetted_surface;
	QVector3D leak;
	float mainframe_area;
	QVector3D mainframe_cog;
	float mainframe_coeff;
	float waterplane_area;
	QVector3D waterplane_cog;
	float waterplane_entrance_angle;
	float waterplane_coeff;
	QVector2D waterplane_mom_inertia;
	float km_transverse;
	float km_longitudinal;
	float lateral_area;
	QVector3D lateral_cog;
	float prism_coefficient;
	float vert_prism_coefficient;
	std::vector<QVector2D> sac;

    void clear();
};

struct CrosscurvesData
{
	Plane waterline_plane;
	float absolute_draft;
	float volume;
	float displacement;
	QVector3D center_of_buoyancy;
	float kn_sin_phi;

    void clear();
};
	
class HydrostaticCalc : public QObject
{
    Q_OBJECT
public:

    explicit HydrostaticCalc(ShipCADModel* owner);
    ~HydrostaticCalc();

    static HydrostaticCalc* construct(ShipCADModel* owner);
	
	void clear();

    ShipCADModel* getOwner() {return _owner;}

	QString getErrorString();
    float getTrimAngle();
    Plane getWlPlane();
    bool isCalculated() {return _calculated;}
	void setCalculated(bool calc);
	void setDraft(float draft);
    HydrostaticsData& getData() {return _data;}
    /*! \brief does this calculation have this type of error
     *
     * \param error the error to check for
     * \return true if calculation has this error
     */
    bool hasError(HydrostaticError error);
    /*! \brief add this error type to the calculation
     *
     * \param error the error to add
     */
    void addError(HydrostaticError error) {_errors.push_back(error);}
    /*! \brief is this type of calculation set
     *
     * \param ty type of calculation to check for
     * \return true if type is part of set
     */
    bool hasCalculation(HydrostaticsCalculation ty);
    /*! \brief set a type of calculation
     *
     * \param ty type of calculation to add to set
     */
    void addCalculationType(HydrostaticsCalculation ty){_calculations.push_back(ty);}
    float getHeelingAngle() {return _heeling_angle;}
	void setHeelingAngle(float angle);
    void setHydrostaticType(HydrostaticType ty);
    /*! \brief set the trim for this calculation (distance, not angle)
     *
     * \param trim the trim of the hull
     */
	void setTrim(float trim);

	void addData(QStringList& strings, HydrostaticsMode mode, QChar separator);
	void addHeader(QStringList& strings);
	void addFooter(QStringList& strings);
    bool balance(float displacement, bool freetotrim, CrosscurvesData& output);
    /*! \brief make all calculations specified
     *
     * For each type of calculation specified, this method will fill out _data
     * with the results of that calculation
     * The calculated flag will also be set
     */
	void calculate();
    /*! \brief calculate the volume of the ship below plane
     *
     * When this method is completed, then _data will have
     * absolute_draft, errors, leak point, center_of_buoyancy, lcb_perc,
     * length_waterline, width_waterline, displacement, volume calculated
     * The calculated flag will also be set
     *
     * \param waterline_plane the plane of the waterline
     */
    void calculateVolume(const Plane& waterline_plane);

	void showData(HydrostaticsMode mode);
										
public slots:

protected:

private:

    ShipCADModel* _owner;
	float _heeling_angle;
	float _trim;
	float _draft;
	bool _calculated;
	std::vector<HydrostaticError> _errors;
	HydrostaticType _hydrostatic_type;
	HydrostaticsData _data;
	std::vector<HydrostaticsCalculation> _calculations;
    Intersection* _mainframe;
	
};

/*! \brief Vector class to contain HydrostaticCalc pointers
 *
 */
class HydrostaticCalcVector
{
public:
    typedef std::vector<HydrostaticCalc*>::iterator hcvec_iterator ;

    HydrostaticCalcVector();
    ~HydrostaticCalcVector();

    void clear();

    size_t size() {return _vec.size();}

    void add(HydrostaticCalc* c) {_vec.push_back(c);}
    void del(HydrostaticCalc* c);

    typedef void apply_fn(HydrostaticCalc* elem);
    void apply(apply_fn* fn)
      {std::for_each(_vec.begin(), _vec.end(), fn);}

    hcvec_iterator begin() {return _vec.begin();}
    hcvec_iterator end() {return _vec.end();}

private:
    std::vector<HydrostaticCalc*> _vec;
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif

