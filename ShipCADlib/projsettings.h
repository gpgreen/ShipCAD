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

#ifndef PROJSETTINGS_H_
#define PROJSETTINGS_H_

#include <vector>
#include <iosfwd>
#include <QtCore>
#include <QtGui>
#include "shipcadlib.h"

namespace ShipCAD {

class ShipCADModel;
class FileBuffer;
struct JPEGImage;
	
//////////////////////////////////////////////////////////////////////////////////////

class ProjectSettings : public QObject
{
    Q_OBJECT
public:

    explicit ProjectSettings(ShipCADModel* owner);
    virtual ~ProjectSettings();

    hydrostatic_coeff_t getHydrostaticCoefficients()
        {return _hydrostatic_coefficients;}
	void setHydrostaticCoefficients(hydrostatic_coeff_t coeff);

    bool isDisableModelCheck() const
        { return _disable_model_check; }
	void setDisableModelCheck(bool val);

    float getAppendageCoefficient() const
        {return _appendage_coefficient;}
	void setAppendageCoefficient(float coeff);

    bool isMainParticularsSet() const
        { return _main_particulars_has_been_set; }
    
    float getBeam() const
        {return _beam;}
	void setBeam(float beam);

    float getDraft() const
        {return _draft;}
	void setDraft(float draft);

    float getLength()
        {return _length;}
	void setLength(float length);

    float getMainframeLocation() const;
    void setMainframeLocation(float loc);
	void setUseDefaultMainframeLocation(bool use);

    float getWaterDensity() const
        {return _water_density;}
	void setWaterDensity(float val);

    /*! \brief get the Save Preview setting
     */
    bool isSavePreview() const
        {return _save_preview;}
    /*! \brief set the Save Preview setting
     */
    void setSavePreview(bool val);

    bool useDisplacementIncrements() const
        { return _use_displ_increments; }
    void setUseDisplacementIncrements(bool set);

    float getStartDraft() const
        { return _start_draft; }
	void setStartDraft(float val);

    float getTrim() const
        { return _trim; }
	void setTrim(float val);

    float getEndDraft() const
        { return _end_draft; }
	void setEndDraft(float val);

    float getDraftStep() const
        { return _draft_step; }
	void setDraftStep(float val);

    float getMinDisplacement() const
        { return _min_displacement; }
    void setMinDisplacement(float val);

    float getMaxDisplacement() const
        { return _max_displacement; }
    void setMaxDisplacement(float val);

    float getDisplacementInc() const
        { return _displ_increment; }
    void setDisplacementInc(float val);

    bool isFreeTrim() const
        { return _free_trim; }
    void setFreeTrim(bool set);

    float getFVCG() const
        { return _fvcg; }
    void setFVCG(float val);
    
    QString getName() const
        {return _name;}
	void setName(const QString& name);

    QString getDesigner() const
        {return _designer;}
	void setDesigner(const QString& designer);

    QString getComment() const
        {return _comment;}
	void setComment(const QString& comment);

    QString getFileCreatedBy() const
        {return _file_created_by;}
	void setFileCreatedBy(const QString& createdby);

    bool isShadeUnderwaterShip() const 
        {return _shade_underwater_ship;}
    void setShadeUnderwaterShip(bool set);

    bool isSimplifyIntersections() const
        {return _simplify_intersections;}
	void setSimplifyIntersections(bool set);

    QColor getUnderWaterColor() const
        { return _underwater_color;}
	void setUnderWaterColor(QColor col);

    unit_type_t getUnits() const {return _units;}
	void setUnits(unit_type_t unit);
								   
    void loadBinary(FileBuffer& source, QImage* img);
    void saveBinary(FileBuffer& dest);
	
	void clear();
				
	void dump(std::ostream& os) const;
	
public slots:

protected:

private:

    // define away
    ProjectSettings(const ProjectSettings&);
    ProjectSettings& operator=(const ProjectSettings&);

    ShipCADModel* _owner;
	bool _main_particulars_has_been_set;
	bool _disable_model_check;
	float _appendage_coefficient;
	float _beam;
	float _draft;
	float _length;
	float _water_density;
	float _mainframe_location;
	bool _use_default_mainframe_location;
	QString _name;
	QString _designer;
	QString _comment;
	QString _file_created_by;
	bool _shade_underwater_ship;
	bool _save_preview;
	QColor _underwater_color;
	unit_type_t _units;
	bool _simplify_intersections;
	hydrostatic_coeff_t _hydrostatic_coefficients;
	float _start_draft;
	float _end_draft;
	float _draft_step;
	float _trim;
	std::vector<float> _displacements;
	float _min_displacement;
	float _max_displacement;
	float _displ_increment;
	bool _use_displ_increments;
	std::vector<float> _angles;
	std::vector<float> _stab_trims;
	bool _free_trim;
	float _fvcg;
	JPEGImage* _preview_img;
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

std::ostream& operator << (std::ostream& os, const ShipCAD::ProjectSettings& settings);
	
#endif

