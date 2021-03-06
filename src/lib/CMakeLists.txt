PROJECT(marblewidget)

if (QTONLY)
   cmake_minimum_required(VERSION 2.4.0)
endif (QTONLY)


INCLUDE_DIRECTORIES(
 ${CMAKE_CURRENT_SOURCE_DIR}
 ${CMAKE_CURRENT_BINARY_DIR}
 ${CMAKE_CURRENT_SOURCE_DIR}/projections
 ${CMAKE_CURRENT_SOURCE_DIR}/routing
 ${CMAKE_CURRENT_SOURCE_DIR}/geodata
 ${CMAKE_CURRENT_SOURCE_DIR}/geodata/data
 ${CMAKE_CURRENT_SOURCE_DIR}/geodata/graphicsitem
 ${CMAKE_CURRENT_SOURCE_DIR}/geodata/scene
 ${CMAKE_CURRENT_SOURCE_DIR}/geodata/handlers
 ${CMAKE_CURRENT_SOURCE_DIR}/geodata/handlers/kml
 ${CMAKE_CURRENT_SOURCE_DIR}/geodata/handlers/dgml
 ${CMAKE_CURRENT_SOURCE_DIR}/geodata/parser
 ${CMAKE_CURRENT_SOURCE_DIR}/graphicsview
 ${CMAKE_CURRENT_SOURCE_DIR}/layers
 ${CMAKE_CURRENT_SOURCE_DIR}/cloudsync
 ${QT_INCLUDE_DIR}
)

if( QT4_FOUND )
  INCLUDE(${QT_USE_FILE})
else()
INCLUDE_DIRECTORIES(
 ${Qt5WebKitWidgets_INCLUDE_DIRS}
 ${Qt5Xml_INCLUDE_DIRS}
 ${Qt5Concurrent_INCLUDE_DIRS}
)
endif()

INCLUDE(geodata/CMakeLists.txt)
INCLUDE(graphicsview/CMakeLists.txt)
INCLUDE(layers/CMakeLists.txt)

set(GENERIC_LIB_VERSION "0.16.80")
set(GENERIC_LIB_SOVERSION "17")

if (QTONLY)
  # ce: don't know why this is needed here - on win32 'O2' is activated by default in release mode
  if(NOT WIN32)
    add_definitions (${QT_DEFINITIONS} -O3)
  endif(NOT WIN32)

  include_directories(
    ${QT_INCLUDE_DIR}
  )
else (QTONLY)
  if(NOT WIN32)
    add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -O3)
  endif(NOT WIN32)

  include_directories(
    ${KDE4_INCLUDES}
  )
endif (QTONLY)

# link_directories (${QT_LIBRARY_DIR})
########### next target ###############

