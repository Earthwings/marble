#include "RouteItem.h"

namespace Marble {

class RouteItem::Private {

public:
    QString m_timestamp;
    QString m_name;
    QString m_distance;
    QString m_duration;
    bool m_isDownloading;
};

RouteItem::RouteItem() : d( new Private )
{
}

RouteItem::~RouteItem()
{
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
