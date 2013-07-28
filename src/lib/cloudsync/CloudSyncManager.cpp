//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013   Utku Aydın <utkuaydin34@gmail.com>
//

#include "CloudSyncManager.h"

namespace Marble
{

class CloudSyncManager::Private {

public:
    Private();

    RouteSyncManager *m_routeSyncManager;

    bool m_syncEnabled;
    bool m_routeSyncEnabled;

    QString m_server;
    QString m_username;
    QString m_password;
};

CloudSyncManager::Private::Private() :
    m_routeSyncManager(),
    m_syncEnabled( false ),
    m_routeSyncEnabled( false ),
    m_server(),
    m_username(),
    m_password()
{
}

CloudSyncManager::CloudSyncManager( QObject *parent ) : QObject( parent ),
    d( new Private() )
{
}

RouteSyncManager* CloudSyncManager::routeSyncManager()
{
    return d->m_routeSyncManager;
}

void CloudSyncManager::initializeRouteSyncManager( RoutingManager *routingManager )
{
    d->m_routeSyncManager = new RouteSyncManager( this, routingManager );
}

CloudSyncManager::Backend CloudSyncManager::backend()
{
    return Owncloud;
}

bool CloudSyncManager::isSyncEnabled()
{
    return d->m_syncEnabled;
}

bool CloudSyncManager::isRouteSyncEnabled()
{
    return d->m_routeSyncEnabled;
}

QString CloudSyncManager::server()
{
    return d->m_server;
}

QString CloudSyncManager::username()
{
    return d->m_username;
}

QString CloudSyncManager::password()
{
    return d->m_password;
}

void CloudSyncManager::setSyncEnabled( const bool &enabled )
{
    d->m_syncEnabled = enabled;
}

void CloudSyncManager::setRouteSyncEnabled( const bool &enabled )
{
    d->m_routeSyncEnabled = enabled;
}

void CloudSyncManager::setOwncloudServer( const QString &server )
{
    d->m_server = server;
}

void CloudSyncManager::setOwncloudUsername( const QString &username )
{
    d->m_username = username;
}

void CloudSyncManager::setOwncloudPassword( const QString &password )
{
    d->m_password = password;
}

QString CloudSyncManager::apiPath()
{
    return "index.php/apps/marble/api/v1";
}

QUrl CloudSyncManager::apiUrl()
{
    return QUrl( QString( "http://%0:%1@%2/%3" )
                .arg( username() ).arg( password() )
                .arg( server() ).arg( apiPath() ) );
}

}

#include "CloudSyncManager.moc"
