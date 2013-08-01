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
#include "MarbleWidget.h"
#include "MarbleModel.h"
#include "RenderPlugin.h"
#include "RoutingModel.h"
#include "RoutingManager.h"
#include "GeoParser.h"
#include "GeoDocument.h"
#include "GeoDataFolder.h"
#include "GeoDataDocument.h"
#include "GeoDataPlacemark.h"
#include "KmlElementDictionary.h"

#include <QNetworkAccessManager>
#include <QScriptValueIterator>
#include <QNetworkRequest>
#include <QHttpMultiPart>
#include <QNetworkReply>
#include <QScriptEngine>
#include <QFileInfo>
#include <QBuffer>
#include <QTimer>
#include <QDebug>
#include <QDir>

namespace Marble
{

/**
 * Class that overrides necessary methods of GeoParser.
 * @see GeoParser
 */
class RouteParser : public GeoParser {
public:
    explicit RouteParser();
    virtual GeoDataDocument* createDocument() const;
    virtual bool isValidRootElement();
};

RouteParser::RouteParser() : GeoParser( 0 )
{
    // Nothing to do.
}

GeoDataDocument* RouteParser::createDocument() const
{
    return new GeoDataDocument;
}

bool RouteParser::isValidRootElement()
{
    return isValidElement(kml::kmlTag_kml);
}

class OwncloudSyncBackend::Private {
    
    public:
        explicit Private();

        QDir *m_cacheDir;
        QNetworkAccessManager *m_network;
        QNetworkReply *m_routeUploadReply;
        QNetworkReply *m_routeListReply;
        QNetworkReply *m_routeDownloadReply;
        QNetworkReply *m_routeDeleteReply;
        QNetworkReply *m_routePreviewReply;

        QVector<RouteItem> m_routeList;
        int m_previewPosition;

