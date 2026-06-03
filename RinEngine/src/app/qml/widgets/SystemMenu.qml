import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import RinEngine.Core 1.0

// SystemMenu: ESC弹出覆盖菜单

Item {
    id: root
    visible: false
    enabled: visible

    signal resumeGame()
    signal saveGame()
    signal loadGame()
    signal openSettings()
    signal returnToTitle()
    signal quitGame()

    function show() { visible = true; enabled = true; }
    function hide() { visible = false; enabled = false; }
    function toggle() { visible ? hide() : show(); }

    // Overlay
    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.6)
    }

    // Menu panel
    Rectangle {
        anchors.centerIn: parent
        width: 400
        height: menuLayout.height + 60
        color: Qt.rgba(0.1, 0.1, 0.15, 0.95)
        radius: 12
        border.color: Qt.rgba(1, 1, 1, 0.1)
        border.width: 1

        ColumnLayout {
            id: menuLayout
            anchors.centerIn: parent
            spacing: 12
            width: parent.width - 60

            Text {
                Layout.alignment: Qt.AlignHCenter
                text: "系统菜单"
                font.pixelSize: 24
                font.bold: true
                color: "#ffffff"
                bottomPadding: 8
            }

            MenuButton {
                text: "继续游戏"
                Layout.fillWidth: true
                onClicked: root.resumeGame()
            }

            MenuButton {
                text: "保存"
                Layout.fillWidth: true
                enabled: ScriptRunner.canSave
                onClicked: root.saveGame()
            }

            MenuButton {
                text: "读取"
                Layout.fillWidth: true
                onClicked: root.loadGame()
            }

            MenuButton {
                text: "设置"
                Layout.fillWidth: true
                onClicked: root.openSettings()
            }

            MenuButton {
                text: "回到标题"
                Layout.fillWidth: true
                onClicked: root.returnToTitle()
            }

            MenuButton {
                text: "退出游戏"
                Layout.fillWidth: true
                onClicked: root.quitGame()
            }
        }
    }
}
