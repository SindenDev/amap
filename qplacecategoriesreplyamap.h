#ifndef QPLACECATEGORIESREPLYAMAP_H
#define QPLACECATEGORIESREPLYAMAP_H

#include <QtLocation/QPlaceReply>

class QPlaceCategoriesReplyAmap : public QPlaceReply
{
    Q_OBJECT

public:
    explicit QPlaceCategoriesReplyAmap(QObject *parent = 0);
    ~QPlaceCategoriesReplyAmap();

    void emitFinished();
    void setError(QPlaceReply::Error errorCode, const QString &errorString);
};

#endif // QPLACECATEGORIESREPLYAMAP_H
