#ifndef QGEOROUTINGMANAGERENGINEAMAP_H
#define QGEOROUTINGMANAGERENGINEAMAP_H

#include <QtLocation/QGeoServiceProvider>
#include <QtLocation/QGeoRoutingManagerEngine>

class QNetworkAccessManager;

class QGeoRoutingManagerEngineAmap : public QGeoRoutingManagerEngine
{
    Q_OBJECT

public:
    QGeoRoutingManagerEngineAmap(const QVariantMap &parameters,
                                QGeoServiceProvider::Error *error,
                                QString *errorString);
    ~QGeoRoutingManagerEngineAmap();

    QGeoRouteReply *calculateRoute(const QGeoRouteRequest &request);

private Q_SLOTS:
    void replyFinished();
    void replyError(QGeoRouteReply::Error errorCode, const QString &errorString);

private:
    QNetworkAccessManager *m_networkManager;
    QByteArray m_userAgent;
    QString m_urlPrefix;
    QString m_apiKey;
};

#endif // QGEOROUTINGMANAGERENGINEAMAP_H

