#############################################################################################}
#    This code is distributed as part of the FREE!ship project. FREE!ship is an               }
#    open source surface-modelling program based on subdivision surfaces and intended for     }
#    designing ships.                                                                         }
#                                                                                             }
#    Copyright © 2005, by Martijn van Engeland                                                }
#    e-mail                  : Info@FREEship.org                                              }
#    FREE!ship project page  : https://sourceforge.net/projects/freeship                      }
#    FREE!ship homepage      : www.FREEship.org                                               }
#                                                                                             }
#    This program is free software; you can redistribute it and/or modify it under            }
#    the terms of the GNU General Public License as published by the                          }
#    Free Software Foundation; either version 2 of the License, or (at your option)           }
#    any later version.                                                                       }
#                                                                                             }
#    This program is distributed in the hope that it will be useful, but WITHOUT ANY          }
#    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A          }
#    PARTICULAR PURPOSE. See the GNU General Public License for more details.                 }
#                                                                                             }
#    You should have received a copy of the GNU General Public License along with             }
#    this program; if not, write to the Free Software Foundation, Inc.,                       }
#    59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                                    }
#                                                                                             }
#############################################################################################}

## interface

import VersionUnit

## uses Windows,
##      FasterList,
##      Messages,
##      SysUtils,
##      Classes,
##      Graphics,
##      Controls,
##      Forms,
##      Math,
##      Dialogs,
##      buttons,
##      StdCtrls,
##      Printers,
##      StrUtils,
##      JPeg,
##      ExtCtrls;

Foot                          = 0.3048
Lbs                           = 0.4535924
WeightConversionFactor        = (1000/Lbs)/((1/Foot)*(1/Foot)*(1/Foot))
IncrementSize                 = 25                            # amount of points which is automaticly allocated extra memory for
Decimals                      = 4                             # When weilding points together this is the accuracy for comparing points
PixelCountMax                 = 32768                         # used for faster pixel acces when shading to viewport
EOL                           = "\r\n"
ZBufferScaleFactor            = 1.004                         # Offset for hidden-line drawing when drawing ontop of shaded triangles
Zoomfactor                    = 1.02
FileBufferBlockSize           = 4096                          # used for reading and writing files using TFilebuffer

