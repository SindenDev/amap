#include "qplacemanagerengineamap.h"
#include "qplacesearchreplyamap.h"
#include "qplacecategoriesreplyamap.h"
#include "qplacesearchsuggestionreplyimpl.h"

#include <QtCore/QUrlQuery>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QRegularExpression>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtPositioning/QGeoCircle>
#include <QtLocation/private/unsupportedreplies_p.h>

#include <QtCore/QElapsedTimer>

namespace
{
QString SpecialPhrasesBaseUrl = QStringLiteral("http://wiki.openstreetmap.org/wiki/Special:Export/Nominatim/Special_Phrases/");

QString nameForTagKey(const QString &tagKey)
{
    if (tagKey == QLatin1String("aeroway"))
        return QPlaceManagerEngineAmap::tr("Aeroway");
    else if (tagKey == QLatin1String("amenity"))
        return QPlaceManagerEngineAmap::tr("Amenity");
    else if (tagKey == QLatin1String("building"))
        return QPlaceManagerEngineAmap::tr("Building");
    else if (tagKey == QLatin1String("highway"))
        return QPlaceManagerEngineAmap::tr("Highway");
    else if (tagKey == QLatin1String("historic"))
        return QPlaceManagerEngineAmap::tr("Historic");
    else if (tagKey == QLatin1String("landuse"))
        return QPlaceManagerEngineAmap::tr("Land use");
    else if (tagKey == QLatin1String("leisure"))
        return QPlaceManagerEngineAmap::tr("Leisure");
    else if (tagKey == QLatin1String("man_made"))
        return QPlaceManagerEngineAmap::tr("Man made");
    else if (tagKey == QLatin1String("natural"))
        return QPlaceManagerEngineAmap::tr("Natural");
    else if (tagKey == QLatin1String("place"))
        return QPlaceManagerEngineAmap::tr("Place");
    else if (tagKey == QLatin1String("railway"))
        return QPlaceManagerEngineAmap::tr("Railway");
    else if (tagKey == QLatin1String("shop"))
        return QPlaceManagerEngineAmap::tr("Shop");
    else if (tagKey == QLatin1String("tourism"))
        return QPlaceManagerEngineAmap::tr("Tourism");
    else if (tagKey == QLatin1String("waterway"))
        return QPlaceManagerEngineAmap::tr("Waterway");
    else
        return tagKey;
}

}


static bool addAtForBoundingArea(const QGeoShape &area,
                                 QUrlQuery *queryItems)
{
    QGeoCoordinate center = area.center();
    if (!center.isValid())
        return false;

    queryItems->addQueryItem(QStringLiteral("location"),
                             QString::number(center.latitude()) +
                             QLatin1Char(',') +
                             QString::number(center.longitude()));
    // for amap maps the shape has to be a circle
    QGeoCircle* cirle = new QGeoCircle(area);
    if(cirle != NULL)
    {
        qreal radiusCorrected = cirle->radius();
        if(radiusCorrected < 1)
            radiusCorrected = 1;
        queryItems->addQueryItem(QStringLiteral("radius"),
                                 QString::number(radiusCorrected));
    }
    return true;
}

QPlaceManagerEngineAmap::QPlaceManagerEngineAmap(const QVariantMap &parameters,
                                               QGeoServiceProvider::Error *error,
                                               QString *errorString)
