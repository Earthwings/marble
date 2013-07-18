//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013   Utku Aydın <utkuaydin34@gmail.com>
//

#include "AbstractSyncBackend.h"

namespace Marble
{
class AbstractSyncBackend::Private {
    
    public:
        explicit Private( QUrl apiUrl );
        
        QUrl m_apiUrl;
};

AbstractSyncBackend::Private::Private( QUrl apiUrl ) : m_apiUrl( apiUrl )
{
}

AbstractSyncBackend::AbstractSyncBackend( QUrl apiUrl ) : QObject(), d( new Private( apiUrl ) )
{
}

QUrl AbstractSyncBackend::endpointUrl( QString endpoint )
{
    QString endpointUrl = QString( "%0/%1" ).arg( d->m_apiUrl.toString() ).arg( endpoint );
    return QUrl( endpointUrl );
}

QUrl AbstractSyncBackend::endpointUrl(QString endpoint, QString parameter)
{
    QString endpointUrl = QString( "%0/%1/%2" ).arg( d->m_apiUrl.toString() ).arg( endpoint ).arg( parameter );
    return QUrl( endpointUrl );
}

}

#include "AbstractSyncBackend.moc"
