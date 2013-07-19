//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013   Utku Aydın <utkuaydin34@gmail.com>
//

#ifndef OWNCLOUDSYNCBACKEND_H
#define OWNCLOUDSYNCBACKEND_H

#include "AbstractSyncBackend.h"
#include "CloudRouteModel.h"

#include <QUrl>
#include <QVector>

namespace Marble {

class OwncloudSyncBackend : public AbstractSyncBackend
{
    Q_OBJECT
    
public:
    explicit OwncloudSyncBackend( QUrl apiUrl );
    ~OwncloudSyncBackend();

    void uploadRoute( QByteArray encodedQuery );
    void downloadRouteList();
    void downloadRoute( QString timestamp );
    void deleteRoute( QString timestamp );

public slots:
    void cancelUpload();

private slots:
    void prepareRouteList();
    void saveDownloadedRoute();

signals:
    void routeListDownloaded( QVector<RouteItem> rawRouteList );
    void routeDownloaded( QString rawRoute, QString timestamp );
    void routeUploadProgress( qint64 sent, qint64 total );
    void routeDownloadProgress( qint64 received, qint64 total );
    
private:
    class Private;
    Private *d;
};

}

#endif // OWNCLOUDSYNCBACKEND_H