:   QPlaceManagerEngine(parameters), m_networkManager(new QNetworkAccessManager(this)),
    m_categoriesReply(0)
{
//    if (parameters.contains(QStringLiteral("ors.useragent")))
//        m_userAgent = parameters.value(QStringLiteral("ors.useragent")).toString().toLatin1();
//    else
//        m_userAgent = "Qt Location based application";

    if (parameters.contains(QStringLiteral("amap.places.apikey")))
        m_apiKey = parameters.value(QStringLiteral("amap.places.apikey")).toString().toLatin1();
    else
        m_apiKey = parameters.value(QStringLiteral("amap.apikey")).toString().toLatin1();

//    if (parameters.contains(QStringLiteral("Ors.places.host")))
//        m_urlPrefix = parameters.value(QStringLiteral("Ors.places.host")).toString();
//    else
        m_urlPrefix = QStringLiteral("https://maps.amap.com/maps/api/place");

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

QPlaceManagerEngineAmap::~QPlaceManagerEngineAmap()
{
}

QPlaceSearchReply *QPlaceManagerEngineAmap::search(const QPlaceSearchRequest &request)
{
    bool unsupported = false;

    // Only public visibility supported
    unsupported |= request.visibilityScope() != QLocation::UnspecifiedVisibility &&
                   request.visibilityScope() != QLocation::PublicVisibility;
    unsupported |= request.searchTerm().isEmpty() && request.categories().isEmpty();

    if (unsupported)
        return QPlaceManagerEngine::search(request);

    QUrlQuery queryItems;

    queryItems.addQueryItem(QStringLiteral("format"), QStringLiteral("jsonv2"));

    //queryItems.addQueryItem(QStringLiteral("accept-language"), QStringLiteral("en"));

    QGeoRectangle boundingBox;
    QGeoShape searchArea = request.searchArea();
    switch (searchArea.type()) {
    case QGeoShape::CircleType: {
        QGeoCircle c(searchArea);
        qreal radius = c.radius();
        if (radius < 0)
            radius = 50000;

        boundingBox = QGeoRectangle(c.center().atDistanceAndAzimuth(radius, -45),
                                    c.center().atDistanceAndAzimuth(radius, 135));
        break;
    }
    case QGeoShape::RectangleType:
        boundingBox = searchArea;
        break;
    default:
        ;
    }

    if (!boundingBox.isEmpty()) {
        queryItems.addQueryItem(QStringLiteral("bounded"), QStringLiteral("1"));
        QString coordinates;
        coordinates = QString::number(boundingBox.topLeft().longitude()) + QLatin1Char(',') +
                      QString::number(boundingBox.topLeft().latitude()) + QLatin1Char(',') +
                      QString::number(boundingBox.bottomRight().longitude()) + QLatin1Char(',') +
                      QString::number(boundingBox.bottomRight().latitude());
        queryItems.addQueryItem(QStringLiteral("viewbox"), coordinates);
    }

    QStringList queryParts;
    if (!request.searchTerm().isEmpty())
        queryParts.append(request.searchTerm());

    foreach (const QPlaceCategory &category, request.categories()) {
        QString id = category.categoryId();
        int index = id.indexOf(QLatin1Char('='));
        if (index != -1)
            id = id.mid(index+1);
        queryParts.append(QLatin1Char('[') + id + QLatin1Char(']'));
    }

    queryItems.addQueryItem(QStringLiteral("q"), queryParts.join(QLatin1Char('+')));

    QVariantMap parameters = request.searchContext().toMap();

    QStringList placeIds = parameters.value(QStringLiteral("ExcludePlaceIds")).toStringList();
    if (!placeIds.isEmpty())
        queryItems.addQueryItem(QStringLiteral("exclude_place_ids"), placeIds.join(QLatin1Char(',')));

    queryItems.addQueryItem(QStringLiteral("addressdetails"), QStringLiteral("1"));

    QUrl requestUrl(m_urlPrefix);
    requestUrl.setQuery(queryItems);

    QNetworkReply *networkReply = m_networkManager->get(QNetworkRequest(requestUrl));

    QPlaceSearchReplyAmap *reply = new QPlaceSearchReplyAmap(request, networkReply, this);
    connect(reply, SIGNAL(finished()), this, SLOT(replyFinished()));
    connect(reply, SIGNAL(error(QPlaceReply::Error,QString)),
            this, SLOT(replyError(QPlaceReply::Error,QString)));

    return reply;
}

QPlaceSearchSuggestionReply *QPlaceManagerEngineAmap::searchSuggestions(const QPlaceSearchRequest &query)
{
    bool unsupported = false;

    unsupported |= query.visibilityScope() != QLocation::UnspecifiedVisibility &&
                   query.visibilityScope() != QLocation::PublicVisibility;

    unsupported |= !query.categories().isEmpty();
    unsupported |= !query.recommendationId().isEmpty();

    if (unsupported) {
        QPlaceSearchSuggestionReplyImpl *reply = new QPlaceSearchSuggestionReplyImpl(0, this);
        connect(reply, SIGNAL(finished()), this, SLOT(replyFinished()));
        connect(reply, SIGNAL(error(QPlaceReply::Error,QString)),
                this, SLOT(replyError(QPlaceReply::Error,QString)));
        QMetaObject::invokeMethod(reply, "setError", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply::Error, QPlaceReply::BadArgumentError),
                                  Q_ARG(QString, "Unsupported search request options specified."));
        return reply;
    }

    QUrl requestUrl(m_urlPrefix + QStringLiteral("/autocomplete/json"));

    QUrlQuery queryItems;

    queryItems.addQueryItem(QStringLiteral("input"), query.searchTerm());

    if (!addAtForBoundingArea(query.searchArea(), &queryItems)) {
        QPlaceSearchSuggestionReplyImpl *reply = new QPlaceSearchSuggestionReplyImpl(0, this);
        connect(reply, SIGNAL(finished()), this, SLOT(replyFinished()));
        connect(reply, SIGNAL(error(QPlaceReply::Error,QString)),
                this, SLOT(replyError(QPlaceReply::Error,QString)));
        QMetaObject::invokeMethod(reply, "setError", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply::Error, QPlaceReply::BadArgumentError),
                                  Q_ARG(QString, "Invalid search area provided"));
        return reply;
    }

    requestUrl.setQuery(queryItems);

    QNetworkReply *networkReply = sendRequest(requestUrl);

    QPlaceSearchSuggestionReplyImpl *reply = new QPlaceSearchSuggestionReplyImpl(networkReply, this);
    connect(reply, SIGNAL(finished()), this, SLOT(replyFinished()));
    connect(reply, SIGNAL(error(QPlaceReply::Error,QString)),
            this, SLOT(replyError(QPlaceReply::Error,QString)));

    return reply;
}

