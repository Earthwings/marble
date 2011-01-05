//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010 Utku Aydin <utkuaydin34@gmail.com>
//

#include "EarthquakePlugin.h"
#include "EarthquakeModel.h"
#include "ui_EarthquakeConfigWidget.h"

#include <QtGui/QPushButton>
#include <QtGui/QSlider>

namespace Marble {

EarthquakePlugin::EarthquakePlugin()
    : m_isInitialized( false ),
      m_configDialog( 0 )
{
    setNameId( "earthquake" );
    setEnabled( true ); // Plugin is enabled by default
    setVisible( false ); // Plugin is invisible by default
    connect( this, SIGNAL( settingsChanged( QString ) ),
             this, SLOT( updateSettings() ) );
}

void EarthquakePlugin::initialize()
{
    EarthquakeModel *m = new EarthquakeModel( pluginManager(), this );
    m->setNumResults( ui_configWidget->m_numResults->value() );
    m->setMinMagnitude( ui_configWidget->m_minMagnitude->value() );
    m->setEndDate( ui_configWidget->m_endDate->dateTime() );
    m->setStartDate( ui_configWidget->m_startDate->dateTime() );
    setModel( m );
    setNumberOfItems( numberOfItemsOnScreen );
    m_isInitialized = true;
}

bool EarthquakePlugin::isInitialized() const
{
    return m_isInitialized;
}

QString EarthquakePlugin::name() const
{
    return tr( "Earthquakes" );
}

QString EarthquakePlugin::guiString() const
{
    return tr( "&Earthquakes" );
}

QString EarthquakePlugin::description() const
{
    return tr( "Shows earthquakes on the map." );
}

QIcon EarthquakePlugin::icon() const
{
    return QIcon();
}

QDialog *EarthquakePlugin::configDialog() const
{
    if ( !m_configDialog ) {
        // Initializing configuration dialog
        m_configDialog = new QDialog();
        ui_configWidget = new Ui::EarthquakeConfigWidget;
        ui_configWidget->setupUi( m_configDialog );
        ui_configWidget->m_numResults->setRange( 1, numberOfItemsOnScreen );
        readSettings();
        connect( ui_configWidget->m_buttonBox, SIGNAL( accepted() ),
                 SLOT( writeSettings() ) );
        connect( ui_configWidget->m_buttonBox, SIGNAL( rejected() ),
                 SLOT( readSettings() ) );
        QPushButton *applyButton = ui_configWidget->m_buttonBox->button( QDialogButtonBox::Apply );
        connect( applyButton, SIGNAL( clicked() ),
                 SLOT( writeSettings() ) );
        connect( ui_configWidget->m_endDate, SIGNAL( dateTimeChanged ( const QDateTime& ) ),
                 SLOT( validateDateRange() ) );
    }
    return m_configDialog;
}

QHash<QString,QVariant> EarthquakePlugin::settings() const
{
    return m_settings;
}

void EarthquakePlugin::setSettings( QHash<QString,QVariant> settings )
{
    if ( !settings.contains( "numResults" ) ) {
        settings.insert( "numResults", numberOfItemsOnScreen );
    }
    if ( !settings.contains( "minMagnitude" ) ) {
        settings.insert( "minMagnitude", 0.0 );
    }
    if ( !settings.contains( "startDate" ) ) {
        settings.insert( "startDate", QDateTime::fromString( "2006-02-04", "yyyy-MM-dd" ) );
    }
    if ( !settings.contains( "endDate" ) ) {
        settings.insert( "endDate", QDateTime::currentDateTime() );
    }

    m_settings = settings;
    readSettings();
    emit settingsChanged( nameId() );
}

void EarthquakePlugin::readSettings() const
{
    if ( !m_configDialog )
        return;

    ui_configWidget->m_numResults->setValue( m_settings.value( "numResults" ).toInt() );
    ui_configWidget->m_minMagnitude->setValue( m_settings.value( "minMagnitude" ).toDouble() );
    ui_configWidget->m_startDate->setDateTime( m_settings.value( "startDate" ).toDateTime() );
    ui_configWidget->m_endDate->setDateTime( m_settings.value( "endDate" ).toDateTime() );
    ui_configWidget->m_startDate->setMaximumDateTime( ui_configWidget->m_endDate->dateTime() );
}

void EarthquakePlugin::writeSettings()
{
    m_settings.insert( "numResults", ui_configWidget->m_numResults->value() );
    m_settings.insert( "minMagnitude", ui_configWidget->m_minMagnitude->value() );
    m_settings.insert( "startDate", ui_configWidget->m_startDate->dateTime() );
    m_settings.insert( "endDate", ui_configWidget->m_endDate->dateTime() );

    emit settingsChanged( nameId() );
}

void EarthquakePlugin::updateSettings()
{
    EarthquakeModel *m = dynamic_cast<EarthquakeModel *>( model() );

    if( m != NULL )
    {
        m = new EarthquakeModel( pluginManager(), this );
        m->setNumResults( ui_configWidget->m_numResults->value() );
        m->setMinMagnitude( ui_configWidget->m_minMagnitude->value() );
        m->setEndDate( ui_configWidget->m_endDate->dateTime() );
        m->setStartDate( ui_configWidget->m_startDate->dateTime() );
        setModel( m );
    }
}

void EarthquakePlugin::validateDateRange()
{
    if( ui_configWidget->m_startDate->dateTime() > 
        ui_configWidget->m_endDate->dateTime() )
    {
        ui_configWidget->m_startDate->setDateTime( ui_configWidget->m_endDate->dateTime() );
    }
    ui_configWidget->m_startDate->setMaximumDateTime( ui_configWidget->m_endDate->dateTime() );
}

}

Q_EXPORT_PLUGIN2( EarthquakePlugin, Marble::EarthquakePlugin )

#include "EarthquakePlugin.moc"
