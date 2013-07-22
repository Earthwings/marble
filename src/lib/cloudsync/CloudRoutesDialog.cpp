//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013   Utku Aydın <utkuaydin34@gmail.com>
//

#include "CloudRoutesDialog.h"
#include "ui_CloudRoutesDialog.h"

#include "RouteItemDelegate.h"

#include <QDebug>
#include <QPainter>
#include <QApplication>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

namespace Marble {

class CloudRoutesDialog::Private : public Ui::CloudRoutesDialog {
    public:
        explicit Private();
        CloudRouteModel *m_model;
};

CloudRoutesDialog::Private::Private() : Ui::CloudRoutesDialog(),
    m_model( new CloudRouteModel() )
{
}

CloudRoutesDialog::CloudRoutesDialog( QVector<RouteItem> routes ) : QDialog(),
    d( new Private )
{
    d->setupUi( this );
    
    RouteItemDelegate *delegate = new RouteItemDelegate( d->listView, d->m_model );
    connect( delegate, SIGNAL(downloadButtonClicked(QString)), this, SIGNAL(downloadButtonClicked(QString)) );
    connect( delegate, SIGNAL(openButtonClicked(QString)), this, SIGNAL(openButtonClicked(QString)) );
    connect( delegate, SIGNAL(deleteButtonClicked(QString)), this, SIGNAL(deleteButtonClicked(QString)) );
    
    d->m_model->setItems( routes );
    d->listView->setItemDelegate( delegate );
    d->listView->setModel( d->m_model );
}

CloudRouteModel* CloudRoutesDialog::model()
{
    return d->m_model;
}

}

#include "CloudRoutesDialog.moc"
