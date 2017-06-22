#include "qgeoserviceproviderpluginamap.h"
#include "qgeocodingmanagerengineamap.h"
#include "qgeoroutingmanagerengineamap.h"
#include "qplacemanagerengineamap.h"
#include "qgeotiledmappingmanagerengineamap.h"


QGeoCodingManagerEngine *QGeoServiceProviderFactoryAmap::createGeocodingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new QGeoCodingManagerEngineAmap(parameters, error, errorString);
}

QGeoRoutingManagerEngine *QGeoServiceProviderFactoryAmap::createRoutingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new QGeoRoutingManagerEngineAmap(parameters, error, errorString);
}

QPlaceManagerEngine *QGeoServiceProviderFactoryAmap::createPlaceManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new QPlaceManagerEngineAmap(parameters, error, errorString);
}

QGeoMappingManagerEngine *QGeoServiceProviderFactoryAmap::createMappingManagerEngine(
        const QVariantMap &parameters,
        QGeoServiceProvider::Error *error,
        QString *errorString) const
{
    return new QGeoTiledMappingManagerEngineAmap(parameters, error, errorString);
}
