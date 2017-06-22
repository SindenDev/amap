#ifndef QGEOTILEDMAPPINGMANAGERENGINEAMAP_H
#define QGEOTILEDMAPPINGMANAGERENGINEAMAP_H

#include "QtLocation/private/qgeotiledmappingmanagerengine_p.h"
#include <QtPositioning/QGeoRectangle>
#include "QtLocation/private/qgeomaptype_p.h"

#include <QGeoServiceProvider>

#include <QList>
#include <QHash>
#include <QSet>

QT_BEGIN_NAMESPACE

class QByteArray;
class QGeoTileSpec;
class QGeoNetworkAccessManager;

class QGeoTiledMappingManagerEngineAmap : public QGeoTiledMappingManagerEngine
{
    Q_OBJECT

public:
    QGeoTiledMappingManagerEngineAmap(const QVariantMap &parameters,
                                       QGeoServiceProvider::Error *error,
                                       QString *errorString);
    ~QGeoTiledMappingManagerEngineAmap();

    virtual QGeoMap *createMap();
    QString getScheme(int mapId);

private:
    void initialize();
//    void populateMapSchemes();

//    QHash<int, QString> m_mapSchemes;
    QString m_cacheDirectory;
};

QT_END_NAMESPACE

#endif // QGEOTILEDMAPPINGMANAGERENGINEAMAP_H
