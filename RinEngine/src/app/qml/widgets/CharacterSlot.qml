import QtQuick
import QtQuick.Controls
import RinEngine.Core 1.0

/// CharacterSlot: 单个角色立绘槽位
/// 管理立绘显示、位置移动、入场/出场动画
Item {
    id: slotRoot

    property string positionName: ""
    property real targetX: root.width * 0.5
    property Item animTarget: charImage
    property bool active: false

    width: parent ? parent.height * 0.6 : 400
    height: parent ? parent.height : 700
    visible: false
    opacity: 0.0

    Image {
        id: charImage
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        width: parent.width
        height: parent.height
        fillMode: Image.PreserveAspectFit
        opacity: 0.0
    }

    /// 设置角色立绘图片和水平位置
    function setCharacter(url, xPos) {
        charImage.source = url;
        targetX = xPos;
        slotRoot.x = xPos - width / 2;
    }

    /// 入场动画（上滑 + 淡入）
    function animateIn(effect, dur, callback) {
        slotRoot.visible = true;
        slotRoot.opacity = 1.0;
        y = slotRoot.parent ? slotRoot.parent.height : 700;
        charImage.opacity = 0.0;

        slideY.from = y;
        slideY.to = 0;
        slideY.duration = 400;
        slideY.start();

        fadeIn.from = 0.0;
        fadeIn.to = 1.0;
        fadeIn.duration = 400;
        fadeIn.callback = callback;
        fadeIn.start();
    }

    /// 出场动画（淡出）
    function animateOut(dur, callback) {
        fadeOut.from = charImage.opacity;
        fadeOut.to = 0.0;
        fadeOut.duration = (dur || 0.3) * 1000;
        fadeOut.callback = callback;
        fadeOut.start();
    }

    /// 水平移动到目标位置
    function moveToX(x, dur, callback) {
        moveAnim.to = x - width / 2;
        moveAnim.duration = dur * 1000;
        moveAnim.callback = callback;
        moveAnim.start();
    }

    NumberAnimation {
        id: slideY; target: slotRoot; property: "y"
        easing.type: Easing.OutCubic
    }
    NumberAnimation {
        id: moveAnim; target: slotRoot; property: "x"
        easing.type: Easing.InOutQuad
        onFinished: { if (moveAnim.callback) moveAnim.callback(); }
    }
    NumberAnimation {
        id: fadeIn; target: charImage; property: "opacity"
        easing.type: Easing.InQuad
        onFinished: { if (fadeIn.callback) fadeIn.callback(); }
    }
    NumberAnimation {
        id: fadeOut; target: charImage; property: "opacity"
        easing.type: Easing.InQuad
        onFinished: { if (fadeOut.callback) fadeOut.callback(); }
    }
}
