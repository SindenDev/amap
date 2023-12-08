#ifndef QGEOROUTEREPLYORS_H
#define QGEOROUTEREPLYORS_H

#include <QtNetwork/QNetworkReply>
#include <QtLocation/QGeoRouteReply>

QT_BEGIN_NAMESPACE

class QGeoRouteReplyAmap : public QGeoRouteReply
{
    Q_OBJECT

public:
    explicit QGeoRouteReplyAmap(QNetworkReply *reply, const QGeoRouteRequest &request, QObject *parent = 0);
    ~QGeoRouteReplyAmap();

    void abort() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void networkReplyFinished();
    void networkReplyError(QNetworkReply::NetworkError error);

private:
    QNetworkReply *m_reply;
};

Q_DECLARE_METATYPE(QGeoRouteReplyAmap)
QT_END_NAMESPACE

#endif // QGEOROUTEREPLYOrs_H

