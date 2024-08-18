#include "qgeoroutereplyamap.h"
#include <QDebug>
#include <QGeoRectangle>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtLocation/QGeoRouteSegment>
#include <QtLocation/QGeoManeuver>

static QList<QGeoCoordinate> parsePolyline(const QByteArray &data)
{
    QList<QGeoCoordinate> path;

    bool parsingLatitude = true;

    int shift = 0;
    int value = 0;

    QGeoCoordinate coord(0, 0);

    for (int i = 0; i < data.length(); ++i) {
        unsigned char c = data.at(i) - 63;

        value |= (c & 0x1f) << shift;
        shift += 5;

        // another chunk
        if (c & 0x20)
            continue;

        int diff = (value & 1) ? ~(value >> 1) : (value >> 1);

        if (parsingLatitude) {
            coord.setLatitude(coord.latitude() + (double)diff/1e5);
        } else {
            coord.setLongitude(coord.longitude() + (double)diff/1e5);
            path.append(coord);
        }

        parsingLatitude = !parsingLatitude;

        value = 0;
        shift = 0;
    }

    return path;
}

static QGeoCoordinate constructCoordiante(const QJsonObject &jsonCoord) {
    QGeoCoordinate coord(0,0);
    coord.setLatitude(jsonCoord.value(QStringLiteral("lat")).toDouble());
    coord.setLongitude(jsonCoord.value(QStringLiteral("lng")).toDouble());
    return coord;
}

/*
ferry-train
ferry
*/

static QGeoManeuver::InstructionDirection gmapsInstructionDirection(const QString &instructionCode)
{
    if (instructionCode == "merge"
            || instructionCode == "straight")
        return QGeoManeuver::DirectionForward;
    else if (instructionCode == "turn-slight-right")
        return QGeoManeuver::DirectionLightRight;
    else if (instructionCode == "turn-right" ||
             instructionCode == "roundabout-right")
        return QGeoManeuver::DirectionRight;
    else if (instructionCode == "turn-sharp-right")
        return QGeoManeuver::DirectionHardRight;
    else if (instructionCode == "turn-sharp-left")
        return QGeoManeuver::DirectionHardLeft;
    else if (instructionCode == "turn-left" ||
             instructionCode == "roundabout-left")
        return QGeoManeuver::DirectionLeft;
    else if (instructionCode == "turn-slight-left")
        return QGeoManeuver::DirectionLightLeft;
    else if (instructionCode == "uturn-right")
        return QGeoManeuver::DirectionUTurnRight;
    else if (instructionCode == "uturn-left")
        return QGeoManeuver::DirectionUTurnLeft;
    else if (instructionCode == "keep-right" ||
             instructionCode == "ramp-right" ||
             instructionCode == "fork-right")
        return QGeoManeuver::DirectionBearRight;
    else if (instructionCode == "keep-left" ||
             instructionCode == "ramp-left" ||
             instructionCode == "fork-left")
        return QGeoManeuver::DirectionBearLeft;
    else
        return QGeoManeuver::DirectionForward;
}




QGeoRouteReplyAmap::QGeoRouteReplyAmap(QNetworkReply *reply, const QGeoRouteRequest &request,
                                     QObject *parent)
    :   QGeoRouteReply(request, parent), m_reply(reply)
{
    connect(m_reply, SIGNAL(finished()), this, SLOT(networkReplyFinished()));
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(networkReplyError(QNetworkReply::NetworkError)));
}

QGeoRouteReplyAmap::~QGeoRouteReplyAmap()
{
    if (m_reply)
        m_reply->deleteLater();
}

void QGeoRouteReplyAmap::abort()
{
    if (!m_reply)
        return;

    m_reply->abort();

    m_reply->deleteLater();
    m_reply = 0;
}

