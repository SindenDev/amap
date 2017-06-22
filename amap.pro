TARGET = qtgeoservices_amap
QT += location-private positioning-private network

PLUGIN_TYPE = geoservices
PLUGIN_CLASS_NAME = QGeoServiceProviderFactoryAmap
load(qt_plugin)

HEADERS += \
    qgeotilefetcheramap.h \
    qplacesearchsuggestionreplyimpl.h \
    qgeoerror_messages.h \
    qgeocodereplyamap.h \
    qgeocodingmanagerengineamap.h \
    qgeomapreplyamap.h \
    qgeoroutereplyamap.h \
    qgeoroutingmanagerengineamap.h \
    qgeoserviceproviderpluginamap.h \
    qgeotiledmapamap.h \
    qgeotiledmappingmanagerengineamap.h \
    qgeotilefetcheramap.h \
    qplacecategoriesreplyamap.h \
    qplacemanagerengineamap.h \
    qplacesearchreplyamap.h

SOURCES += \
    qplacesearchsuggestionreplyimpl.cpp \
    qgeoerror_messages.cpp \
    qgeocodereplyamap.cpp \
    qgeocodingmanagerengineamap.cpp \
    qgeomapreplyamap.cpp \
    qgeoroutereplyamap.cpp \
    qgeoroutingmanagerengineamap.cpp \
    qgeoserviceproviderpluginamap.cpp \
    qgeotiledmapamap.cpp \
    qgeotiledmappingmanagerengineamap.cpp \
    qgeotilefetcheramap.cpp \
    qplacecategoriesreplyamap.cpp \
    qplacemanagerengineamp.cpp \
    qplacesearchreplyamap.cpp


OTHER_FILES += \
    amap_plugin.json

