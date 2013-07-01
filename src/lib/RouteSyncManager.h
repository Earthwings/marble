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

#include "SyncManager.h"
#include "MarbleWidget.h"
#include "RoutingManager.h"

#include <QNetworkReply>

namespace Marble {

class RouteParser;
class RouteModel;

class RouteSyncManager : public SyncManager
{
    Q_OBJECT
    
    public:
        explicit RouteSyncManager( RoutingManager *routingManager, MarbleWidget *marbleWidget );
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
         * Uploads a route to an ownCloud server via Marble ownCloud application.
         */
        void uploadRoute();
        
        /**
         * Downloads route list from cloud API.
         */
        void downloadRouteList();
        
        /**
         * Saves the given KML to local cache.
         * @param kml The KML which will be saved to local cache.
         * @param filename KML's filename. It should be a timestamp.
         */
        void saveDownloadedToCache( QString kml, QString filename ) const;
        
        /**
         * Opens a CloudRoutesDialog.
         * @param routeJson List of routes, in JSON format.
         * @param marbleWidget A MarbleWidget to be used as parent.
         */
        void openCloudRoutesDialog( QString routeJson, MarbleWidget* marbleWidget );
        
    public slots:
        /**
         * Parses network response.
         * @param reply Reply of network request.
         */
        void parseNetworkResponse( QNetworkReply* reply );
        
        /**
         * Starts the download of specified route. After
         * the download is complete, parseNetworkResponse()
         * will forward the response to saveDownloadedToCache().
         * @param timestamp Timestamp of the route that will be downloaded.
         * @see RouteSyncManager::parseNetworkResponse()
         * @see RouteSyncManager::saveDownloadedToCache()
         */
        void downloadRoute( QString timestamp );
        
        /**
         * Opens route.
         * @param timestamp Timestamp of the route that will be opened.
         */
        void openRoute( QString timestamp );

    private:
        class Private;
        Private const *d;
};

}

#endif // ROUTESYNCMANAGER_H
