//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013   Utku Aydın <utkuaydin34@gmail.com>
//

#include "MergeItem.h"

#include "GeoDataPlacemark.h"

namespace Marble {

class MergeItem::Private
{
public:
    QString m_pathA;
    QString m_pathB;
    GeoDataPlacemark m_placemarkA;
    GeoDataPlacemark m_placemarkB;
    MergeItem::Resolution m_resolution;
};

MergeItem::MergeItem()
{
}

QString MergeItem::pathA()
{
    return d->m_pathA;
}
void MergeItem::setPathA( const QString &path )
{
    d->m_pathA = path;
}

QString MergeItem::pathB()
{
    return d->m_pathB;
}

void MergeItem::setPathB( const QString &path )
{
    d->m_pathB = path;
}

GeoDataPlacemark MergeItem::placemarkA()
{
    return d->m_placemarkA;
}

void MergeItem::setPlacemarkA( const GeoDataPlacemark &placemark )
{
    d->m_placemarkA = placemark;
}

GeoDataPlacemark MergeItem::placemarkB()
{
    return d->m_placemarkB;
}

void MergeItem::setPlacemarkB( const GeoDataPlacemark &placemark )
{
    d->m_placemarkB = placemark;
}

MergeItem::Resolution MergeItem::resolution()
{
    return d->m_resolution;
}

void MergeItem::setResolution( MergeItem::Resolution resolution )
{
    d->m_resolution = resolution;
}

}
