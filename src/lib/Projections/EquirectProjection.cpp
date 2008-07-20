//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007-2008 Inge Wallin  <ingwa@kde.org>"
//


// Local
#include "EquirectProjection.h"

// Marble
#include "EquirectProjectionHelper.h"
#include "ViewportParams.h"


static EquirectProjectionHelper  theHelper;


EquirectProjection::EquirectProjection()
    : AbstractProjection()
{
    m_maxLat  = 90.0 * DEG2RAD;
    m_traversableMaxLat = false;
    m_repeatX = true;
}

EquirectProjection::~EquirectProjection()
{
}

AbstractProjectionHelper *EquirectProjection::helper()
{
    return &theHelper;
}

bool EquirectProjection::screenCoordinates( const double lon, const double lat,
                                            const ViewportParams *viewport,
                                            int& x, int& y,
					    CoordinateType coordType )
{
    Q_UNUSED( coordType );

    // Calculate translation of center point
    double  centerLon;
    double  centerLat;
    viewport->centerCoordinates( centerLon, centerLat );

    double  rad2Pixel = 2.0 * viewport->radius() / M_PI;
 
    // Let (x, y) be the position on the screen of the point.
    x = (int)( viewport->width()  / 2.0 + ( lon - centerLon ) * rad2Pixel );
    y = (int)( viewport->height() / 2.0 - ( lat - centerLat ) * rad2Pixel );

    // Return true if the calculated point is inside the screen area,
    // otherwise return false.
    return ( ( y >= 0 && y < viewport->height() )
	     && ( ( x >= 0 && x < viewport->width() )
		  || ( x - 4 * viewport->radius() >= 0
		       && x - 4 * viewport->radius() < viewport->width() )
		  || ( x + 4 * viewport->radius() >= 0
		       && x + 4 * viewport->radius() < viewport->width() ) ) );
}

bool EquirectProjection::screenCoordinates( const GeoDataPoint &geopoint, 
                                            const ViewportParams *viewport,
                                            int &x, int &y, bool &globeHidesPoint )
{
    globeHidesPoint = false;

    double  lon;
    double  lat;
    double  rad2Pixel = 2.0 * viewport->radius() / M_PI;

    double  centerLon;
    double  centerLat;
    viewport->centerCoordinates( centerLon, centerLat );

    geopoint.geoCoordinates( lon, lat );

    // Let (x, y) be the position on the screen of the geopoint.
    x = (int)(viewport->width()  / 2.0 + rad2Pixel * (lon - centerLon));
    y = (int)(viewport->height() / 2.0 - rad2Pixel * (lat - centerLat));

    // Return true if the calculated point is inside the screen area,
    // otherwise return false.
    return ( ( y >= 0 && y < viewport->height() )
	     && ( ( x >= 0 && x < viewport->width() )
		  || ( x - 4 * viewport->radius() >= 0
		       && x - 4 * viewport->radius() < viewport->width() )
		  || ( x + 4 * viewport->radius() >= 0
		       && x + 4 * viewport->radius() < viewport->width() ) ) );
}

bool EquirectProjection::screenCoordinates( const GeoDataPoint &geopoint,
					    const ViewportParams *viewport,
					    int *x, int &y,
					    int &pointRepeatNum,
					    bool &globeHidesPoint )
{
    // On flat projections the observer's view onto the point won't be 
    // obscured by the target planet itself.
    globeHidesPoint = false;

    double  lon;
    double  lat;
    double  rad2Pixel = 2.0 * viewport->radius() / M_PI;

    double  centerLon;
    double  centerLat;
    viewport->centerCoordinates( centerLon, centerLat );

    geopoint.geoCoordinates( lon, lat );

    // Let (itX, y) be the first guess for one possible position on screen.
    int itX = (int)( viewport->width()  / 2.0 + rad2Pixel * ( lon - centerLon ) );
    y = (int)( viewport->height() / 2.0 - rad2Pixel * ( lat - centerLat ) );

    // Make sure that the requested point is within the visible y range:
    if ( y >= 0 && y < viewport->height() ) {
        // First we deal with the case where the repetition doesn't happen
        if ( m_repeatX == false ) {
            *x = itX;
            if ( itX > 0 && itX < viewport->width() ) {
                return true;
            }
            else {
                // the requested point is out of the visible x range:
                return false;
            }
        }
        // For the repetition case the same geopoint gets displayed on 
        // the map many times.across the longitude.

        int xRepeatDistance = 4 * viewport->radius();

        // Finding the leftmost positive x value
        if ( itX > xRepeatDistance ) {
            itX %= xRepeatDistance;
        }
        if ( itX < 0 ) {
            itX += xRepeatDistance;
        }
        // The requested point is out of the visible x range:
        if ( itX > viewport->width() ) {
            return false;
        }

        // Now iterate through all visible x screen coordinates for the point 
        // from left to right.
        int itNum = 0;
        while ( itX < viewport->width() ) {
            *x = itX;
            ++x;
            ++itNum;
            itX += xRepeatDistance;
        }

        pointRepeatNum = itNum;

        return true;
    }

    // The requested point is out of the visible y range.
    return false;
}

