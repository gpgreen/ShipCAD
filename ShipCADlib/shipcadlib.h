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

#ifndef SHIPCADLIB_H
#define SHIPCADLIB_H

#include <QtCore>
#include <QtGui>
#include <vector>

namespace ShipCAD {

// forward declarations
class SubdivisionControlPoint;

//////////////////////////////////////////////////////////////////////////////////////

const float kFoot = 0.3048f;
const float kLbs = 0.4535924f;
const float kWeightConversionFactor = (1000/kLbs)/((1/kFoot)*(1/kFoot)*(1/kFoot));
const int kIncrementSize = 25;
const int kDecimals = 4;
const int kPixelCountMax = 32768;
const float kZBufferScaleFactor = 1.004f;
const float kZoomfactor = 1.02f;
const int FileBufferBlockSize = 4096;

extern const char* kFileExtension; /**< default binary file extension */
	
extern const QVector3D ZERO; /**< vector(0,0,0) */
extern const QVector3D ONE;  /**< vector(1,1,1) */
extern const QVector2D ZERO2; /**< vector(0,0) */

//////////////////////////////////////////////////////////////////////////////////////

enum viewport_mode_t {
    vmWireFrame = 0,
    vmShade,
    vmShadeGauss,
    vmShadeDevelopable,
    vmShadeZebra,
};

enum viewport_type_t {
    fvBodyplan = 0,
    fvProfile,
    fvPlan,
    fvPerspective,
};

enum camera_type_t {
    ftWide = 0,
    ftStandard,
    ftShortTele,
    ftMediumTele,
    ftFarTele,
};

enum hydrostatic_type_t {
    fhShort = 0,
    fhExtensive,
};

enum hydrostatics_mode_t {
    fhSingleCalculation = 0,
    fhMultipleCalculations,
};

enum hydrostatics_error_t {
    feNothingSubmerged = 0,
    feMakingWater,
    feNotEnoughBuoyancy,
};

enum hydrostatics_calc_t {
    hcAll = 0,
    hcVolume,
    hcMainframe,
    hcWaterline,
    hcSAC,
    hcLateralArea,
};

enum intersection_type_t {
    fiFree = 0,
    fiStation,
    fiButtock,
    fiWaterline,
    fiDiagonal,
};

enum unit_type_t {
    fuMetric = 0,
    fuImperial,
};

enum hydrostatic_coeff_t {
    fcProjectSettings = 0,
    fcActualData,
};

enum precision_t {
    fpLow = 0,
    fpMedium,
    fpHigh,
    fpVeryHigh,
};

enum edit_mode_t {
    emSelectItems = 0,
};

enum vertex_type_t {
    svRegular = 0,			/**< a regular point, no crease edges */
    svCrease,				/**< crease point, exactly 2 crease edges */
    svDart,					/**< dart point, exactly 1 crease edge */
    svCorner				/**< corner point that has more than 2 crease edges */
};

enum subdiv_mode_t {
    fmQuadTriangle = 0,
    fmCatmullClark,
};

enum assemble_mode_t {
    amRegular = 0,
    amNURBS,
};

enum model_view_t {
    mvPort = 0,
    mvBoth,
};

enum plane_selected_t {
    transverse = 0,
    horizontal,
    vertical
};
    
//////////////////////////////////////////////////////////////////////////////////////

struct LayerProperties
{
    float surface_area;
    float weight;
    QVector3D surface_center_of_gravity;
    LayerProperties() : surface_area(0), weight(0) 
        {}
};

//////////////////////////////////////////////////////////////////////////////////////

/*! \brief used to collect statistics when doing the model check
 */
struct SurfaceCheckResult
{
    size_t non_manifold;
    size_t inconsistent;
    size_t inverted_faces;
    size_t double_edges;
    std::vector<SubdivisionControlPoint*> leaks;
};

//////////////////////////////////////////////////////////////////////////////////////

extern QString AreaStr(unit_type_t units);
extern QString LengthStr(unit_type_t units);
extern QString InertiaStr(unit_type_t units);
extern QString VolStr(unit_type_t units);
extern QString DensityStr(unit_type_t units);
extern QString WeightStr(unit_type_t units);

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif // SHIPCADLIB_H
