//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2007 Torsten Rahn <tackat@kde.org>
// Copyright 2007      Inge Wallin  <ingwa@kde.org>
//

//
// MarbleThemeSelectView lets the user choose a map theme
//

#ifndef MARBLE_MARBLETHEMESELECTVIEW_H
#define MARBLE_MARBLETHEMESELECTVIEW_H


#include <QtCore/QModelIndex>
#include <QtCore/QSettings>
#include <QtGui/QListView>

#include "marble_export.h"

class QResizeEvent;
class QString;

namespace Marble
{

class MARBLE_EXPORT MarbleThemeSelectView : public QListView
{
    Q_OBJECT

 public:
    explicit MarbleThemeSelectView( QWidget *parent = 0 );
    ~MarbleThemeSelectView();
    // void setModel( QAbstractItemModel * model );
    
 protected:
    void resizeEvent( QResizeEvent *event );

 private Q_SLOTS:
    void selectedMapTheme( QModelIndex index );
    void uploadDialog();
    void mapWizard();
    void showContextMenu( const QPoint& pos );
    void deleteMap();
    void toggleFavorite();

 Q_SIGNALS:
    void selectMapTheme( const QString& );
    void showMapWizard();
    void showUploadDialog();
    //void 

 private:
    Q_DISABLE_COPY( MarbleThemeSelectView )
    void loadFavorites();
    bool currentIsFavorite();
    QSettings m_settings;
    class Private;
    Private  *const d;
};

}

#endif