QPlaceReply *QPlaceManagerEngineAmap::initializeCategories()
{
    // Only fetch categories once
    if (m_categories.isEmpty() && !m_categoriesReply) {
        m_categoryLocales = m_locales;
        m_categoryLocales.append(QLocale(QLocale::English));
        fetchNextCategoryLocale();
    }

    QPlaceCategoriesReplyAmap *reply = new QPlaceCategoriesReplyAmap(this);
    connect(reply, SIGNAL(finished()), this, SLOT(replyFinished()));
    connect(reply, SIGNAL(error(QPlaceReply::Error,QString)),
            this, SLOT(replyError(QPlaceReply::Error,QString)));

    // TODO delayed finished() emission
    if (!m_categories.isEmpty())
        reply->emitFinished();

    m_pendingCategoriesReply.append(reply);
    return reply;
}

QString QPlaceManagerEngineAmap::parentCategoryId(const QString &categoryId) const
{
    Q_UNUSED(categoryId)

    // Only a two category levels
    return QString();
}

QStringList QPlaceManagerEngineAmap::childCategoryIds(const QString &categoryId) const
{
    return m_subcategories.value(categoryId);
}

QPlaceCategory QPlaceManagerEngineAmap::category(const QString &categoryId) const
{
    return m_categories.value(categoryId);
}

QList<QPlaceCategory> QPlaceManagerEngineAmap::childCategories(const QString &parentId) const
{
    QList<QPlaceCategory> categories;
    foreach (const QString &id, m_subcategories.value(parentId))
        categories.append(m_categories.value(id));
    return categories;
}

QList<QLocale> QPlaceManagerEngineAmap::locales() const
{
    return m_locales;
}

void QPlaceManagerEngineAmap::setLocales(const QList<QLocale> &locales)
{
    m_locales = locales;
}

