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

#include "MarbleDirs.h"
#include "CloudSyncManager.h"
#include "OwncloudSyncBackend.h"

#include <QFile>

namespace Marble {

class BookmarkSyncManager::Private
{
public:
    Private( CloudSyncManager *cloudSyncManager );
    ~Private();

    CloudSyncManager *m_cloudSyncManager;
    OwncloudSyncBackend *m_owncloudBackend;
};

BookmarkSyncManager::Private::Private( CloudSyncManager *cloudSyncManager ) :
    m_cloudSyncManager( cloudSyncManager ),
    m_owncloudBackend( new OwncloudSyncBackend( m_cloudSyncManager->apiUrl().toString() ) )
{
}

BookmarkSyncManager::Private::~Private()
{
}

BookmarkSyncManager::BookmarkSyncManager( CloudSyncManager *cloudSyncManager ) : QObject(),
    d( new Private( cloudSyncManager ) )
{
    connect( d->m_owncloudBackend, SIGNAL(bookmarksUpdateProgress(qint64,qint64)), this, SIGNAL(bookmarksUpdateProgress(qint64,qint64)) );
    connect( d->m_owncloudBackend, SIGNAL(bookmarksDownloadProgress(qint64,qint64)), this, SIGNAL(bookmarksDownloadProgress(qint64,qint64)) );
    connect( d->m_owncloudBackend, SIGNAL(bookmarksUpdated()), this, SIGNAL(bookmarksUpdated()) );
    connect( d->m_owncloudBackend, SIGNAL(bookmarksDownloaded()), this, SIGNAL(bookmarksDownloaded()) );
}

BookmarkSyncManager::~BookmarkSyncManager()
{
    delete d;
}

void BookmarkSyncManager::updateBookmarks()
{
    if( d->m_cloudSyncManager->backend() == CloudSyncManager::Owncloud )
    {
        d->m_owncloudBackend->updateBookmarks();
    }
}

void BookmarkSyncManager::downloadBookmarks()
{
    if( d->m_cloudSyncManager->backend() == CloudSyncManager::Owncloud )
    {
        d->m_owncloudBackend->downloadBookmarks();
    }
}

}