set(marblewidget_SRCS
    ${geodata_SRCS}
    ${graphicsview_SRCS}
    ${layers_SRCS}
    blendings/Blending.cpp
    blendings/BlendingAlgorithms.cpp
    blendings/BlendingFactory.cpp
    blendings/SunLightBlending.cpp
    DownloadRegion.cpp
    DownloadRegionDialog.cpp
    LatLonBoxWidget.cpp
    MarbleWidget.cpp
    MarbleWebView.cpp
    MarbleModel.cpp
    MarbleMap.cpp
    MarbleControlBox.cpp
    NavigationWidget.cpp
    MapViewWidget.cpp
    FileViewWidget.cpp
    LegendWidget.cpp
    CurrentLocationWidget.cpp
    MarbleNavigator.cpp
    MarbleLegendBrowser.cpp
    MarbleAboutDialog.cpp
    MarbleWidgetInputHandler.cpp
    MarbleWidgetPopupMenu.cpp
    MarblePlacemarkModel.cpp
    GeoDataTreeModel.cpp
    kdescendantsproxymodel.cpp
    BranchFilterProxyModel.cpp
    MarbleDebug.cpp
    Tile.cpp
    TextureTile.cpp
    TileCoordsPyramid.cpp
    TileLevelRangeWidget.cpp
    TileLoader.cpp
    QtMarbleConfigDialog.cpp
    ClipPainter.cpp
    DownloadPolicy.cpp
    DownloadQueueSet.cpp
    GeoPainter.cpp
    GeoPolygon.cpp
    HttpDownloadManager.cpp
    HttpJob.cpp
    LayerManager.cpp
    PluginManager.cpp
    TimeControlWidget.cpp
    AbstractFloatItem.cpp
    PopupItem.cpp
    MarbleGlobal.cpp
    MarbleDirs.cpp
    MarbleLocale.cpp
    MarblePhysics.cpp
    DeferredFlag.cpp
    TileCreatorDialog.cpp
    MapThemeManager.cpp
    ViewportParams.cpp
    ViewParams.cpp
    projections/AbstractProjection.cpp
    projections/CylindricalProjection.cpp
    projections/SphericalProjection.cpp
    projections/EquirectProjection.cpp
    projections/MercatorProjection.cpp
    VisiblePlacemark.cpp
    PlacemarkLayout.cpp
    Planet.cpp
    Quaternion.cpp
    TextureColorizer.cpp
    TextureMapperInterface.cpp
    ScanlineTextureMapperContext.cpp
    SphericalScanlineTextureMapper.cpp
    EquirectScanlineTextureMapper.cpp
    MercatorScanlineTextureMapper.cpp
    TileScalingTextureMapper.cpp
    VectorTileModel.cpp
    DiscCache.cpp
    ServerLayout.cpp
    StoragePolicy.cpp
    CacheStoragePolicy.cpp
    FileStoragePolicy.cpp
    FileStorageWatcher.cpp
    StackedTile.cpp
    TileId.cpp
    StackedTileLoader.cpp
    TileLoaderHelper.cpp
    TileCreator.cpp
    TinyWebBrowser.cpp
    #jsonparser.cpp
    VectorComposer.cpp
    VectorMap.cpp
    FileLoader.cpp
    FileManager.cpp
    PositionTracking.cpp
    DataMigration.cpp
    ImageF.cpp

    AbstractDataPlugin.cpp
    AbstractDataPluginModel.cpp
    AbstractDataPluginItem.cpp
    AbstractWorkerThread.cpp

    PluginInterface.cpp
    DialogConfigurationInterface.cpp
    LayerInterface.cpp
    RenderPlugin.cpp
    RenderPluginInterface.cpp
    PositionProviderPlugin.cpp
    PositionProviderPluginInterface.cpp
    MarblePluginSettingsWidget.cpp
    RenderPluginModel.cpp
    PluginAboutDialog.cpp
    PluginItemDelegate.cpp

    SunLocator.cpp
    MarbleClock.cpp
    SunControlWidget.cpp
    MergedLayerDecorator.cpp

    MathHelper.cpp

    LatLonEdit.cpp

    MapThemeSortFilterProxyModel.cpp

    TemplateDocument.cpp

    routing/AlternativeRoutesModel.cpp
    routing/Maneuver.cpp
    routing/Route.cpp
    routing/RouteRequest.cpp
    routing/RouteSegment.cpp
    routing/RoutingModel.cpp
    routing/RoutingProfile.cpp
    routing/RoutingManager.cpp
    routing/RoutingLayer.cpp
    routing/RoutingInputWidget.cpp
    routing/RoutingWidget.cpp
    routing/RoutingProfilesWidget.cpp
    routing/RoutingProfilesModel.cpp
    routing/RoutingProfileSettingsDialog.cpp
    routing/SpeakersModel.cpp
    routing/VoiceNavigationModel.cpp
    routing/instructions/InstructionTransformation.cpp
    routing/instructions/RoutingInstruction.cpp
    routing/instructions/RoutingPoint.cpp
    routing/instructions/RoutingWaypoint.cpp
    routing/instructions/WaypointParser.cpp

    ParsingRunnerManager.cpp
    ReverseGeocodingRunnerManager.cpp
    RoutingRunnerManager.cpp
    SearchRunnerManager.cpp

    AutoNavigation.cpp

    SearchRunnerPlugin.cpp
    ReverseGeocodingRunnerPlugin.cpp
    RoutingRunnerPlugin.cpp
    ParseRunnerPlugin.cpp
    SearchRunner.cpp
    ReverseGeocodingRunner.cpp
    RoutingRunner.cpp
    ParsingRunner.cpp
    RunnerTask.cpp

    BookmarkManager.cpp
    EditBookmarkDialog.cpp
    BookmarkManagerDialog.cpp
    NewBookmarkFolderDialog.cpp

    PrintOptionsWidget.cpp
    ExternalEditorDialog.cpp
    GoToDialog.cpp

    MapWizard.cpp
    MapThemeDownloadDialog.cpp
    GeoGraphicsScene.cpp
    ElevationModel.cpp
    MarbleLineEdit.cpp
    SearchInputWidget.cpp
    SearchWidget.cpp
    kineticmodel.cpp
    NewstuffModel.cpp
    
    cloudsync/CloudSyncManager.cpp
    cloudsync/RouteSyncManager.cpp
    cloudsync/AbstractSyncBackend.cpp
    cloudsync/OwncloudSyncBackend.cpp
    cloudsync/CloudRouteModel.cpp
    cloudsync/CloudRoutesDialog.cpp
    cloudsync/RouteItem.cpp
    cloudsync/RouteItemDelegate.cpp
    cloudsync/BookmarkSyncManager.cpp
    cloudsync/MergeItem.cpp
    cloudsync/ConflictDialog.cpp
)

