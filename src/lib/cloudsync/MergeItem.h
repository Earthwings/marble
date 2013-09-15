//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013   Utku Aydın <utkuaydin34@gmail.com>
//

#ifndef MERGEITEM_H
#define MERGEITEM_H

#include "marble_export.h"
#include "GeoDataPlacemark.h"

#include <QString>

namespace Marble {

class GeoDataPlacemark;

class MARBLE_EXPORT MergeItem
{

public:
    MergeItem();

    enum Resolution {
        None,
        A,
        B
    };

    QString pathA();
    void setPathA( const QString &path );

    QString pathB();
    void setPathB( const QString &path );

    GeoDataPlacemark placemarkA();
    void setPlacemarkA( const GeoDataPlacemark &placemark );

    GeoDataPlacemark placemarkB();
    void setPlacemarkB( const GeoDataPlacemark &placemark );

    MergeItem::Resolution resolution();
    void setResolution( MergeItem::Resolution resolution );

private:
    QString m_pathA;
    QString m_pathB;
    GeoDataPlacemark m_placemarkA;
    GeoDataPlacemark m_placemarkB;
    MergeItem::Resolution m_resolution;

};

}

#endif // MERGEITEM_H
