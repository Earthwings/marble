//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008 Patrick Spendrin      <ps_ml@gmx.de>
// Copyright 2008 Simon Schmeisser      <mail_to_wrt@gmx.de>
//

//
// This class is the placemark layer plugin.
//

#ifndef MARBLEDATAVIEWPLUGIN_H
#define MARBLEDATAVIEWPLUGIN_H

#include <QtCore/QObject>
#include <QtGui/QBrush>
#include <QtGui/QPen>

#include "MarbleRenderPlugin.h"

class QTreeView;

class GeoDataGeometry;
class GeoDataFeature;
class GeoDataDocument;



namespace Marble
{

class MarbleGeoDataDebugModel;

/**
 * @short The class that specifies the Marble layer interface of the plugin.
 *
 * MarbleDataViewPlugin is the beginning of a plugin, that displays the geodata as it is stored internally
 */

class MarbleDataViewPlugin : public MarbleRenderPlugin
{
    Q_OBJECT
    Q_INTERFACES( Marble::MarbleRenderPluginInterface )
    MARBLE_PLUGIN(MarbleDataViewPlugin)

    public:

    MarbleDataViewPlugin();
    ~MarbleDataViewPlugin();

    QStringList backendTypes() const;

    QString renderPolicy() const;

    QStringList renderPosition() const;

    QString name() const;

    QString guiString() const;

    QString nameId() const;

    QString description() const;

    QIcon icon () const;


    void initialize ();

    bool isInitialized () const;


    bool render( GeoPainter *painter, ViewportParams *viewport, const QString& renderPos, GeoSceneLayer * layer = 0 );
    
    private:
    QTreeView *m_dataView;
    MarbleGeoDataDebugModel *m_debugModel;
};

}
#endif
