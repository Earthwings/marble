#include "RouteItemDelegate.h"
#include <QApplication>
#include <QAbstractTextDocumentLayout>
#include <QPainter>

namespace Marble {

RouteItemDelegate::RouteItemDelegate( QListView *view, CloudRouteModel *model ) :
    m_view( view ),
    m_model( model ),
    m_buttonWidth( 0 ),
    m_iconSize( 16 )
{
}

RouteItemDelegate::~RouteItemDelegate()
{
}

void RouteItemDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    
    QStyleOptionViewItemV4 styleOption = option;
    styleOption.text = QString();
    QApplication::style()->drawControl( QStyle::CE_ItemViewItem, &styleOption, painter );
    
    QAbstractTextDocumentLayout::PaintContext paintContext;
    if ( styleOption.state & QStyle::State_Selected)  {
        paintContext.palette.setColor( QPalette::Text, styleOption.palette.color( QPalette::Active, QPalette::HighlightedText ) );
    }
    
    QTextDocument document;
    QRect const textRect = position( Text, option );
    document.setTextWidth( textRect.width() );
    document.setDefaultFont( option.font );
    document.setHtml( text( index ) );

    painter->save();
    painter->translate( textRect.topLeft() );
    painter->setClipRect( 0, 0, textRect.width(), textRect.height() );
    document.documentLayout()->draw( painter, paintContext );
    painter->restore();
    
    bool cached = index.data( CloudRouteModel::IsCached ).toBool();
    bool downloading = index.data( CloudRouteModel::IsDownloading ).toBool();
    
    if ( downloading ) {
        qint64 total = qVariantValue<qint64>( index.data( CloudRouteModel::TotalSize ) );
        qint64 progress = qVariantValue<qint64>( index.data( CloudRouteModel::DownloadedSize ) );
        
        QStyleOptionProgressBar progressBarOption;
        progressBarOption.rect = position( Progressbar, option );
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.progress = ( 100.0 * progress / total );
        progressBarOption.text = QString::number( progressBarOption.progress ) + "%";
        progressBarOption.textVisible = true;
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
    } else if ( cached ) {
        QStyleOptionButton openButton = button( OpenButton, option );
        QRect openRect = position( OpenButton, option );
        openButton.rect = openRect;
        QApplication::style()->drawControl( QStyle::CE_PushButton, &openButton, painter );

        QStyleOptionButton cacheRemoveButton = button( RemoveFromCacheButton, option );
        QRect cacheRemoveRect = position( RemoveFromCacheButton, option );
        cacheRemoveButton.rect = cacheRemoveRect;
        QApplication::style()->drawControl( QStyle::CE_PushButton, &cacheRemoveButton, painter );
    } else {
        QStyleOptionButton downloadButton = button( DownloadButton, option );
        QRect downloadRect = position( DownloadButton, option );
        downloadButton.rect = downloadRect;
        QApplication::style()->drawControl( QStyle::CE_PushButton, &downloadButton, painter );

        QStyleOptionButton cloudRemoveButton = button( RemoveFromCloudButton, option );
        QRect cloudRemoveRect = position( RemoveFromCloudButton, option );
        cloudRemoveButton.rect = cloudRemoveRect;
        QApplication::style()->drawControl( QStyle::CE_PushButton, &cloudRemoveButton, painter );
    }
}

QSize RouteItemDelegate::sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    if ( index.column() == 0 ) {
        QTextDocument doc;
        doc.setDefaultFont( option.font );
        doc.setTextWidth( qMax( 200, m_view->contentsRect().width() - buttonWidth( option ) ) );
        doc.setHtml( text( index ) );
        return QSize( 0, qMax( 55, qRound( doc.size().height() ) ) );
    }

    return QSize();
}

bool RouteItemDelegate::editorEvent( QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index )
{
    Q_UNUSED( model );

    if ( ( event->type() == QEvent::MouseButtonRelease ) ) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>( event );
        QPoint pos = mouseEvent->pos();
        
        bool cached = index.data( CloudRouteModel::IsCached ).toBool();

        if ( cached ) {
            QRect openRect = position( OpenButton, option );
            QRect cacheRemoveRect = position( RemoveFromCacheButton, option );
            
            if ( openRect.contains( pos ) ) {
                QString timestamp = index.data( CloudRouteModel::Timestamp ).toString();
                emit openButtonClicked( timestamp );
                return true;
            } else if ( cacheRemoveRect.contains( pos ) ) {
                m_model->removeFromCache( index );
                return true;
            }
        } else {
            QRect downloadRect = position( DownloadButton, option );
            QRect cloudRemoveRect = position( RemoveFromCloudButton, option );
            
            if ( downloadRect.contains( pos ) ) {
                QString timestamp = index.data( CloudRouteModel::Timestamp ).toString();
                m_model->setCurrentlyDownloading( index );
                emit downloadButtonClicked( timestamp );
                return true;
            }
            
            if ( cloudRemoveRect.contains( pos ) ) {
                QString timestamp = index.data( CloudRouteModel::Timestamp ).toString();
                emit deleteButtonClicked( timestamp );
                return true;
            }
        }
    }
    
    return false;
}