bool EquirectProjection::geoCoordinates( const int x, const int y,
                                         const ViewportParams *viewport,
                                         double& lon, double& lat,
                                         GeoDataPoint::Unit unit )
{
    int   radius          = viewport->radius();
    int   halfImageWidth  = viewport->width() / 2;
    int   halfImageHeight = viewport->height() / 2;

    // Get the Lat and Lon of the center point of the screen.
    double  centerLon;
    double  centerLat;
    viewport->centerCoordinates( centerLon, centerLat );

    // Get yTop and yBottom, the limits of the map on the screen.
    int yCenterOffset = (int)( centerLat * (double)(2 * radius) / M_PI);
    int yTop          = halfImageHeight - radius + yCenterOffset;
    int yBottom       = yTop + 2 * radius;

    // Return here if the y coordinate is outside the map
    if ( y < yTop || y >= yBottom )
        return false;

    int const xPixels = x - halfImageWidth;
    int const yPixels = y - halfImageHeight;

    double const pixel2Rad = M_PI / (2.0 * radius);
    lat = - yPixels * pixel2Rad + centerLat;
    lon = + xPixels * pixel2Rad + centerLon;

    while ( lon > M_PI )  lon -= 2.0 * M_PI;
    while ( lon < -M_PI ) lon += 2.0 * M_PI;

    if ( unit == GeoDataPoint::Degree ) {
        lon *= RAD2DEG;
        lat *= RAD2DEG;
    }

    return true;
}


bool EquirectProjection::geoCoordinates( int x, int y, 
                                         const ViewportParams *viewport,
                                         Quaternion &q )
{
    // NYI
    return false;
}


GeoDataLatLonAltBox EquirectProjection::latLonAltBox( const QRect& screenRect,
						      const ViewportParams *viewport )
{
    // For the case where the whole viewport gets covered there is a 
    // pretty dirty and generic detection algorithm:
    GeoDataLatLonAltBox latLonAltBox = AbstractProjection::latLonAltBox( screenRect, viewport );

    // The remaining algorithm should be pretty generic for all kinds of 
    // flat projections:

    // If the whole globe is visible we can easily calculate
    // analytically the lon-/lat- range
    // FIXME: Unused variable.  Remove?
    //double pitch = GeoDataPoint::normalizeLat( viewport->planetAxis().pitch() );

    if ( m_repeatX ) {
        int xRepeatDistance = 4 * viewport->radius();
        if ( viewport->width() >= xRepeatDistance ) {
            latLonAltBox.setWest( -M_PI );
            latLonAltBox.setEast( +M_PI );
        }
    }
    else {
        // We need a point on the screen at maxLat that definetely gets displayed:
        double averageLatitude = ( latLonAltBox.north() + latLonAltBox.south() ) / 2.0;
    
        GeoDataPoint maxLonPoint( +M_PI, averageLatitude, GeoDataPoint::Radian );
        GeoDataPoint minLonPoint( -M_PI, averageLatitude, GeoDataPoint::Radian );
    
        int dummyX, dummyY; // not needed
        bool dummyVal;
    
        if ( screenCoordinates( maxLonPoint, viewport, dummyX, dummyY, dummyVal ) ) {
            latLonAltBox.setEast( +M_PI );
        }
        if ( screenCoordinates( minLonPoint, viewport, dummyX, dummyY, dummyVal ) ) {
            latLonAltBox.setWest( -M_PI );
        }
    }

    // Now we need to check whether maxLat (e.g. the north pole) gets displayed
    // inside the viewport.

    // We need a point on the screen at maxLat that definetely gets displayed:
    double averageLongitude = latLonAltBox.west();

    GeoDataPoint maxLatPoint( averageLongitude, +m_maxLat, 0.0, GeoDataPoint::Radian );
    GeoDataPoint minLatPoint( averageLongitude, -m_maxLat, 0.0, GeoDataPoint::Radian );

    int dummyX, dummyY; // not needed
    bool dummyVal;

    if ( screenCoordinates( maxLatPoint, viewport, dummyX, dummyY, dummyVal ) ) {
        latLonAltBox.setEast( +M_PI );
        latLonAltBox.setWest( -M_PI );
    }
    if ( screenCoordinates( minLatPoint, viewport, dummyX, dummyY, dummyVal ) ) {
        latLonAltBox.setEast( +M_PI );
        latLonAltBox.setWest( -M_PI );
    }

//    qDebug() << latLonAltBox.text( GeoDataPoint::Degree );

    return latLonAltBox;
}


bool EquirectProjection::mapCoversViewport( const ViewportParams *viewport ) const
{
    int   radius          = viewport->radius();
    int   halfImageHeight = viewport->height() / 2;

    // Get the Lat and Lon of the center point of the screen.
    double  centerLon;
    double  centerLat;
    viewport->centerCoordinates( centerLon, centerLat );

    // Calculate how many pixel are being represented per radians.
    const float rad2Pixel = (double)( 2 * radius )/M_PI;

    // Get yTop and yBottom, the limits of the map on the screen.
    int yCenterOffset = (int)( centerLat * rad2Pixel );
    int yTop          = halfImageHeight - radius + yCenterOffset;
    int yBottom       = yTop + 2 * radius;

    if ( yTop >= 0 ||  yBottom < viewport->height() )
        return false;

    return true;
}

