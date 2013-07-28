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

#include <QTimer>

namespace Marble {

class CloudRoutesDialog::Private : public Ui::CloudRoutesDialog {
    public:
        explicit Private( CloudRouteModel *model );
        CloudRouteModel *m_model;
};

CloudRoutesDialog::Private::Private( CloudRouteModel *model ) : Ui::CloudRoutesDialog(),
    m_model( model )
{
}

CloudRoutesDialog::CloudRoutesDialog( CloudRouteModel *model ) : QDialog(),
    d( new Private( model ) )
{
    d->setupUi( this );
    
    RouteItemDelegate *delegate = new RouteItemDelegate( d->listView, d->m_model );
    connect( delegate, SIGNAL(downloadButtonClicked(QString)), this, SIGNAL(downloadButtonClicked(QString)) );
    connect( delegate, SIGNAL(openButtonClicked(QString)), this, SIGNAL(openButtonClicked(QString)) );
    connect( delegate, SIGNAL(deleteButtonClicked(QString)), this, SIGNAL(deleteButtonClicked(QString)) );
    connect( d->m_model, SIGNAL(modelReset()), this, SLOT(displayNoRouteLabel()) );

    d->progressBar->setHidden( true );
    d->labelNoRoute->setHidden( true );

    d->listView->setItemDelegate( delegate );
    d->listView->setModel( d->m_model );
}

CloudRouteModel* CloudRoutesDialog::model()
{
    return d->m_model;
}

void CloudRoutesDialog::updateListDownloadProgressbar( qint64 received, qint64 total )
{
    d->progressBar->setHidden( false );
    d->progressBar->setValue( qRound( 100.0 * qreal( received ) / total ) );
    if( received == total ) {
        QTimer::singleShot( 1000, this, SLOT(hideListDownloadProgressbar()) );
    }
}

void CloudRoutesDialog::hideListDownloadProgressbar() {
    d->progressBar->setHidden( true );
}

void CloudRoutesDialog::displayNoRouteLabel() {
    bool const isEmpty = d->listView->model()->rowCount() == 0;
    d->listView->setHidden( isEmpty );
    d->labelNoRoute->setVisible( isEmpty );
}

}

#include "CloudRoutesDialog.moc"
