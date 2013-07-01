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
        explicit CloudRoutesDialog( QString json, Marble::MarbleWidget* marbleWidget );
        
    signals:
        void downloadButtonClicked( QString timestamp );
        void openButtonClicked( QString timestamp );
        
    private:
        class Private;
        Private *d;
};

class RouteItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
    
    public:
        explicit RouteItemDelegate( QListView *view, CloudRouteModel *model, MarbleWidget *marbleWidget );
        void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
        QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const;
        bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index);
        
    signals:
        void downloadButtonClicked( QString timestamp );
        void openButtonClicked( QString timestamp );
        
    private:
        enum Element {
            Text,
            OpenButton,
            DownloadButton,
            RemoveFromCacheButton,
            RemoveFromCloudButton
        };
        
        int buttonWidth( const QStyleOptionViewItem &option ) const;
        QStyleOptionButton button( Element element, const QStyleOptionViewItem &option ) const;
        QString text( const QModelIndex &index ) const;
        QRect position( Element element, const QStyleOptionViewItem &option ) const;
        
        QListView *m_view;
        CloudRouteModel *m_model;
        mutable int m_buttonWidth;
        int const m_iconSize;
};

}
#endif // CLOUDROUTESDIALOG_H