int RouteItemDelegate::buttonWidth( const QStyleOptionViewItem &option ) const
{
    if ( m_buttonWidth <= 0 ) {
        int const openWidth = option.fontMetrics.size( 0, tr( "Open" ) ).width();
        int const downloadWidth = option.fontMetrics.size( 0, tr( "Load" ) ).width();
        int const cacheWidth = option.fontMetrics.size( 0, tr( "Remove from device" ) ).width();
        int const cloudWidth = option.fontMetrics.size( 0, tr( "Delete from cloud" ) ).width();
        m_buttonWidth = 2 * m_iconSize + qMax( qMax( openWidth, downloadWidth ), qMax( cacheWidth, cloudWidth ) );
    }

    return m_buttonWidth;
}

QStyleOptionButton RouteItemDelegate::button( Element element, const QStyleOptionViewItem &option ) const
{
    QStyleOptionButton result;
    result.state = option.state;
    result.state &= ~QStyle::State_HasFocus;

    result.palette = option.palette;
    result.features = QStyleOptionButton::None;

    switch ( element ) {
    case OpenButton:
        result.text = tr( "Open" );
        result.icon = QIcon( ":/marble/document-open.png" );
        result.iconSize = QSize( m_iconSize, m_iconSize );
        break;
    case DownloadButton:
        result.text = tr( "Load" );
        result.icon = QIcon( ":/marble/dialog-ok.png" );
        result.iconSize = QSize( m_iconSize, m_iconSize );
        break;
    case RemoveFromCacheButton:
        result.text = tr( "Remove from device" );
        result.icon = QIcon( ":/marble/edit-clear.png" );
        result.iconSize = QSize( m_iconSize, m_iconSize );
        break;
    case RemoveFromCloudButton:
        result.text = tr( "Delete from cloud" );
        result.icon = QIcon( ":/marble/edit-delete.png" );
        result.iconSize = QSize( m_iconSize, m_iconSize );
        break;
    default:
        // Ignored.
        break;
    }

    return result;
}

QString RouteItemDelegate::text( const QModelIndex& index ) const
{
    return QString( "%0" ).arg( index.data( CloudRouteModel::Name ).toString() );
    // TODO: Show distance and duration
    //return QString( "%0<br /><b>Duration:</b> %1<br/><b>Distance:</b> %2" )
            //.arg( index.data( CloudRouteModel::Name ).toString() )
            //.arg( index.data( CloudRouteModel::Duration ).toString() )
            //.arg( index.data( CloudRouteModel::Distance ).toString() );
}

QRect RouteItemDelegate::position( Element element, const QStyleOptionViewItem& option ) const
{   
    int const width = buttonWidth( option );
    QPoint const firstColumn = option.rect.topLeft();
    QPoint const secondColumn = firstColumn + QPoint( option.decorationSize.width(), 0 );
    QPoint const thirdColumn = secondColumn + QPoint( option.rect.width() - width - option.decorationSize.width(), 0 );

    switch (element) {
    case Text:
        return QRect( secondColumn, QSize( thirdColumn.x() - secondColumn.x(), option.rect.height() ) );
    case OpenButton:
    case DownloadButton:
    {
        QStyleOptionButton optionButton = button( element, option );
        QSize size = option.fontMetrics.size( 0, optionButton.text ) + QSize( 4, 4 );
        QSize buttonSize = QApplication::style()->sizeFromContents( QStyle::CT_PushButton, &optionButton, size );
        buttonSize.setWidth( width );
        return QRect( thirdColumn, buttonSize );
    }
    case RemoveFromCacheButton:
    case RemoveFromCloudButton:
    {
        QStyleOptionButton optionButton = button( element, option );
        QSize size = option.fontMetrics.size( 0, optionButton.text ) + QSize( 4, 4 );
        QSize buttonSize = QApplication::style()->sizeFromContents( QStyle::CT_PushButton, &optionButton, size );
        buttonSize.setWidth( width );
        return QRect( thirdColumn + QPoint( 0, option.fontMetrics.height() + 10 ), buttonSize );
    }
    case Progressbar:
    {
        QSize const progressSize = QSize( width, option.fontMetrics.height() + 4 );
        return QRect( thirdColumn + QPoint( 0, 10 ), progressSize );
    }
    }
    
    return QRect();
}

}

#include "RouteItemDelegate.moc"
