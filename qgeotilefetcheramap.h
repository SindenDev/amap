#ifndef QGEOTILEFETCHERAMAP_H
#define QGEOTILEFETCHERAMAP_H

#include "qgeoserviceproviderpluginamap.h"

#include <QtLocation/private/qgeotilefetcher_p.h>
#include <QMutex>
#include <QNetworkReply>

QT_BEGIN_NAMESPACE

class QGeoTiledMapReply;
class QGeoTileSpec;
class QGeoTiledMappingManagerEngine;
class QGeoTiledMappingManagerEngineAmap;
class QNetworkAccessManager;

class QGeoTileFetcherAmap : public QGeoTileFetcher
{
    Q_OBJECT

public:
    QGeoTileFetcherAmap(const QVariantMap &parameters,
                         QGeoTiledMappingManagerEngineAmap *engine, const QSize &tileSize);
    ~QGeoTileFetcherAmap();

    QGeoTiledMapReply *getTileImage(const QGeoTileSpec &spec);

private:
    QString _getURL(int type, int x, int y, int zoom);
    void _tryCorrectAmapVersions(QNetworkAccessManager *networkManager);
    void _getSecAmapWords(int x, int y, QString &sec1, QString &sec2);
    void _getSessionToken();

private slots:
    void _networkReplyError(QNetworkReply::NetworkError error);
    void _replyDestroyed();
    void _amapVersionCompleted();

private:
    Q_DISABLE_COPY(QGeoTileFetcherAmap)

    QNetworkAccessManager *m_networkManager;

    QPointer<QGeoTiledMappingManagerEngineAmap> m_engineAmap;
    QSize m_tileSize;
    QString m_apiKey;
    QString m_signature;
    QString m_client;
    QString m_baseUri;

    int             _timeout;
    bool            _amapVersionRetrieved;
    QNetworkReply*  _amapReply;
    QMutex          _amapVersionMutex;
    QByteArray      _userAgent;
    QString         _language;
    QJsonValue      _sessionToken;

    // Amap version strings
    QString         _versionAmapMap;
    QString         _versionAmapSatellite;
    QString         _versionAmapLabels;
    QString         _versionAmapTerrain;
    QString         _secAmapWord;

    QNetworkRequest netRequest;
};

QT_END_NAMESPACE

#endif
