#ifndef ROUTEITEM_H
#define ROUTEITEM_H

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
