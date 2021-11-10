#include "QtLocation/private/qgeocameracapabilities_p.h"
#include "qgeotiledmappingmanagerengineamap.h"
#include "qgeotiledmapamap.h"
#include "qgeotilefetcheramap.h"
#include "QtLocation/private/qgeotilespec_p.h"
#include "QtLocation/private/qgeofiletilecache_p.h"

#include <QDebug>
#include <QDir>
#include <QVariant>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>
#include <QtCore/qmath.h>
#include <QtCore/qstandardpaths.h>

QT_BEGIN_NAMESPACE

QGeoTiledMappingManagerEngineAmap::QGeoTiledMappingManagerEngineAmap(const QVariantMap &parameters,
    QGeoServiceProvider::Error *error,
    QString *errorString)
    : QGeoTiledMappingManagerEngine()
{
    Q_UNUSED(error);
    Q_UNUSED(errorString);

    QGeoCameraCapabilities capabilities;
    capabilities.setMinimumZoomLevel(1.96);
    capabilities.setMaximumZoomLevel(20.88);
    capabilities.setSupportsBearing(true);
    capabilities.setSupportsTilting(true);
    capabilities.setMinimumTilt(0);
    capabilities.setMaximumTilt(80);
    capabilities.setMinimumFieldOfView(20.0);
    capabilities.setMaximumFieldOfView(120.0);
    capabilities.setOverzoomEnabled(true);

    setCameraCapabilities(capabilities);

    int tile = parameters.value(QStringLiteral("amap.maps.tilesize"), 256).toInt();

    setTileSize(QSize(tile, tile));

    QList<QGeoMapType> types;
    #if QT_VERSION < QT_VERSION_CHECK(5,9,0)
          types << QGeoMapType(QGeoMapType::StreetMap, tr("Road Map"), tr("Normal map view in daylight mode"), false, false, 1);
          types << QGeoMapType(QGeoMapType::SatelliteMapDay, tr("Satellite"), tr("Satellite map view in daylight mode"), false, false, 2);
          types << QGeoMapType(QGeoMapType::TerrainMap, tr("Terrain"), tr("Terrain map view in daylight mode"), false, false, 3);
          types << QGeoMapType(QGeoMapType::HybridMap, tr("Hybrid"), tr("Satellite map view with streets in daylight mode"), false, false, 4);
    #elif QT_VERSION < QT_VERSION_CHECK(5,10,0)
         types << QGeoMapType(QGeoMapType::StreetMap, tr("Road Map"), tr("Normal map view in daylight mode"), false, false, 1, "amap");
         types << QGeoMapType(QGeoMapType::SatelliteMapDay, tr("Satellite"), tr("Satellite map view in daylight mode"), false, false, 2, "amap");
         types << QGeoMapType(QGeoMapType::TerrainMap, tr("Terrain"), tr("Terrain map view in daylight mode"), false, false, 3, "amap");
         types << QGeoMapType(QGeoMapType::HybridMap, tr("Hybrid"), tr("Satellite map view with streets in daylight mode"), false, false, 4, "amap");
    #else
        //QGeoCameraCapabilities cameraCapabilities;
        types << QGeoMapType(QGeoMapType::StreetMap, tr("Road Map"), tr("Normal map view in daylight mode"), false,false, 1, "amap", capabilities);
        types << QGeoMapType(QGeoMapType::TerrainMap, tr("Terrain"), tr("Terrain map view in daylight mode"), false, false, 2, "amap", capabilities);
        types << QGeoMapType(QGeoMapType::SatelliteMapDay, tr("Satellite"), tr("Satellite map view in daylight mode"), false, false, 3, "amap", capabilities);
        types << QGeoMapType(QGeoMapType::HybridMap, tr("Hybrid"), tr("Satellite map view with streets in daylight mode"), false, false, 4, "amap", capabilities);
    #endif

    setSupportedMapTypes(types);

    QGeoTileFetcherAmap *fetcher = new QGeoTileFetcherAmap(parameters, this, tileSize());
    setTileFetcher(fetcher);

    if (parameters.contains(QStringLiteral("amap.cachefolder")))
        m_cacheDirectory = parameters.value(QStringLiteral("amap.cachefolder")).toString().toLatin1();
    else
        m_cacheDirectory = QAbstractGeoTileCache::baseCacheDirectory() + QLatin1String("amap");

    QAbstractGeoTileCache *tileCache = new QGeoFileTileCache(m_cacheDirectory);
    tileCache->setMaxDiskUsage(100 * 1024 * 1024);
    setTileCache(tileCache);

//    populateMapSchemes();
//    *error = QGeoServiceProvider::NoError;
//    errorString->clear();
    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

QGeoTiledMappingManagerEngineAmap::~QGeoTiledMappingManagerEngineAmap()
{
}

//void QGeoTiledMappingManagerEngineAmap::populateMapSchemes()
//{
//    m_mapSchemes[0] = QStringLiteral("roadmap");
//    m_mapSchemes[1] = QStringLiteral("roadmap");
//    m_mapSchemes[2] = QStringLiteral("satellite");
//    m_mapSchemes[3] = QStringLiteral("terrain");
//    m_mapSchemes[4] = QStringLiteral("hybrid");
//}

//QString QGeoTiledMappingManagerEngineAmap::getScheme(int mapId)
//{
//    return m_mapSchemes[mapId];
//}

QGeoMap *QGeoTiledMappingManagerEngineAmap::createMap()
{
    return new QGeoTiledMapAmap(this);
}

QT_END_NAMESPACE

