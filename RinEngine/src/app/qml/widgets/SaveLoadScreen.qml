import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import RinEngine.Core 1.0

// SaveLoadScreen: 存档/读档界面

Item {
    id: root
    visible: false
    enabled: visible

    property bool saveMode: true  // true=save, false=load
    property int _refreshCounter: 0

    signal closed()

    function show() { visible = true; enabled = true; }
    function hide() { visible = false; enabled = false; }

    // Overlay
    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.75)
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: 40
        color: Qt.rgba(0.1, 0.1, 0.15, 0.95)
        radius: 12
        border.color: Qt.rgba(1, 1, 1, 0.1)
        border.width: 1

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 16

            // Header
            RowLayout {
                Layout.fillWidth: true

                Text {
                    text: root.saveMode ? "保存游戏" : "读取游戏"
                    font.pixelSize: 28
                    font.bold: true
                    color: "#ffffff"
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: root.saveMode ? "切换到读取" : "切换到保存"
                    flat: true
                    onClicked: root.saveMode = !root.saveMode
                    contentItem: Text {
                        text: parent.text
                        color: "#e94560"
                        font.pixelSize: 14
                    }
                    background: Rectangle {
                        color: parent.hovered ? Qt.rgba(1,1,1,0.1) : "transparent"
                        radius: 4
                    }
                }

                Button {
                    text: "✕"
                    flat: true
                    onClicked: root.closed()
                    contentItem: Text {
                        text: parent.text
                        color: "#ffffff"
                        font.pixelSize: 20
                    }
                    background: Rectangle {
                        color: parent.hovered ? Qt.rgba(1,1,1,0.1) : "transparent"
                        radius: 4
                    }
                }
            }

            // Save slots grid
            GridLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                columns: 4
                columnSpacing: 16
                rowSpacing: 16

                Repeater {
                    model: 20  // Show 20 slots (4x5)

                    Rectangle {
                        id: slotRect
                        width: 200
                        height: 150
                        color: Qt.rgba(0.2, 0.2, 0.3, 0.6)
                        radius: 8
                        border.color: mouseArea.containsMouse ? "#e94560" : Qt.rgba(1,1,1,0.1)
                        border.width: mouseArea.containsMouse ? 2 : 1

                        property int slotIndex: index
                        property var info: { root._refreshCounter; return GameStateManager.slotInfo(index); }
                        property bool hasData: info.exists

                        // Thumbnail
                        Image {
                            anchors.fill: parent
                            anchors.margins: 4
                            source: ""
                            fillMode: Image.PreserveAspectCrop
                            visible: false
                            opacity: 0.6
                        }

                        // Slot label
                        Text {
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 4
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: slotRect.hasData ? info.timestamp : "空槽位 " + (index + 1)
                            color: slotRect.hasData ? "#cccccc" : "#666666"
                            font.pixelSize: 11
                        }

                        // Empty slot indicator
                        Text {
                            anchors.centerIn: parent
                            text: "Slot " + (index + 1)
                            color: Qt.rgba(1,1,1,0.15)
                            font.pixelSize: 16
                            visible: !slotRect.hasData
                        }

                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                if (root.saveMode) {
                                    GameStateManager.save(slotRect.slotIndex);
                                    root._refreshCounter++;
                                } else {
                                    if (slotRect.hasData) {
                                        GameStateManager.load(slotRect.slotIndex);
                                        root.closed();
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Quick save/load buttons
            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                MenuButton {
                    text: "快速保存 (F5)"
                    Layout.preferredWidth: 180
                    onClicked: {
                        GameStateManager.quickSave();
                    }
                }

                MenuButton {
                    text: "快速读取 (F7)"
                    Layout.preferredWidth: 180
                    enabled: GameStateManager.slotExists(98)
                    onClicked: {
                        GameStateManager.quickLoad();
                        root.closed();
                    }
                }

                Item { Layout.fillWidth: true }

                MenuButton {
                    text: "关闭"
                    Layout.preferredWidth: 120
                    onClicked: root.closed()
                }
            }
        }
    }
}
