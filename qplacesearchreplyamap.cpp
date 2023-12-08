#include "qplacesearchreplyamap.h"
#include "qplacemanagerengineamap.h"
#include <QPlace>
#include <QPlaceIcon>
#include <QGeoAddress>
#include <QGeoLocation>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtNetwork/QNetworkReply>
#include <QtPositioning/QGeoCircle>
#include <QtPositioning/QGeoRectangle>
#include <QtLocation/QPlaceResult>
#include <QtLocation/QPlaceSearchRequest>

QPlaceSearchReplyAmap::QPlaceSearchReplyAmap(const QPlaceSearchRequest &request,
                                             QNetworkReply *reply, QPlaceManagerEngineAmap *parent)
:   QPlaceSearchReply(parent), m_reply(reply)
{
    Q_ASSERT(parent);

    setRequest(request);

    if (!m_reply)
        return;

    m_reply->setParent(this);
    connect(m_reply, SIGNAL(finished()), this, SLOT(replyFinished()));
}

QPlaceSearchReplyAmap::~QPlaceSearchReplyAmap()
{
}

void QPlaceSearchReplyAmap::abort()
{
    if (m_reply)
        m_reply->abort();
}

void QPlaceSearchReplyAmap::setError(QPlaceReply::Error errorCode, const QString &errorString)
{
    QPlaceReply::setError(errorCode, errorString);

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    Q_EMIT errorOccurred(errorCode, errorString);
#else
    Q_EMIT error(errorCode, errorString);
#endif
    setFinished(true);
    emit finished();
}

static QGeoRectangle parseBoundingBox(const QJsonArray &coordinates)
{
    if (coordinates.count() != 4)
        return QGeoRectangle();

    double bottom = coordinates.at(0).toString().toDouble();
    double top = coordinates.at(1).toString().toDouble();
    double left = coordinates.at(2).toString().toDouble();
    double right = coordinates.at(3).toString().toDouble();

    return QGeoRectangle(QGeoCoordinate(top, left), QGeoCoordinate(bottom, right));
}

void QPlaceSearchReplyAmap::replyFinished()
{
    QNetworkReply *reply = m_reply;
    m_reply->deleteLater();
    m_reply = 0;

    if (reply->error() != QNetworkReply::NoError) {
        setError(CommunicationError, tr("Communication error"));
        return;
    }

    QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    if (!document.isArray()) {
        setError(ParseError, tr("Response parse error"));
        return;
    }

    QJsonArray resultsArray = document.array();

    QGeoCoordinate searchCenter = request().searchArea().center();

    QStringList placeIds;

    QList<QPlaceSearchResult> results;
    for (int i = 0; i < resultsArray.count(); ++i) {
        QJsonObject item = resultsArray.at(i).toObject();
        QPlaceResult pr = parsePlaceResult(item);
        pr.setDistance(searchCenter.distanceTo(pr.place().location().coordinate()));
        placeIds.append(pr.place().placeId());
        results.append(pr);
    }

    QVariantMap searchContext = request().searchContext().toMap();
    QStringList excludePlaceIds =
        searchContext.value(QStringLiteral("ExcludePlaceIds")).toStringList();

    if (!excludePlaceIds.isEmpty()) {
        QPlaceSearchRequest r = request();
        QVariantMap parameters = searchContext;

        QStringList epi = excludePlaceIds;
        epi.removeLast();

        parameters.insert(QStringLiteral("ExcludePlaceIds"), epi);
        r.setSearchContext(parameters);
        setPreviousPageRequest(r);
    }

    if (!placeIds.isEmpty()) {
        QPlaceSearchRequest r = request();
        QVariantMap parameters = searchContext;

        QStringList epi = excludePlaceIds;
        epi.append(placeIds.join(QLatin1Char(',')));

        parameters.insert(QStringLiteral("ExcludePlaceIds"), epi);
        r.setSearchContext(parameters);
        setNextPageRequest(r);
    }

    setResults(results);

    setFinished(true);
    emit finished();
}

QPlaceResult QPlaceSearchReplyAmap::parsePlaceResult(const QJsonObject &item) const
{
    QPlace place;

    QGeoCoordinate coordinate = QGeoCoordinate(item.value(QStringLiteral("lat")).toString().toDouble(),
                                               item.value(QStringLiteral("lon")).toString().toDouble());

    //const QString placeRank = item.value(QStringLiteral("place_rank")).toString();
    //const QString category = item.value(QStringLiteral("category")).toString();
    const QString type = item.value(QStringLiteral("type")).toString();
    //double importance = item.value(QStringLiteral("importance")).toDouble();

    place.setAttribution(item.value(QStringLiteral("licence")).toString());
    place.setPlaceId(item.value(QStringLiteral("place_id")).toString());

    QVariantMap iconParameters;
    iconParameters.insert(QPlaceIcon::SingleUrl,
                          QUrl(item.value(QStringLiteral("icon")).toString()));
    QPlaceIcon icon;
    icon.setParameters(iconParameters);
    place.setIcon(icon);

    QJsonObject addressDetails = item.value(QStringLiteral("address")).toObject();

    const QString title = addressDetails.value(type).toString();

    place.setName(title);

    QGeoAddress address;
    address.setCity(addressDetails.value(QStringLiteral("city")).toString());
    address.setCountry(addressDetails.value(QStringLiteral("country")).toString());
    // FIXME: country_code is alpha-2 setCountryCode takes alpha-3
    //address.setCountryCode(addressDetails.value(QStringLiteral("country_code")).toString());
    address.setPostalCode(addressDetails.value(QStringLiteral("postcode")).toString());
    address.setStreet(addressDetails.value(QStringLiteral("road")).toString());
    address.setState(addressDetails.value(QStringLiteral("state")).toString());
    address.setDistrict(addressDetails.value(QStringLiteral("suburb")).toString());

    QGeoLocation location;
    location.setCoordinate(coordinate);
    location.setAddress(address);
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    location.setBoundingShape(parseBoundingBox(item.value(QStringLiteral("boundingbox")).toArray()));
#else
    location.setBoundingBox(parseBoundingBox(item.value(QStringLiteral("boundingbox")).toArray()));
#endif
    place.setLocation(location);

    QPlaceResult result;
    result.setIcon(icon);
    result.setPlace(place);
    result.setTitle(title);

    return result;
}
