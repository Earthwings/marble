//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013   Utku Aydın <utkuaydin34@gmail.com>
//

#ifndef ABSTRACTSYNCBACKEND_H
#define ABSTRACTSYNCBACKEND_H

#include <QUrl>
#include <QDir>
#include <QObject>

namespace Marble {

class AbstractSyncBackend : public QObject
{
    Q_OBJECT
    
public:
    explicit AbstractSyncBackend( const QUrl &apiUrl, QObject *parent = 0 );
    ~AbstractSyncBackend();

    QUrl endpointUrl( const QString &endpoint );
    QUrl endpointUrl( const QString &endpoint, const QString &parameter );
    void removeFromCache( const QDir &cacheDir, const QString &timestamp );

private:
    class Private;
    Private *d;
};

}

#endif // ABSTRACTSYNCBACKEND_H
