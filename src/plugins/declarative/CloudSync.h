//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013   Utku Aydın <utkuaydin34@gmail.com>
//

#ifndef CLOUDSYNC_H
#define CLOUDSYNC_H

#include <QObject>
#include <QVector>

class MarbleWidget;

class CloudSync : public QObject
{

    Q_OBJECT
    Q_PROPERTY( QObject* routeModel READ routeModel NOTIFY routeModelChanged )
    Q_PROPERTY( MarbleWidget* map READ map WRITE setMap NOTIFY mapChanged )

public:
    explicit CloudSync( QObject *parent = 0 );

    QObject* routeModel();
    void setRouteModel( QObject *model );

    MarbleWidget* map();
    void setMap( MarbleWidget *map );

public slots:
    void uploadRoute();
    void downloadRoute( const QString &timestamp );
    void removeRouteFromDevice( const QString &timestamp );
    void deleteRouteFromCloud( const QString &timestamp );

signals:
    void routeModelChanged();
    void mapChanged();

private:
    class Private;
    Private *d;
};

#endif // CLOUDSYNC_H
