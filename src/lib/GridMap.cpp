//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2007 Torsten Rahn <tackat@kde.org>"
// Copyright 2007      Inge Wallin  <ingwa@kde.org>"
//

#include "GridMap.h"

#include <cmath>
#include <stdlib.h>

#include <QtCore/QVector>
#include <QtCore/QTime>
#include <QtCore/QDebug>
#include <QtGui/QColor>

#include "GeoDataPoint.h"       // In geodata/data/
#include "global.h"
#include "ClipPainter.h"
#include "ViewParams.h"

// Except for the equator the major circles of latitude are defined via 
// the earth's axial tilt, which currently measures about 23°26'21".
 
const double  AXIALTILT = DEG2RAD * ( 23.0
                                           + 26.0 / 60.0
                                           + 21.0 / 3600.0 );
const double  PIHALF    = M_PI / 2;


GridMap::GridMap()
{
    m_imageWidth  = 0;
    m_imageHeight = 0;
    m_imageRadius = 0;

    //	Initialising booleans for horizoncrossing
    m_lastVisible      = false;
    m_currentlyVisible = false;

    m_radius = 0; 

    m_pen = QPen(QColor( 255, 255, 255, 128));
    m_precision = 10;
}

GridMap::~GridMap()
{
}


void GridMap::createTropics( ViewParams* viewParams )
{
    clear();
    m_radius = viewParams->m_radius;
    viewParams->m_planetAxis.inverse().toMatrix( m_planetAxisMatrix );

    // Turn on the major circles of latitude if we've zoomed in far
    // enough (radius > 400 pixels)
    if ( m_radius >  400 ) {
        createCircle( PIHALF - AXIALTILT , Latitude, viewParams ); // Arctic Circle
        createCircle( AXIALTILT - PIHALF , Latitude, viewParams ); // Antarctic Circle
        createCircle( AXIALTILT , Latitude,  viewParams );          // Tropic of Cancer 
        createCircle( -AXIALTILT , Latitude, viewParams );         // Tropic of Capricorn
    }
}

void GridMap::createEquator( ViewParams* viewParams ) 
{
    clear();
    m_radius = viewParams->m_radius;
    viewParams->m_planetAxis.inverse().toMatrix( m_planetAxisMatrix );

    if( viewParams->m_projection == Equirectangular )
        m_planetAxis = viewParams->m_planetAxis;

    createCircle( 0.0 , Latitude, viewParams );
}

void GridMap::createGrid( ViewParams* viewParams )
{
    clear();
    m_radius = viewParams->m_radius;
    viewParams->m_planetAxis.inverse().toMatrix( m_planetAxisMatrix );

    if( viewParams->m_projection == Equirectangular )
        m_planetAxis = viewParams->m_planetAxis;

    //	FIXME:	- Higher precision after optimization 
    //		  ( will keep grid lines from vanishing at high zoom levels ) 

    //	if ( m_radius > 6400 ) { m_precision = 30; createCircles( 64, 48 ); return; } else 
    if ( m_radius > 3200 ) {
        m_precision = 40;
        createCircles( 32, 24, viewParams ); 
        return;
    } 
    else if ( m_radius > 1600 ) {
        m_precision = 30;
        createCircles( 16, 12, viewParams );
        return;
    }	
    else if ( m_radius >  700 ) {
        m_precision = 30;
        createCircles( 8, 6, viewParams );
        return;
    }	
    else if ( m_radius >  400 ) {
        m_precision = 20;
        createCircles( 4, 3, viewParams );
        return;
    }	
    else if ( m_radius >  100 ) {
        m_precision = 10;
        createCircles( 2, 3, viewParams );
        return;
    }	

    createCircles( 2, 1, viewParams );	
}


void GridMap::createCircles( const int lonNum, const int latNum, ViewParams *viewParams )
{
    switch( viewParams->m_projection ) {
        case Spherical:
            sphericalCreateCircles( lonNum, latNum, viewParams );
            break;
        case Equirectangular:
            rectangularCreateCircles( lonNum, latNum, viewParams );
            break;
    }
}

