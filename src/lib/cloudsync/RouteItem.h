//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013   Utku Aydın <utkuaydin34@gmail.com>
//

#ifndef ROUTEITEM_H
#define ROUTEITEM_H

#include <QIcon>
#include <QString>

namespace Marble {

class RouteItem {

public:
    RouteItem();
    ~RouteItem();

    bool operator==( const RouteItem &other ) const;

    QString timestamp() const;
    void setTimestamp( const QString &timestamp );

    QString name() const;
    void setName( const QString &name );

    QIcon preview() const;
    void setPreview(const QIcon &preview );

    QString distance() const;
    void setDistance( const QString &distance );

    QString duration() const;
    void setDuration( const QString &duration );

private:
    class Private;
    Private *d;
};

}

#endif // ROUTEITEM_H
