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
    emit error(errorCode, errorString);
}
