//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013   Utku Aydın <utkuaydin34@gmail.com>
//

#include "RouteItem.h"

namespace Marble {

class RouteItem::Private {

public:
    QString m_timestamp;
    QString m_name;
    QIcon m_preview;
    QString m_distance;
    QString m_duration;
};

RouteItem::RouteItem() : d( new Private )
{
}

RouteItem::~RouteItem()
{
//    delete d;
}

bool RouteItem::operator==( const RouteItem& other ) const
{
    return timestamp() == other.timestamp();
}

QString RouteItem::timestamp() const
{
    return d->m_timestamp;
}

void RouteItem::setTimestamp( const QString &timestamp )
{
    d->m_timestamp = timestamp;
}

QString RouteItem::name() const
{
    return d->m_name;
}

void RouteItem::setName( const QString &name )
{
    d->m_name = name;
}

QIcon RouteItem::preview() const
{
    return d->m_preview;
}

void RouteItem::setPreview( const QIcon &preview )
{
    d->m_preview = preview;
}

QString RouteItem::distance() const
{
    return d->m_distance;
}

void RouteItem::setDistance( const QString &distance )
{
    d->m_distance = distance;
}

QString RouteItem::duration() const
{
    return d->m_duration;
}

void RouteItem::setDuration( const QString &duration )
{
    d->m_duration = duration;
}

}