void GridMap::sphericalCreateCircles( const int lonNum, const int latNum, ViewParams *viewParams )
{
    // latNum: number of latitude circles between lat = 0 deg and lat < 90 deg
    // lonNum: number of longitude circles between lon = 0 deg and lon < 90 deg

    if ( latNum != 0 ) {

        // Circles of latitude:
        for ( int i = 1; i < latNum; ++i ) {
            createCircle( + (double)(i) * PIHALF / (double)(latNum), Latitude, viewParams );
            createCircle( - (double)(i) * PIHALF / (double)(latNum), Latitude, viewParams );
        } 
    } 

    if ( lonNum == 0 )
        return;

    // Universal prime meridian and its orthogonal great circle:
    createCircle( + 0,      Longitude, viewParams );
    createCircle( + PIHALF, Longitude, viewParams );	

    for ( int i = 1; i < lonNum; ++i ) {
        double cutOff = PIHALF / (double)(latNum);
        createCircle( i * PIHALF / lonNum,          Longitude, viewParams, cutOff );
        createCircle( i * PIHALF / lonNum + PIHALF, Longitude, viewParams, cutOff );	
    }
}

void GridMap::rectangularCreateCircles( const int lonNum, const int latNum, ViewParams *viewParams )
{

    // latNum: number of latitude circles between lat = 0 deg and lat < 90 deg
    // lonNum: number of longitude circles between lon = 0 deg and lon < 90 deg

    if ( latNum != 0 ) {

        // Circles of latitude:
        for ( int i = 1; i < latNum; ++i ) {
            createCircle( + (double)(i) * PIHALF / (double)(latNum), Latitude, viewParams );
            createCircle( - (double)(i) * PIHALF / (double)(latNum), Latitude, viewParams );
        } 
    } 

    if ( lonNum == 0 )
        return;

    // Universal prime meridian
    createCircle( + 0,      Longitude, viewParams );

    for ( int i = 0; i <= lonNum; ++i ) {
        double cutOff = PIHALF / (double)(latNum);
        createCircle( i * M_PI / lonNum,        Longitude, viewParams, cutOff );
        createCircle( i * M_PI / lonNum + M_PI, Longitude, viewParams, cutOff );	
    }
}

void GridMap::createCircle( double val, SphereDim dim, ViewParams *viewParams, double cutOff)
{
    switch( viewParams->m_projection ) {
        case Spherical:
            sphericalCreateCircle( val, dim, viewParams, cutOff );
            break;
        case Equirectangular:
            rectangularCreateCircle( val, dim, viewParams, cutOff );
            break;
    }
}

void GridMap::sphericalCreateCircle( double val, SphereDim dim, ViewParams *viewParams, double cutOff )
{
    // cutOff: the amount of each quarter circle that is cut off at
    //         the pole in radians

    const double cutCoeff   = 1.0 - cutOff / PIHALF;

    // We draw each circle in quarters ( or parts of those ).
    // This is especially convenient for the great longitude circles which 
    // are being cut off close to the poles.
    // quartSteps: the number of nodes in a "quarter" of a circle.

    const double quartSteps = (double) m_precision;

    double coeff  = 1.0;
    double offset = 0.0;

    const int steps = (int) ( cutCoeff * quartSteps );

    for ( int i = 0; i < 4; ++i ) {

        if ( i > 1 ) 
            coeff = - 1.0;
        offset = ( i % 2 ) ? 1.0 : 0.0;

        m_polygon.clear();
        m_polygon.reserve( steps + 1 );

        for ( int j = 0; j < steps + 1; ++j ) {

            double itval  = (j != steps) ? (double)(j) / quartSteps : cutCoeff;
            double dimVal = coeff * ( PIHALF * fabs( offset - itval ) + offset * PIHALF );

            double lat = ( dim == Latitude )  ? val : dimVal;
            double lon = ( dim == Longitude ) ? val : dimVal;

            GeoDataPoint    geoit( lon, -lat );
            Quaternion  qpos = geoit.quaternion();
            qpos.rotateAroundAxis(m_planetAxisMatrix);

            m_currentPoint = QPointF( (double)(m_imageWidth / 2 + m_radius * qpos.v[Q_X]),
                                      (double)(m_imageHeight / 2 - m_radius * qpos.v[Q_Y]) );
            //qDebug() << "Radius: " << m_radius
            //         << "QPointF(" << (double)(m_imageWidth / 2 + m_radius*qpos.v[Q_X])+1
            //        << ", " << (double)(m_imageHeight / 2 + m_radius*qpos.v[Q_Y])+1 << ")";

            // Take care of horizon crossings if horizon is visible.
            m_lastVisible = m_currentlyVisible;
            m_currentlyVisible = (qpos.v[Q_Z] >= 0) ? true : false;

            // Initialize crossing of the horizon.
            if ( j == 0 ) {

                m_lastVisible = m_currentlyVisible;

                // Initially m_lastPoint MUST NOT equal m_currentPoint
                m_lastPoint = QPointF( m_currentPoint.x(), 
                                       m_currentPoint.y() );
            }

            if (m_currentlyVisible != m_lastVisible) {
                m_polygon << horizonPoint();

                if (m_polygon.size() >= 2) {
                    append(m_polygon);
                }

                m_polygon.clear();

                if ( m_lastVisible == true )
                    break;
            }

            // Take care of screencrossing crossings if horizon is visible.
            // Filter points which aren't on the visible hemisphere.
            if ( m_currentlyVisible ) {
                // most recent addition: m_currentPoint != m_lastPoint
                //			qDebug("accepted");
                m_polygon << m_currentPoint;
            }

            m_lastPoint = m_currentPoint;
        }

        if (m_polygon.size() >= 2) {
            append(m_polygon);
        }
    }
}