# TColors
DXFLayerColors = [
    $0000FF, $00FFFF, $00FF00, $FFFF00, $FF0000, $FF00FF, $000000, $808080, $C0C0C0, $0000FF,
    $7F7FFF, $0000A5, $5252A5, $00007F, $3F3F7F, $00004C, $26264C, $000026, $131326, $003FFF,
    $7F9FFF, $0029A5, $5267A5, $001F7F, $3F4F7F, $00134C, $262F4C, $000926, $131726, $007FFF,
    $7FBFFF, $0052A5, $527CA5, $003F7F, $3F5F7F, $00264C, $26394C, $001326, $131C26, $00BFFF,
    $7FDFFF, $007CA5, $5291A5, $005F7F, $3F6F7F, $00394C, $26424C, $001C26, $132126, $00FFFF,
    $7FFFFF, $00A5A5, $52A5A5, $007F7F, $3F7F7F, $004C4C, $264C4C, $002626, $132626, $00FFBF,
    $7FFFDF, $00A57C, $52A591, $007F5F, $3F7F6F, $004C39, $264C42, $00261C, $132621, $00FF7F,
    $7FFFBF, $00A552, $52A57C, $007F3F, $3F7F5F, $004C26, $264C39, $002613, $13261C, $00FF3F,
    $7FFF9F, $00A529, $52A567, $007F1F, $3F7F4F, $004C13, $264C2F, $002609, $132617, $00FF00,
    $7FFF7F, $00A500, $52A552, $007F00, $3F7F3F, $004C00, $264C26, $002600, $132613, $3FFF00,
    $9FFF7F, $29A500, $67A552, $1F7F00, $4F7F3F, $134C00, $2F4C26, $092600, $172613, $7FFF00,
    $BFFF7F, $52A500, $7CA552, $3F7F00, $5F7F3F, $264C00, $394C26, $132600, $1C2613, $BFFF00,
    $DFFF7F, $7CA500, $91A552, $5F7F00, $6F7F3F, $394C00, $424C26, $1C2600, $212613, $FFFF00,
    $FFFF7F, $A5A500, $A5A552, $7F7F00, $7F7F3F, $4C4C00, $4C4C26, $262600, $262613, $FFBF00,
    $FFDF7F, $A57C00, $A59152, $7F5F00, $7F6F3F, $4C3900, $4C4226, $261C00, $262113, $FF7F00,
    $FFBF7F, $A55200, $A57C52, $7F3F00, $7F5F3F, $4C2600, $4C3926, $261300, $261C13, $FF3F00,
    $FF9F7F, $A52900, $A56752, $7F1F00, $7F4F3F, $4C1300, $4C2F26, $260900, $261713, $FF0000,
    $FF7F7F, $A50000, $A55252, $7F0000, $7F3F3F, $4C0000, $4C2626, $260000, $261313, $FF003F,
    $FF7F9F, $A50029, $A55267, $7F001F, $7F3F4F, $4C0013, $4C262F, $260009, $261317, $FF007F,
    $FF7FBF, $A50052, $A5527C, $7F003F, $7F3F5F, $4C0026, $4C2639, $260013, $26131C, $FF00BF,
    $FF7FDF, $A5007C, $A55291, $7F005F, $7F3F6F, $4C0039, $4C2642, $26001C, $261321, $FF00FF,
    $FF7FFF, $A500A5, $A552A5, $7F007F, $7F3F7F, $4C004C, $4C264C, $260026, $261326, $BF00FF,
    $DF7FFF, $7C00A5, $9152A5, $5F007F, $6F3F7F, $39004C, $42264C, $1C0026, $211326, $7F00FF,
    $BF7FFF, $5200A5, $7C52A5, $3F007F, $5F3F7F, $26004C, $39264C, $130026, $1C1326, $3F00FF,
    $9F7FFF, $2900A5, $6752A5, $1F007F, $4F3F7F, $13004C, $2F264C, $090026, $171326, $000000,
    $2D2D2D, $5B5B5B, $898989, $B7B7B7, $B3B3B3
]

class T2DCoordinate:
    """ 2D coordinate type """
    def __init__(self, X, Y):
        self._points = [X, Y]

    def getX(self):
        return self._points[0]
    def getY(self):
        return self._points[1]
    def setX(self, value):
        self._points[0] = value
    def setY(self, value):
        self._points[1] = value
        
    X = property(getX, setX)
    Y = property(getY, setY)

class T3DCoordinate:
    """ 3D coordinate type """
    def __init__(self, X, Y, Z):
        self._points = [X, Y, Z]
        
    def getX(self):
        return self._points[0]
    def getY(self):
        return self._points[1]
    def getZ(self):
        return self._points[2]
    def setX(self, value):
        self._points[0] = value
    def setY(self, value):
        self._points[1] = value
    def getZ(self):
        self._points[2] = value
        
    X = property(getX, setX)
    Y = property(getY, setY)
    Z = property(getZ, setZ)
    
class T3DPlane:
    """ Description of a 3D plane: a*x + b*y + c*z - d = 0.0 """
    def __init__(self, a, b, c, d):
        self._points = [a, b, c, d]

class TShadePoint:
    """ Used for drawing to the Z-buffer """
    def __init__(self, X, Y, Z, R, G, B):
        self._points = [X, Y, Z]
        self._color = [R, G, B]
        
class TRGBTriple:
    pass

class TLayerProperties:
    pass
class TFreeLight:
    pass
class TFreeSubdivisionBase:
    pass
