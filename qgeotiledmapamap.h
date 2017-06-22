#ifndef QGEOMAPAMAP_H
#define QGEOMAPAMAP_H

#include "QtLocation/private/qgeotiledmap_p.h"
#include <QtGui/QImage>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class QGeoTiledMappingManagerEngineAmap;

class QGeoTiledMapAmap: public QGeoTiledMap
{
Q_OBJECT
public:
    QGeoTiledMapAmap(QGeoTiledMappingManagerEngineAmap *engine, QObject *parent = 0);
    ~QGeoTiledMapAmap();

    QString getViewCopyright();
    void evaluateCopyrights(const QSet<QGeoTileSpec> &visibleTiles);

private:
    //QImage m_logo;
    QImage m_copyrightsSlab;
    QString m_lastCopyrightsString;
    QPointer<QGeoTiledMappingManagerEngineAmap> m_engine;

    Q_DISABLE_COPY(QGeoTiledMapAmap)
};

QT_END_NAMESPACE

#endif // QGEOMAPAMAP_H
