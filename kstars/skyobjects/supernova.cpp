#include "supernova.h"
#include "kspopupmenu.h"

Supernova::Supernova(dms ra, dms dec, const QString& date ,float m, const QString& serialNo,
                     const QString& type, const QString& hostGalaxy, const QString& offset,
                     const QString& discoverer)
                    : SkyObject(SkyObject::SUPERNOVA,ra, dec, m, serialNo),
                      serialNumber(serialNo),
                      RA(ra),
                      Dec(dec),
                      Magnitude(m),
                      type(type),
                      hostGalaxy(hostGalaxy),
                      offset(offset),
                      discoverers(discoverer), 
                      date(date)
{}


void Supernova::initPopupMenu(KSPopupMenu* pmenu)
{
    pmenu->createSupernovaMenu(this);
}
