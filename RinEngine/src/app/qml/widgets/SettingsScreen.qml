import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import RinEngine 1.0
import RinEngine.Core 1.0

// SettingsScreen: 设置界面

Item {
    id: root
    visible: false
    enabled: visible

    function t(key) { L10n.localeVersion; return L10n.tr(key); }

    signal closed()
    signal fullscreenToggled(bool fs)

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.75)
    }

    Rectangle {
        anchors.centerIn: parent
        width: 520
        height: 480
        color: Qt.rgba(0.1, 0.1, 0.15, 0.95)
        radius: 12
        border.color: Qt.rgba(1, 1, 1, 0.1)
        border.width: 1

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 32
            spacing: 20

            // Title
            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: t("SETTINGS_TITLE")
                    font.pixelSize: 28
                    font.bold: true
                    color: "#ffffff"
                }
                Item { Layout.fillWidth: true }
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

            // Text speed
            RowLayout {
                Layout.fillWidth: true
                Text { text: t("SETTINGS_TEXT_SPEED"); color: "#cccccc"; font.pixelSize: 16; Layout.preferredWidth: 100 }
                Slider {
                    id: textSpeedSlider
                    Layout.fillWidth: true
                    from: 10; to: 200; value: Settings.textSpeed
                    onValueChanged: Settings.textSpeed = value
                }
                Text { text: Math.round(textSpeedSlider.value); color: "#aaaaaa"; font.pixelSize: 14; Layout.preferredWidth: 30 }
            }

            // Auto speed
            RowLayout {
                Layout.fillWidth: true
                Text { text: t("SETTINGS_AUTO_SPEED"); color: "#cccccc"; font.pixelSize: 16; Layout.preferredWidth: 100 }
                Slider {
                    id: autoSpeedSlider
                    Layout.fillWidth: true
                    from: 1; to: 10; value: Settings.autoSpeed
                    onValueChanged: Settings.autoSpeed = value
                }
                Text { text: Math.round(autoSpeedSlider.value) + "s"; color: "#aaaaaa"; font.pixelSize: 14; Layout.preferredWidth: 30 }
            }

            // Separator
            Rectangle { Layout.fillWidth: true; height: 1; color: Qt.rgba(1,1,1,0.1) }

            // BGM Volume
            RowLayout {
                Layout.fillWidth: true
                Text { text: t("SETTINGS_BGM"); color: "#cccccc"; font.pixelSize: 16; Layout.preferredWidth: 100 }
                Slider {
                    id: bgmSlider
                    Layout.fillWidth: true
                    from: 0; to: 100; value: Settings.bgmVolume
                    onValueChanged: Settings.bgmVolume = value
                }
                Text { text: Math.round(bgmSlider.value); color: "#aaaaaa"; font.pixelSize: 14; Layout.preferredWidth: 30 }
            }

            // SE Volume
            RowLayout {
                Layout.fillWidth: true
                Text { text: t("SETTINGS_SE"); color: "#cccccc"; font.pixelSize: 16; Layout.preferredWidth: 100 }
                Slider {
                    id: seSlider
                    Layout.fillWidth: true
                    from: 0; to: 100; value: Settings.seVolume
                    onValueChanged: Settings.seVolume = value
                }
                Text { text: Math.round(seSlider.value); color: "#aaaaaa"; font.pixelSize: 14; Layout.preferredWidth: 30 }
            }

            // Voice Volume
            RowLayout {
                Layout.fillWidth: true
                Text { text: t("SETTINGS_VOICE"); color: "#cccccc"; font.pixelSize: 16; Layout.preferredWidth: 100 }
                Slider {
                    id: voiceSlider
                    Layout.fillWidth: true
                    from: 0; to: 100; value: Settings.voiceVolume
                    onValueChanged: Settings.voiceVolume = value
                }
                Text { text: Math.round(voiceSlider.value); color: "#aaaaaa"; font.pixelSize: 14; Layout.preferredWidth: 30 }
            }

            // Separator
            Rectangle { Layout.fillWidth: true; height: 1; color: Qt.rgba(1,1,1,0.1) }

            // Fullscreen
            RowLayout {
                Layout.fillWidth: true
                Text { text: t("SETTINGS_FULLSCREEN"); color: "#cccccc"; font.pixelSize: 16; Layout.preferredWidth: 100 }
                Switch {
                    checked: Settings.fullscreen
                    onCheckedChanged: {
                        Settings.fullscreen = checked;
                        root.fullscreenToggled(checked);
                    }
                }
            }

            // Language
            RowLayout {
                Layout.fillWidth: true
                Text { text: t("SETTINGS_LANGUAGE"); color: "#cccccc"; font.pixelSize: 16; Layout.preferredWidth: 100 }
                ComboBox {
                    model: ["中文", "English"]
                    currentIndex: Settings.language === "zh" ? 0 : 1
                    onCurrentIndexChanged: {
                        Settings.language = currentIndex === 0 ? "zh" : "en";
                    }
                }
            }

            Item { Layout.fillHeight: true }

            // Close button
            MenuButton {
                text: t("SETTINGS_CLOSE")
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 200
                onClicked: root.closed()
            }
        }
    }

    function showFullscreen() {
        // This needs access to the window, handled from parent
    }

    function showNormal() {
    }
}
