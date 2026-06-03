import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import RinEngine.Core 1.0

// ChoiceBox: 选项列表
// 接收 ScriptRunner 的 showChoices 信号并显示

Item {
    id: root
    visible: false
    enabled: visible

    property var currentOptions: []

    // Semi-transparent overlay
    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.4)
    }

    ColumnLayout {
        id: choiceList
        anchors.centerIn: parent
        spacing: 12
        width: parent.width * 0.5

        Repeater {
            id: choiceRepeater
            model: root.currentOptions

            delegate: Button {
                id: choiceBtn
                Layout.fillWidth: true
                Layout.preferredHeight: 52

                property int optionIndex: index

                property string optionText: modelData ? (Array.isArray(modelData) ? modelData[0] : "") : ""
                property string optionJump: modelData ? (Array.isArray(modelData) && modelData.length > 1 ? modelData[1] : "") : ""
                property string optionCond: modelData ? (Array.isArray(modelData) && modelData.length > 2 ? modelData[2] : "") : ""
                property bool conditionMet: {
                    if (optionCond === "") return true;
                    return VariableManager.evaluate(optionCond);
                }

                text: optionText
                enabled: conditionMet
                focusPolicy: Qt.NoFocus
                opacity: conditionMet ? 1.0 : 0.3
                visible: conditionMet || true  // hide unmet conditions? set to conditionMet to hide

                background: Rectangle {
                    color: choiceBtn.hovered ? Qt.rgba(0.3, 0.5, 0.9, 0.8) :
                           (choiceBtn.activeFocus ? Qt.rgba(0.2, 0.4, 0.8, 0.6) :
                            Qt.rgba(0.1, 0.1, 0.1, 0.7))
                    radius: 6
                    border.color: choiceBtn.hovered ? "#6699ff" : "#444444"
                    border.width: 1
                }

                contentItem: Text {
                    text: choiceBtn.text
                    color: choiceBtn.enabled ? "#ffffff" : "#666666"
                    font.pixelSize: 20
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    if (conditionMet) {
                        root.hide();
                        ScriptRunner.selectChoice(optionIndex);
                    }
                }
            }
        }
    }

    function show(options) {
        root.currentOptions = options;
        root.visible = true;
        root.enabled = true;
    }

    function hide() {
        root.visible = false;
        root.enabled = false;
        root.currentOptions = [];
    }
}
