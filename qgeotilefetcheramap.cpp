#include "qgeotilefetcheramap.h"
#include "qgeomapreplyamap.h"
#include "qgeotiledmapamap.h"
#include "qgeotiledmappingmanagerengineamap.h"
#include <QtLocation/private/qgeotilespec_p.h>

#include <QDebug>
#include <QSize>
#include <QDir>
#include <QUrl>
#include <QUrlQuery>
#include <QTime>
#include <QNetworkProxy>
#include <QtCore/QJsonDocument>

#include <math.h>

#include <map>

QT_BEGIN_NAMESPACE

namespace
{

    int _getServerNum(int x, int y, int max)
    {
        return (x + 2 * y) % max;
    }

}

QGeoTileFetcherAmap::QGeoTileFetcherAmap(const QVariantMap &parameters,
                                           QGeoTiledMappingManagerEngineAmap *engine,
                                           const QSize &tileSize)
:   QGeoTileFetcher(engine),
  m_networkManager(new QNetworkAccessManager(this)),
  m_engineAmap(engine),
  m_tileSize(tileSize),
  _amapVersionRetrieved(false)
{
    if(parameters.contains(QStringLiteral("amap.maps.apikey")))
        m_apiKey = parameters.value(QStringLiteral("amap.maps.apikey")).toString();
    else
        m_apiKey = parameters.value(QStringLiteral("amap.apikey")).toString();
    m_signature = parameters.value(QStringLiteral("amap.maps.signature")).toString();
    m_client = parameters.value(QStringLiteral("amap.maps.client")).toString();
    m_baseUri = QStringLiteral("http://maps.amap.com/maps/api/staticmap");
    if (parameters.contains(QStringLiteral("amap.useragent")))
        _userAgent = parameters.value(QStringLiteral("amap.useragent")).toString().toLatin1();
    else
        _userAgent = "";
//        _userAgent = "Mozilla/5.0 (X11; Linux i586; rv:31.0) Gecko/20100101 Firefox/31.0";

    QStringList langs = QLocale::system().uiLanguages();
    if (langs.length() > 0) {
        _language = langs[0];
    }

    // Amap version strings
    _versionAmapMap            = "m@338000000";
    _versionAmapSatellite      = "198";
    _versionAmapLabels         = "h@336";
    _versionAmapTerrain        = "t@132,r@338000000";
    _secAmapWord               = "Galileo";

//    _tryCorrectAmapVersions(m_networkManager);

//    netRequest.setRawHeader("Referrer", "https://www.amap.com/maps/preview");
//    netRequest.setRawHeader("Accept", "*/*");
//    netRequest.setRawHeader("User-Agent", _userAgent);

    /*  2017: support new Amap Maps Tile API (yet under development)
        You have to be whitelisted to use the Tile API. I can't tell how to get whitelisted.
        see https://developers.amap.com/maps/documentation/tile/
        To use the new feature getUrl() and parsing the response has to be adapted. Maybe more than that...
    */
//    _getSessionToken();
}

QGeoTileFetcherAmap::~QGeoTileFetcherAmap()
{
}

void QGeoTileFetcherAmap::_getSessionToken()
{
    QUrl sessionUrl("https://www.amap.com/tile/v1/createSession");

    QUrlQuery queryItems;
    queryItems.addQueryItem("key", m_apiKey);
    queryItems.addQueryItem("mapType", "roadmap");
    queryItems.addQueryItem("language", _language);
    queryItems.addQueryItem("region", "de");

    sessionUrl.setQuery(queryItems);
    netRequest.setUrl(sessionUrl);
    QNetworkReply *sessionReply = m_networkManager->get(netRequest);


    if (sessionReply->error() != QNetworkReply::NoError)
        return;

    QJsonDocument document = QJsonDocument::fromJson(sessionReply->readAll());
    if (!document.isObject())
        return;

    QJsonObject object = document.object();
    QJsonValue status = object.value(QStringLiteral("session"));
    printf(status.toString().toLatin1().data());
}

QGeoTiledMapReply *QGeoTileFetcherAmap::getTileImage(const QGeoTileSpec &spec)
{
    QString surl = _getURL(spec.mapId(), spec.x(), spec.y(), spec.zoom());
    // qDebug()<<"_getURL:" << surl;
    QUrl url(surl);

    netRequest.setUrl(url);

    QNetworkReply *netReply = m_networkManager->get(netRequest);

    QGeoTiledMapReply *mapReply = new QGeoMapReplyAmap(netReply, spec);

    return mapReply;
}

void QGeoTileFetcherAmap::_getSecAmapWords(int x, int y, QString &sec1, QString &sec2)
{
    sec1 = ""; // after &x=...
    sec2 = ""; // after &zoom=...
    int seclen = ((x * 3) + y) % 8;
    sec2 = _secAmapWord.left(seclen);
    if (y >= 10000 && y < 100000) {
        sec1 = "&s=";
    }
}

