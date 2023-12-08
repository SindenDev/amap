#include "qgeocodereplyamap.h"


#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoAddress>
#include <QtPositioning/QGeoLocation>
#include <QtPositioning/QGeoRectangle>

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>

static QGeoCoordinate constructCoordiante(const QJsonObject &jsonCoord) {
    QGeoCoordinate coord(0,0);
    coord.setLatitude(jsonCoord.value(QStringLiteral("lat")).toDouble());
    coord.setLongitude(jsonCoord.value(QStringLiteral("lng")).toDouble());
    return coord;
}

static bool checkAddressType(const QJsonObject &jsonAddress, const QString &type) {
    QJsonArray a = jsonAddress.value("types").toArray();
    for (int i = 0; i < a.size(); i++) {
        if (a.at(i).toString() == type)
            return true;
    }
    return false;
}

QGeoCodeReplyAmap::QGeoCodeReplyAmap(QNetworkReply *reply, QObject *parent)
    :   QGeoCodeReply(parent), m_reply(reply)
{
    connect(m_reply, SIGNAL(finished()), this, SLOT(networkReplyFinished()));
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(networkReplyError(QNetworkReply::NetworkError)));

    setLimit(1);
    setOffset(0);
}

QGeoCodeReplyAmap::~QGeoCodeReplyAmap()
{
    if (m_reply)
        m_reply->deleteLater();
}

void QGeoCodeReplyAmap::abort()
{
    if (!m_reply)
        return;

    m_reply->abort();

    m_reply->deleteLater();
    m_reply = 0;
}


void QGeoCodeReplyAmap::networkReplyFinished()
{
    if (!m_reply)
        return;

    if (m_reply->error() != QNetworkReply::NoError)
        return;

    QString status;

    QList<QGeoLocation> locations;
    //        setError(QGeoCodeReply::ParseError, QStringLiteral("Error parsing OpenRouteService xml response:") + xml.errorString() + " at line: " + xml.lineNumber());
    QJsonDocument document = QJsonDocument::fromJson(m_reply->read(m_reply->bytesAvailable()));
    if (document.isObject()) {
        QJsonObject object = document.object();

        status = object.value(QStringLiteral("status")).toString();
        if (status == "OK") {
            QJsonArray jsonlocations = object.value(QStringLiteral("results")).toArray();
            qDebug() << "locations:" << jsonlocations.size();
            for(int i = 0; i < jsonlocations.size(); i++) {
                QGeoLocation location;
                QGeoAddress address;

                QJsonObject o = jsonlocations.at(i).toObject();
                address.setText(o.value("formatted_address").toString());
                QJsonObject ogeometry = o.value("geometry").toObject();

                location.setCoordinate(constructCoordiante(ogeometry.value("location").toObject()));

                QJsonObject jaddressRanges = ogeometry.value("viewport").toObject();
                if (!jaddressRanges.isEmpty()) {
                    QGeoRectangle r;
                    r.setTopRight(constructCoordiante(jaddressRanges.value("northeast").toObject()));
                    r.setBottomLeft(constructCoordiante(jaddressRanges.value("southwest").toObject()));
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
                    location.setBoundingShape(r);
#else
                    location.setBoundingBox(r);
#endif

                }

                QJsonArray jaddress = o.value("address_components").toArray();

                QString street_name;
                QString street_num;
                for(int j = 0; j < jaddress.size(); j++) {
                    QJsonObject addobj = jaddress.at(j).toObject();
                    if (checkAddressType(addobj, "street_number"))
                        street_num = addobj.value("long_name").toString();
                    if (checkAddressType(addobj, "route"))
                        street_name = addobj.value("long_name").toString();
                    if (checkAddressType(addobj, "country"))
                        address.setCountryCode(addobj.value("short_name").toString());
                    if (checkAddressType(addobj, "administrative_area_level_3"))
                        address.setCity(addobj.value("long_name").toString());
                    if (checkAddressType(addobj, "administrative_area_level_2"))
                        address.setCounty(addobj.value("long_name").toString());
                    if (checkAddressType(addobj, "administrative_area_level_1"))
                        address.setState(addobj.value("long_name").toString());
                    if (checkAddressType(addobj, "sublocality"))
                        address.setDistrict(addobj.value("long_name").toString());
                    if (checkAddressType(addobj, "postal_code"))
                        address.setPostalCode(addobj.value("long_name").toString());
                }
                address.setStreet(street_name + (street_num.size() > 0 ? ( ", " + street_num) : ""));

                location.setAddress(address);

                locations.append(location);
            }
        }
    } else {
        if (status == "ZERO_RESULTS")
            setError(CombinationError, "Geocode was successful but returned no results. This may occur if the geocoder was passed a non-existent address");
        else if (status == "OVER_QUERY_LIMIT")
            setError(CommunicationError, "Request quota is over");
        else if (status == "REQUEST_DENIED")
            setError(CommunicationError, "Request denied");
        else if (status == "INVALID_REQUEST")
            setError(UnsupportedOptionError, "Address, components or latlng is missing");
        else if (status == "UNKNOWN_ERROR")
            setError(UnknownError, "Request could not be processed due to a server error. Try again later");
    }

    setLocations(locations);
    setFinished(true);

    m_reply->deleteLater();
    m_reply = 0;
}

void QGeoCodeReplyAmap::networkReplyError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)

    if (!m_reply)
        return;

    setError(QGeoCodeReply::CommunicationError, m_reply->errorString());

    m_reply->deleteLater();
    m_reply = 0;
}
