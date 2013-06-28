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

class CloudRouteModel : public QAbstractListModel
{
    Q_OBJECT
        
    public:
        enum RouteRoles {
            Timestamp = Qt::UserRole + 1,
            Name,
            Distance,
            Duration,
            IsCached
        };
        
        explicit CloudRouteModel( QObject *parent = 0 );
        
        QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
        int rowCount( const QModelIndex& parent = QModelIndex() ) const;
        
        /**
         * Parses route list JSON, assigns every route to a model,
         * adds all models to a list.
         * @param jsonResponse Raw JSON of route list.
         */
        void parseRouteList( QString jsonResponse );
        
        /**
         * Checks if specified route exists in the local cache.
         * @param index Index of the route.
         * @return true, if exists.
         */
        bool isCached( const QModelIndex& index ) const;
        
        /**
         * Removes route with given timestamp from local cache.
         * @param timestamp Route's timestamp.
         */
        void removeFromCache( QModelIndex index );

    private:
        class Private;
        Private *d;
    };

}
#endif // CLOUDROUTEMODEL_H