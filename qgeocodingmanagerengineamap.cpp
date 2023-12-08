#include "qgeocodingmanagerengineamap.h"
#include "qgeocodereplyamap.h"

#include <QtCore/QVariantMap>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtCore/QLocale>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoAddress>
#include <QtPositioning/QGeoShape>
#include <QtPositioning/QGeoRectangle>

static QString addressToQuery(const QGeoAddress &address)
{
    return address.street() + QStringLiteral(",+") +
            address.district() + QStringLiteral(",+") +
            address.city() + QStringLiteral(",+") +
            address.state() + QStringLiteral(",+") +
            address.country();
}

static QString coordinateToQuery(const QGeoCoordinate &coordinate)
{
    return QString::number(coordinate.latitude()) + QStringLiteral(",") +
           QString::number(coordinate.longitude());
}

QGeoCodingManagerEngineAmap::QGeoCodingManagerEngineAmap(const QVariantMap &parameters,
                                                                     QGeoServiceProvider::Error *error,
                                                                     QString *errorString)
    :   QGeoCodingManagerEngine(parameters), m_networkManager(new QNetworkAccessManager(this))
{
    if (parameters.contains(QStringLiteral("amap.useragent")))
        m_userAgent = parameters.value(QStringLiteral("amap.useragent")).toString().toLatin1();
    else
        m_userAgent = "Qt Location based application";

    if(parameters.contains((QStringLiteral("amap.geocode.apikey"))))
        m_apiKey = parameters.value(QStringLiteral("amap.geocode.apikey")).toString();
    else
        m_apiKey = parameters.value(QStringLiteral("amap.apikey")).toString();

    m_urlPrefix = QStringLiteral("https://maps.amap.com/maps/api/geocode/json");

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

QGeoCodingManagerEngineAmap::~QGeoCodingManagerEngineAmap()
{
}

QGeoCodeReply *QGeoCodingManagerEngineAmap::geocode(const QGeoAddress &address, const QGeoShape &bounds)
{
    return geocode(addressToQuery(address), -1, -1, bounds);
}

QGeoCodeReply *QGeoCodingManagerEngineAmap::geocode(const QString &address, int limit, int offset, const QGeoShape &bounds)
{
    Q_UNUSED(offset)
    Q_UNUSED(limit)

    QNetworkRequest request;
    request.setRawHeader("User-Agent", m_userAgent);

    QUrl url(m_urlPrefix);
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("address"), address);
    query.addQueryItem(QStringLiteral("key"), m_apiKey);
    if (bounds.isValid() && !bounds.isEmpty() && bounds.type() != QGeoShape::UnknownType) {
        if (bounds.type() == QGeoShape::RectangleType) {
            const QGeoRectangle &r = static_cast<const QGeoRectangle&>(bounds);
            query.addQueryItem(QStringLiteral("bounds"),
                               (coordinateToQuery(r.topRight()) + "|" + coordinateToQuery(r.bottomLeft())));
        }
    }
    url.setQuery(query);
    request.setUrl(url);

    QNetworkReply *reply = m_networkManager->get(request);

    QGeoCodeReplyAmap *geocodeReply = new QGeoCodeReplyAmap(reply, this);

    connect(geocodeReply, SIGNAL(finished()), this, SLOT(replyFinished()));
    connect(geocodeReply, SIGNAL(error(QGeoCodeReply::Error,QString)),
            this, SLOT(replyError(QGeoCodeReply::Error,QString)));

    return geocodeReply;
}

QGeoCodeReply *QGeoCodingManagerEngineAmap::reverseGeocode(const QGeoCoordinate &coordinate,
                                                                 const QGeoShape &bounds)
{
    Q_UNUSED(bounds)

    QNetworkRequest request;
    request.setRawHeader("User-Agent", m_userAgent);

    QUrl url(m_urlPrefix);
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("latlng"), coordinateToQuery(coordinate));
    query.addQueryItem(QStringLiteral("key"), m_apiKey);

    url.setQuery(query);
    request.setUrl(url);

    QNetworkReply *reply = m_networkManager->get(request);

    QGeoCodeReplyAmap *geocodeReply = new QGeoCodeReplyAmap(reply, this);

    connect(geocodeReply, SIGNAL(finished()), this, SLOT(replyFinished()));
    connect(geocodeReply, SIGNAL(error(QGeoCodeReply::Error,QString)),
            this, SLOT(replyError(QGeoCodeReply::Error,QString)));

    return geocodeReply;
}

void QGeoCodingManagerEngineAmap::replyFinished()
{
    QGeoCodeReply *reply = qobject_cast<QGeoCodeReply *>(sender());
    if (reply)
        emit finished(reply);
}

void QGeoCodingManagerEngineAmap::replyError(QGeoCodeReply::Error errorCode, const QString &errorString)
{
    QGeoCodeReply *reply = qobject_cast<QGeoCodeReply *>(sender());
    if (reply){
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
        Q_EMIT errorOccurred(reply, errorCode, errorString);
#else
        Q_EMIT error(reply, errorCode, errorString);
#endif

    }
}
