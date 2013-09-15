//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013   Utku Aydın <utkuaydin34@gmail.com>
//

#ifndef CONFLICTDIALOG_H
#define CONFLICTDIALOG_H

#include "MergeItem.h"
#include "marble_export.h"

#include <QDialog>
#include <QDialogButtonBox>

namespace Marble {

class MARBLE_EXPORT ConflictDialog : public QDialog
{
    Q_OBJECT

public:
    enum Button {
        Local = 1,
        Cloud,
        AllLocal,
        AllCloud
    };

    ConflictDialog( MergeItem *mergeItem, QWidget *parent = 0 );

signals:
    void conflictResolved( MergeItem *mergeItem );

private slots:
    void resolveConflict(QPushButton *button );

private:
    MergeItem *m_mergeItem;
    QDialogButtonBox *m_box;
};

}

#endif // CONFLICTDIALOG_H