class TFreeSubdivisionSurface:
    pass
class TFreesubdivisionPoint:
    pass
class TFreeSubdivisionEdge:
    pass
class TFreeSubdivisionFace:
    pass
class TFreeSubdivisionControlPoint:
    pass
class TFreeSubdivisionControlEdge:
    pass
class TFreeSubdivisionControlFace:
    pass
class TFreeSubdivisionLayer:
    pass

class TFreeFileBuffer (object):
    def __init__(self):
        self.FCapacity = 0
        self.FCount = 0
        self.FPosition = 0
        self.FVersion = 0
        self.FData = []

    def _FGrow(self, sz):
        pass

    def _FSetCapacity(self, capacity):
        pass

    def Add(self, val):
        pass

    def Load(self, val):
        pass

    def Create(self):
        pass

    def Clear(self):
        pass

    def LoadFromFile(self, filename):
        pass

    def Reset(self):
        pass

    def SaveToFile(self, filename):
        pass

    def Destroy(self):
        pass

    Capacity = property(getCapacity, setCapacity)
    Count = property(getCount, setCount)
    Version = property(getVersion, setVersion)

class TFreeAlphaBuffer (object):
    """ Alpha-buffer class used in the shading algorithm """
    def __init__(self):
        self.FViewport = TFreeViewport()
        self.FBuffer = []
        self.FWidth = 0
        self.FHeight = 0
        self.FFirstRow = 0
        self.FLastRow = 0

    def AddPixelData(self, X, Y, R, G, B, Z):
        pass

    def Initialize(self):
        pass

    def Draw(self):
        pass

class TFreeZBuffer (object):
    """ Z-buffer class used in the shading algorithm """
    def __init__(self):
        self.FViewport = TFreeViewport()
        self.FBuffer = []
        self.FWidth = 0
        self.FHeight = 0

    def Initialize(self):
        pass
    
class TFreeViewport:
    def __init__(self):
        self.FAngle = 0.0
        self.FDistance = 0.0
        self.FElevation = 0.0
        self.FFieldOfView = 0.0
        self.FDoubleBuffer = True
        self.FPrinting = True
        self.FPrintResolution = 1
        self.FDestinationWidth = 1
        self.FMin3D = T3DCoordinate()
        self.FMax3D = T3DCoordinate()
        self.FMidPoint = T3DCoordinate()
        self.FMargin = 0.0
        self.FBackgroundMode = 0
        self.FViewType = 0
        self.FCameraLocation = T3DCoordinate()

class TFreeBackgroundImage:
    pass

class TFreeEntity (object):
    """ This is the base class of all 3D entities
    """
    
    def __init__(self):
        # Flag to check if the entity has been already been built
        self.FBuild = false
        # min/max coordinates of the entity after built
        self.FMin = 0
        self.FMax = 0
        # pen thickness when drawing
        self.FPenWidth = 1
        # pen color when drawing
        self.FColor = 0
        # pen style for drawing
        self.FPenStyle = 0

    def _FGetMin(self):
        if not self.FBuild:
            self.Rebuild()
        return self.FMin

    def _FGetMax(self):
        if not self.FBuild:
            self.Rebuild()
        return self.FMax

    def _FSetBuild(self, Val):
        if Val != self.FBuild:
            self.FBuild = Val
            if not Val:
                self.FMin = 0.0
                self.FMax = 0.0

    def Create(self):
        self.Clear()
        self.FBuild = False

    def Clear(self):
        self.FBuild = False
        self.FMin = 0.0
        self.FMax = 0.0
        self.FColor = colorBlack
        self.FPenWidth = -1
        self.FPenStyle = solid
        
    def Extents(self, Min, Max):
        if not self.FBuild:
            self.Rebuild()
        MinMax(self.FMin, Min, Max)
        MinMax(self.FMax, Min, Max)

    def Draw(self, Viewport):
        pass

    def Rebuild(self):
        pass

    def getBuild(self):
        return self.FBuild

    def getColor(self):
        return self.FColor

    def setColor(self, nColor):
        self.FColor = nColor

    def getMin(self):
        return self.FMin

    def setMin(self, Val):
        self.FMin = Val

    def getMax(self):
        return self.FMax
    
    def setMax(self, Val):
        self.FMax = Val

    def getPenStyle(self):
        return self.FPenStyle
    
    def setPenStyle(self, nPenStyle):
        self.FPenStyle = nPenStyle

    def getPenWidth(self):
        return self.FPenWidth
    
    def setPenWidth(self, nPenW):
        self.FPenWidth = nPenW
        
    Build = property(getBuild, _FSetBuild)
    Color = property(getColor, setColor)
    Min = property(getMin, setMin)
    Max = property(getMax, setMax)
    PenStyle = property(getPenStyle, setPenStyle)
    PenWidth = property(getPenWidth, setPenWidth)

