/***************************************************************************
                          geolocation.h  -  K Desktop Planetarium
                             -------------------
    begin                : Sun Feb 11 2001
    copyright            : (C) 2001 by Jason Harris
    email                : jharris@30doradus.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/




#ifndef GEOLOCATION_H
#define GEOLOCATION_H

#include <qstring.h>

#include "dms.h"

/**
	*Contains all relevant information for specifying an observing
	*location on Earth: City Name, State/Country Name, Longitude,
	*Latitude, and Time Zone.
	*@short Relevant data about an observing location on Earth.
  *@author Jason Harris
	*@version 0.4
  */

class GeoLocation {
public: 
/**
	*Default constructor; sets coordinates to zero.	
	*/
	GeoLocation();
/**
	*Copy Constructor
	*/	
	GeoLocation( const GeoLocation &g );
	GeoLocation( GeoLocation *g );
/**
	*Constructor using dms objects to specify longitude and latitude.
	*/
	
	GeoLocation( dms lng, dms lat, QString name="Nowhere", QString state="Nowhere", int TZ=0 );
/**
	*Constructor using doubles to specify longitude and latitude.
	*/
	GeoLocation( double lng, double lat, QString name="Nowhere", QString state="Nowhere", int TZ=0 );
/**
	*Destructor (empty)
	*/
	~GeoLocation() {};

/**
	*Returns longitude
	*/	
	dms lng() { return Longitude; }
/**
	*Returns latitude
	*/
	dms lat()  { return Latitude; }
/**
	*Return City name as a C char array
	*/
//	const char *name() { return Name.latin1(); }
	const QString name() { return Name; }
/**
	*Return State/Country name as a C char array
	*/
//	const char *state() { return State.latin1(); }
//	QString state() { return QString( State.latin1() ); }
	const QString state() { return State; }
/**
	*Return time zone
	*/
	int   TZ()    { return TimeZone; }

/**
	*Set longitude according to argument.
	*/
	void setLong( dms l ) { Longitude = l; }
/**
	*Set longitude according to argument.
	*Differs from above function only in argument type.
	*/
	void setLong( double l ) { Longitude.setD( l ); }
/**
	*Set latitude according to argument.
	*/
	void setLat( dms l ) { Latitude  = l; }
/**
	*Set latitude according to argument.
	*Differs from above function only in argument type.
	*/
	void setLat( double l ) { Latitude.setD( l ); }
/**
	*Set City name according to argument.
	*/
	void setName( QString n ) { Name = n; }
/**
	*Set State/Country name according to argument.
	*/
	void setState( QString s ) { State = s; }
/**
	*Sets Time Zone according to argument.
	*/
	void setTZ( int tz ) { TimeZone = tz; }
/**
	*Set location data to that of the GeoLocation pointed to by argument.
	*Similar to copy constructor.
	*/
	void reset( GeoLocation *g );		

private:
	dms Longitude, Latitude;
	QString Name;
	QString State;
	int TimeZone;
};

#endif
