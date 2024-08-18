TARGET = qtgeoservices_amap
QT += location-private positioning-private network

PLUGIN_TYPE = geoservices
PLUGIN_CLASS_NAME = QGeoServiceProviderFactoryAmap
greaterThan(QT_MAJOR_VERSION, 5){
    TEMPLATE = lib
    CONFIG += plugin c++17
}else{
    load(qt_plugin)
}
!isEmpty(target.path): INSTALLS += target

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

DISTFILES += \
    README.md

