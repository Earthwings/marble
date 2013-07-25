//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013   Utku Aydın <utkuaydin34@gmail.com>
//

#include "OwncloudSyncBackend.h"
#include "CloudRouteModel.h"
#include "MarbleDirs.h"
#include "MarbleDebug.h"

#include <QNetworkAccessManager>
#include <QScriptValueIterator>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QScriptEngine>
#include <QFileInfo>
#include <QDebug>
#include <QDir>

namespace Marble
{

class OwncloudSyncBackend::Private {
    
    public:
        explicit Private();
        
        QDir *m_cacheDir;
        QNetworkAccessManager *m_network;
        QNetworkReply *m_routeUploadReply;
        QNetworkReply *m_routeListReply;
        QNetworkReply *m_routeDownloadReply;
        QNetworkReply *m_routeDeleteReply;
        
        QString m_routeUploadEndpoint;
        QString m_routeListEndpoint;
        QString m_routeDownloadEndpoint;
        QString m_routeDeleteEndpoint;
};

OwncloudSyncBackend::Private::Private() :
    m_cacheDir( new QDir( MarbleDirs::localPath() + "/cloudsync/cache/routes/" ) ),
    m_network( new QNetworkAccessManager() ),
    m_routeUploadReply(),
    m_routeListReply(),
    m_routeDownloadReply(),
    // Route API endpoints
    m_routeUploadEndpoint( "routes/create" ),
    m_routeListEndpoint( "routes" ),
    m_routeDownloadEndpoint( "routes" ),
    m_routeDeleteEndpoint( "routes/delete" )
{
}

OwncloudSyncBackend::OwncloudSyncBackend( QUrl apiUrl ) :
    AbstractSyncBackend( apiUrl ),
    d( new Private() )
{
}

OwncloudSyncBackend::~OwncloudSyncBackend()
{
}

void OwncloudSyncBackend::uploadRoute( QByteArray encodedQuery )
{
    QNetworkRequest request( endpointUrl( d->m_routeUploadEndpoint ) );
    request.setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
    
    d->m_routeUploadReply = d->m_network->post( request, encodedQuery ); 
    connect( d->m_routeUploadReply, SIGNAL(uploadProgress(qint64,qint64)), this, SIGNAL(routeUploadProgress(qint64,qint64)) );
}

void OwncloudSyncBackend::downloadRouteList()
{    
    QNetworkRequest request( endpointUrl( d->m_routeListEndpoint ) );
    d->m_routeListReply = d->m_network->get( request );
    connect( d->m_routeListReply, SIGNAL(finished()), this, SLOT(prepareRouteList()) );
}

void OwncloudSyncBackend::downloadRoute( QString timestamp )
{
    QNetworkRequest request( endpointUrl( d->m_routeDownloadEndpoint, timestamp ) );
    d->m_routeDownloadReply = d->m_network->get( request );
    connect( d->m_routeDownloadReply, SIGNAL(finished()), this, SLOT(saveDownloadedRoute()) );
    connect( d->m_routeDownloadReply, SIGNAL(downloadProgress(qint64,qint64)), this, SIGNAL(routeDownloadProgress(qint64,qint64)) );
}

void OwncloudSyncBackend::deleteRoute(QString timestamp)
{
    QUrl url( endpointUrl( d->m_routeDeleteEndpoint, timestamp ) );
    QNetworkRequest request( url );
    d->m_routeDeleteReply = d->m_network->deleteResource( request );
    connect( d->m_routeDeleteReply, SIGNAL(finished()), this, SIGNAL(routeDeleted()) );
}

void OwncloudSyncBackend::cancelUpload()
{
    d->m_routeUploadReply->abort();
}

void OwncloudSyncBackend::prepareRouteList()
{
    QString result = d->m_routeListReply->readAll();
    
    QVector<RouteItem> vector;
    QScriptEngine engine;
    QScriptValue response = engine.evaluate( QString( "(%0)" ).arg( result ) );
    QScriptValue routes = response.property( "data" );
    
    if( routes.isArray() ) {
        QScriptValueIterator iterator( routes );
        
        while( iterator.hasNext() ) {
            iterator.next();
            
            RouteItem route;
            route.setTimestamp( iterator.value().property( "timestamp" ).toString() );
            route.setName ( iterator.value().property( "name" ).toString() );
            route.setDistance( iterator.value().property( "distance" ).toString() );
            route.setDuration( iterator.value().property( "duration" ).toString() );
            
            vector.append( route );
        }
    }
    
    // FIXME Find why an empty item added to the end.
    if( !vector.isEmpty() ) { vector.remove( vector.count() - 1 ); } 
    emit routeListDownloaded( vector );
}

void OwncloudSyncBackend::saveDownloadedRoute()
{
    QString result = d->m_routeDownloadReply->readAll();
    QString timestamp = QFileInfo( d->m_routeDownloadReply->url().toString() ).fileName();
    
    QScriptEngine engine;
    QScriptValue response = engine.evaluate( QString( "(%0)" ).arg( result ) );
    QScriptValue kml = response.property( "data" );
    
    bool pathCreated = d->m_cacheDir->mkpath( d->m_cacheDir->absolutePath() );
    if ( !pathCreated ) {
        mDebug() << "Couldn't create the path " << d->m_cacheDir->absolutePath() <<
                    ". Check if your user has sufficent permissions for this operation.";
    }
    
    QFile file( d->m_cacheDir->absolutePath() + QString( "/%0.kml").arg( timestamp ) );
    
    bool fileOpened = file.open( QFile::ReadWrite );
    if ( !fileOpened ) {
        mDebug() << "Failed to open file" << d->m_cacheDir->absolutePath() + QString( "/%0.kml").arg( timestamp )
                 <<  " for writing. Its directory either is missing or is not writable.";
    }

    file.write( kml.toString().toUtf8() );
    file.close();
}

}

#include "OwncloudSyncBackend.moc"
