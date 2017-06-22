#include "qgeomapreplyamap.h"
#include <QNetworkAccessManager>
#include <QNetworkCacheMetaData>
#include <QDateTime>

QT_BEGIN_NAMESPACE

QGeoMapReplyAmap::QGeoMapReplyAmap(QNetworkReply *reply, const QGeoTileSpec &spec, QObject *parent)
        : QGeoTiledMapReply(spec, parent),
        m_reply(reply)
{
    connect(m_reply,
            SIGNAL(finished()),
            this,
            SLOT(networkFinished()));

    connect(m_reply,
            SIGNAL(error(QNetworkReply::NetworkError)),
            this,
            SLOT(networkError(QNetworkReply::NetworkError)));
}

QGeoMapReplyAmap::~QGeoMapReplyAmap()
{
}

QNetworkReply *QGeoMapReplyAmap::networkReply() const
{
    return m_reply;
}

void QGeoMapReplyAmap::abort()
{
    if (!m_reply)
        return;

    m_reply->abort();
}

void QGeoMapReplyAmap::networkFinished()
{
    if (!m_reply)
        return;

    if (m_reply->error() != QNetworkReply::NoError)
        return;

    setMapImageData(m_reply->readAll());
    const int _mid = tileSpec().mapId();
    if (_mid == 2)
        setMapImageFormat("jpeg");
    else
        setMapImageFormat("png");
    setFinished(true);

    m_reply->deleteLater();
    m_reply = 0;
}

void QGeoMapReplyAmap::networkError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);
    if (!m_reply)
        return;

    setFinished(true);
    setCached(false);
    m_reply->deleteLater();
    m_reply = 0;
}

QT_END_NAMESPACE
