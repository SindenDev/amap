#include "qplacecategoriesreplyamap.h"

QPlaceCategoriesReplyAmap::QPlaceCategoriesReplyAmap(QObject *parent)
:   QPlaceReply(parent)
{
}

QPlaceCategoriesReplyAmap::~QPlaceCategoriesReplyAmap()
{
}

void QPlaceCategoriesReplyAmap::emitFinished()
{
    setFinished(true);
    emit finished();
}

void QPlaceCategoriesReplyAmap::setError(QPlaceReply::Error errorCode, const QString &errorString)
{
    QPlaceReply::setError(errorCode, errorString);
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    Q_EMIT errorOccurred(errorCode, errorString);
#else
    Q_EMIT error(errorCode, errorString);
#endif
}
