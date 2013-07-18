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
#include <QObject>
#include <QUrl>

namespace Marble {

class Private;
    
class AbstractSyncBackend : public QObject
{
    Q_OBJECT
    
    public:
        explicit AbstractSyncBackend( QUrl apiUrl );
        
        QUrl endpointUrl( QString endpoint );
        QUrl endpointUrl( QString endpoint, QString parameter);
        
    private:
        class Private;
        Private *d;
};

}

#endif // ABSTRACTSYNCBACKEND_H
