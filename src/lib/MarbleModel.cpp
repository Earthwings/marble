//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2007 Torsten Rahn <tackat@kde.org>"
// Copyright 2007      Inge Wallin  <ingwa@kde.org>"
//

#include "MarbleModel.h"

#include <cmath>

#include <QtCore/QDebug>
#include <QtCore/QTime>
#include <QtCore/QTimer>
#include <QtGui/QSortFilterProxyModel>

#include "global.h"
#include "ClipPainter.h"
#include "HttpDownloadManager.h"
#include "FileViewModel.h"
#include "GeoPolygon.h"
#include "KMLFileViewItem.h"
#include "ViewParams.h"
#include "TextureColorizer.h"
#include "TileLoader.h"
#include "TileCreator.h"
#include "MapTheme.h"
#include "MarbleDirs.h"
#include "TileCreatorDialog.h"
#include "PlaceMarkManager.h"
#include "PlaceMarkPainter.h"
#include "XmlHandler.h"
#include "gps/GpsLayer.h"


class MarbleModelPrivate
{
 public:
    // View and paint stuff
    MapTheme            *m_maptheme;
    QString              m_selectedMap;
    bool                 m_previousMapLoadedFine;
    TextureColorizer    *m_texcolorizer;

    HttpDownloadManager *m_downloadManager;

    TileLoader          *m_tileLoader;
    AbstractScanlineTextureMapper   *m_texmapper;

    VectorComposer      *m_veccomposer; // FIXME: Make not a pointer.
    GridMap             *m_gridmap;

    // Places on the map
    PlaceMarkManager      *m_placemarkmanager;
    MarblePlacemarkModel  *m_placemarkmodel;
    PlaceMarkLayout       *m_placeMarkLayout;

    // Selection handling
    QItemSelectionModel *m_placemarkselectionmodel;

    //Gps Stuff
    GpsLayer            *m_gpsLayer;
    GpxFileModel        *m_gpxFileModel;

    QTimer       *m_timer;
    Projection   m_projection;

    FileViewModel       *m_fileviewmodel;
};

MarbleModel::MarbleModel( QWidget *parent )
    : d( new MarbleModelPrivate )
{
    d->m_timer = new QTimer( this );
    d->m_timer->start( 200 );

    connect( d->m_timer, SIGNAL( timeout() ),
             this,       SIGNAL( timeout() ) );

    d->m_downloadManager = 0;
    d->m_tileLoader = new TileLoader( d->m_downloadManager );

    d->m_texmapper = 0;
    d->m_veccomposer = new VectorComposer();

    d->m_placeMarkLayout   = 0;

    d->m_maptheme = new MapTheme();
    d->m_previousMapLoadedFine = false;

    QStringList  mapthemedirs = MapTheme::findMapThemes( "maps/earth/" );
    QString      selectedmap;

    // We need at least one maptheme to run Marble.
    if ( mapthemedirs.count() == 0 ) {
        qDebug() << "Could not find any maps! Exiting ...";
        exit(-1);
    }

    // If any map directories were found, try to find the default map:
    // srtm.  If we can find that, just grab the first one.
    if ( mapthemedirs.count() >= 1 ) {
        QStringList  tmp = mapthemedirs.filter( "srtm.dgml" );
        if ( tmp.count() >= 1 )
            selectedmap = tmp[0];
        else
            selectedmap = mapthemedirs[0];
    }
    setMapTheme( selectedmap, parent, Spherical );

    d->m_gridmap      = new GridMap();
    d->m_texcolorizer = new TextureColorizer( MarbleDirs::path( "seacolors.leg" ),
                                              MarbleDirs::path( "landcolors.leg" ) );

    d->m_placemarkmanager   = new PlaceMarkManager();

    connect( d->m_placemarkmanager, SIGNAL( geoDataDocumentLoaded( GeoDataDocument& ) ),
             this,                  SLOT( geoDataDocumentLoaded( GeoDataDocument& ) ) );

    d->m_placemarkmodel = new MarblePlacemarkModel( d->m_placemarkmanager, this );
    d->m_placemarkselectionmodel = new QItemSelectionModel( d->m_placemarkmodel );

    connect( d->m_placemarkselectionmodel, SIGNAL( selectionChanged ( QItemSelection, QItemSelection) ),
             d->m_placeMarkLayout,         SLOT( requestStyleReset() ) ); 
    connect( d->m_placemarkmodel, SIGNAL( modelReset() ),
             d->m_placeMarkLayout,         SLOT( requestStyleReset() ) ); 

    d->m_placemarkmanager->loadStandardPlaceMarks();

    d->m_gpxFileModel = new GpxFileModel( this );
    d->m_gpsLayer = new GpsLayer( d->m_gpxFileModel );

    connect( d->m_gpxFileModel, SIGNAL( updateRegion( BoundingBox ) ),
             this,              SIGNAL( regionChanged( BoundingBox ) ) );

    d->m_projection = Spherical;

    /*
     * Create FileViewModel
     */
    d->m_fileviewmodel = new FileViewModel( this );

    connect( d->m_fileviewmodel, SIGNAL( updateRegion( BoundingBox ) ),
             this,               SIGNAL( regionChanged( BoundingBox ) ) );
}