void QPlaceManagerEngineAmap::categoryReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply)
        return;

    reply->deleteLater();

    QXmlStreamReader parser(reply);
    while (!parser.atEnd() && parser.readNextStartElement()) {
        if (parser.name() == QLatin1String("mediawiki"))
            continue;
        if (parser.name() == QLatin1String("page"))
            continue;
        if (parser.name() == QLatin1String("revision"))
            continue;
        if (parser.name() == QLatin1String("text")) {
            // parse
            QString page = parser.readElementText();
            QRegularExpression regex(QStringLiteral("\\| ([^|]+) \\|\\| ([^|]+) \\|\\| ([^|]+) \\|\\| ([^|]+) \\|\\| ([\\-YN])"));
            QRegularExpressionMatchIterator i = regex.globalMatch(page);
            while (i.hasNext()) {
                QRegularExpressionMatch match = i.next();
                QString name = match.capturedRef(1).toString();
                QString tagKey = match.capturedRef(2).toString();
                QString tagValue = match.capturedRef(3).toString();
                QString op = match.capturedRef(4).toString();
                QString plural = match.capturedRef(5).toString();

                // Only interested in any operator plural forms
                if (op != QLatin1String("-") || plural != QLatin1String("Y"))
                    continue;

                if (!m_categories.contains(tagKey)) {
                    QPlaceCategory category;
                    category.setCategoryId(tagKey);
                    category.setName(nameForTagKey(tagKey));
                    m_categories.insert(category.categoryId(), category);
                    m_subcategories[QString()].append(tagKey);
                    emit categoryAdded(category, QString());
                }

                QPlaceCategory category;
                category.setCategoryId(tagKey + QLatin1Char('=') + tagValue);
                category.setName(name);

                if (!m_categories.contains(category.categoryId())) {
                    m_categories.insert(category.categoryId(), category);
                    m_subcategories[tagKey].append(category.categoryId());
                    emit categoryAdded(category, tagKey);
                }
            }
        }

        parser.skipCurrentElement();
    }

    if (m_categories.isEmpty() && !m_categoryLocales.isEmpty()) {
        fetchNextCategoryLocale();
        return;
    } else {
        m_categoryLocales.clear();
    }

    foreach (QPlaceCategoriesReplyAmap *reply, m_pendingCategoriesReply)
        reply->emitFinished();
    m_pendingCategoriesReply.clear();
}

void QPlaceManagerEngineAmap::categoryReplyError()
{
    foreach (QPlaceCategoriesReplyAmap *reply, m_pendingCategoriesReply)
        reply->setError(QPlaceReply::CommunicationError, tr("Network request error"));
}

void QPlaceManagerEngineAmap::replyFinished()
{
    QPlaceReply *reply = qobject_cast<QPlaceReply *>(sender());
    if (reply)
        emit finished(reply);
}

void QPlaceManagerEngineAmap::replyError(QPlaceReply::Error errorCode, const QString &errorString)
{
    QPlaceReply *reply = qobject_cast<QPlaceReply *>(sender());
    if (reply)
        emit error(reply, errorCode, errorString);
}

void QPlaceManagerEngineAmap::fetchNextCategoryLocale()
{
    if (m_categoryLocales.isEmpty()) {
        qWarning("No locales specified to fetch categories for");
        return;
    }

    QLocale locale = m_categoryLocales.takeFirst();

    // FIXME: Categories should be cached.
    QUrl requestUrl = QUrl(SpecialPhrasesBaseUrl + locale.name().left(2).toUpper());

    m_categoriesReply = m_networkManager->get(QNetworkRequest(requestUrl));
    connect(m_categoriesReply, SIGNAL(finished()), this, SLOT(categoryReplyFinished()));
    connect(m_categoriesReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(categoryReplyError()));
}


QNetworkReply *QPlaceManagerEngineAmap::sendRequest(const QUrl &url)
{
    QUrlQuery queryItems(url);
    queryItems.addQueryItem(QStringLiteral("key"), m_apiKey);

    QUrl requestUrl = url;
    requestUrl.setQuery(queryItems);

    QNetworkRequest request;
    request.setUrl(requestUrl);

    request.setRawHeader("Accept", "application/json");
//    request.setRawHeader("Accept-Language", createLanguageString());

    return m_networkManager->get(request);
}
