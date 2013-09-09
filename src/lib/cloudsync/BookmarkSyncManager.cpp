//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013   Utku Aydın <utkuaydin34@gmail.com>
//

#include "BookmarkSyncManager.h"

#include "MarbleMath.h"
#include "MarbleDirs.h"
#include "MarbleDebug.h"
#include "GeoDataParser.h"
#include "GeoDataFolder.h"
#include "GeoDataDocument.h"
#include "CloudSyncManager.h"
#include "GeoDataCoordinates.h"
#include "OwncloudSyncBackend.h"

#include <QFile>
#include <QDebug>
#include <QBuffer>
#include <QScriptValue>
#include <QScriptEngine>
#include <QNetworkAccessManager>


namespace Marble {

class BookmarkSyncManager::Private
{
public:
    Private( CloudSyncManager *cloudSyncManager );
    ~Private();

    CloudSyncManager *m_cloudSyncManager;

    QNetworkAccessManager m_network;
    QUrl m_uploadEndpoint;
    QUrl m_downloadEndpoint;
    QUrl m_timestampEndpoint;

    QString m_cachePath;
    QString m_localBookmarksPath;
    QString m_bookmarksTimestamp;
};

BookmarkSyncManager::Private::Private( CloudSyncManager *cloudSyncManager ) : m_cloudSyncManager( cloudSyncManager )
{
    m_cachePath = QString( "%0/cloudsync/cache/bookmarks" ).arg( MarbleDirs::localPath() );
    m_localBookmarksPath = QString( "%0/bookmarks/bookmarks.kml" ).arg( MarbleDirs::localPath() );
    m_downloadEndpoint = QUrl( QString( "%0/%1" ).arg( m_cloudSyncManager->apiUrl().toString() ).arg( "bookmarks/kml" ) );
    m_uploadEndpoint = QUrl( QString( "%0/%1" ).arg( m_cloudSyncManager->apiUrl().toString() ).arg( "bookmarks/update" ) );
    m_timestampEndpoint = QUrl( QString( "%0/%1" ).arg( m_cloudSyncManager->apiUrl().toString() ).arg( "bookmarks/timestamp" ) );
}

BookmarkSyncManager::Private::~Private()
{
}

BookmarkSyncManager::BookmarkSyncManager( CloudSyncManager *cloudSyncManager ) : QObject(), d( new Private( cloudSyncManager ) )
{
}

BookmarkSyncManager::~BookmarkSyncManager()
{
    delete d;
}

void BookmarkSyncManager::startBookmarkSync()
{
    //QString lastSyncedPath = "/home/utku/.local/share/marble/cloudsync/cache/bookmarks/1234567890.kml";
    QString remotePath = QString( "/home/utku/remote.kml" );
    QString lastSyncedPath = lastSyncedKmlPath();
    QList<DiffItem> diffItemsA = diff( lastSyncedPath, d->m_localBookmarksPath );
    QList<DiffItem> diffItemsB = diff( lastSyncedPath, remotePath );
    merge( diffItemsA, diffItemsB );
}

void BookmarkSyncManager::uploadBookmarks()
{
    QByteArray data;
    QByteArray lineBreak = "\r\n";
    QString word = "----MarbleCloudBoundary";
    QString boundary = QString( "--%0" ).arg( word );
    QNetworkRequest request( d->m_uploadEndpoint );
    request.setHeader( QNetworkRequest::ContentTypeHeader, QString( "multipart/form-data; boundary=%0" ).arg( word ) );

    data.append( QString( boundary + lineBreak ).toUtf8() );
    data.append( "Content-Disposition: form-data; name=\"bookmarks\"; filename=\"bookmarks.kml\"" + lineBreak );
    data.append( "Content-Type: application/vnd.google-earth.kml+xml" + lineBreak + lineBreak );

    QFile bookmarksFile( d->m_localBookmarksPath );
    if( !bookmarksFile.open( QFile::ReadOnly ) ) {
        mDebug() << "Failed to open file" << bookmarksFile.fileName()
                 <<  ". It is either missing or not readable.";
        return;
    }

    data.append( bookmarksFile.readAll() + lineBreak + lineBreak );
    data.append( QString( boundary ).toUtf8() );
    bookmarksFile.close();

    QNetworkReply *reply = d->m_network.post( request, data );
    connect( reply, SIGNAL(uploadProgress(qint64,qint64)),
             this, SIGNAL(uploadProgress(qint64,qint64)) );
    connect( &d->m_network, SIGNAL(finished(QNetworkReply*)),
             this, SLOT(saveUploadedAsSynced(QNetworkReply*)) );
}

void BookmarkSyncManager::downloadBookmarks()
{
    QNetworkRequest request( d->m_downloadEndpoint );
    QNetworkReply *reply = d->m_network.get( request );
    connect( &d->m_network, SIGNAL(finished(QNetworkReply*)),
             this, SLOT(saveDownloaded(QNetworkReply*)) );
    connect( reply, SIGNAL(downloadProgress(qint64,qint64)),
             this, SIGNAL(downloadProgress(qint64,qint64)) );
}

void BookmarkSyncManager::downloadTimestamp()
{
    d->m_network.get( QNetworkRequest( d->m_timestampEndpoint ) );
    connect( &d->m_network, SIGNAL(finished(QNetworkReply*)),
             this, SLOT(parseTimestamp(QNetworkReply*)) );
}

bool BookmarkSyncManager::cloudBookmarksModified()
{
    QEventLoop loop;
    QNetworkRequest request( d->m_timestampEndpoint );
    QNetworkReply *reply = d->m_network.get( request );
    connect( reply, SIGNAL(finished()), &loop, SLOT(quit()) );
    loop.exec();

    QScriptEngine engine;
    QString response = reply->readAll();
    QScriptValue parsedResponse = engine.evaluate( QString( "(%0)" ).arg( response ) );
    QString timestamp = parsedResponse.property( "data" ).toString();

    QStringList entryList = QDir( d->m_cachePath ).entryList(
                QStringList() << "*.kml",
                QDir::NoFilter, QDir::Name );
    QString lastSynced = entryList.last();
    lastSynced.chop( 4 );

    return timestamp != lastSynced;
}

void BookmarkSyncManager::clearCache()
{
    QDir cacheDir( d->m_cachePath );
    QFileInfoList fileInfoList = cacheDir.entryInfoList(
                // FIXME QStringList() << "^[0-9]{10}.kml$",
                QStringList() << "*.kml",
                QDir::NoFilter, QDir::Name );
    fileInfoList.removeLast();
    foreach ( QFileInfo fileInfo, fileInfoList ) {
        QFile( fileInfo.absolutePath() ).remove();
    }
}

QString BookmarkSyncManager::lastSyncedKmlPath()
{
    QDir cacheDir( d->m_cachePath );
    QFileInfoList fileInfoList = cacheDir.entryInfoList(
                // FIXME QStringList() << "^[0-9]{10}.kml$",
                QStringList() << "*.kml",
                QDir::NoFilter, QDir::Name );
    return fileInfoList.last().absoluteFilePath();
}

QList<DiffItem> BookmarkSyncManager::getPlacemarks( GeoDataDocument *document, GeoDataDocument *other, DiffItem::Status diffDirection )
{
    QList<DiffItem> diffItems;
    foreach ( GeoDataFolder *folder, document->folderList() ) {
        QString path = QString( "/%0" ).arg( folder->name() );
        diffItems.append( getPlacemarks( folder, path, other, diffDirection ) );
    }

    return diffItems;
}

QList<DiffItem> BookmarkSyncManager::getPlacemarks( GeoDataFolder *folder, QString &path, GeoDataDocument *other, DiffItem::Status diffDirection )
{
    QList<DiffItem> diffItems;
    foreach ( GeoDataFolder *folder, folder->folderList() ) {
        QString newPath = QString( "%0/%1" ).arg( path, folder->name() );
        diffItems.append( getPlacemarks( folder, newPath, other, diffDirection ) );
    }

    foreach( GeoDataPlacemark *placemark, folder->placemarkList() ) {
        DiffItem diffItem;
        diffItem.m_path = path;
        diffItem.m_placemarkA = *placemark;
        switch ( diffDirection ) {
        case DiffItem::Source:
            diffItem.m_origin = DiffItem::Destination;
            break;
        case DiffItem::Destination:
            diffItem.m_origin = DiffItem::Source;
            break;
        default:
            break;
        }

        findCounterpart( diffItem, other );

        if( !( diffItem.m_action == DiffItem::NoAction && diffItem.m_origin == DiffItem::Destination )
                && !( diffItem.m_action == DiffItem::Changed && diffItem.m_origin == DiffItem::Source ) ) {
            diffItems.append( diffItem );
        }
    }

    return diffItems;
}

GeoDataPlacemark* BookmarkSyncManager::findPlacemark( GeoDataContainer* container, const GeoDataPlacemark &bookmark ) const
{
    foreach( GeoDataPlacemark* placemark, container->placemarkList() ) {
        /** @todo Replace with distance based equality check */
        if ( distanceSphere( placemark->coordinate(), bookmark.coordinate() ) <= 1 ) {
            return placemark;
        }
    }

    foreach( GeoDataFolder* folder, container->folderList() ) {
        GeoDataPlacemark* placemark = findPlacemark( folder, bookmark );
        if ( placemark ) {
            return placemark;
        }
    }

    return 0;
}

void BookmarkSyncManager::findCounterpart( DiffItem &item, GeoDataDocument *document )
{
    GeoDataPlacemark *match = findPlacemark( document, item.m_placemarkA );

    if( match != 0 ) {
        item.m_placemarkB = *match;
        bool nameChanged = item.m_placemarkA.name() != item.m_placemarkB.name();
        bool descChanged = item.m_placemarkA.description() != item.m_placemarkB.description();
        bool lookAtChanged = item.m_placemarkA.lookAt()->latitude() != item.m_placemarkB.lookAt()->latitude() ||
                item.m_placemarkA.lookAt()->longitude() != item.m_placemarkB.lookAt()->longitude() ||
                item.m_placemarkA.lookAt()->altitude() != item.m_placemarkB.lookAt()->altitude() ||
                item.m_placemarkA.lookAt()->range() != item.m_placemarkB.lookAt()->range();
        if(  nameChanged || descChanged || lookAtChanged ) {
            item.m_action = DiffItem::Changed;
        } else {
            item.m_action = DiffItem::NoAction;
        }
    } else {
        switch( item.m_origin ) {
        case DiffItem::Source:
            item.m_action = DiffItem::Deleted;
            break;
        case DiffItem::Destination:
            item.m_action = DiffItem::Created;
            break;
        }

    }
}

QList<DiffItem> BookmarkSyncManager::diff( QString &sourcePath, QString &destinationPath )
{
    GeoDataParser parserA( GeoData_KML );
    QFile fileA( sourcePath );
    fileA.open( QFile::ReadOnly );
    parserA.read( &fileA );
    GeoDataDocument *documentA = dynamic_cast<GeoDataDocument*>( parserA.releaseDocument() );

    GeoDataParser parserB( GeoData_KML );
    QFile fileB( destinationPath );
    fileB.open( QFile::ReadOnly );
    parserB.read( &fileB );
    GeoDataDocument *documentB = dynamic_cast<GeoDataDocument*>( parserB.releaseDocument() );

    QList<DiffItem> diffItems = getPlacemarks( documentA, documentB, DiffItem::Destination ); // Compare old to new
    diffItems.append( getPlacemarks( documentB, documentA, DiffItem::Source ) ); // Compare new to old

    // Compare paths
    for( int i = 0; i < diffItems.count(); i++ ) {
        for( int p = i + 1; p < diffItems.count(); p++ ) {
            if( ( diffItems[i].m_origin == DiffItem::Source )
                    && ( diffItems[i].m_action == DiffItem::NoAction )
                    && ( distanceSphere( diffItems[i].m_placemarkA.coordinate(), diffItems[p].m_placemarkB.coordinate() ) <= 1 )
                    && ( distanceSphere( diffItems[i].m_placemarkB.coordinate(), diffItems[p].m_placemarkA.coordinate() ) <= 1 )
                    && ( diffItems[i].m_path != diffItems[p].m_path ) ) {
                diffItems[p].m_action = DiffItem::Changed;
            }
        }
    }

    foreach( DiffItem item, diffItems ) {
        switch(item.m_action) {
        case DiffItem::Changed: qDebug() << item.m_path << "/" << item.m_placemarkA.name() << ": Changed"; break;
        case DiffItem::Deleted: qDebug() << item.m_path << "/" << item.m_placemarkA.name() << ": Deleted"; break;
        case DiffItem::Created: qDebug() << item.m_path << "/" << item.m_placemarkA.name() << ": Created"; break;
        case DiffItem::NoAction: qDebug() << item.m_path << "/" << item.m_placemarkA.name() << ": No Action"; break;
        }
    }

    qDebug() << "--- end of diff ---";

    return diffItems;
}

void BookmarkSyncManager::merge( QList<DiffItem> diffListA, QList<DiffItem> diffListB )
{
    QList<DiffItem> mergedList;

    foreach( DiffItem itemA, diffListA ) {
        if( itemA.m_action == DiffItem::NoAction ) {
            bool deleted = false;

            foreach( DiffItem itemB, diffListB ) {
                if( distanceSphere( itemA.m_placemarkA.coordinate(), itemB.m_placemarkA.coordinate() ) <= 1 && itemB.m_action == DiffItem::Deleted ) {
                    deleted = true;
                }
            }

            if( !deleted ) {
                mergedList.append( itemA );
            }
        } else if( itemA.m_action == DiffItem::Created ) {
            mergedList.append( itemA );
        } else if( itemA.m_action == DiffItem::Changed ) {
            bool conflict = false;

            // Find a better solution than this
            DiffItem other;
            foreach( DiffItem itemB, diffListB ) {
                if( distanceSphere( itemA.m_placemarkB.coordinate(), itemB.m_placemarkB.coordinate() ) <= 1
                        && itemB.m_action == DiffItem::Changed ) {
                    conflict = true;
                    other = itemB;
                }
            }

            if( !conflict ) {
                mergedList.append( itemA );
            } else {
                // Emit conflict signal, raise dialogs etc.
                // Favor B (other) for now.
                mergedList.append( other );
            }
        }
    }

    foreach( DiffItem itemB, diffListB ) {
        if( itemB.m_action == DiffItem::Created ) {
            mergedList.append( itemB );
        }
    }

    foreach( DiffItem item, mergedList ) {
        switch(item.m_action) {
        case DiffItem::Changed: qDebug() << item.m_path << "/" << item.m_placemarkA.name() << ": Changed"; break;
        case DiffItem::Deleted: qDebug() << item.m_path << "/" << item.m_placemarkA.name() << ": Deleted"; break;
        case DiffItem::Created: qDebug() << item.m_path << "/" << item.m_placemarkA.name() << ": Created"; break;
        case DiffItem::NoAction: qDebug() << item.m_path << "/" << item.m_placemarkA.name() << ": No Action"; break;
        }
    }

    //return mergedList;
}

void BookmarkSyncManager::saveDownloaded( QNetworkReply *reply )
{
    disconnect( &d->m_network, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(saveDownloaded(QNetworkReply*)) );
    QByteArray kmlResponse = reply->readAll();

    QString localBookmarksDir = d->m_localBookmarksPath;
    QDir().mkdir( localBookmarksDir.remove( "bookmarks.kml" ) );
    QFile bookmarksFile( d->m_localBookmarksPath );
    if( !bookmarksFile.open( QFile::ReadWrite ) ) {
        mDebug() << "Failed to open file" << bookmarksFile.fileName()
                 <<  ". It is either missing or not readable.";
        return;
    }

    bookmarksFile.write( kmlResponse );
    bookmarksFile.close();

    connect( this, SIGNAL(timestampDownloaded(QString)),
             this, SLOT(copyLocalToCache(QString)) );
    downloadTimestamp(); // parseTimestamp() emits timestampDownloaded()
}

void BookmarkSyncManager::saveUploadedAsSynced( QNetworkReply *reply )
{
    disconnect( &d->m_network, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(saveUploadedAsSynced(QNetworkReply*)) );
    QScriptEngine engine;
    QString timestampResponse = reply->readAll();
    QScriptValue parsedResponse = engine.evaluate( QString( "(%0)" ).arg( timestampResponse ) );
    QScriptValue timestamp = parsedResponse.property( "data" );
    copyLocalToCache( timestamp.toString() );
}

void BookmarkSyncManager::parseTimestamp( QNetworkReply *reply )
{
    disconnect( &d->m_network, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(parseTimestamp(QNetworkReply*)) );
    QString response = reply->readAll();

    QScriptEngine engine;
    QScriptValue parsedResponse = engine.evaluate( QString( "(%0)" ).arg( response ) );
    QString timestamp = parsedResponse.property( "data" ).toString();
    emit timestampDownloaded( timestamp );
}
void BookmarkSyncManager::copyLocalToCache( const QString &timestamp )
{
    disconnect( this, SIGNAL(timestampDownloaded(QString)),
             this, SLOT(copyLocalToCache(QString)) );
    QDir().mkpath( d->m_cachePath );
    clearCache();

    QFile bookmarksFile( d->m_localBookmarksPath );
    bookmarksFile.copy( QString( "%0/%1.kml" ).arg( d->m_cachePath, timestamp ) );
}

}

#include "BookmarkSyncManager.moc"