MarbleModel::~MarbleModel()
{
    delete d->m_texmapper;

    delete d->m_tileLoader; // disconnects from downloadManager in dtor
    delete d->m_downloadManager;

    delete d->m_veccomposer;
    delete d->m_texcolorizer; 
    delete d->m_gridmap;
    delete d->m_placemarkmodel;
    delete d->m_placemarkmanager;
    delete d->m_gpsLayer;
    delete d->m_maptheme;
    delete d;
}

bool MarbleModel::showGps() const
{
    return d->m_gpsLayer->visible();
}

void MarbleModel::setShowGps( bool visible )
{
    d->m_gpsLayer->setVisible( visible );
}

void MarbleModel::startPolling( int time )
{
    d->m_timer->start( time );
}

void MarbleModel::stopPolling()
{
    d->m_timer->stop();
}

QString MarbleModel::mapTheme() const
{
    return d->m_selectedMap;
}

// Set a particular theme for the map and load the appropriate tile level.
// If the tiles (for the lowest tile level) haven't been created already
// then create them here and now.
//
// FIXME: Get rid of that awful 'parent' parameter and move the tile
//        creation dialogs out of this function.  Change them into signals 
//        instead.
// FIXME: Get rid of 'currentProjection' here.  It's totally misplaced.
//

void MarbleModel::setMapTheme( const QString &selectedMap, QWidget *parent,
			       Projection currentProjection )
{
    // Read the maptheme into d->m_maptheme.
    QString mapPath = QString("maps/earth/%1").arg( selectedMap );
    qDebug( "Setting map theme to : %s",
	    qPrintable( MarbleDirs::path( mapPath ) ) );

    int error = d->m_maptheme->open( MarbleDirs::path( mapPath ) );
    if ( error < 0 ){
        if ( d->m_previousMapLoadedFine == true )
            return;
        else { 
            // Actually this case can't really happen as the
            // existance of valid .dgml files gets checked before and
            // the directory string can't get invalid either.
            qDebug() << "Couldn't find a valid map.";
            exit(-1);
        } 
    }
    d->m_previousMapLoadedFine = true;

    // If this layer is a bitmaplayer, check if the cached tiles for
    // it are already generated, and if not, do so.
    if ( d->m_maptheme->bitmaplayer().enabled ) {
        // If the tiles aren't already there, put up a progress dialog
        // while creating them.
        if ( !TileLoader::baseTilesAvailable( "maps/earth/"
                                              + d->m_maptheme->tilePrefix() ) )
        {
            qDebug("Base tiles not available. Creating Tiles ... ");

            TileCreator *tileCreator = new TileCreator(
                                     d->m_maptheme->prefix(),
                                     d->m_maptheme->installMap(),
                                     d->m_maptheme->bitmaplayer().dem );
            TileCreatorDialog tileCreatorDlg( tileCreator, parent );
            tileCreatorDlg.setSummary( d->m_maptheme->name(),
                                       d->m_maptheme->description() );
            tileCreatorDlg.exec();
            qDebug("Tile creation completed");
        }

        if ( d->m_texmapper != 0 )
            delete d->m_texmapper;

        d->m_tileLoader->setMapTheme( "maps/earth/" + d->m_maptheme->tilePrefix() );

        switch( currentProjection ) {
            case Spherical:
                d->m_texmapper = new GlobeScanlineTextureMapper( d->m_tileLoader, this );
                break;
            case Equirectangular:
                d->m_texmapper = new FlatScanlineTextureMapper( d->m_tileLoader, this );
                break;
        }
//         else
//             d->m_texmapper->setMapTheme( "maps/earth/"
//                                          + d->m_maptheme->tilePrefix() );
        d->m_projection = currentProjection;

        connect( d->m_texmapper, SIGNAL( mapChanged() ),
		 this,           SLOT( notifyModelChanged() ) );
    }
    else {
        d->m_tileLoader->flush();
    }

    // Set all the colors for the vector layers
    d->m_veccomposer->setOceanColor( d->m_maptheme->oceanColor() );
    d->m_veccomposer->setLandColor( d->m_maptheme->landColor() );
    d->m_veccomposer->setCountryBorderColor( d->m_maptheme->countryBorderColor() );
    d->m_veccomposer->setStateBorderColor( d->m_maptheme->countryBorderColor() );
    d->m_veccomposer->setLakeColor( d->m_maptheme->lakeColor() );
    d->m_veccomposer->setRiverColor( d->m_maptheme->riverColor() );

    if ( d->m_placeMarkLayout == 0)
        d->m_placeMarkLayout = new PlaceMarkLayout( this );
    d->m_placeMarkLayout->requestStyleReset();

    // FIXME: To be removed after MapTheme / KML refactoring
    d->m_placeMarkLayout->placeMarkPainter()->setDefaultLabelColor( d->m_maptheme->labelColor() );

    d->m_selectedMap = selectedMap;
    d->m_projection = currentProjection;
    emit themeChanged( selectedMap );
    notifyModelChanged();
}


