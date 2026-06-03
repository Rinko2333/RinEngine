import QtQuick
import QtQuick.Controls

// Reusable menu button with hover effect
Button {
    id: btn
    property alias shadowColor: shadow.color

    background: Rectangle {
        id: bg
        color: btn.hovered ? Qt.rgba(0.5, 0.5, 0.7, 0.3) : Qt.rgba(0.2, 0.2, 0.3, 0.5)
        radius: 8
        border.color: btn.hovered ? "#e94560" : Qt.rgba(1, 1, 1, 0.15)
        border.width: 1

        Rectangle {
            id: shadow
            anchors.fill: parent
            anchors.topMargin: 2
            color: Qt.rgba(0, 0, 0, 0.2)
            radius: 8
            z: -1
        }
    }

    contentItem: Text {
        text: btn.text
        color: btn.enabled ? "#ffffff" : "#666666"
        font.pixelSize: 20
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
