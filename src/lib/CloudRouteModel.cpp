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

class RouteItem {
    public:
        QString m_timestamp;
        QString m_name;
        QString m_distance;
        QString m_duration;
};

class CloudRouteModel::Private {
    public:
        explicit Private();
        
        QVector<RouteItem> m_items;
        QString m_cacheDir;
};

CloudRouteModel::Private::Private()
{
    m_cacheDir = MarbleDirs::localPath() + "/owncloud/cache/routes/";
}

CloudRouteModel::CloudRouteModel( QObject* parent ) :
    QAbstractListModel( parent ), d( new Private() )
{
}

QVariant CloudRouteModel::data( const QModelIndex& index, int role ) const
{
    if ( index.isValid() && index.row() >= 0 && index.row() < d->m_items.size() ) {
        switch( role ) {
            case Timestamp: return d->m_items.at( index.row() ).m_timestamp;
            case Name: return d->m_items.at( index.row() ).m_name;
            case Distance: return d->m_items.at( index.row() ).m_distance;
            case Duration: return d->m_items.at( index.row() ).m_duration;
            case IsCached: return isCached( index );
        }
    }
    
    return QVariant();
}

int CloudRouteModel::rowCount( const QModelIndex& parent ) const
{
    return d->m_items.count();
}

void CloudRouteModel::parseRouteList( QString jsonResponse )
{
    QScriptEngine engine;
    QScriptValue routes = engine.evaluate( QString( "(%0)" ).arg( jsonResponse ) );
    
    if( routes.isArray() ) {
        QScriptValueIterator iterator( routes );
        
        while( iterator.hasNext() ) {
            iterator.next();
            
            RouteItem route;
            route.m_timestamp = iterator.value().property( "timestamp" ).toString();
            route.m_name = iterator.value().property( "name" ).toString();
            route.m_distance = iterator.value().property( "distance" ).toString();
            route.m_duration = iterator.value().property( "duration" ).toString();
            
            d->m_items.append( route );
        }
    }
    
    // FIXME Find why an empty item added to the end.
    d->m_items.remove( d->m_items.count() - 1 );
}

bool CloudRouteModel::isCached( const QModelIndex& index ) const
{
    QDir cacheDir( d->m_cacheDir );
    return cacheDir.exists( index.data( Timestamp ).toString() + ".kml" );
}

void CloudRouteModel::removeFromCache( QModelIndex index )
{
    QString timestamp = index.data( Timestamp ).toString();
    QString filename = timestamp + ".kml";
    QFile file( QDir( d->m_cacheDir ).entryInfoList( QStringList() << filename ).at( 0 ).absoluteFilePath() );
    file.remove();
}

}

#include "CloudRouteModel.moc"