class TFreeSpline (TFreeEntity):
    """ 3D CSpline

    Copied from page 107 of the book: 'Numerical recipes in fortan 77'
    Url: http://www.library.cornell.edu/nr/bookpdf/f3-3.pdf
    Modified to use centripetal parametrisation for smoother interpolation and to accept
    knuckles in the controlpoints
    """

    def __init__(self):
        self.FFragments = 0               # number of straight-line segments used when drawing the curve
        self.FShowCurvature = False
        self.FShowPoints = False
        self.FCurvatureScale = 0.0        # scale factor used to increase or decrease the scale of the curvature plot
        self.FTotalLength = 0.0
        self.FPoints = []                 # array containing all controlpoints
        self.FKnuckles = []
        self.FParameters = []
        self.FDerivatives = []

    def _FSetBuild(self, Val):
        if not Val:
            self.FDerivatives = []
            self.FParameters = []
            self.FMin = T3DCoordinate(0,0,0)
            self.FMax = T3DCooordinate(1,1,1)
            self.FTotalLength = 0
        super(self, TFreeSpline)._FSetBuild(Val)
            
    def _FGetFragments(self):
        return self.FFragments

    def _FSetFragments(self, Val):
        if Val != self.FFragments:
            self.FFragments = Val
            self.Build = False

    def _FGetKnuckle(self, Index):
        if Index >= 0 and Index < len(self.FPoints):
            return self.FKnuckles[Index]
        raise Exception()

    def _FSetKnuckle(self, Index, Value):
        if Index >= 0 and Index < len(self.FPoints):
            self.FKnuckles[Index] = Value
            self.Build = False
        else:
            raise Exception()

    def _FGetParameter(self, Index):
        pass

    def _FGetPoint(self, Index):
        pass

    def _FSetBuild(self, val):
        pass

    def _FSetPoint(self, Index, P):
        pass

    def getNoPoints(self):
        return len(self.FPoints)

    def Add(self, P):
        pass

    def Assign(self, Spline):
        pass

    def CoordLength(self, T1, T2):
        pass

    def ChordlengthApproximation(self, Percentage):
        pass

    def Clear(self):
        pass

    def Create(self):
        pass

    CurvatureColor = property(getCurvatureColor, setCurvatureColor)
    CurvatureScale = property(getCurvatureScale, setCurvatureScale)
    Fragments = property(_FGetFragments, _FSetFragments)
    NumberOfPoints = property(getNoPoints)

property    Knuckle[Index:integer]  : Boolean  read FGetKnuckle write FSetKnuckle;
property    Parameter[Index:integer]: TFloatType read FGetParameter;
property    Point [Index:Integer]   : T3DCoordinate read FGetPoint write FSetPoint;
property    ShowCurvature           : Boolean read FShowCurvature write FShowCurvature;
property    ShowPoints              : boolean read FShowPoints write FShowPoints;
property    TotalLength             : TFloatType read FTotalLength;
    
