import QtQuick

// AnimationPlayer: 动画播放管理器
// 不使用 Qt.createQmlObject（JS变量在字符串中不可达），
// 而是用预定义的 SequentialAnimation + 动态设置目标

Item {
    id: root

    // 动画完成回调
    property var animCallback: null

    // 预定义的 bounce 动画模板
    SequentialAnimation {
        id: bounceAnim
        property var animTarget: null
        property real originY: 0
        property real step1Y: 0
        property real step2Y: 0

        PropertyAnimation {
            id: bounceUp
            target: bounceAnim.animTarget
            property: "y"
            duration: 150
            easing.type: Easing.OutQuad
        }
        PropertyAnimation {
            id: bounceDown
            target: bounceAnim.animTarget
            property: "y"
            duration: 350
            easing.type: Easing.OutBounce
        }

        onFinished: {
            if (root.animCallback) {
                var cb = root.animCallback;
                root.animCallback = null;
                cb();
            }
        }
    }

    // 预定义的 shake 动画模板
    SequentialAnimation {
        id: shakeAnim
        property var animTarget: null
        property real originX: 0
        property real step1: 0
        property real step2: 0
        property real step3: 0
        property real step4: 0
        property real step5: 0

        PropertyAnimation { target: shakeAnim.animTarget; property: "x"; duration: 50 }
        PropertyAnimation { target: shakeAnim.animTarget; property: "x"; duration: 50 }
        PropertyAnimation { target: shakeAnim.animTarget; property: "x"; duration: 50 }
        PropertyAnimation { target: shakeAnim.animTarget; property: "x"; duration: 50 }
        PropertyAnimation { target: shakeAnim.animTarget; property: "x"; duration: 50 }
        PropertyAnimation { target: shakeAnim.animTarget; property: "x"; duration: 250 }

        onFinished: {
            if (root.animCallback) {
                var cb = root.animCallback;
                root.animCallback = null;
                cb();
            }
        }
    }

    // 预定义的 pulse 动画模板
    SequentialAnimation {
        id: pulseAnim
        property var animTarget: null
        property real fromScale: 1.0
        property real toScale: 1.05

        PropertyAnimation {
            target: pulseAnim.animTarget; property: "scale"
            duration: 200; easing.type: Easing.InOutQuad
        }
        PropertyAnimation {
            target: pulseAnim.animTarget; property: "scale"
            duration: 300; easing.type: Easing.InOutQuad
        }

        onFinished: {
            if (root.animCallback) {
                var cb = root.animCallback;
                root.animCallback = null;
                cb();
            }
        }
    }

    // Play an animation on a target element
    function play(target, preset, duration, callback) {
        if (!target || !preset) {
            if (callback) callback();
            return;
        }

        root.animCallback = callback;

        switch (preset) {
        case "bounce":
            bounceAnim.animTarget = target;
            bounceAnim.originY = target.y;
            bounceUp.to = target.y - 30;
            bounceUp.duration = (duration || 0.3) * 300;
            bounceDown.to = target.y;
            bounceDown.duration = (duration || 0.3) * 700;
            bounceAnim.start();
            break;

        case "shake": {
            var ox = target.x;
            shakeAnim.animTarget = target;
            shakeAnim.originX = ox;
            var props = shakeAnim.children || [];
            var targets = [ox-8, ox+8, ox-6, ox+6, ox-3, ox];
            var durs = [(duration||0.3)*167, (duration||0.3)*167, (duration||0.3)*167, (duration||0.3)*167, (duration||0.3)*167, (duration||0.3)*332];
            for (var si = 0; si < 6 && si < props.length; si++) {
                if (props[si] instanceof PropertyAnimation) {
                    props[si].to = targets[si];
                    props[si].duration = durs[si];
                }
            }
            shakeAnim.start();
            break;
        }

        case "pulse":
            pulseAnim.animTarget = target;
            pulseAnim.toScale = 1.05;
            pulseAnim.children[0].to = 1.05;
            pulseAnim.children[0].duration = (duration || 0.3) * 400;
            pulseAnim.children[1].to = 1.0;
            pulseAnim.children[1].duration = (duration || 0.3) * 600;
            pulseAnim.start();
            break;

        default:
            root.animCallback = null;
            if (callback) callback();
            break;
        }
    }
}
