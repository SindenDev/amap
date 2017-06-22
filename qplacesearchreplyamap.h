#ifndef QPLACESEARCHREPLYAMAP_H
#define QPLACESEARCHREPLYAMAP_H

#include <QtLocation/QPlaceSearchReply>

QT_BEGIN_NAMESPACE

class QNetworkReply;
class QPlaceManagerEngineAmap;
class QPlaceResult;

class QPlaceSearchReplyAmap : public QPlaceSearchReply
{
    Q_OBJECT

public:
    QPlaceSearchReplyAmap(const QPlaceSearchRequest &request, QNetworkReply *reply,
                          QPlaceManagerEngineAmap *parent);
    ~QPlaceSearchReplyAmap();

    void abort();

private slots:
    void setError(QPlaceReply::Error errorCode, const QString &errorString);
    void replyFinished();

private:
    QPlaceResult parsePlaceResult(const QJsonObject &item) const;

    QNetworkReply *m_reply;
};

QT_END_NAMESPACE

#endif // QPLACESEARCHREPLYORS_H