        QString m_routeUploadEndpoint;
        QString m_routeListEndpoint;
        QString m_routeDownloadEndpoint;
        QString m_routeDeleteEndpoint;
        QString m_routePreviewEndpoint;
};

OwncloudSyncBackend::Private::Private() :
    m_cacheDir( new QDir( MarbleDirs::localPath() + "/cloudsync/cache/routes/" ) ),
    m_network( new QNetworkAccessManager() ),
    m_routeUploadReply(),
    m_routeListReply(),
    m_routeDownloadReply(),
    m_routeList( QVector<RouteItem>() ),
    m_previewPosition( 0 ),
    // Route API endpoints
    m_routeUploadEndpoint( "routes/create" ),
    m_routeListEndpoint( "routes" ),
    m_routeDownloadEndpoint( "routes" ),
    m_routeDeleteEndpoint( "routes/delete" ),
    m_routePreviewEndpoint( "routes/preview" )
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

void OwncloudSyncBackend::uploadRoute( const QString &timestamp )
{
    QString word = "----MarbleCloudBoundary";
    QString boundary = QString( "--%0" ).arg( word );
    QNetworkRequest request( endpointUrl( d->m_routeUploadEndpoint ) );
    request.setHeader( QNetworkRequest::ContentTypeHeader, QString( "multipart/form-data; boundary=" + word ) );

    QByteArray data;
    data.append( QString( boundary + "\r\n" ).toUtf8() );

    // Timestamp part
    data.append( "Content-Disposition: form-data; name=\"timestamp\"" );
    data.append( "\r\n\r\n" );
    data.append( QString( timestamp + "\r\n" ).toUtf8() );
    data.append( QString( boundary + "\r\n" ).toUtf8() );

    // Name part
    data.append( "Content-Disposition: form-data; name=\"name\"" );
    data.append( "\r\n\r\n" );
    data.append( routeName( timestamp ).toUtf8() );
    data.append( "\r\n" );
    data.append( QString( boundary + "\r\n" ).toUtf8() );

    // Duration part
    data.append( "Content-Disposition: form-data; name=\"duration\"" );
    data.append( "\r\n\r\n" );
    data.append( "0\r\n" );
    data.append( QString( boundary + "\r\n" ).toUtf8() );

    // Distance part
    data.append( "Content-Disposition: form-data; name=\"distance\"" );
    data.append( "\r\n\r\n" );
    data.append( "0\r\n" );
    data.append( QString( boundary + "\r\n" ).toUtf8() );

    // KML part
    data.append( QString( "Content-Disposition: form-data; name=\"kml\"; filename=\"" + timestamp + ".kml\"" ).toUtf8() );
    data.append( "\r\n" );
    data.append( "Content-Type: application/vnd.google-earth.kml+xml" );
    data.append( "\r\n\r\n" );

    QFile kmlFile( d->m_cacheDir->absolutePath() + QString( "/%0.kml").arg( timestamp ) );
    kmlFile.open( QFile::ReadOnly );
    data.append( kmlFile.readAll() );

    data.append( "\r\n" );
    data.append( QString( boundary + "\r\n" ).toUtf8() );

    // Preview part
    data.append( QString( "Content-Disposition: form-data; name=\"preview\"; filename=\"" + timestamp + ".png\"" ).toUtf8() );
    data.append( "\r\n" );
    data.append( "Content-Type: image/png" );
    data.append( "\r\n\r\n" );

    QByteArray previewBytes;
    QBuffer previewBuffer( &previewBytes );
    QPixmap preview = createPreview( timestamp );
    preview.save( &previewBuffer, "JPG" );

    data.append( previewBytes );
    data.append( "\r\n" );
    data.append( QString( boundary + "\r\n" ).toUtf8() );

    d->m_routeUploadReply = d->m_network->post( request, data );
    connect( d->m_routeUploadReply, SIGNAL(uploadProgress(qint64,qint64)), this, SIGNAL(routeUploadProgress(qint64,qint64)) );
}

void OwncloudSyncBackend::downloadRouteList()
{    
    QNetworkRequest request( endpointUrl( d->m_routeListEndpoint ) );
    d->m_routeListReply = d->m_network->get( request );
    connect( d->m_routeListReply, SIGNAL(downloadProgress(qint64,qint64)), this, SIGNAL(routeListDownloadProgress(qint64,qint64)) );
    connect( d->m_routeListReply, SIGNAL(finished()), this, SLOT(prepareRouteList()) );
}

void OwncloudSyncBackend::downloadRoute( const QString &timestamp )
{
    QNetworkRequest request( endpointUrl( d->m_routeDownloadEndpoint, timestamp ) );
    d->m_routeDownloadReply = d->m_network->get( request );
    connect( d->m_routeDownloadReply, SIGNAL(finished()), this, SLOT(saveDownloadedRoute()) );
    connect( d->m_routeDownloadReply, SIGNAL(downloadProgress(qint64,qint64)), this, SIGNAL(routeDownloadProgress(qint64,qint64)) );
}

void OwncloudSyncBackend::deleteRoute( const QString &timestamp )
{
    QUrl url( endpointUrl( d->m_routeDeleteEndpoint, timestamp ) );
    QNetworkRequest request( url );
    d->m_routeDeleteReply = d->m_network->deleteResource( request );
    connect( d->m_routeDeleteReply, SIGNAL(finished()), this, SIGNAL(routeDeleted()) );
}

QPixmap OwncloudSyncBackend::createPreview( const QString &timestamp )
{
    MarbleWidget *mapWidget = new MarbleWidget;
    foreach( RenderPlugin* plugin, mapWidget->renderPlugins() ) {
        plugin->setEnabled( false );
    }

    mapWidget->setProjection( Mercator );
    mapWidget->setMapThemeId( "earth/openstreetmap/openstreetmap.dgml" );
    mapWidget->resize( 512, 512 );

    RoutingManager* manager = mapWidget->model()->routingManager();
    manager->loadRoute( d->m_cacheDir->absolutePath() + QString( "/%0.kml" ).arg( timestamp ) );
    GeoDataLatLonBox const bbox = manager->routingModel()->route().bounds();

    if ( !bbox.isEmpty() ) {
        mapWidget->centerOn( bbox );
    }

    return QPixmap::grabWidget( mapWidget );
}

QString OwncloudSyncBackend::routeName( const QString &timestamp )
{
    QFile file( d->m_cacheDir->absolutePath() + QString( "/%0.kml" ).arg( timestamp ) );
    file.open( QFile::ReadOnly );

    RouteParser parser;
    parser.read( &file );
    file.close();

    QString routeName;
    GeoDocument *geoDoc = parser.releaseDocument();
    GeoDataDocument *container = dynamic_cast<GeoDataDocument*>( geoDoc );
    if ( container && container->size() > 0 ) {
        GeoDataFolder *folder = container->folderList().at( 0 );
        foreach ( GeoDataPlacemark *placemark, folder->placemarkList() ) {
            routeName.append( placemark->name() );
            routeName.append( " - " );
        }
    }

    return routeName.left( routeName.length() - 3 );
}

void OwncloudSyncBackend::downloadPreviews()
{
    if( d->m_routeList.count() != 0 ) {
        if( d->m_previewPosition == d->m_routeList.count() ) {
            emit routeListDownloaded( d->m_routeList );
            return;
        }

        QNetworkRequest request( endpointUrl( d->m_routePreviewEndpoint, d->m_routeList.at( d->m_previewPosition ).timestamp() ) );
        d->m_routePreviewReply = d->m_network->get( request );
        connect( d->m_routePreviewReply, SIGNAL(finished()), SLOT(savePreview()) );
    } else {
        emit routeListDownloaded( d->m_routeList );
    }
}

void OwncloudSyncBackend::cancelUpload()
{
    d->m_routeUploadReply->abort();
}

void OwncloudSyncBackend::prepareRouteList()
{
    QString result = d->m_routeListReply->readAll();

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
            
            d->m_routeList.append( route );
        }
    }
    
    // FIXME Find why an empty item added to the end.
    if( !d->m_routeList.isEmpty() ) { d->m_routeList.remove( d->m_routeList.count() - 1 ); }

    downloadPreviews();
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

void OwncloudSyncBackend::savePreview()
{
    const QImage image = QImage::fromData( d->m_routePreviewReply->readAll() );
    const QPixmap pixmap = QPixmap::fromImage( image );
    const QIcon previewIcon( pixmap );

    RouteItem *route = &( d->m_routeList[ d->m_previewPosition ] );
    route->setPreview( previewIcon );

    d->m_previewPosition++;
    downloadPreviews();
}

}

#include "OwncloudSyncBackend.moc"