QString QGeoTileFetcherAmap::_getURL(int type, int x, int y, int zoom)
{
    // qDebug() << "Type:" << type;
    switch (type) {
    case 0:
    case 1: //Road Map
    {
        QString sec1    = ""; // after &x=...
        QString sec2    = ""; // after &zoom=...
        _getSecAmapWords(x, y, sec1, sec2);

        return QString("http://wprd03.is.autonavi.com/appmaptile?style=7&x=%1&y=%2&z=%3").arg(x).arg(y).arg(zoom);
    }
    break;
    case 2: //Satallite Map
    {
        QString sec1    = ""; // after &x=...
        QString sec2    = ""; // after &zoom=...
        _getSecAmapWords(x, y, sec1, sec2);
        return QString("http://wprd03.is.autonavi.com/appmaptile?lang=zh_cn&size=1&style=7&x=%1&y=%2&z=%3").arg(x).arg(y).arg(zoom);
    }
    break;
    case 3: //Terrain Map
    {
        QString sec1    = ""; // after &x=...
        QString sec2    = ""; // after &zoom=...
        _getSecAmapWords(x, y, sec1, sec2);
        return QString("http://wprd03.is.autonavi.com/appmaptile?lang=zh_cn&size=1&style=6&x=%1&y=%2&z=%3").arg(x).arg(y).arg(zoom);
    }
    break;
    case 4: //Hybrid Map
    {
        QString sec1    = ""; // after &x=...
        QString sec2    = ""; // after &zoom=...
        _getSecAmapWords(x, y, sec1, sec2);
        return QString("http://wprd03.is.autonavi.com/appmaptile?lang=zh_cn&size=1&style=8&x=%1&y=%2&z=%3").arg(x).arg(y).arg(zoom);
    }
    break;
    }
    return "";
}

void QGeoTileFetcherAmap::_networkReplyError(QNetworkReply::NetworkError error)
{
    qWarning() << "Could not connect to amap maps. Error:" << error;
    if(_amapReply)
    {
        _amapReply->deleteLater();
        _amapReply = NULL;
    }
}

void QGeoTileFetcherAmap::_replyDestroyed()
{
    _amapReply = NULL;
}

void QGeoTileFetcherAmap::_amapVersionCompleted()
{
    if (!_amapReply || (_amapReply->error() != QNetworkReply::NoError)) {
        qDebug() << "Error collecting Amap maps version info";
        return;
    }
    QString html = QString(_amapReply->readAll());

    QRegExp reg("\"*https?://mt\\D?\\d..*/vt\\?lyrs=m@(\\d*)", Qt::CaseInsensitive);
    if (reg.indexIn(html) != -1) {
        QStringList gc = reg.capturedTexts();
        _versionAmapMap = QString("m@%1").arg(gc[1]);
    }
    reg = QRegExp("\"*https?://khm\\D?\\d.amap.com/kh\\?v=(\\d*)", Qt::CaseInsensitive);
    if (reg.indexIn(html) != -1) {
        QStringList gc = reg.capturedTexts();
        _versionAmapSatellite = gc[1];
    }
    reg = QRegExp("\"*https?://mt\\D?\\d..*/vt\\?lyrs=t@(\\d*),r@(\\d*)", Qt::CaseInsensitive);
    if (reg.indexIn(html) != -1) {
        QStringList gc = reg.capturedTexts();
        _versionAmapTerrain = QString("t@%1,r@%2").arg(gc[1]).arg(gc[2]);
    }

    _amapReply->deleteLater();
    _amapReply = NULL;
}


void QGeoTileFetcherAmap::_tryCorrectAmapVersions(QNetworkAccessManager* networkManager)
{
    QMutexLocker locker(&_amapVersionMutex);
    if (_amapVersionRetrieved) {
        return;
    }
    _amapVersionRetrieved = true;
    if(networkManager)
    {
        QNetworkRequest qheader;
        QNetworkProxy proxy = networkManager->proxy();
        QNetworkProxy tProxy;
        tProxy.setType(QNetworkProxy::DefaultProxy);
        networkManager->setProxy(tProxy);
        QSslConfiguration conf = qheader.sslConfiguration();        conf.setPeerVerifyMode(QSslSocket::VerifyNone);
        qheader.setSslConfiguration(conf);

        QString url = "http://maps.amap.com/maps/api/js?v=3.2&sensor=false";
        qheader.setUrl(QUrl(url));
        qheader.setRawHeader("User-Agent", _userAgent);
        _amapReply = networkManager->get(qheader);
        connect(_amapReply, &QNetworkReply::finished, this, &QGeoTileFetcherAmap::_amapVersionCompleted);
        connect(_amapReply, &QNetworkReply::destroyed, this, &QGeoTileFetcherAmap::_replyDestroyed);
        connect(_amapReply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
                this, &QGeoTileFetcherAmap::_networkReplyError);
        networkManager->setProxy(proxy);
    }
}

QT_END_NAMESPACE