int MarbleModel::minimumZoom() const
{
    return d->m_maptheme->minimumZoom();
}

int MarbleModel::maximumZoom() const
{
    return d->m_maptheme->maximumZoom();
}

HttpDownloadManager* MarbleModel::downloadManager() const
{
    return d->m_downloadManager;
}


void MarbleModel::setDownloadManager( HttpDownloadManager *downloadManager )
{
    HttpDownloadManager *previousDownloadManager = d->m_downloadManager;

    d->m_tileLoader->setDownloadManager( downloadManager );
    d->m_downloadManager = downloadManager;

    if ( previousDownloadManager != 0 ) 
        delete previousDownloadManager;
}


void MarbleModel::resize( int width, int height )
{
    if ( d->m_maptheme->bitmaplayer().enabled == true ) {
        d->m_texmapper->resizeMap( width, height );
    }

    d->m_veccomposer->resizeMap( width, height );
    d->m_gridmap->resizeMap( width, height );
}


void MarbleModel::paintGlobe( ClipPainter* painter,
                              int width, int height,
                              ViewParams *viewParams,
                              bool redrawBackground,
                              const QRect& dirtyRect )
{
    resize( width, height );

    if ( redrawBackground ) {

        if ( d->m_maptheme->bitmaplayer().enabled == true ) {

            d->m_texmapper->mapTexture( viewParams );

            if ( !viewParams->m_showElevationModel
                && d->m_maptheme->bitmaplayer().dem == "true" )
            {
                viewParams->m_coastImage->fill( Qt::transparent );

                // Create VectorMap
                d->m_veccomposer->drawTextureMap( viewParams );

                // Recolorize the heightmap using the VectorMap
                d->m_texcolorizer->colorize( viewParams );
            }
        }
    }
    // Paint the map on the Widget
//    QTime t;
//    t.start();
    int radius = (int)(1.05 * (double)(viewParams->m_radius));

    if ( d->m_maptheme->bitmaplayer().enabled == true ) {
        if ( viewParams->m_projection == Spherical ) {
            QRect rect( width / 2 - radius , height / 2 - radius, 2 * radius, 2 * radius);
            rect = rect.intersect( dirtyRect );
            painter->drawImage( rect, *viewParams->m_canvasImage, rect );
        }
        else {
            painter->drawImage( dirtyRect, *viewParams->m_canvasImage, dirtyRect );
        }
    }

//    qDebug( "Painted in %ims", t.elapsed() );

    // Paint the vector layer.
    if ( d->m_maptheme->vectorlayer().enabled ) {

        if ( d->m_maptheme->bitmaplayer().enabled == false ) {
            d->m_veccomposer->paintBaseVectorMap( painter, viewParams );
        }
        // Add further Vectors
        d->m_veccomposer->paintVectorMap( painter, viewParams );
    }

    // Paint the grid around the earth.
    if ( viewParams->m_showGrid ) {
        QPen  gridpen( QColor( 231, 231, 231, 255 ) );

        // Create and paint a grid
        d->m_gridmap->createGrid( viewParams );
        d->m_gridmap->setPen( gridpen );
        d->m_gridmap->paintGridMap( painter, false );

        // Create and paint the tropics and polar circles
        d->m_gridmap->createTropics( viewParams );
        gridpen.setStyle( Qt::DotLine );
//        gridpen.setWidthF( 1.5f );
        d->m_gridmap->setPen( gridpen );
        d->m_gridmap->paintGridMap( painter, false );

        // Create Equator
        d->m_gridmap->createEquator( viewParams );
//        gridpen.setWidthF( 2.0f );
        d->m_gridmap->setPen( gridpen );
        d->m_gridmap->paintGridMap( painter, false );
    }

    // Paint the GeoDataPlaceMark layer
#ifndef KML_GSOC
    if ( viewParams->m_showPlaceMarks && ( viewParams->m_showCities || 
         viewParams->m_showTerrain || viewParams->m_showOtherPlaces )
         && d->m_placemarkmodel->rowCount() > 0 ) {
        d->m_placeMarkLayout->paintPlaceFolder( painter, viewParams,
                                                 d->m_placemarkmodel,
                                                 d->m_placemarkselectionmodel );
    }
#else
    /*
     * Iterate over our KMLFolders and show placemarks from each folderList
     * This folder list also will be displayed in MarbleControlBox
     * So user could enable/disable each folder
     */
    if ( viewParams->m_showPlaceMarks ) {
        QTime t;
        t.start ();

        const QList < GeoDataFolder* >& folderList = d->m_placemarkmanager->getFolderList();

        bool firstTime = true;

        for ( QList<GeoDataFolder*>::const_iterator iterator = folderList.constBegin();
            iterator != folderList.constEnd();
            ++iterator )
        {
            GeoDataFolder& folder = *( *iterator );

            /*
             * Show only placemarks which are visible
             */
            if ( folder.isVisible() ) {

                PlaceMarkContainer& container = folder.activePlaceMarkContainer( *viewParams );

                d->m_placeMarkLayout->paintPlaceFolder( painter, viewParams,
                                                        &container, firstTime );

                firstTime = false;
            }
        }

        qDebug("Paint kml folder list. Elapsed: %d", t.elapsed());
    }
#endif

    // Paint the Gps Layer
    d->m_gpsLayer->setVisible( viewParams->m_showGps );
    //FIXME:We might just send ViewParams instead of this bunch of parameters
    d->m_gpsLayer->paintLayer( painter,
                               viewParams->m_canvasImage->size(),
                               viewParams,
                               viewParams->m_boundingBox );
}


