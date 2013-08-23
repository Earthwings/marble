//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013   Utku Aydın <utkuaydin34@gmail.com>
//

#include "CloudSync.h"

#include "MarbleModel.h"
#include "MarbleDeclarativeWidget.h"
#include "cloudsync/RouteSyncManager.h"
#include "cloudsync/CloudRouteModel.h"

class CloudSync::Private
{
public:
    Private();

    MarbleWidget *m_map;
    Marble::CloudRouteModel *m_routeModel;
    Marble::RouteSyncManager *m_routeSyncManager;
};

CloudSync::Private::Private() :
    m_map( 0 ),
    m_routeModel( 0 ),
    m_routeSyncManager( 0 )
{
}

CloudSync::CloudSync( QObject *parent ) : QObject( parent ),
    d( new Private() )
{
}

QObject* CloudSync::routeModel()
{
    return d->m_routeModel;
}

void CloudSync::setRouteModel( QObject *model )
{
    d->m_routeModel = qobject_cast<Marble::CloudRouteModel*>( model );
}

MarbleWidget* CloudSync::map()
{
    return d->m_map;
}

void CloudSync::setMap( MarbleWidget *map )
{
    d->m_map = map;
    delete d->m_routeSyncManager;
    d->m_routeSyncManager = new Marble::RouteSyncManager( d->m_map->model()->cloudSyncManager(),
                                                          d->m_map->model()->routingManager() );
}

void CloudSync::uploadRoute()
{
    if( d->m_routeSyncManager != 0 ) {
        d->m_routeSyncManager->uploadRoute();
    }
}

void CloudSync::downloadRoute( const QString &timestamp )
{
    if( d->m_routeSyncManager != 0 ) {
        d->m_routeSyncManager->downloadRoute( timestamp );
    }
}

void CloudSync::removeRouteFromDevice( const QString &timestamp )
{
    if( d->m_routeSyncManager != 0 ) {
        d->m_routeSyncManager->removeRouteFromCache( timestamp );
    }
}

void CloudSync::deleteRouteFromCloud( const QString &timestamp )
{
    if( d->m_routeSyncManager != 0 ) {
        d->m_routeSyncManager->deleteRoute( timestamp );
    }
}

#include "CloudSync.moc"
