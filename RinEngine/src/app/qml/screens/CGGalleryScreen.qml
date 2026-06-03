import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import RinEngine 1.0
import RinEngine.Core 1.0

Item {
    id: root

    signal back()

    Rectangle {
        anchors.fill: parent
        color: "#0d1117"
    }

    // Header
    RowLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 16
        height: 50

        Button {
            text: "← 返回"
            flat: true
            onClicked: root.back()
            contentItem: Text { text: parent.text; color: "#e94560"; font.pixelSize: 18 }
            background: Rectangle {
                color: parent.hovered ? Qt.rgba(1,1,1,0.05) : "transparent"
                radius: 6
            }
        }

        Item { Layout.fillWidth: true }

        Text {
            text: "CG 鉴赏"
            font.pixelSize: 22
            font.bold: true
            color: "#ffffff"
            Layout.alignment: Qt.AlignHCenter
        }

        Item { Layout.fillWidth: true }
        Item { Layout.preferredWidth: 80 }
    }

    // CG catalog — demo entries (no real assets yet)
    property var cgCatalog: [
        { "id": "cg_school",    "name": "学校"},
        { "id": "cg_garden",    "name": "花园"},
        { "id": "cg_library",   "name": "图书馆"},
        { "id": "cg_rooftop",   "name": "天台"},
        { "id": "cg_classroom", "name": "教室"},
        { "id": "cg_sunset",    "name": "夕阳"},
        { "id": "cg_night",     "name": "夜景"},
        { "id": "cg_rain",      "name": "雨天"}
    ]

    GridView {
        id: gridView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 70
        anchors.bottom: parent.bottom
        anchors.margins: 24
        cellWidth: 280
        cellHeight: 200
        clip: true
        model: cgCatalog

        delegate: Rectangle {
            width: gridView.cellWidth - 16
            height: gridView.cellHeight - 16
            color: Qt.rgba(0.1, 0.12, 0.18, 0.95)
            radius: 8
            border.color: Qt.rgba(1,1,1,0.08)
            border.width: 1

            property bool unlocked: GalleryManager.isCgUnlocked(modelData.id)

            // Thumbnail or placeholder
            Rectangle {
                anchors.fill: parent
                anchors.margins: 2
                radius: 6
                color: unlocked ? "#1a1a2e" : "#0a0a15"

                Image {
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectCrop
                    source: unlocked ? ResourceManager.getImage("cg:" + modelData.id) : ""
                    visible: unlocked && source.toString() !== ""
                }

                Column {
                    anchors.centerIn: parent
                    visible: !unlocked || parent.children[0].source.toString() === ""
                    spacing: 8

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: unlocked ? "🖼" : "🔒"
                        font.pixelSize: 36
                        color: unlocked ? "#e94560" : "#444444"
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: unlocked ? modelData.name : "???";
                        font.pixelSize: 14
                        color: unlocked ? "#cccccc" : "#555555"
                    }
                }

                // Hover highlight
                Rectangle {
                    anchors.fill: parent
                    radius: 6
                    color: Qt.rgba(1,1,1,0.03)
                    visible: mouseArea.containsMouse && unlocked
                }
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                enabled: unlocked
                cursorShape: unlocked ? Qt.PointingHandCursor : Qt.ArrowCursor
                onClicked: {
                    if (unlocked) {
                        zoomImage.source = ResourceManager.getImage("cg:" + modelData.id);
                        zoomOverlay.visible = true;
                    }
                }
            }
        }
    }

    // Zoom overlay
    Item {
        id: zoomOverlay
        anchors.fill: parent
        visible: false
        z: 10

        Rectangle {
            anchors.fill: parent
            color: Qt.rgba(0, 0, 0, 0.92)
        }

        Image {
            id: zoomImage
            anchors.centerIn: parent
            width: Math.min(parent.width * 0.9, implicitWidth)
            height: Math.min(parent.height * 0.9, implicitHeight)
            fillMode: Image.PreserveAspectFit
        }

        Text {
            anchors.top: parent.top
            anchors.topMargin: 16
            anchors.horizontalCenter: parent.horizontalCenter
            text: "点击任意位置返回"
            color: Qt.rgba(1,1,1,0.4)
            font.pixelSize: 14
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                zoomOverlay.visible = false;
                zoomImage.source = "";
            }
        }
    }
}
