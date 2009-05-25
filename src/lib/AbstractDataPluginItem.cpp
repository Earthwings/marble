//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Bastian Holst <bastianholst@gmx.de>
//

// self
#include "AbstractDataPluginItem.h"

// Marble
#include "AbstractProjection.h"
#include "GeoDataCoordinates.h"
#include "ViewportParams.h"

// Qt
#include <QtGui/QAction>
#include <QtCore/QDebug>
#include <QtCore/QRect>
#include <QtCore/QSize>

namespace Marble {

class AbstractDataPluginItemPrivate {
 public:
    AbstractDataPluginItemPrivate()
        : m_addedAngularResolution( 0 )
    {
    };
    
    QString m_id;
    GeoDataCoordinates m_coordinates;
    QString m_target;
    qreal m_addedAngularResolution;
};

AbstractDataPluginItem::AbstractDataPluginItem( QObject *parent )
    : QObject( parent ),
      GeoGraphicsItem(),
      d( new AbstractDataPluginItemPrivate )
{
}

AbstractDataPluginItem::~AbstractDataPluginItem() {
    delete d;
}

QString AbstractDataPluginItem::target() {
    return d->m_target;
}

void AbstractDataPluginItem::setTarget( const QString& target ) {
    d->m_target = target;
}

QString AbstractDataPluginItem::id() const {
    return d->m_id;
}

void AbstractDataPluginItem::setId( const QString& id ) {
    d->m_id = id;
}

qreal AbstractDataPluginItem::addedAngularResolution() const {
    return d->m_addedAngularResolution;
}

void AbstractDataPluginItem::setAddedAngularResolution( qreal resolution ) {
    d->m_addedAngularResolution = resolution;
}

} // Marble namespace

#include "AbstractDataPluginItem.moc"
