//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013   Utku Aydın <utkuaydin34@gmail.com>
//

#ifndef CLOUDROUTEMODEL_H
#define CLOUDROUTEMODEL_H

#include <QModelIndex>

namespace Marble
{

class RouteItem {
    public:
        bool operator==( const RouteItem &other ) const;

        QString m_timestamp;
        QString m_name;
        QString m_distance;
        QString m_duration;
        bool m_isDownloading;
};

class CloudRouteModel : public QAbstractListModel
{
    Q_OBJECT
        
    public:        
        enum RouteRoles {
            Timestamp = Qt::UserRole + 1,
            Name,
            Distance,
            Duration,
            IsCached,
            IsDownloading,
            TotalSize,
            DownloadedSize
        };
        
        explicit CloudRouteModel( QObject *parent = 0 );
        
        QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
        int rowCount( const QModelIndex& parent = QModelIndex() ) const;
        
        void setItems( QVector<RouteItem> items );
        
        /**
         * Checks if specified route exists in the local cache.
         * @param index Index of the route.
         * @return true, if exists.
         */
        bool isCached( const QModelIndex& index ) const;
        
        /**
         * Removes route with given timestamp from local cache.
         * @param index Index of the route.
         */
        void removeFromCache( const QModelIndex index );
        
        /**
         * Marks the route at given index as being downloaded.
         * @param index Index of the route.
         */
        void setCurrentlyDownloading( const QModelIndex index );
        
        /**
         * Checks if route is being downloaded.
         * @param index Index of the route.
         * @return true, if downloading.
         */
        bool isDownloading( const QModelIndex index ) const;
        
    public slots:
        void updateProgress( qint64 currentSize, qint64 totalSize );

    private:
        class Private;
        Private *d;
    };
}
#endif // CLOUDROUTEMODEL_H