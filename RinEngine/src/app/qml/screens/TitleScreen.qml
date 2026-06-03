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
    signal quitGame()

    // Background
    Rectangle {
        anchors.fill: parent
        color: "#1a1a2e"
    }

    // Decorative gradient
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#16213e" }
            GradientStop { position: 0.5; color: "#1a1a2e" }
            GradientStop { position: 1.0; color: "#0f3460" }
        }
    }

    // Title
    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height * 0.18
        text: "RinEngine"
        font.pixelSize: 72
        font.bold: true
        color: "#e94560"
        style: Text.Outline
        styleColor: "#000000"
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height * 0.28
        text: "Visual Novel Engine"
        font.pixelSize: 22
        color: Qt.rgba(255, 255, 255, 0.6)
    }

    // Menu buttons
    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height * 0.45
        spacing: 16

        MenuButton {
            text: "开始游戏"
            width: 280
            onClicked: root.startGame()
        }

        MenuButton {
            text: "继续游戏"
            width: 280
            enabled: GameStateManager.slotExists(98) || GameStateManager.listSaves().length > 0
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
            enabled: GameStateManager.listSaves().length > 0
            onClicked: root.loadGame()
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
