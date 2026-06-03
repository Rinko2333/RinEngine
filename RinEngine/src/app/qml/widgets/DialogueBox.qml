import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import RinEngine 1.0
import RinEngine.Core 1.0

// DialogueBox: 底部对话框
// 显示说话者名字 + 逐字弹出文本

Rectangle {
    id: root

    property string speaker: ""
    property string fullText: ""
    property real textSpeed: Settings.textSpeed  // ms per character
    property bool isTyping: false
    property bool textComplete: false
    property var onCompleteCallback: null

    color: Qt.rgba(0, 0, 0, 0.65)
    radius: 8
    border.color: Qt.rgba(1, 1, 1, 0.15)
    border.width: 1

    // Edge shadow — manual gradient-based shadow to avoid Qt5Compat dependency
    layer.enabled: false

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.top
        height: 12
        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.rgba(0, 0, 0, 0.0) }
            GradientStop { position: 0.5; color: Qt.rgba(0, 0, 0, 0.15) }
            GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.35) }
        }
    }

    // Speaker name label
    Rectangle {
        id: nameBox
        x: 40
        y: -height + 4
        height: nameText.height + 16
        width: nameText.width + 32
        color: Qt.rgba(0.2, 0.4, 0.8, 0.85)
        radius: 6

        Label {
            id: nameText
            anchors.centerIn: parent
            text: root.speaker
            color: "#ffffff"
            font.pixelSize: 18
            font.bold: true
        }
    }

    // Dialogue text area
    Rectangle {
        anchors.fill: parent
        anchors.margins: 16
        anchors.topMargin: 28
        color: "transparent"
        clip: true

        Text {
            id: dialogueText
            anchors.fill: parent
            text: ""
            color: "#f0f0f0"
            font.pixelSize: 22
            font.family: "Microsoft YaHei"
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignTop
            lineHeight: 1.5
        }
    }

    // "Click to continue" indicator
    Rectangle {
        id: continueIndicator
        anchors.right: parent.right
        anchors.rightMargin: 28
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 12
        width: 24
        height: 24
        radius: 12
        color: Qt.rgba(1, 1, 1, 0.15)
        visible: textComplete && !isTyping

        Text {
            anchors.centerIn: parent
            text: "▼"
            color: "#ffffff"
            font.pixelSize: 12
        }

        SequentialAnimation on opacity {
            loops: Animation.Infinite
            PropertyAnimation { to: 0.3; duration: 600 }
            PropertyAnimation { to: 1.0; duration: 600 }
        }
    }

    // Typewriter timer
    Timer {
        id: typeTimer
        interval: 40  // ms per character — adjusted by textSpeed
        repeat: true
        onTriggered: {
            var currentLen = dialogueText.text.length;
            var targetLen = root.fullText ? root.fullText.length : 0;
            if (currentLen < targetLen) {
                // Show next chunk
                var charsPerTick = Math.max(1, Math.round((100 - root.textSpeed) / 10) + 1);
                var nextLen = Math.min(currentLen + charsPerTick, targetLen);
                dialogueText.text = root.fullText.substring(0, nextLen);
            } else {
                typeTimer.stop();
                root.isTyping = false;
                root.textComplete = true;
            }
        }
    }

    // Show text with typewriter effect
    function showText(speakerName, text, callback) {
        root.speaker = speakerName || "";
        root.fullText = text;
        root.onCompleteCallback = callback;
        root.textComplete = false;

        dialogueText.text = "";
        root.isTyping = true;

        // Calculate timer interval based on text speed setting
        var speedFactor = Math.max(10, Math.min(200, root.textSpeed));
        typeTimer.interval = Math.max(10, 200 - speedFactor * 1.8);

        typeTimer.start();
    }

    // Finish typing immediately
    function finishTyping() {
        if (isTyping) {
            typeTimer.stop();
            dialogueText.text = root.fullText;
            root.isTyping = false;
            root.textComplete = true;
        } else if (textComplete) {
            // If text is already complete, clicking again does nothing here
            // The main.qml handles advancing to next command
        }
    }

    // Reset
    function clear() {
        typeTimer.stop();
        root.speaker = "";
        root.fullText = "";
        dialogueText.text = "";
        root.isTyping = false;
        root.textComplete = false;
    }
}
