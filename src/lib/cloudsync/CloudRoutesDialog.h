//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013   Utku Aydın <utkuaydin34@gmail.com>
//

#ifndef CLOUDROUTESDIALOG_H
#define CLOUDROUTESDIALOG_H

#include "MarbleWidget.h"
#include "CloudRouteModel.h"

#include <QDialog>
#include <QListView>
#include <QStyledItemDelegate>

namespace Marble {

class CloudRoutesDialog : public QDialog
{
    Q_OBJECT
    
    public:
        explicit CloudRoutesDialog( CloudRouteModel *model );
        CloudRouteModel *model();
        
    signals:
        void downloadButtonClicked( QString timestamp );
        void openButtonClicked( QString timestamp );
        void deleteButtonClicked( QString timestamp );
        
    private:
        class Private;
        Private *d;
};

}
#endif // CLOUDROUTESDIALOG_H
