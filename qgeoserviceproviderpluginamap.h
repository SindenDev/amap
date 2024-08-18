#ifndef QGEOSERVICEPROVIDERAMAP_H
#define QGEOSERVICEPROVIDERAMAP_H

#include <QtCore/QObject>
#include <QtLocation/QGeoServiceProviderFactory>

class QGeoServiceProviderFactoryAmap: public QObject, public QGeoServiceProviderFactory
{
    Q_OBJECT
    Q_INTERFACES(QGeoServiceProviderFactory)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    Q_PLUGIN_METADATA(IID "org.qt-project.qt.geoservice.serviceproviderfactory/6.0"
                      FILE "amap_plugin.json")
#elif
    Q_PLUGIN_METADATA(IID  "org.qt-project.qt.geoservice.serviceproviderfactory/5.0"
                      FILE "amap_plugin.json")
#endif

public:
    QGeoCodingManagerEngine *createGeocodingManagerEngine(const QVariantMap &parameters,
                                                          QGeoServiceProvider::Error *error,
                                                          QString *errorString) const;
    QGeoRoutingManagerEngine *createRoutingManagerEngine(const QVariantMap &parameters,
                                                         QGeoServiceProvider::Error *error,
                                                         QString *errorString) const;
    QPlaceManagerEngine *createPlaceManagerEngine(const QVariantMap &parameters,
                                                  QGeoServiceProvider::Error *error,
                                                  QString *errorString) const;
    QGeoMappingManagerEngine *createMappingManagerEngine(const QVariantMap &parameters,
                                                         QGeoServiceProvider::Error *error,
                                                         QString *errorString) const;

};

#endif
