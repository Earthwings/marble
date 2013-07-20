//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013   Utku Aydın <utkuaydin34@gmail.com>
//

#include "CloudRouteModel.h"

#include "MarbleDirs.h"

#include <QDebug>
#include <QVector>
#include <QScriptValue>
#include <QScriptEngine>
#include <QScriptValueIterator>

namespace Marble {

class CloudRouteModel::Private {
    public:
        explicit Private();

        QVector<RouteItem> m_items;
        QString m_cacheDir;
        QModelIndex m_currentlyDownloading;
        qint64 m_totalDownloadSize;
        qint64 m_downloadedSize;
};

CloudRouteModel::Private::Private() :
    m_totalDownloadSize( -1 ),
    m_downloadedSize( 0 )
{
    m_cacheDir = MarbleDirs::localPath() + "/sync/cache/routes/";
}

CloudRouteModel::CloudRouteModel( QObject* parent ) :
    QAbstractListModel( parent ), d( new Private() )
{
}

QVariant CloudRouteModel::data( const QModelIndex& index, int role ) const
{
    if ( index.isValid() && index.row() >= 0 && index.row() < d->m_items.size() ) {
        switch( role ) {
        case Timestamp: return d->m_items.at( index.row() ).timestamp();
        case Name: return d->m_items.at( index.row() ).name();
        case Distance: return d->m_items.at( index.row() ).distance();
        case Duration: return d->m_items.at( index.row() ).duration();
        case IsCached: return isCached( index );
        case IsDownloading: return isDownloading( index );
        case TotalSize: return d->m_totalDownloadSize;
        case DownloadedSize: return d->m_downloadedSize;
        }
    }
    
    return QVariant();
}

int CloudRouteModel::rowCount( const QModelIndex& parent ) const
{
    return d->m_items.count();
}

void CloudRouteModel::setItems( QVector<RouteItem> items )
{
    d->m_items = items;
}

bool CloudRouteModel::isCached( const QModelIndex& index ) const
{
    QDir cacheDir( d->m_cacheDir );
    return cacheDir.exists( index.data( Timestamp ).toString() + ".kml" );
}

void CloudRouteModel::removeFromCache( const QModelIndex index )
{
    QString timestamp = index.data( Timestamp ).toString();
    QString filename = timestamp + ".kml";
    QFile file( QDir( d->m_cacheDir ).entryInfoList( QStringList() << filename ).at( 0 ).absoluteFilePath() );
    file.remove();
}

void CloudRouteModel::setCurrentlyDownloading( const QModelIndex index )
{
    d->m_currentlyDownloading = index;
}

bool CloudRouteModel::isDownloading( const QModelIndex index ) const
{
    return d->m_currentlyDownloading == index;
}

void CloudRouteModel::updateProgress( qint64 currentSize, qint64 totalSize )
{
    d->m_totalDownloadSize = totalSize;
    d->m_downloadedSize = currentSize;
    dataChanged( d->m_currentlyDownloading, d->m_currentlyDownloading );
    if( currentSize == totalSize ) {
        d->m_currentlyDownloading = QModelIndex();
        d->m_totalDownloadSize = -1;
        d->m_downloadedSize = 0;
    }
}

}

#include "CloudRouteModel.moc"
