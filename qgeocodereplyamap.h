#ifndef QGEOCODEREPLYAMAP_H
#define QGEOCODEREPLYAMAP_H

#include <QtNetwork/QNetworkReply>
#include <QtLocation/QGeoCodeReply>

class QGeoCodeReplyAmap : public QGeoCodeReply
{
    Q_OBJECT

public:
    explicit QGeoCodeReplyAmap(QNetworkReply *reply, QObject *parent = 0);
    ~QGeoCodeReplyAmap();

    void abort();

private Q_SLOTS:
    void networkReplyFinished();
    void networkReplyError(QNetworkReply::NetworkError error);

private:
    QNetworkReply *m_reply;
};

#endif // QGEOCODEREPLYORS_H
