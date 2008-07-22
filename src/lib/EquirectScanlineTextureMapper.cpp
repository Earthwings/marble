//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007      Carlos Licea     <carlos _licea@hotmail.com>
//


// local
#include"EquirectScanlineTextureMapper.h"

// posix
#include <cmath>

// Qt
#include <QtCore/QDebug>

// Marble
#include "GeoDataPoint.h"
#include "GeoPolygon.h"
#include "MarbleDirs.h"
#include "TextureTile.h"
#include "TileLoader.h"
#include "ViewParams.h"
#include "ViewportParams.h"
#include "AbstractProjection.h"

EquirectScanlineTextureMapper::EquirectScanlineTextureMapper( TileLoader *tileLoader, QObject * parent )
    : AbstractScanlineTextureMapper( tileLoader, parent )
{
    m_oldCenterLon   = 0.0;
    m_oldYPaintedTop = 0;
}


void EquirectScanlineTextureMapper::mapTexture( ViewParams *viewParams )
{
    QImage    *canvasImage = viewParams->canvasImage();
    const int  radius      = viewParams->radius();

    const bool highQuality = ( viewParams->mapQuality() == Marble::High
			       || viewParams->mapQuality() == Marble::Print );
    //const bool printQuality = ( viewParams->mapQuality() == Marble::Print );

    // Initialize needed variables:
    double  lon = 0.0;
    double  lat = 0.0;

    m_tilePosX = 65535;
    m_tilePosY = 65535;

    m_toTileCoordinatesLon = (double)(globalWidth() / 2 - m_tilePosX);
    m_toTileCoordinatesLat = (double)(globalHeight() / 2 - m_tilePosY);

    // Calculate how many degrees are being represented per pixel.
    const float rad2Pixel = (float)( 2 * radius ) / M_PI;

    int yTop;
    int yPaintedTop;
    int yPaintedBottom;

    // Reset backend
    m_tileLoader->resetTilehash();
    selectTileLevel( viewParams );

    // Calculate translation of center point
    double centerLon, centerLat;

    viewParams->centerCoordinates( centerLon, centerLat );

    int yCenterOffset = (int)( centerLat * rad2Pixel );

    // Calculate y-range the represented by the center point, yTop and
    // yBottom and what actually can be painted
    yPaintedTop    = yTop = m_imageHeight / 2 - radius + yCenterOffset;
    yPaintedBottom        = m_imageHeight / 2 + radius + yCenterOffset;
 
    if (yPaintedTop < 0)                yPaintedTop = 0;
    if (yPaintedTop > m_imageHeight)    yPaintedTop = m_imageHeight;
    if (yPaintedBottom < 0)             yPaintedBottom = 0;
    if (yPaintedBottom > m_imageHeight) yPaintedBottom = m_imageHeight;

    float leftLon = + centerLon - ( m_imageWidth / 2 / rad2Pixel );
    while ( leftLon < -M_PI ) leftLon += 2 * M_PI;
    while ( leftLon >  M_PI ) leftLon -= 2 * M_PI;

    const double pixel2Rad = 1.0/rad2Pixel;

    // Paint the map.
    for ( int y = yPaintedTop ;y < yPaintedBottom; ++y ) {
      
        lat = M_PI/2 - (y - yTop )* pixel2Rad;

        m_scanLine = (QRgb*)( canvasImage->scanLine( y ) );
        lon = leftLon;

        QRgb * scanLineBegin = m_scanLine;
        const QRgb * scanLineEnd   = m_scanLine + m_imageWidth;

        for ( QRgb * scanLine = scanLineBegin;
              scanLine < scanLineEnd;
              ++scanLine )
        {
            lon += pixel2Rad;
            if ( lon < -M_PI ) lon += 2 * M_PI;
            if ( lon >  M_PI ) lon -= 2 * M_PI;
            pixelValue( lon, lat, scanLine, highQuality );
        }
    }

    // Remove unused lines
    const int clearStart = ( yPaintedTop - m_oldYPaintedTop <= 0 ) ? yPaintedBottom : 0;
    const int clearStop  = ( yPaintedTop - m_oldYPaintedTop <= 0 ) ? m_imageHeight  : yTop;

    for ( int y = clearStart; y < clearStop; ++y ) {
        m_scanLine = (QRgb*)( canvasImage->scanLine( y ) );
        for ( int x = 0; x < m_imageWidth; ++x ) {
            *(m_scanLine + x) = 0;
        }
    }
    m_oldYPaintedTop = yPaintedTop;

    m_tileLoader->cleanupTilehash();
}


#include "EquirectScanlineTextureMapper.moc"