QAbstractItemModel *MarbleModel::placeMarkModel() const
{
    return d->m_placemarkmodel;
}

QItemSelectionModel *MarbleModel::placeMarkSelectionModel() const
{
    return d->m_placemarkselectionmodel;
}

VectorComposer    *MarbleModel::vectorComposer()   const
{
    return d->m_veccomposer;
}

TextureColorizer  *MarbleModel::textureColorizer() const
{
    return d->m_texcolorizer;
}

AbstractScanlineTextureMapper    *MarbleModel::textureMapper()    const
{
    return d->m_texmapper;
}

PlaceMarkLayout *MarbleModel::placeMarkLayout() const
{
    return d->m_placeMarkLayout;
}

GpsLayer *MarbleModel::gpsLayer()         const
{
    return d->m_gpsLayer;
}

GpxFileModel *MarbleModel::gpxFileModel()   const
{
    return d->m_gpxFileModel;
}

FileViewModel *MarbleModel::fileViewModel() const
{
    return d->m_fileviewmodel;
}

void MarbleModel::addPlaceMarkFile( const QString& filename )
{
    d->m_placemarkmanager->loadKml( filename, true );

    notifyModelChanged();
}

QVector<QPersistentModelIndex> MarbleModel::whichFeatureAt( const QPoint& curpos ) const
{
    return d->m_placeMarkLayout->whichPlaceMarkAt( curpos );
}

void MarbleModel::notifyModelChanged()
{
    emit modelChanged();
}

void MarbleModel::geoDataDocumentLoaded( GeoDataDocument& document )
{
    AbstractFileViewItem* item = new KMLFileViewItem( *d->m_placemarkmanager,
                                                      document );

    d->m_fileviewmodel->append( item );
}


#include "MarbleModel.moc"
