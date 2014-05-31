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
##############################################################################################}

# Unit to keep track of fileversions, bux fixes and release dates

fv100 = 1
fv110 = 2
fv120 = 3
fv130 = 4
fv140 = 5
fv150 = 6
fv160 = 7
fv165 = 8
fv170 = 9
fv180 = 10
fv190 = 11
fv191 = 12
fv195 = 13
fv198 = 14
fv200 = 15
fv201 = 16
fv210 = 17
fv220 = 18
fv230 = 19
fv240 = 20
fv250 = 21
fv260 = 22

CurrentVersion = fv260    # Current (latest) version of the FREE!ship project.
                          # All new created models are initialized to this version
ReleasedDate   = "April 21, 2006"

def VersionString(Version):
    if Version == fv100:
        return "1.0"
    elif Version == fv110:
        return "1.1"
    elif Version == fv120:
        return "1.2"
    elif Version == fv130:
        return "1.3"
    elif Version == fv140:
        return "1.4"
    elif Version == fv150:
        return "1.5"
    elif Version == fv160:
        return "1.6"
    elif Version == fv165:
        return "1.65"
    elif Version == fv170:
        return "1.7"
    elif Version == fv180:
        return "1.8"
    elif Version == fv190:
        return "1.9"
    elif Version == fv191:
        return "1.91"
    elif Version == fv195:
        return "1.95"
    elif Version == fv198:
        return "1.98"
    elif Version == fv200:
        return "2.0"
    elif Version == fv201:
        return "2.01"
    elif Version == fv210:
        return "2.1"
    elif Version == fv220:
        return "2.2"
    elif Version == fv230:
        return "2.3"
    elif Version == fv240:
        return "2.4"
    elif Version == fv250:
        return "2.5"
    elif Version == fv260:
        return "2.6"
    else:
         MessageDlg(Userstring(204)+'!',mtError,[mbok],0);
