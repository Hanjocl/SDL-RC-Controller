import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Universal
import QtQuick.Layouts

ApplicationWindow {
    visible: true
    width: 1024
    height: 768
    id: window
    title: qsTr("SDL Controller Input - Example")

    Universal.theme: Universal.Dark

    Rectangle {
        anchors.fill: parent
        color: "#2b2b2b"

    }
}
