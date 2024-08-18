import QtQuick 2.12
import QtQuick.Window 2.12
import QtPositioning 5.12
import QtLocation 5.12

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Hello World")
    Component.onCompleted: {
        for(var i in m_MapPlugin.availableServiceProviders){
            var it = m_MapPlugin.availableServiceProviders[i]
            console.log("Available Service Provider:", it)
        }
    }

    Map{
        anchors.fill:parent
        plugin: Plugin{
            id:m_MapPlugin;
            name: "amap";
        }
    }
}
