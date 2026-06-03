import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import RinEngine 1.0
import RinEngine.Core 1.0

Item {
    id: root

    signal back()

    property string currentTrack: ""
    property bool isPlaying: false

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
            onClicked: {
                AudioManager.stopBgm(200);
                root.back();
            }
            contentItem: Text { text: parent.text; color: "#e94560"; font.pixelSize: 18 }
            background: Rectangle {
                color: parent.hovered ? Qt.rgba(1,1,1,0.05) : "transparent"
                radius: 6
            }
        }

        Item { Layout.fillWidth: true }

        Text {
            text: "音乐鉴赏"
            font.pixelSize: 22
            font.bold: true
            color: "#ffffff"
            Layout.alignment: Qt.AlignHCenter
        }

        Item { Layout.fillWidth: true }
        Item { Layout.preferredWidth: 80 }
    }

    // BGM catalog
    property var trackCatalog: [
        { "id": "morning",   "name": "清晨",     "desc": "轻松愉快的早晨旋律" },
        { "id": "everyday",  "name": "日常",     "desc": "平静的日常生活" },
        { "id": "school",    "name": "校园",     "desc": "青春活力的校园时光" },
        { "id": "sad",       "name": "悲伤",     "desc": "忧伤的回忆片段" },
        { "id": "suspense",  "name": "悬疑",     "desc": "紧张不安的时刻" },
        { "id": "romance",   "name": "恋爱",     "desc": "甜蜜的心动瞬间" },
        { "id": "battle",    "name": "战斗",     "desc": "激烈的对抗场景" },
        { "id": "ending",    "name": "结局",     "desc": "故事的终章" }
    ]

    ListView {
        id: listView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 70
        anchors.bottom: parent.bottom
        anchors.margins: 24
        spacing: 8
        clip: true
        model: trackCatalog

        delegate: Rectangle {
            width: listView.width
            height: 64
            color: Qt.rgba(0.1, 0.12, 0.18, 0.95)
            radius: 8
            border.color: Qt.rgba(1,1,1,0.06)
            border.width: 1

            property bool unlocked: GalleryManager.isBgmUnlocked(modelData.id)
            property bool active: root.currentTrack === modelData.id && root.isPlaying

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 12

                // Play button or lock icon
                Button {
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    enabled: unlocked
                    flat: true
                    onClicked: {
                        if (root.currentTrack === modelData.id && root.isPlaying) {
                            AudioManager.stopBgm(200);
                            root.currentTrack = "";
                            root.isPlaying = false;
                        } else {
                            AudioManager.playBgm(modelData.id, 80);
                            root.currentTrack = modelData.id;
                            root.isPlaying = true;
                        }
                    }
                    contentItem: Text {
                        text: {
                            if (!unlocked) return "🔒";
                            if (active) return "⏸";
                            return "▶";
                        }
                        font.pixelSize: 20
                        color: unlocked ? (active ? "#e94560" : "#cccccc") : "#444444"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        radius: 20
                        color: parent.hovered && unlocked ? Qt.rgba(1,1,1,0.08) : "transparent"
                    }
                }

                Column {
                    Layout.fillWidth: true
                    spacing: 2

                    Text {
                        text: unlocked ? modelData.name : "???";
                        font.pixelSize: 16
                        font.bold: true
                        color: unlocked ? "#ffffff" : "#555555"
                    }

                    Text {
                        text: unlocked ? modelData.desc : "尚未解锁";
                        font.pixelSize: 12
                        color: unlocked ? "#888888" : "#444444"
                    }
                }

                // Active indicator
                Rectangle {
                    visible: active
                    Layout.preferredWidth: 6
                    Layout.preferredHeight: 6
                    radius: 3
                    color: "#e94560"
                }
            }

            // Hover
            Rectangle {
                anchors.fill: parent
                radius: 8
                color: Qt.rgba(1,1,1,0.02)
                visible: mouseArea.containsMouse
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: unlocked ? Qt.PointingHandCursor : Qt.ArrowCursor
            }
        }
    }

    // Empty state
    Text {
        anchors.centerIn: parent
        visible: trackCatalog.length === 0
        text: "暂无曲目"
        font.pixelSize: 18
        color: "#666666"
    }
}
