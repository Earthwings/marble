#include "MapThemeSortFilterProxyModel.h"
#include <QtCore/QString>

using namespace Marble;

MapThemeSortFilterProxyModel::MapThemeSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool MapThemeSortFilterProxyModel::lessThan ( const QModelIndex & left, const QModelIndex & right ) const
{
    QString leftData = sourceModel()->data( left ).toString();
    QString rightData = sourceModel()->data( right ).toString();
    if ( leftData == tr("Atlas") ||
         leftData == tr("Satellite View") ||
         leftData == tr("OpenStreetMap") )
        return true;
    else if ( rightData == tr("Atlas") ||
         rightData == tr("Satellite View") ||
         rightData == tr("OpenStreetMap") )
        return false;
    else
        return QSortFilterProxyModel::lessThan( left, right);
}

#include "MapThemeSortFilterProxyModel.moc"
