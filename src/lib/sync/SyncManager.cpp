//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013   Utku Aydın <utkuaydin34@gmail.com>
//

#include "SyncManager.h"
namespace Marble
{

SyncManager::Backend SyncManager::backend()
{
    return Owncloud;
}

QString SyncManager::server()
{
    return "";
}

QString SyncManager::username()
{
    return "";
}

QString SyncManager::password()
{
    return "";
}

QString SyncManager::apiPath()
{
    return "index.php/apps/marble/api/v1";
}

QUrl SyncManager::apiUrl()
{
    return QUrl( QString( "http://%0:%1@%2/%3" )
                .arg( username() ).arg( password() )
                .arg( server() ).arg( apiPath() ) );
}

}

#include "SyncManager.moc"
