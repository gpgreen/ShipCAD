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

#ifndef VISIBILITY_H
#define VISIBILITY_H

#include <vector>
#include <string>
#include <QtCore>
#include <QtGui>
#include "shipcadlib.h"
#include "tempvar.h"

namespace ShipCAD {

class ShipCADModel;
class FileBuffer;

//////////////////////////////////////////////////////////////////////////////////////

/*! \brief Settings class for visibility of different features
 */
class Visibility : public QObject
{
    Q_OBJECT
public:

	/*! \brief constructor
	 */
    explicit Visibility(ShipCADModel* owner);
	/*! \brief destructor
	 */
    virtual ~Visibility() {}

    /*! \brief get cursor increment
     *
     * \return cursor increment
     */
    float getCursorInc() const
        {return _cursor_increment;}
    void setCursorIncrement(float val);

    /*! \brief get curvature scale
     *
     * \return curvature scale
     */
    float getCurvatureScale() const
        {return _curvature_scale;}

    /*! \brief set curvature scale
     *
     * \param scale curvature scale
     */
    void setCurvatureScale(float val);

    /*! \brief show visibility of buttocks
	 *
	 * \return true if buttocks shown
	 */
	bool isShowButtocks() const
		{ return _show_buttocks;}
	/*! \brief set visibility of buttocks
	 *
	 * \param show visibility of buttocks
	 */
    void setShowButtocks(bool show);
	/*! \brief is control net shown
	 *
	 * \return true if control net shown
	 */
	bool isShowControlNet() const
		{return _show_control_net;}
	/*! \brief set visibility of control net
	 *
	 * \param show visiblity of control net
	 */
    void setShowControlNet(bool show);
    /*! \brief show visibility of curvature
     *
     * \return true if curvature visible
     */
    bool isShowCurvature() const
        {return _show_curvature;}
    /*! \brief set show curvature
     *
     * \param show visibility of curvature
     */
    void setShowCurvature(bool show);
    /*! \brief show visibility of diagonals
     *
     * \return true if diagonals visible
     */
	bool isShowDiagonals() const
		{return _show_diagonals;}
	/*! \brief set show diagonals
	 *
	 * \param show diagonals
	 */
    void setShowDiagonals(bool show);
    /*! \brief show visibility of flowlines
     *
     * \return true if flowlines visible
     */
	bool isShowFlowlines() const
		{ return _show_flowlines;}
	/*! \brief set show flowlines
	 *
	 * \param show flowlines
	 */
    void setShowFlowlines(bool show);
	/*! \brief is grid shown
	 *
	 * \return true if grid shown
	 */
	bool isShowGrid() const
		{return _show_grid;}
	/*! \brief set visibility of grid
	 *
	 * \param show visibility of grid
	 */
    void setShowGrid(bool show);
	/*! \brief get model view setting
	 *
	 * \brief setting of model view
	 */
    model_view_t getModelView() const
        {return _model_view;}
	/*! \brief set model view
	 *
	 * \param vw model view setting
	 */
    void setModelView(model_view_t vw);
	/*! \brief is interior edges shown
	 *
	 * \return true if interior edges shown
	 */
    bool isShowInteriorEdges() const
		{return _show_interior_edges;}
	/*! \brief set visibility of interior edges
	 *
	 * \param show visibility of interior edges
	 */
    void setShowInteriorEdges(bool show);
	/*! \brief show visibility of markers
	 *
	 * \return true if markers visible
	 */
    bool isShowMarkers() const
        {return _show_markers;}
	/*! \brief set visiblity of markers
	 *
	 * \param show visibility of markers
	 */
    void setShowMarkers(bool show);
	/*! \brief show visibility of normals
	 * 
	 * \return true if normals visible
	 */
	bool isShowNormals() const
		{return _show_normals;}
	/*! \brief set visibility of normals
	 *
	 * \param show visibility of normals
	 */
    void setShowNormals(bool show);
	/*! \brief show visibility of stations
	 *
	 * \return true if stations visible
	 */
	bool isShowStations() const
		{return _show_stations;}
	/*! \brief set visibility of stations
	 *
	 * \param show visibility of stations
	 */
    void setShowStations(bool show);
	/*! \brief show visiblity of waterlines
	 *
	 * \return true if waterlines visible
	 */
	bool isShowWaterlines() const
		{return _show_waterlines;}
	/*! \brief set visibility of waterlines
	 *
	 * \param show visibility of waterlines
	 */
    void setShowWaterlines(bool show);
    /*! \brief show visibility of control curves
     *
     * \return true if control curves shown
     */
    bool isShowControlCurves() const
        {return _show_control_curves;}
    /*! \brief set visibility of control curves
     *
     * \param show visibility of control curves
     */
    void setShowControlCurves(bool show);
	/*! \brief show visibility of hydrostatic data
	 *
	 * \return true if hydrostatic data visible
	 */
	bool isShowHydrostaticData() const
		{return _show_hydrostatic_data;}
    /*! \brief set visibility of Hydrostatic data
     *
     * \param show visibility of Hydrostatic data
     */
    void setShowHydrostaticData(bool show);
	/*! \brief show visibility of hydrostatic displacement
	 *
	 * \return true if hydrostatic displacement visible
	 */
	bool isShowHydrostaticDisplacement() const
		{return _show_hydro_displacement;}
    /*! \brief set visibility of Hydrostatic displacement
     *
     * \param show visibility of Hydrostatic displacement
     */
    void setShowHydrostaticDisplacement(bool show);
	/*! \brief show visibility of hydrostatic lateral area
	 *
	 * \return true if hydrostatic lateral area visible
	 */
	bool isShowHydrostaticLateralArea() const
		{return _show_hydro_lateral_area;}
    /*! \brief set visibility of Hydrostatic lateral area
     *
     * \param show visibility of Hydrostatic lateral area
     */
    void setShowHydrostaticLateralArea(bool show);
	/*! \brief show visibility of hydrostatic section areas
	 *
	 * \return true if hydrostatic section areas visible
	 */
	bool isShowHydrostaticSectionalAreas() const
		{return _show_hydro_sectional_areas;}
    /*! \brief set visibility of Hydrostatic section areas
     *
     * \param show visibility of Hydrostatic section areas
     */
    void setShowHydrostaticSectionalAreas(bool show);
	/*! \brief show visibility of hydrostatic metacenter
	 *
	 * \return true if hydrostatic metacenter visible
	 */
	bool isShowHydrostaticMetacentricHeight() const
		{return _show_hydro_metacentric_height;}
    /*! \brief set visibility of Hydrostatic metacenter
     *
     * \param show visibility of Hydrostatic metacenter
     */
    void setShowHydrostaticMetacentricHeight(bool show);
	/*! \brief show visibility of hydrostatic center of flotation
	 *
	 * \return true if hydrostatic center of flotation visible
	 */
	bool isShowHydrostaticLCF() const
		{return _show_hydro_lcf;}
    /*! \brief set visibility of Hydrostatic center of flotation
     *
     * \param show visibility of Hydrostatic center of flotation
     */
    void setShowHydrostaticLCF(bool show);

