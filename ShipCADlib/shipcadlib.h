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

namespace ShipCAD {

//////////////////////////////////////////////////////////////////////////////////////

const float kFoot = 0.3048;
const float kLbs = 0.4535924;
const float kWeightConversionFactor = (1000/kLbs)/((1/kFoot)*(1/kFoot)*(1/kFoot));
const int kIncrementSize = 25;
const int kDecimals = 4;
const int kPixelCountMax = 32768;
const float kZBufferScaleFactor = 1.004;
const float kZoomfactor = 1.02;
const int FileBufferBlockSize = 4096;

//////////////////////////////////////////////////////////////////////////////////////

enum viewport_mode_t {
    vmWireFrame,
    vmShade,
    vmShadeGauss,
    vmShadeDevelopable,
    vmShadeZebra
};
enum viewport_type_t {
    fvBodyplan, fvProfile, fvPlan, fvPerspective
};
enum camera_type_t {
  ftWide, ftStandard, ftShortTele, ftMediumTele, ftFarTele
};

enum HydrostaticType {
    fhShort,
    fhExtensive,
};

enum HydrostaticsMode {
};

enum HydrostaticError {
    feNothingSubmerged,
    feMakingWater,
    feNotEnoughBuoyance,
};

enum HydrostaticsCalculation {
    hcAll,
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
    fcActualData
};

enum precision_t {
    fpLow,
    fpMedium,
    fpHigh,
    fpVeryHigh,
};

enum edit_mode_t {
    emSelectItems,
};

enum vertex_type_t {
    svRegular=0,			/**< a regular point, no crease edges */
    svCrease,				/**< crease point, exactly 2 crease edges */
    svDart,					/**< dart point, exactly 1 crease edge */
    svCorner				/**< corner point that has more than 2 crease edges */
};

enum subdiv_mode_t {
    fmQuadTriangle,
    fmCatmullClark
};

enum assemble_mode_t {
    amRegular,
    amNURBS
};

enum model_view_t {
    mvPort=0,
    mvBoth,
};


//////////////////////////////////////////////////////////////////////////////////////


};				/* end namespace */

#endif // SHIPCADLIB_H