set (marblewidget_UI
    LatLonBoxWidget.ui
    NavigationWidget.ui
    LegendWidget.ui
    MapViewWidget.ui
    CurrentLocationWidget.ui
    FileViewWidget.ui
    MarbleNavigator.ui
    MarbleCacheSettingsWidget.ui
    MarbleViewSettingsWidget.ui
    MarbleNavigationSettingsWidget.ui
    TimeControlWidget.ui
    MarbleTimeSettingsWidget.ui
    MarblePluginSettingsWidget.ui
    TileCreatorDialog.ui
    TileLevelRangeWidget.ui
    MarbleAboutDialog.ui
    SunControlWidget.ui
    LatLonEdit.ui
    routing/RoutingWidget.ui
    DataMigrationWidget.ui
    EditBookmarkDialog.ui
    BookmarkManagerDialog.ui
    NewBookmarkFolderDialog.ui
    PrintOptions.ui
    ExternalEditor.ui
    GoToDialog.ui
    routing/RoutingSettingsWidget.ui
    routing/RoutingProfileSettingsDialog.ui
    MapWizard.ui
    MapThemeDownloadDialog.ui
    WebPopupWidget.ui
    cloudsync/CloudRoutesDialog.ui
    MarbleCloudSyncSettingsWidget.ui
)

# FIXME: cleaner approach of src/lib/MarbleControlBox.* vs. marble.qrc
qt_add_resources(marblewidget_SRCS libmarble.qrc ../marble.qrc)

qt_wrap_ui(marblewidget_SRCS ${marblewidget_UI})
marble_qt4_automoc(${marblewidget_SRCS})


