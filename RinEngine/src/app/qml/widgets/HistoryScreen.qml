import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import RinEngine.Core 1.0

// HistoryScreen: 对话历史回看

Item {
    id: root
    visible: false
    enabled: visible

    signal closed()

    function show() { visible = true; enabled = true; }
    function hide() { visible = false; enabled = false; }

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.8)
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: 40
        color: Qt.rgba(0.08, 0.08, 0.12, 0.95)
        radius: 12
        border.color: Qt.rgba(1, 1, 1, 0.1)
        border.width: 1

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 12

            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: "对话历史"
                    font.pixelSize: 24
                    font.bold: true
                    color: "#ffffff"
                }
                Item { Layout.fillWidth: true }
                Text {
                    text: "共 " + historyList.count + " 条"
                    color: "#888888"
                    font.pixelSize: 14
                }
                Button {
                    text: "✕"
                    flat: true
                    onClicked: root.closed()
                    contentItem: Text { text: parent.text; color: "#ffffff"; font.pixelSize: 20 }
                    background: Rectangle {
                        color: parent.hovered ? Qt.rgba(1,1,1,0.1) : "transparent"
                        radius: 4
                    }
                }
            }

            ListView {
                id: historyList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: ScriptRunner.dialogueHistory
                spacing: 8
                ScrollBar.vertical: ScrollBar {}

                delegate: Rectangle {
                    width: historyList.width
                    height: entryText.height + 20
                    color: Qt.rgba(1, 1, 1, 0.03)
                    radius: 4

                    Text {
                        id: entryText
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.right: parent.right
                        anchors.rightMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData
                        color: "#d0d0d0"
                        font.pixelSize: 16
                        wrapMode: Text.WordWrap
                    }
                }

                // Scroll to bottom on new entries
                onCountChanged: {
                    if (count > 0) currentIndex = count - 1;
                }
            }
        }
    }
}
