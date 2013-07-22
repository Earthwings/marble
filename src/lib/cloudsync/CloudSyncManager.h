//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013   Utku Aydın <utkuaydin34@gmail.com>
//

#ifndef CLOUDSYNCMANAGER_H
#define CLOUDSYNCMANAGER_H

#include <QObject>
#include <QUrl>

#include "RouteSyncManager.h"
#include "marble_export.h"
#include "routing/RoutingManager.h"

namespace Marble {

class MARBLE_EXPORT CloudSyncManager : public QObject
{
    Q_OBJECT
    
public:
    CloudSyncManager( QObject *parent = 0 );

    RouteSyncManager* routeSyncManager();
    void initializeRouteSyncManager(  RoutingManager *routingManager );

    enum Backend {
        Owncloud
    };

    Backend backend();
    QString server();
    QString username();
    QString password();

    void setOwncloudServer( const QString &server );
    void setOwncloudUsername( const QString &username );
    void setOwncloudPassword( const QString &password );

    QString apiPath();
    QUrl apiUrl();

private:
    class Private;
    Private *d;
};

}

#endif // CLOUDSYNCMANAGER_H
