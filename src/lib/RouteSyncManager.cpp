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
#include "CloudRouteModel.h"
#include "RoutingModel.h"
#include "GeoDataFolder.h"
#include "GeoDataDocument.h"
#include "GeoDataPlacemark.h"
#include "KmlElementDictionary.h"
#include "CloudRoutesDialog.h"

#include <QDir>
#include <QUrl>
#include <QFile>
#include <QDebug>
#include <QBuffer>
#include <QScriptValue>
#include <QScriptEngine>
#include <QTemporaryFile>
#include <QNetworkRequest>
#include <QCryptographicHash>
#include <QNetworkAccessManager>

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

/**
 * Private class for RouteSyncManager.
 */
class RouteSyncManager::Private {
    public:
        explicit Private( RoutingManager *routingManager, MarbleWidget *marbleWidget );
        
        struct Api {
            QString list;
            QString upload;
            QString download;
        };
        
        Api m_apiRoutes;
        QString m_cacheDir;
        MarbleWidget *m_marbleWidget;
        RoutingManager *m_routingManager;
        QNetworkAccessManager *m_network;
};

RouteSyncManager::Private::Private( RoutingManager* routingManager, MarbleWidget *marbleWidget ) :
    m_marbleWidget( marbleWidget ),
    m_routingManager( routingManager ),
    m_network( new QNetworkAccessManager() )
{
    m_cacheDir = MarbleDirs::localPath() + "/owncloud/cache/routes/";
    m_apiRoutes.list = "/~utku/list.php"; // List
    m_apiRoutes.upload = "/~utku/upload.php"; // Upload
    m_apiRoutes.download = "/~utku/download.php"; // Download        
}

RouteSyncManager::RouteSyncManager( RoutingManager *routingManager, MarbleWidget *marbleWidget ) :
    d( new Private( routingManager, marbleWidget ) )
{
    connect( d->m_network, SIGNAL(finished(QNetworkReply*)), this, SLOT(parseNetworkResponse(QNetworkReply*)) );
}

QString RouteSyncManager::generateTimestamp() const
{
    qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    return QString::number( timestamp );
}
    
QString RouteSyncManager::saveDisplayedToCache() const
{
    // Save KML temporarily, to get its SHA1 summary.
    QTemporaryFile temporaryFile;
    temporaryFile.open();
    d->m_routingManager->saveRoute( temporaryFile.fileName() );
    QByteArray tempKml = temporaryFile.readAll();
    
    const QString stamp = generateTimestamp();
    QDir cacheDir = QDir( d->m_cacheDir );
    if( !cacheDir.exists() ) { 
        cacheDir.mkpath( cacheDir.absolutePath() );
    }
    const QString filename = cacheDir.absolutePath() + "/" + stamp + ".kml";
    
    // Move the temporary file to local cache, with its
    // filename as timestamp.
    // TODO: temporaryFile.rename( filename );
    d->m_routingManager->saveRoute( filename );
    return filename;
}

void RouteSyncManager::uploadRoute()
{
    QString routeFilename = saveDisplayedToCache();
    QUrl url( "http://localhost" + d->m_apiRoutes.upload );
    QNetworkRequest request( url );
    request.setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
    
    QByteArray data;
    QUrl parameters;
    
    // All data from the KML file has to be transferred into a
    // QByteArray for later use because once QFile::readAll()
    // done, all the data gets flushed and QFile::readAll()
    // returns nothing when called later.
    QFile routeFile( routeFilename );
    routeFile.open( QFile::ReadOnly );
    QByteArray kml = routeFile.readAll();
    routeFile.close();
    
    // A QBuffer which uses the QByteArray "kml" needs to be
    // created for RouteParser::read().
    QBuffer buffer( &kml );
    buffer.open( QBuffer::ReadOnly );
        
    RouteParser parser;
    parser.read( &buffer );
    buffer.close();
    
    QString routeName;
    GeoDocument *geoDoc = parser.releaseDocument();
    GeoDataDocument *container = dynamic_cast<GeoDataDocument*>( geoDoc );
    if ( container && container->size() > 0 ) {
        GeoDataFolder *folder = container->folderList().at( 0 );
        foreach ( GeoDataPlacemark *placemark, folder->placemarkList() ) {
            routeName.append( placemark->name() );
            routeName.append( " " );
        }
    }
    
    parameters.addQueryItem( "kml", kml );
    parameters.addQueryItem( "name", routeName.trimmed() );
    data = parameters.encodedQuery();
    
    d->m_network->post( request, data );
}

void RouteSyncManager::downloadRouteList()
{
    QNetworkRequest request( QUrl( "http://localhost" +  d->m_apiRoutes.list ) );
    d->m_network->get( request );
}


void RouteSyncManager::saveDownloadedToCache( QString kml, QString filename ) const
{
    QFile file( d->m_cacheDir + filename + ".kml" );
    file.open( QFile::ReadWrite );
    file.write( kml.toUtf8() );
    file.close();
}

void RouteSyncManager::openCloudRoutesDialog( QString routeJson, MarbleWidget* marbleWidget )
{
    CloudRoutesDialog *dialog = new CloudRoutesDialog( routeJson, marbleWidget );
    connect( dialog, SIGNAL(downloadButtonClicked(QString)), this, SLOT(downloadRoute(QString)) );
    connect( dialog, SIGNAL(openButtonClicked(QString)), this, SLOT(openRoute(QString)) );
    
    dialog->exec();
}

void RouteSyncManager::parseNetworkResponse( QNetworkReply *reply )
{
    QString path = reply->url().path();
    
    if( path == d->m_apiRoutes.upload ) {
        qDebug() << "Upload complete";
    } else if ( path == d->m_apiRoutes.list ) {
        openCloudRoutesDialog( reply->readAll(), d->m_marbleWidget );
    } else if ( path == d->m_apiRoutes.download ) {
        QString filename = reply->url().queryItemValue( "timestamp" );
        saveDownloadedToCache( reply->readAll(), filename );
    }
}

void RouteSyncManager::downloadRoute( QString timestamp )
{
    QUrl url( "http://localhost" +  d->m_apiRoutes.download );
    url.addQueryItem( "timestamp", timestamp );
    
    QNetworkRequest request( url );
    d->m_network->get( request );
}

void RouteSyncManager::openRoute( QString timestamp )
{
    d->m_routingManager->loadRoute( d->m_cacheDir + timestamp + ".kml" );
}

}

#include "RouteSyncManager.moc"