    void loadBinary(FileBuffer& source);
    void saveBinary(FileBuffer& dest);

    TempVarChange<model_view_t> tempChangeModelView(model_view_t temp)
        { return TempVarChange<model_view_t>(temp, &_model_view); }
    void clear();

    /*! \brief copy settings from dialog
     *
     * \param pointer to dialog visibility state
     */
    void copy_from_dialog(Visibility* dialog_state);
    
    /*! \brief copy settings to dialog data
     *
     * \param reference to original visibility state
     */
    void copy_to_dialog(Visibility& original);

    public slots:

    void decreaseCurvatureScale();
    void increaseCurvatureScale();

protected:

private:

    ShipCADModel* _owner;
    bool _show_control_net;
    bool _show_interior_edges;
    bool _show_stations;
    bool _show_buttocks;
    bool _show_waterlines;
    bool _show_diagonals;
    model_view_t _model_view;
    bool _show_normals;
    bool _show_grid;
    bool _show_markers;
    bool _show_control_curves;
    bool _show_curvature;
    bool _show_hydrostatic_data;
    bool _show_hydro_displacement;
    bool _show_hydro_lateral_area;
    bool _show_hydro_sectional_areas;
    bool _show_hydro_metacentric_height;
    bool _show_hydro_lcf;
    bool _show_flowlines;
    float _curvature_scale;
    float _cursor_increment;

};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */


#endif // VISIBILITY_H