void GridMap::rectangularCreateCircle( double val, SphereDim dim, ViewParams *viewParams, double cutOff )
{
    // Only used in spherical projection.
    Q_UNUSED( cutOff );

    // Calculate translation of center point
    double centerLon, centerLat;
    viewParams->centerCoordinates( centerLon, centerLat );

    double       rad2Pixel  = (float)( 2 * m_radius ) / M_PI;
    m_polygon.clear();

    if ( dim == Latitude ) {
        QPointF beginPoint( 0.0f, m_imageHeight / 2 + ( centerLat - val ) * rad2Pixel );
        QPointF endPoint( m_imageWidth, m_imageHeight / 2 + ( centerLat - val ) * rad2Pixel );
        m_polygon << beginPoint << endPoint;
        append( m_polygon );
    }
    else {
        float beginY = m_imageHeight / 2 - m_radius + centerLat * rad2Pixel;
        float endY   = beginY + 2 * m_radius;
        if ( beginY < 0 ) 
            beginY = 0;
        if ( endY > m_imageHeight )
            endY = m_imageHeight ;

        float x = m_imageWidth / 2 + ( val - centerLon ) * rad2Pixel;
        while ( x > 4 * m_radius ) 
            x -= 4 * m_radius;
        while ( x < m_imageWidth ) {
            QPointF beginPoint( x , beginY );
            QPointF endPoint( x , endY );
            m_polygon<<beginPoint<<endPoint;
            x+=4*m_radius;
            append(m_polygon);
            m_polygon.clear();
        }
    }
}

void GridMap::paintGridMap(ClipPainter * painter, bool antialiasing)
{
    if ( size() == 0 )
        return;

    if ( antialiasing == true )
        painter->setRenderHint(QPainter::Antialiasing, true);

    painter->setPen(m_pen);

    ScreenPolygon::Vector::const_iterator  itEndPolygon = end();

    for ( ScreenPolygon::Vector::const_iterator itPolygon=begin(); 
          itPolygon != itEndPolygon;
          ++itPolygon )
    {
        painter->drawPolyline(*itPolygon);
    }

    if (antialiasing)
        painter->setRenderHint( QPainter::Antialiasing, false );
}


const QPointF GridMap::horizonPoint()
{
    // qDebug("Interpolating");
    double  xa = 0;
    double  ya = 0;

    xa = m_currentPoint.x() - ( m_imageWidth / 2 ) ;

    // Move the m_currentPoint along the y-axis to match the horizon.
    double  radicant = (double)(m_radius) * (double)( m_radius) - xa*xa;
    if ( radicant > 0 )
        ya = sqrt( radicant );

    if ( ( m_currentPoint.y() - ( m_imageHeight / 2 ) ) < 0 )
        ya = -ya; 

    return QPointF( (double)m_imageWidth / 2  + xa,
                    (double)m_imageHeight / 2 + ya );
}



void GridMap::resizeMap( int width, int height )
{
    m_imageWidth  = width;
    m_imageHeight = height;
    m_imageRadius     = ( m_imageWidth * m_imageWidth / 4
                          + m_imageHeight * m_imageHeight / 4 );
}
