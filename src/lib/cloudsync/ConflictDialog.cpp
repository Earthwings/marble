//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013   Utku Aydın <utkuaydin34@gmail.com>
//

#include "ConflictDialog.h"

#include "MergeItem.h"
#include "GeoDataPlacemark.h"

#include <QLabel>
#include <QVariant>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace Marble {

ConflictDialog::ConflictDialog(MergeItem *mergeItem, QWidget *parent ) : QDialog( parent )
{
    m_mergeItem = mergeItem;
    m_box = new QDialogButtonBox( QDialogButtonBox::Cancel );

    QPushButton *localButton = new QPushButton( tr( "Use local" ) );
    QPushButton *cloudButton = new QPushButton( tr( "Use cloud" ) );
    QPushButton *allLocalButton = new QPushButton( tr( "Always use local" ) );
    QPushButton *allCloudButton = new QPushButton( tr( "Always use cloud" ) );

    localButton->setDefault( true );
    localButton->setProperty( "ActionRole", ConflictDialog::Local );
    cloudButton->setProperty( "ActionRole", ConflictDialog::Cloud );
    allLocalButton->setProperty( "ActionRole", ConflictDialog::AllLocal );
    allCloudButton->setProperty( "ActionRole", ConflictDialog::AllCloud );

    m_box->addButton( localButton, QDialogButtonBox::ActionRole );
    m_box->addButton( cloudButton, QDialogButtonBox::ActionRole );
    m_box->addButton( allLocalButton, QDialogButtonBox::ActionRole );
    m_box->addButton( allCloudButton, QDialogButtonBox::ActionRole );

    QVBoxLayout *leftLayout = new QVBoxLayout();
    QString localHeaderText = tr( "Local placemark" );
    QString localDetailText = tr( "Path: %0 <br /> Name: %1" )
            .arg( m_mergeItem->pathA(), m_mergeItem->placemarkA().name() );
    QLabel *localHeaderLabel = new QLabel( localHeaderText );
    QLabel *localDetailLabel = new QLabel( localDetailText );
    leftLayout->addWidget( localHeaderLabel );
    leftLayout->addWidget( localDetailLabel );

    QVBoxLayout *rightLayout = new QVBoxLayout();
    QString cloudHeaderText = tr( "Cloud placemark" );
    QString cloudDetailText = tr( "Path: %0 <br /> Name: %1" )
            .arg( m_mergeItem->pathB(), m_mergeItem->placemarkB().name() );
    QLabel *cloudHeaderLabel = new QLabel( cloudHeaderText );
    QLabel *cloudDetailLabel = new QLabel( cloudDetailText );
    rightLayout->addWidget( cloudHeaderLabel );
    rightLayout->addWidget( cloudDetailLabel );

    QHBoxLayout *detailLayout = new QHBoxLayout();
    detailLayout->addLayout( leftLayout );
    detailLayout->addLayout( rightLayout );

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout( detailLayout );
    mainLayout->addWidget( m_box );

    connect( m_box, SIGNAL(clicked(QAbstractButton*)),
             this, SLOT(resolveConflict(QPushButton*)) );
}

void ConflictDialog::resolveConflict( QPushButton *button )
{
    QDialogButtonBox::StandardButton standardButton = m_box->standardButton( button );
    switch(standardButton) {
    case QDialogButtonBox::Cancel:
        break;

    case QDialogButtonBox::NoButton:
       int actionRole = button->property( "ActionRole" ).toInt();
       switch( actionRole ) {
       case ConflictDialog::Local:
           m_mergeItem->setResolution( MergeItem::A );
           emit conflictResolved( m_mergeItem );
           break;
       case ConflictDialog::Cloud:
           m_mergeItem->setResolution( MergeItem::B );
           emit conflictResolved( m_mergeItem );
           break;
       case ConflictDialog::AllLocal:
           break;
       case ConflictDialog::AllCloud:
           break;
      default:
           break;
       }

    }
}

}

#include "ConflictDialog.moc"
