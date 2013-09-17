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

#include <QObject>

namespace Marble {

class GeoDataPlacemark;

class MARBLE_EXPORT MergeItem : public QObject
{

    Q_OBJECT

public:
    MergeItem();

    Q_PROPERTY( QString pathA READ pathA WRITE setPathA NOTIFY pathAChanged )
    Q_PROPERTY( QString pathB READ pathB WRITE setPathB NOTIFY pathBChanged )
    Q_PROPERTY( GeoDataPlacemark placemarkA READ placemarkA WRITE setPlacemarkA NOTIFY placemarkAChanged )
    Q_PROPERTY( GeoDataPlacemark placemarkB READ placemarkB WRITE setPlacemarkB NOTIFY placemarkBChanged )
    Q_PROPERTY( MergeItem::Resolution resolution READ resolution WRITE setResolution NOTIFY resolutionChanged )

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

signals:
    void pathAChanged();
    void pathBChanged();
    void placemarkAChanged();
    void placemarkBChanged();
    void resolutionChanged();

private:
    QString m_pathA;
    QString m_pathB;
    GeoDataPlacemark m_placemarkA;
    GeoDataPlacemark m_placemarkB;
    MergeItem::Resolution m_resolution;

};

}

#endif // MERGEITEM_H
