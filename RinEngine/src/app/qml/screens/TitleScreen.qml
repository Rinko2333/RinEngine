import QtQuick
import QtQuick.Controls
import RinEngine 1.0
import RinEngine.Core 1.0

Item {
    id: root

    signal startGame()
    signal continueGame()
    signal loadGame()
    signal openSettings()
    signal openCgGallery()
    signal openMusicRoom()
    signal quitGame()

    // Background
    Rectangle {
        anchors.fill: parent
        color: "#1a1a2e"
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#16213e" }
            GradientStop { position: 0.5; color: "#1a1a2e" }
            GradientStop { position: 1.0; color: "#0f3460" }
        }
    }

    // Title area
    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height * 0.14
        spacing: 8

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "RinEngine"
            font.pixelSize: 72
            font.bold: true
            color: "#e94560"
            style: Text.Outline
            styleColor: "#000000"
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Visual Novel Engine"
            font.pixelSize: 22
            color: Qt.rgba(255, 255, 255, 0.6)
        }
    }

    // Main menu buttons
    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height * 0.39
        spacing: 12

        MenuButton {
            text: "开始游戏"
            width: 280
            onClicked: root.startGame()
        }

        MenuButton {
            text: "继续游戏"
            width: 280
            enabled: GameStateManager.hasAnySave()
            onClicked: {
                if (GameStateManager.slotExists(98)) {
                    GameStateManager.scheduleLoad(98);
                }
                root.continueGame();
            }
        }

        MenuButton {
            text: "加载游戏"
            width: 280
            enabled: GameStateManager.hasAnySave()
            onClicked: root.loadGame()
        }

        // Divider
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 200; height: 1
            color: Qt.rgba(1, 1, 1, 0.1)
        }

        MenuButton {
            text: "CG 鉴赏"
            width: 280
            onClicked: root.openCgGallery()
        }

        MenuButton {
            text: "音乐鉴赏"
            width: 280
            onClicked: root.openMusicRoom()
        }

        // Divider
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 200; height: 1
            color: Qt.rgba(1, 1, 1, 0.1)
        }

        MenuButton {
            text: "设置"
            width: 280
            onClicked: root.openSettings()
        }

        MenuButton {
            text: "退出"
            width: 280
            onClicked: root.quitGame()
        }
    }

    // Version info
    Text {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 16
        anchors.horizontalCenter: parent.horizontalCenter
        text: "v0.1.0 — Qt6 + QML"
        font.pixelSize: 12
        color: Qt.rgba(255, 255, 255, 0.3)
    }
}
