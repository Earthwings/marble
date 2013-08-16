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

    bool m_offlineMode;

    bool m_syncEnabled;
    bool m_routeSyncEnabled;

    QString m_ownloudServer;
    QString m_owncloudUsername;
    QString m_owncloudPassword;
};

CloudSyncManager::Private::Private() :
    m_routeSyncManager( 0 ),
    m_offlineMode( false ),
    m_syncEnabled( false ),
    m_routeSyncEnabled( true ),
    m_ownloudServer(),
    m_owncloudUsername(),
    m_owncloudPassword()
{
}

CloudSyncManager::CloudSyncManager( QObject *parent ) : QObject( parent ),
    d( new Private() )
{
}

CloudSyncManager::~CloudSyncManager()
{
    delete d;
}

RouteSyncManager* CloudSyncManager::routeSyncManager()
{
    return d->m_routeSyncManager;
}

void CloudSyncManager::setRouteSyncManager( RoutingManager *routingManager )
{
    delete d->m_routeSyncManager;
    d->m_routeSyncManager = new RouteSyncManager( this, routingManager );
}

bool CloudSyncManager::workOffline()
{
    return d->m_offlineMode;
}

void CloudSyncManager::setWorkOffline( bool offline )
{
    d->m_offlineMode = offline;
}

CloudSyncManager::Backend CloudSyncManager::backend() const
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

QString CloudSyncManager::server() const
{
    return d->m_ownloudServer;
}

QString CloudSyncManager::username() const
{
    return d->m_owncloudUsername;
}

QString CloudSyncManager::password() const
{
    return d->m_owncloudPassword;
}

void CloudSyncManager::setSyncEnabled( bool enabled )
{
    d->m_syncEnabled = enabled;
}

void CloudSyncManager::setRouteSyncEnabled( bool enabled )
{
    d->m_routeSyncEnabled = enabled;
}

void CloudSyncManager::setOwncloudServer( const QString &server )
{
    d->m_ownloudServer = server;
}

void CloudSyncManager::setOwncloudUsername( const QString &username )
{
    d->m_owncloudUsername = username;
}

void CloudSyncManager::setOwncloudPassword( const QString &password )
{
    d->m_owncloudPassword = password;
}

QString CloudSyncManager::apiPath() const
{
    return "index.php/apps/marble/api/v1";
}

QUrl CloudSyncManager::apiUrl() const
{
    return QUrl( QString( "http://%0:%1@%2/%3" )
                .arg( username() ).arg( password() )
                .arg( server() ).arg( apiPath() ) );
}

}

#include "CloudSyncManager.moc"