# link with release version of Qt libs
ADD_LIBRARY(marblewidget SHARED ${marblewidget_SRCS} ${SOURCES_UI_HDRS})
if( QT4_FOUND )
  TARGET_LINK_LIBRARIES (marblewidget 
    ${QT_QTCORE_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${QT_QTSVG_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTSCRIPT_LIBRARY}
    ${QT_QTMAIN_LIBRARY}
    ${QT_QTWEBKIT_LIBRARY}
    ${QT_QTDECLARATIVE_LIBRARY}
  )
else()
  TARGET_LINK_LIBRARIES (marblewidget
    ${Qt5Core_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5WebKitWidgets_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5Script_LIBRARIES}
    )
endif()

if (APPLE)
  #defined in top level makefile
  TARGET_LINK_LIBRARIES(marblewidget ${MAC_EXTRA_LIBS} )
endif (APPLE)

if (CMAKE_SYSTEM_NAME MATCHES "SunOS")
  TARGET_LINK_LIBRARIES(marblewidget m)
endif (CMAKE_SYSTEM_NAME MATCHES "SunOS")

if(WIN32)
  TARGET_LINK_LIBRARIES(marblewidget ws2_32 imm32 winmm)
endif(WIN32)


set_target_properties(marblewidget  PROPERTIES
                                    VERSION ${GENERIC_LIB_VERSION}
                                    SOVERSION ${GENERIC_LIB_SOVERSION}
                                    COMPILE_FLAGS "-DKML_LAZY_IMP")
# needed for marble_export.h
set_target_properties(marblewidget PROPERTIES
  DEFINE_SYMBOL MAKE_MARBLE_LIB
)

# choose the correct target install library path
if(WIN32 AND QTONLY)
  install(TARGETS marblewidget RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
else(WIN32 AND QTONLY)
  if(APPLE AND QTONLY)
    install (TARGETS marblewidget LIBRARY DESTINATION
      ${CMAKE_INSTALL_PREFIX}/Marble.app/Contents/MacOS/lib)

  else(APPLE AND QTONLY)
    if(QTONLY)
      install(TARGETS marblewidget LIBRARY DESTINATION ${EXEC_INSTALL_PREFIX}/lib${LIB_SUFFIX})
    else(QTONLY)
        install(TARGETS marblewidget  ${INSTALL_TARGETS_DEFAULT_ARGS})
    endif(QTONLY)
  endif(APPLE AND QTONLY)
endif(WIN32 AND QTONLY)

########### install files ###############

if (APPLE AND QTONLY)
  # do nothing because we don't put .h files in the bundle
else (APPLE AND QTONLY)
  install( FILES
    ${graphicsview_HDRS}
    AutoNavigation.h
    BookmarkManager.h
    DownloadRegion.h
    DownloadRegionDialog.h
    LatLonBoxWidget.h
    MarbleWidget.h
    MarbleWebView.h
    MarbleMap.h
    MarbleModel.h
    MarbleControlBox.h
    NavigationWidget.h
    MapViewWidget.h
    LegendWidget.h
    FileViewWidget.h
    CurrentLocationWidget.h
    MarbleNavigator.h
    AbstractFloatItem.h
    MapThemeManager.h
    MarbleAboutDialog.h
    MarbleWidgetInputHandler.h
    MarbleWidgetPopupMenu.h
    TileId.h
    TileCoordsPyramid.h
    TileLevelRangeWidget.h
    TinyWebBrowser.h
    QtMarbleConfigDialog.h
    global.h
    MarbleColors.h
    MarbleGlobal.h
    MarbleDebug.h
    MarbleDirs.h
    GeoPainter.h
    TileCreatorDialog.h
    ViewportParams.h
    projections/AbstractProjection.h
    PositionTracking.h
    Quaternion.h
    SunLocator.h
    ClipPainter.h
    GeoGraphicsScene.h
    GeoDataTreeModel.h
    geodata/data/GeoDataAbstractView.h
    geodata/data/GeoDataAccuracy.h
    geodata/data/GeoDataBalloonStyle.h
    geodata/data/GeoDataColorStyle.h
    geodata/data/GeoDataContainer.h
    geodata/data/GeoDataCoordinates.h
    geodata/data/GeoDataDocument.h
    geodata/data/GeoDataFeature.h
    geodata/data/GeoDataFolder.h
    geodata/data/GeoDataGeometry.h
    geodata/data/GeoDataGroundOverlay.h
    geodata/data/GeoDataHotSpot.h
    geodata/data/GeoDataIconStyle.h
    geodata/data/GeoDataItemIcon.h
    geodata/data/GeoDataLabelStyle.h
    geodata/data/GeoDataLatLonAltBox.h
    geodata/data/GeoDataLatLonBox.h
    geodata/data/GeoDataLinearRing.h
    geodata/data/GeoDataLineString.h
    geodata/data/GeoDataLineStyle.h
    geodata/data/GeoDataListStyle.h
    geodata/data/GeoDataLod.h
    geodata/data/GeoDataLookAt.h
    geodata/data/GeoDataOverlay.h
    geodata/data/GeoDataMultiGeometry.h
    geodata/data/GeoDataObject.h
    geodata/data/GeoDataPlacemark.h
    geodata/data/GeoDataPoint.h
    geodata/data/GeoDataPolygon.h
    geodata/data/GeoDataPolyStyle.h
    geodata/data/GeoDataRegion.h
    geodata/data/GeoDataStyle.h
    geodata/data/GeoDataStyleMap.h
    geodata/data/GeoDataStyleSelector.h
    geodata/data/GeoDataTrack.h
    geodata/data/GeoDataTimeSpan.h
    geodata/data/GeoDataTimeStamp.h
    geodata/data/GeoDataTimePrimitive.h
    geodata/data/Serializable.h
    geodata/geodata_export.h
    geodata/parser/GeoDocument.h
    geodata/writer/GeoWriter.h
    routing/RoutingWidget.h
    routing/RoutingManager.h
    TileCreator.h
    PluginManager.h
    PluginInterface.h
    DialogConfigurationInterface.h
    PositionProviderPlugin.h
    PositionProviderPluginInterface.h
    RenderPlugin.h
    RenderPluginInterface.h
    ParsingRunnerManager.h
    ReverseGeocodingRunnerManager.h
    RoutingRunnerManager.h
    SearchRunnerManager.h
    ParsingRunner.h
    SearchRunner.h
    ReverseGeocodingRunner.h
    RoutingRunner.h
    SearchRunnerPlugin.h
    ReverseGeocodingRunnerPlugin.h
    RoutingRunnerPlugin.h
    ParseRunnerPlugin.h
    LayerInterface.h
    PluginAboutDialog.h
    marble_export.h
    Planet.h

    AbstractDataPlugin.h
    AbstractDataPluginModel.h
    AbstractDataPluginItem.h
    AbstractWorkerThread.h

    LatLonEdit.h
    MapWizard.h
    MapThemeDownloadDialog.h
    ElevationModel.h

    routing/AlternativeRoutesModel.h
    routing/Route.h
    routing/Maneuver.h
    routing/RouteRequest.h
    routing/RouteSegment.h
    routing/RoutingManager.h
    routing/RoutingModel.h
    routing/RoutingProfile.h

    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/marble
)
endif (APPLE AND QTONLY)
