//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007-2008 Inge Wallin  <ingwa@kde.org>
//


// Local
#include "EquirectProjection.h"

// Marble
#include "EquirectProjectionHelper.h"
#include "ViewportParams.h"

#include <QtCore/QDebug>

using namespace Marble;

static EquirectProjectionHelper  theHelper;


EquirectProjection::EquirectProjection()
    : AbstractProjection()
{
    m_maxLat  = 90.0 * DEG2RAD;
    m_minLat  = -90.0 * DEG2RAD;
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

bool EquirectProjection::screenCoordinates( const qreal lon, const qreal lat,
                                            const ViewportParams *viewport,
                                            int& x, int& y )
{
    // Convenience variables
    int  radius = viewport->radius();
    int  width  = viewport->width();
    int  height = viewport->height();

    // Calculate translation of center point
    qreal  centerLon;
    qreal  centerLat;
    viewport->centerCoordinates( centerLon, centerLat );

    qreal  rad2Pixel = 2.0 * viewport->radius() / M_PI;
 
    // Let (x, y) be the position on the screen of the point.
    x = (int)( width  / 2.0 + ( lon - centerLon ) * rad2Pixel );
    y = (int)( height / 2.0 - ( lat - centerLat ) * rad2Pixel );

    // Return true if the calculated point is inside the screen area,
    // otherwise return false.
    return ( ( 0 <= y && y < height )
             && ( ( 0 <= x && x < width )
                  || ( 0 <= x - 4 * radius && x - 4 * radius < width )
                  || ( 0 <= x + 4 * radius && x + 4 * radius < width ) ) );
}

bool EquirectProjection::screenCoordinates( const GeoDataCoordinates &geopoint, 
                                            const ViewportParams *viewport,
                                            int &x, int &y, bool &globeHidesPoint )
{
    globeHidesPoint = false;

    // Convenience variables
    int  radius = viewport->radius();
    int  width  = viewport->width();
    int  height = viewport->height();

    qreal  lon;
    qreal  lat;
    qreal  rad2Pixel = 2.0 * viewport->radius() / M_PI;

    qreal  centerLon;
    qreal  centerLat;
    viewport->centerCoordinates( centerLon, centerLat );

    geopoint.geoCoordinates( lon, lat );

    // Let (x, y) be the position on the screen of the geopoint.
    x = (int)(viewport->width()  / 2.0 + rad2Pixel * (lon - centerLon));
    y = (int)(viewport->height() / 2.0 - rad2Pixel * (lat - centerLat));

    // Return true if the calculated point is inside the screen area,
    // otherwise return false.
    return ( ( 0 <= y && y < height )
             && ( ( 0 <= x && x < width )
                  || ( 0 <= x - 4 * radius && x - 4 * radius < width )
                  || ( 0 <= x + 4 * radius && x + 4 * radius < width ) ) );
}

bool EquirectProjection::screenCoordinates( const GeoDataCoordinates &geopoint,
                                            const ViewportParams *viewport,
                                            int *x, int &y,
                                            int &pointRepeatNum,
                                            bool &globeHidesPoint )
{
    // On flat projections the observer's view onto the point won't be 
    // obscured by the target planet itself.
    globeHidesPoint = false;

    // Convenience variables
    int  radius = viewport->radius();
    int  width  = viewport->width();
    int  height = viewport->height();

    qreal  lon;
    qreal  lat;
    qreal  rad2Pixel = 2.0 * radius / M_PI;

    qreal  centerLon;
    qreal  centerLat;
    viewport->centerCoordinates( centerLon, centerLat );

    geopoint.geoCoordinates( lon, lat );

    // Let (itX, y) be the first guess for one possible position on screen.
    int itX = (int)( width  / 2.0 + rad2Pixel * ( lon - centerLon ) );
    y = (int)( height / 2.0 - rad2Pixel * ( lat - centerLat ) );

    // Make sure that the requested point is within the visible y range:
    if ( 0 <= y && y < height ) {
        // First we deal with the case where the repetition doesn't happen
        if ( m_repeatX == false ) {
            *x = itX;
            if ( 0 < itX && itX < width ) {
                return true;
            }
            else {
                // the requested point is out of the visible x range:
                return false;
            }
        }

        // For the repetition case the same geopoint gets displayed on 
        // the map many times.across the longitude.

        int xRepeatDistance = 4 * radius;

        // Finding the leftmost positive x value
        if ( itX > xRepeatDistance ) {
            itX %= xRepeatDistance;
        }
        if ( itX < 0 ) {
            itX += xRepeatDistance;
        }
        // The requested point is out of the visible x range:
        if ( itX > width ) {
            return false;
        }

        // Now iterate through all visible x screen coordinates for the point 
        // from left to right.
        int itNum = 0;
        while ( itX < width ) {
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


bool EquirectProjection::screenCoordinates( const GeoDataLineString &lineString, 
                                    const ViewportParams *viewport,
                                    QVector<QPolygonF *> &polygons )
{
    // Compare bounding box size of the line string with the angularResolution
    // Immediately return if the latLonAltBox is smaller.

    if ( !viewport->resolves( lineString.latLonAltBox() ) ) {
//      qDebug() << "Object too small to be resolved";
        return false;
    }

    int x = 0;
    int y = 0;
    bool globeHidesPoint = false;
    bool isVisible = false;

    int previousX = -1; 
    int previousY = -1;
    bool previousGlobeHidesPoint = false;
    bool previousIsVisible = false;

    QPolygonF  *polygon = new QPolygonF;
    
    GeoDataLineString::ConstIterator itCoords = lineString.constBegin();
    GeoDataLineString::ConstIterator itPreviousCoords = lineString.constBegin();

    GeoDataCoordinates previousCoords;
    GeoDataCoordinates currentCoords;

    GeoDataLineString::ConstIterator itEnd = lineString.constEnd();

    bool processingLastNode = false;

    // We use a while loop to be able to cover linestrings as well as linear rings:
    // Linear rings require to tessellate the path from the last node to the first node
    // which isn't really convenient to achieve with a for loop ...

    const qreal precision = 20.0;
    const bool isLong = lineString.size() > 50;

    while ( itCoords != itEnd )
    {
        // Optimization for line strings with a big amount of nodes
        bool skipNode = isLong && viewport->resolves( **itPreviousCoords, **itCoords); 

        if ( !skipNode ) {

            previousCoords = **itPreviousCoords;
            currentCoords  = **itCoords;

            isVisible = screenCoordinates( currentCoords, viewport, x, y, globeHidesPoint );

            // Initializing variables that store the values of the previous iteration
            if ( !processingLastNode && itCoords == lineString.constBegin() ) {
                previousGlobeHidesPoint = globeHidesPoint;
                previousIsVisible = isVisible;
                itPreviousCoords = itCoords;
                previousX = x;
                previousY = y;
            }

            // TODO: on flat maps we need to take the date line into account right here.

            // This if-clause contains the section that tessellates the line 
            // segments of a linestring. If you are about to learn how the code of 
            // this class works you can safely ignore this section for a start.

            if ( lineString.tessellate() && ( isVisible || previousIsVisible ) ) {
                // let the line segment follow the spherical surface
                // if the distance between the previous point and the current point 
                // on screen is too big

                // We take the manhattan length as a distance approximation
                // that can be too big by a factor of sqrt(2)
                qreal distance = fabs((qreal)(x - previousX)) + fabs((qreal)(y - previousY));

                // FIXME: This is a work around: remove as soon as we handle horizon crossing
                if ( globeHidesPoint || previousGlobeHidesPoint ) {
                    distance = 350;
                }

                const qreal safeDistance = - 0.5 * distance;

                // Interpolate additional nodes if the current or previous nodes are visible
                // or if the line segment that connects them might cross the viewport.
                // The latter can pretty safely be excluded for most projections if both points 
                // are located on the same side relative to the viewport boundaries and if they are 
                // located more than half the line segment distance away from the viewport.

                if (   !( x < safeDistance && previousX < safeDistance )
                    || !( y < safeDistance && previousY < safeDistance )
                    || !( x + safeDistance > viewport->width() 
                        && previousX + safeDistance > viewport->width() )
                    || !( y + safeDistance > viewport->height()
                        && previousY + safeDistance > viewport->height() )
                ){
                    int suggestedCount = (int)( distance / precision );

                    if ( distance > precision ) {
    //                    qDebug() << "Distance: " << distance;
                        *polygon << tessellateLineSegment( previousCoords, currentCoords, 
                                                        suggestedCount, viewport,
                                                        lineString.tessellationFlags() );
                    }
                }
            }

            if ( !globeHidesPoint ) {
                polygon->append( QPointF( x, y ) );
            }
            else {
                if (   !previousGlobeHidesPoint 
                    && !lineString.isClosed() // FIXME: this probably needs to take rotation 
                                                //        into account for some cases
                    ) {
                    polygons.append( polygon );
                    polygon = new QPolygonF;
                }
            }

            previousGlobeHidesPoint = globeHidesPoint;
            previousIsVisible = isVisible;
            itPreviousCoords = itCoords;
            previousX = x;
            previousY = y;
        }

        // Here we modify the condition to be able to process the 
        // first node after the last node in a LinearRing.

        if ( processingLastNode ) {
            break;
        }
        ++itCoords;

        if ( itCoords == itEnd  && lineString.isClosed() ) {
            itCoords = lineString.constBegin();
            processingLastNode = true;
        }        
    }

    if ( polygon->size() > 1 ){
        polygons.append( polygon );
    }
    else {
        delete polygon;
    }

    return polygons.isEmpty();
}


bool EquirectProjection::geoCoordinates( int x, int y,
                                         const ViewportParams *viewport,
                                         qreal& lon, qreal& lat,
                                         GeoDataCoordinates::Unit unit )
{
    int   radius          = viewport->radius();
    int   halfImageWidth  = viewport->width() / 2;
    int   halfImageHeight = viewport->height() / 2;

    // Get the Lat and Lon of the center point of the screen.
    qreal  centerLon;
    qreal  centerLat;
    viewport->centerCoordinates( centerLon, centerLat );

    // Get yTop and yBottom, the limits of the map on the screen.
    int yCenterOffset = (int)( centerLat * (qreal)(2 * radius) / M_PI);
    int yTop          = halfImageHeight - radius + yCenterOffset;
    int yBottom       = yTop + 2 * radius;

    // Return here if the y coordinate is outside the map
    if ( y < yTop || y >= yBottom )
        return false;

    int const xPixels = x - halfImageWidth;
    int const yPixels = y - halfImageHeight;

    qreal const pixel2Rad = M_PI / (2.0 * radius);
    lat = - yPixels * pixel2Rad + centerLat;
    lon = + xPixels * pixel2Rad + centerLon;

    while ( lon > M_PI )  lon -= 2.0 * M_PI;
    while ( lon < -M_PI ) lon += 2.0 * M_PI;

    if ( unit == GeoDataCoordinates::Degree ) {
        lon *= RAD2DEG;
        lat *= RAD2DEG;
    }

    return true;
}

GeoDataLatLonAltBox EquirectProjection::latLonAltBox( const QRect& screenRect,
                                                      const ViewportParams *viewport )
{
    // Convenience variables
    int  radius = viewport->radius();
    int  width  = viewport->width();
    int  height = viewport->height();

    // For the case where the whole viewport gets covered there is a 
    // pretty dirty and generic detection algorithm:
    GeoDataLatLonAltBox latLonAltBox = AbstractProjection::latLonAltBox( screenRect, viewport );

    // The remaining algorithm should be pretty generic for all kinds of 
    // flat projections:

    // If the whole globe is visible we can easily calculate
    // analytically the lon-/lat- range
    // FIXME: Unused variable.  Remove?
    //qreal pitch = GeoDataCoordinates::normalizeLat( viewport->planetAxis().pitch() );

    if ( m_repeatX ) {
        int xRepeatDistance = 4 * radius;
        if ( width >= xRepeatDistance ) {
            latLonAltBox.setWest( -M_PI );
            latLonAltBox.setEast( +M_PI );
        }
    }
    else {
        // We need a point on the screen at maxLat that definetely gets displayed:
        qreal averageLatitude = ( latLonAltBox.north() + latLonAltBox.south() ) / 2.0;
    
        GeoDataCoordinates maxLonPoint( +M_PI, averageLatitude, GeoDataCoordinates::Radian );
        GeoDataCoordinates minLonPoint( -M_PI, averageLatitude, GeoDataCoordinates::Radian );
    
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
    qreal averageLongitude = latLonAltBox.east();

    GeoDataCoordinates maxLatPoint( averageLongitude, m_maxLat, 0.0, GeoDataCoordinates::Radian );
    GeoDataCoordinates minLatPoint( averageLongitude, m_minLat, 0.0, GeoDataCoordinates::Radian );

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

//    qDebug() << latLonAltBox.text( GeoDataCoordinates::Degree );

    return latLonAltBox;
}


bool EquirectProjection::mapCoversViewport( const ViewportParams *viewport ) const
{
    // Convenience variables
    int  radius          = viewport->radius();
    //int  width         = viewport->width();
    int  height          = viewport->height();
    int  halfImageHeight = viewport->height() / 2;

    // Get the Lat and Lon of the center point of the screen.
    qreal  centerLon;
    qreal  centerLat;
    viewport->centerCoordinates( centerLon, centerLat );

    // Calculate how many pixel are being represented per radians.
    const float rad2Pixel = (qreal)( 2 * radius )/M_PI;

    // Get yTop and yBottom, the limits of the map on the screen.
    int yCenterOffset = (int)( centerLat * rad2Pixel );
    int yTop          = halfImageHeight - radius + yCenterOffset;
    int yBottom       = yTop + 2 * radius;

    if ( yTop >= 0 || yBottom < height )
        return false;

    return true;
}

