//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013   Utku Aydın <utkuaydin34@gmail.com>
//

#ifndef BOOKMARKSYNCMANAGER_H
#define BOOKMARKSYNCMANAGER_H

#include "GeoDataPlacemark.h"

#include <QObject>
#include <QNetworkReply>

namespace Marble {

class GeoDataFolder;
class GeoDataDocument;
class GeoDataContainer;
class CloudSyncManager;

class DiffItem
{
public:
    enum Action {
        NoAction,
        Created,
        Changed,
        Deleted
    };

    enum Status {
        Source,
        Destination
    };

    QString m_path;
    Action m_action;
    Status m_origin;
    GeoDataPlacemark m_placemarkA;
    GeoDataPlacemark m_placemarkB;
};

class BookmarkSyncManager : public QObject
{
    Q_OBJECT

public:
    BookmarkSyncManager( CloudSyncManager *cloudSyncManager );
    ~BookmarkSyncManager();

    /**
     * Initiates running of synchronization "method chain".
     */
    void startBookmarkSync();

private:
    /**
     * Uploads local bookmarks.kml to cloud.
     */
    void uploadBookmarks();

    /**
     * Downloads bookmarks.kml from cloud.
     */
    void downloadBookmarks();

    /**
     * Gets cloud bookmarks' timestamp from cloud.
     */
    void downloadTimestamp();

    /**
     * Compares cloud bookmarks.kml to last synced bookmarks.kml.
     * @return true if cloud one is different from last synced one.
     */
    bool cloudBookmarksModified();

    /**
     * Removes all KMLs with timestamps except the
     * one with yougnest timestamp.
     */
    void clearCache();

    QString lastSyncedKmlPath();

    QList<DiffItem> getPlacemarks(GeoDataDocument *document, GeoDataDocument *other, DiffItem::Status diffDirection );

    QList<DiffItem> getPlacemarks( GeoDataFolder *folder, QString &path, GeoDataDocument *other, DiffItem::Status diffDirection );

    GeoDataPlacemark* findPlacemark( GeoDataContainer* container, const GeoDataPlacemark &bookmark ) const;

    void findCounterpart( DiffItem &item, GeoDataDocument* document );

    /**
     * Finds differences between two bookmark files
     * @param sourcePath Source bookmark
     * @param destinationPath Destination bookmark
     * @return A list of differences
     */
    QList<DiffItem> diff( QString &sourcePath, QString &destinationPath );

    void merge( QList<DiffItem> diffListA, QList<DiffItem> diffListB );

private slots:
    void saveDownloaded( QNetworkReply* reply );
    void saveUploadedAsSynced( QNetworkReply* reply );
    void parseTimestamp( QNetworkReply* reply );
    void copyLocalToCache( const QString &timestamp );

signals:
    void uploadProgress( qint64 sent, qint64 total );
    void downloadProgress( qint64 received, qint64 total );
    void timestampDownloaded( const QString &timestamp );

private:
    class Private;
    Private *d;
};

}

#endif // BOOKMARKSYNCMANAGER_H
