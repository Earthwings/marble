//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013   Utku Aydın <utkuaydin34@gmail.com>
//

#include "RouteSyncManager.h"

#include "GeoParser.h"
#include "MarbleDirs.h"
#include "GeoDataFolder.h"
#include "GeoDataDocument.h"
#include "GeoDataPlacemark.h"
#include "KmlElementDictionary.h"
#include "CloudRoutesDialog.h"
#include "OwncloudSyncBackend.h"

#include <QDebug>
#include <QDir>
#include <QUrl>
#include <QFile>
#include <QBuffer>
#include <QScriptValue>
#include <QScriptEngine>
#include <QTemporaryFile>
#include <QNetworkRequest>
#include <QCryptographicHash>
#include <QNetworkAccessManager>
#include <QProgressDialog>
#include <QNetworkReply>

namespace Marble
{

/**
 * Private class for RouteSyncManager.
 */
class RouteSyncManager::Private {
public:
    Private( CloudSyncManager *cloudSyncManager, RoutingManager *routingManager );

    QProgressDialog *m_listDownloadProgressDialog;
    QProgressDialog *m_uploadProgressDialog;
    CloudSyncManager *m_cloudSyncManager;
    RoutingManager *m_routingManager;
    CloudRouteModel *m_model;

    QDir m_cacheDir;
};

RouteSyncManager::Private::Private( CloudSyncManager *cloudSyncManager, RoutingManager *routingManager ) :
    m_listDownloadProgressDialog( new QProgressDialog() ),
    m_uploadProgressDialog( new QProgressDialog() ),
    m_cloudSyncManager( cloudSyncManager ),
    m_routingManager( routingManager ),
    m_model( new CloudRouteModel() )
{
    m_cacheDir = QDir( MarbleDirs::localPath() + "/cloudsync/cache/routes/" );
    
    m_uploadProgressDialog->setMinimum( 0 );
    m_uploadProgressDialog->setMaximum( 100 );
    m_uploadProgressDialog->setWindowTitle( tr( "Uploading route..." ) );
    
    m_listDownloadProgressDialog->setMinimum( 0 );
    m_listDownloadProgressDialog->setMaximum( 100 );
    m_listDownloadProgressDialog->setWindowTitle( tr( "Downloading route list..." ) );
}

RouteSyncManager::RouteSyncManager( CloudSyncManager *cloudSyncManager, RoutingManager *routingManager ) :
    d( new Private( cloudSyncManager, routingManager ) )
{
}

QString RouteSyncManager::generateTimestamp() const
{
    qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    return QString::number( timestamp );
}

QString RouteSyncManager::saveDisplayedToCache() const
{
    d->m_cacheDir.mkpath( d->m_cacheDir.absolutePath() );
    
    const QString timestamp = generateTimestamp();
    const QString filename = d->m_cacheDir.absolutePath() + "/" + timestamp + ".kml";
    d->m_routingManager->saveRoute( filename );
    return timestamp;
}

void RouteSyncManager::uploadRoute()
{
    OwncloudSyncBackend *syncBackend = new OwncloudSyncBackend( d->m_cloudSyncManager->apiUrl()  );
    syncBackend->uploadRoute( saveDisplayedToCache() );
    connect( syncBackend, SIGNAL(routeUploadProgress(qint64,qint64)), this, SLOT(updateUploadProgressbar(qint64,qint64)) );
    //connect( d->m_uploadProgressDialog, SIGNAL(canceled()), syncBackend, SLOT(cancelUpload()) );
    d->m_uploadProgressDialog->exec();
}

void RouteSyncManager::downloadRouteList()
{
    if( d->m_cloudSyncManager->backend()  == CloudSyncManager::Owncloud ) {
        OwncloudSyncBackend *syncBackend = new OwncloudSyncBackend( d->m_cloudSyncManager->apiUrl()  );
        connect( syncBackend, SIGNAL(routeListDownloaded(QVector<RouteItem>)), this, SLOT(processRouteList(QVector<RouteItem>)) );
        connect( syncBackend, SIGNAL(routeListDownloadProgress(qint64,qint64)), this, SIGNAL(routeListDownloadProgress(qint64,qint64)) );
        syncBackend->downloadRouteList();
    }
}

CloudRouteModel* RouteSyncManager::model()
{
    return d->m_model;
}

void RouteSyncManager::processRouteList( QVector<RouteItem> routeList )
{
    d->m_model->setItems( routeList );
}

void RouteSyncManager::downloadRoute( QString timestamp )
{
    if( d->m_cloudSyncManager->backend() == CloudSyncManager::Owncloud ) {
        OwncloudSyncBackend *syncBackend = new OwncloudSyncBackend( d->m_cloudSyncManager->apiUrl()  );
        connect( syncBackend, SIGNAL(routeDownloadProgress(qint64,qint64)), this, SIGNAL(routeDownloadProgress(qint64,qint64)) );
        syncBackend->downloadRoute( timestamp );
    }
}

void RouteSyncManager::openRoute( QString timestamp )
{
    d->m_routingManager->loadRoute( QString( "%0/%1.kml" )
                                    .arg( d->m_cacheDir.absolutePath() ).arg( timestamp ) );
}

void RouteSyncManager::deleteRoute( QString timestamp )
{
    if( d->m_cloudSyncManager->backend() == CloudSyncManager::Owncloud ) {
        OwncloudSyncBackend *syncBackend = new OwncloudSyncBackend( d->m_cloudSyncManager->apiUrl()  );
        connect( syncBackend, SIGNAL(routeDeleted()), this, SLOT(downloadRouteList()) );
        syncBackend->deleteRoute( timestamp );
    }
}

void RouteSyncManager::updateUploadProgressbar( qint64 sent, qint64 total )
{
    qint64 percentage = 100.0 * sent / total;
    d->m_uploadProgressDialog->setValue( percentage );
    
    if( sent == total ) {
        d->m_uploadProgressDialog->accept();
        disconnect( this, SLOT(updateUploadProgressbar(qint64,qint64)) );
    }
}

void RouteSyncManager::updateListDownloadProgressbar(qint64 received, qint64 total)
{
    qint64 percentage = qRound( 100.0 * qreal( received ) / total );
    d->m_listDownloadProgressDialog->setValue( percentage );
    
    if( received == total ) {
        d->m_listDownloadProgressDialog->accept();
        disconnect( this, SLOT(updateListDownloadProgressbar(qint64,qint64)) );
    }
}

}

#include "RouteSyncManager.moc"
