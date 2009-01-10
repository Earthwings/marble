//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008 Torsten Rahn <tackat@kde.org>"
//

//
// This class is a test plugin.
//

#ifndef MARBLEOVERVIEWMAP_H
#define MARBLEOVERVIEWMAP_H

#include <QtCore/QObject>

#include "GeoDataLatLonAltBox.h"
#include "MarbleAbstractFloatItem.h"

class QSvgRenderer;

namespace Marble
{

/**
 * @short The class that creates an overview map.
 *
 */

class MarbleOverviewMap : public MarbleAbstractFloatItem
{
    Q_OBJECT
    Q_INTERFACES( Marble::MarbleRenderPluginInterface )
    MARBLE_PLUGIN( MarbleOverviewMap )
    
 public:
    explicit MarbleOverviewMap( const QPointF &point = QPointF( 10.5, 10.5 ),
                                const QSizeF &size = QSizeF( 166.0, 86.0 ) );
    ~MarbleOverviewMap();

    QStringList backendTypes() const;

    QString name() const;

    QString guiString() const;

    QString nameId() const;

    QString description() const;

    QIcon icon () const;


    void initialize ();

    bool isInitialized () const;

    bool needsUpdate( ViewportParams *viewport );

    bool renderFloatItem( GeoPainter *painter, ViewportParams *viewport, GeoSceneLayer * layer = 0 );

 protected:
    bool eventFilter( QObject *object, QEvent *e );

 private:
    void changeBackground( const QString& target );

    QString m_target;
    QSvgRenderer  *m_svgobj;
    QPixmap        m_worldmap;

    GeoDataLatLonAltBox m_latLonAltBox;
    qreal m_centerLat;
    qreal m_centerLon;
};

}

#endif
