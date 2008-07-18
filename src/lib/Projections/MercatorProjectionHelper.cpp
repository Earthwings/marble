//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008      Inge Wallin  <ingwa@kde.org>"
//


// Local
#include "MercatorProjectionHelper.h"
#include "AbstractProjectionHelper_p.h"

// Marble
#include "AbstractProjection.h"
#include "GeoPainter.h"
#include "ViewportParams.h"


MercatorProjectionHelper::MercatorProjectionHelper()
    : AbstractProjectionHelper()
{
}

MercatorProjectionHelper::~MercatorProjectionHelper()
{
}


void MercatorProjectionHelper::paintBase( GeoPainter     *painter, 
					   ViewportParams *viewport,
					   QPen           &pen,
					   QBrush         &brush,
					   bool            antialiasing )
{
    // Convenience variables
    //int  radius = viewport->radius();
    int  width  = viewport->width();
    int  height = viewport->height();

    // Igor, prepare the painter!
    painter->setRenderHint( QPainter::Antialiasing, antialiasing );
    painter->setPen( pen );
    painter->setBrush( brush );

    int  yTop;
    int  yBottom;
    int  xDummy;
    AbstractProjection *proj = viewport->currentProjection();

    // Get the top and bottom y coordinates of the projected map.
    proj->screenCoordinates( 0.0, proj->maxLat(), viewport, 
			     xDummy, yTop );
    proj->screenCoordinates( 0.0, -proj->maxLat(), viewport, 
			     xDummy, yBottom );
    if ( yTop < 0 )
	yTop = 0;
    if ( yBottom > height )
	yBottom = height;

    painter->drawRect( 0, yTop, width - 1, yBottom - yTop );
}


void MercatorProjectionHelper::createActiveRegion( ViewportParams *viewport )
{
    // Convenience variables
    //int  radius = viewport->radius();
    int  width  = viewport->width();
    int  height = viewport->height();

    // Calculate translation of center point
    double  centerLon;
    double  centerLat;
    viewport->centerCoordinates( centerLon, centerLat );

    int  yTop;
    int  yBottom;
    int  xDummy;
    AbstractProjection *proj = viewport->currentProjection();

    // Get the top and bottom y coordinates of the projected map.
    proj->screenCoordinates( 0.0, proj->maxLat(), viewport, 
			     xDummy, yTop );
    proj->screenCoordinates( 0.0, -proj->maxLat(), viewport, 
			     xDummy, yBottom );

    // Don't let the active area be outside the image, and also let a
    // thin strip navigationStripe be outside it.
    if ( yTop < navigationStripe() )
	yTop = navigationStripe();
    if ( yBottom > height - navigationStripe() )
	yBottom =  height - navigationStripe();

    setActiveRegion( QRegion(
                      yTop,
                      width - 2 * navigationStripe(),
                      yBottom - yTop,
                      QRegion::Rectangle ) );
}

void MercatorProjectionHelper::createProjectedRegion( ViewportParams *viewport )
{
    // Convenience variables
    //int  radius = viewport->radius();
    int  width  = viewport->width();
    int  height = viewport->height();

    // Calculate translation of center point
    double  centerLon;
    double  centerLat;
    viewport->centerCoordinates( centerLon, centerLat );

    int  yTop;
    int  yBottom;
    int  xDummy;
    AbstractProjection *proj = viewport->currentProjection();

    // Get the top and bottom y coordinates of the projected map.
    proj->screenCoordinates( 0.0, proj->maxLat(), viewport, 
                 xDummy, yTop );
    proj->screenCoordinates( 0.0, -proj->maxLat(), viewport, 
                 xDummy, yBottom );

    if ( yTop < 0 )
        yTop = 0;
    if ( yBottom > height )
        yBottom =  height;

    setProjectedRegion( QRegion(
                0, 
                yTop,
                width,
                yBottom - yTop,
                QRegion::Rectangle ) );
}
