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

#include <QObject>

namespace Marble {

class CloudSyncManager;

class BookmarkSyncManager : QObject
{
public:
    BookmarkSyncManager( CloudSyncManager *cloudSyncManager );
    ~BookmarkSyncManager();

    void updateBookmarks();
    void downloadBookmarks();
    void diff();
    void merge();

signals:
    void bookmarksUpdateProgress( qint64 sent, qint64 total );
    void bookmarksDownloadProgress( qint64 received, qint64 total );
    void bookmarksUpdated();
    void bookmarksDownloaded();

private:
    class Private;
    Private *d;
};

}

#endif // BOOKMARKSYNCMANAGER_H
