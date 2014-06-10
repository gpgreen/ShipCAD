#include <stdexcept>

#include "version.h"

using namespace ShipCADGeometry;

//////////////////////////////////////////////////////////////////////////////////////

QString ShipCADGeometry::versionString(version_t v)
{
    if (v == fv100)
        return QString("1.0");
    else if (v == fv110)
        return QString("1.1");
    else if (v == fv120)
        return QString("1.2");
    else if (v == fv130)
        return QString("1.3");
    else if (v == fv140)
        return QString("1.4");
    else if (v == fv150)
        return QString("1.5");
    else if (v == fv160)
        return QString("1.6");
    else if (v == fv165)
        return QString("1.65");
    else if (v == fv170)
        return QString("1.7");
    else if (v == fv180)
        return QString("1.8");
    else if (v == fv190)
        return QString("1.9");
    else if (v == fv191)
        return QString("1.91");
    else if (v == fv195)
        return QString("1.95");
    else if (v == fv198)
        return QString("1.98");
    else if (v == fv200)
        return QString("2.0");
    else if (v == fv201)
        return QString("2.01");
    else if (v == fv210)
        return QString("2.1");
    else if (v == fv220)
        return QString("2.2");
    else if (v == fv230)
        return QString("2.3");
    else if (v == fv240)
        return QString("2.4");
    else if (v == fv250)
        return QString("2.5");
    else if (v == fv260)
        return QString("2.6");
    else
      //MessageDlg(Userstring(204)+'!',mtError,[mbok],0);
      throw std::runtime_error("Bad version enum");
}