void QGeoRouteReplyAmap::networkReplyFinished()
{
    if (!m_reply)
        return;

    if (m_reply->error() != QNetworkReply::NoError) {
        setError(QGeoRouteReply::CommunicationError, m_reply->errorString());
        m_reply->deleteLater();
        m_reply = 0;
        return;
    }
    QList<QGeoRoute> routes;

    QJsonDocument document = QJsonDocument::fromJson(m_reply->readAll());
    if (document.isObject()) {
        QJsonObject object = document.object();

        QString status = object.value(QStringLiteral("status")).toString();
        QString statusMessage = object.value(QStringLiteral("error_message")).toString();

        // status code is OK in case of success
        // an error occurred when trying to find a route
        if (status != "OK") {
            setError(QGeoRouteReply::UnknownError, statusMessage);
            m_reply->deleteLater();
            m_reply = 0;
            return;
        }
        QJsonArray jsonroutes = object.value(QStringLiteral("routes")).toArray();
        qDebug() << "routes:" << jsonroutes.size();
        for(int i = 0; i < jsonroutes.size(); i++) {
            QGeoRoute route;

            QJsonObject o = jsonroutes.at(i).toObject();

            QList<QGeoCoordinate> path;

            QJsonObject bo = o.value(QStringLiteral("bounds")).toObject();
            QJsonObject ne_loc_o = bo.value(QStringLiteral("northeast")).toObject();
            QJsonObject sw_loc_o = bo.value(QStringLiteral("southwest")).toObject();
            QGeoRectangle r(constructCoordiante(ne_loc_o),
                            constructCoordiante(sw_loc_o));
            route.setBounds(r);

            QJsonArray legs = o.value(QStringLiteral("legs")).toArray();
            QGeoRouteSegment firstSegment;
            QGeoRouteSegment prevSegment;
            for(int l = 0; l < legs.size(); l++) {
                QJsonObject lego = legs.at(l).toObject();
                route.setDistance(lego.value("distance").toObject().value("value").toDouble());
                route.setTravelTime(lego.value("duration").toObject().value("value").toInt());

                QJsonArray steps = lego.value(QStringLiteral("steps")).toArray();

                for(int s = 0; s < steps.size(); s++) {
                    QJsonObject stepo = steps.at(s).toObject();

                    QGeoRouteSegment segment;
                    QGeoManeuver maneuver;

                    QString instructionText = stepo.value("html_instructions").toString();
                    qreal distance = stepo.value("distance").toObject().value("value").toDouble();
                    int segmentTime = stepo.value("duration").toObject().value("value").toInt();
                    QByteArray stepGeometry = stepo.value(QStringLiteral("polyline")).
                            toObject().value(QStringLiteral("points")).toString().toLatin1();
                    QList<QGeoCoordinate> steppath = parsePolyline(stepGeometry);
                    QString directionCode = stepo.value("maneuver").toString();

                    path += steppath;

                    maneuver.setDirection(gmapsInstructionDirection(directionCode));
                    maneuver.setDistanceToNextInstruction(distance);
                    maneuver.setInstructionText(instructionText);
                    if (steppath.size() > 0) {
                        maneuver.setPosition(steppath.at(0));
                        maneuver.setWaypoint(steppath.at(0));
                    }
                    maneuver.setTimeToNextInstruction(segmentTime);

                    segment.setDistance(distance);
                    segment.setManeuver(maneuver);
                    segment.setPath(steppath);
                    segment.setTravelTime(segmentTime);

                    if (s == 0 && l == 0) {
                        firstSegment = segment;
                    }
                    if (prevSegment.isValid())
                        prevSegment.setNextRouteSegment(segment);
                    prevSegment = segment;
                }
            }
            route.setFirstRouteSegment(firstSegment);
            route.setPath(path);
            routes.append(route);
        }

        setRoutes(routes);
        setFinished(true);
    }
    else {
        setError(QGeoRouteReply::ParseError, QStringLiteral("Error parsing Amap Maps JSON response:"));
    }

    m_reply->deleteLater();
    m_reply = 0;
}

void QGeoRouteReplyAmap::networkReplyError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)

    if (!m_reply)
        return;

    setError(QGeoRouteReply::CommunicationError, m_reply->errorString());

    m_reply->deleteLater();
    m_reply = 0;
}
