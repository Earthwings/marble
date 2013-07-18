//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013   Utku Aydın <utkuaydin34@gmail.com>
//

#ifndef SYNCMANAGER_H
#define SYNCMANAGER_H
#include <QObject>
#include <QUrl>

namespace Marble {
    
class SyncManager : public QObject
{
    Q_OBJECT
    
    public:        
        enum Backend {
            Owncloud
        };
        
        Backend backend();
        QString server();
        QString username();
        QString password();
        QString apiPath();
        QUrl apiUrl();
};

}

#endif // SYNCMANAGER_H
