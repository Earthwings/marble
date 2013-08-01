//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013   Utku Aydın <utkuaydin34@gmail.com>
//

#ifndef ROUTESYNCMANAGER_H
#define ROUTESYNCMANAGER_H

#include "routing/RoutingManager.h"
#include "CloudRouteModel.h"
#include "marble_export.h"

namespace Marble {

class CloudSyncManager;

class RouteSyncManager : public QObject
{
    Q_OBJECT
    
public:
    RouteSyncManager( CloudSyncManager *cloudSyncManager, RoutingManager *routingManager );

    /**
     * Generates a timestamp which will be used as an unique identifier.
     * @return A timestamp.
     */
    QString generateTimestamp() const;

    /**
     * Saves the route displayed in Marble's routing widget to local cache directory.
     * Uses the RoutingManager passed as a parameter to the constructor.
     * @return Filename of saved file.
     */
    QString saveDisplayedToCache() const;

    /**
     * Uploads currently displayed route to cloud.
     * Initiates necessary methods of backends.
     * Note that, this also runs saveDisplayedToCache() method.
     */
    void uploadRoute();

    CloudRouteModel *model();

public slots:
    /**
     * Downloads route list from cloud.
     * Initiates necessary methods of backends.
     */
    void downloadRouteList();

    /**
     * Opens a CloudRoutesDialog.
     * @param rawRouteList A route list, ready to be parsed.
     */
    //void openCloudRoutesDialog( QVector<RouteItem> routes );

    void processRouteList( QVector<RouteItem> routeList );

    /**
     * Starts the download of specified route.
     * @param timestamp Timestamp of the route that will be downloaded.
     * @see RouteSyncManager::saveDownloadedToCache()
     */
    void downloadRoute( QString timestamp );

    /**
     * Opens route.
     * @param timestamp Timestamp of the route that will be opened.
     */
    void openRoute( QString timestamp );

    /**
     * Deletes route from cloud.
     * @param timestamp Timestamp of the route that will be deleted.
     */
    void deleteRoute( QString timestamp );

    /**
     * Updates upload progressbar.
     * @param sent Bytes sent.
     * @param total Total bytes.
     */
    void updateUploadProgressbar( qint64 sent, qint64 total );

    /**
     * Updates route list downlod progressbar.
     * @param received Bytes received.
     * @param total Total bytes.
     */
    void updateListDownloadProgressbar( qint64 received, qint64 total );

signals:
    /**
     * Emitted when route list ready to use.
     */
    void routeListReady();
    void routeDownloadProgress( qint64 received, qint64 total );
    void routeListDownloadProgress( qint64 received, qint64 total );

private:
    class Private;
    Private *d;
};

}

#endif // ROUTESYNCMANAGER_H
