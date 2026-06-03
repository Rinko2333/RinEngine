import QtQuick
import QtQuick.Controls
import RinEngine 1.0
import RinEngine.Core 1.0

// BackgroundLayer: 全屏背景显示与切换
// 支持淡入淡出转换

Item {
    id: root

    property alias currentSource: frontImg.source
    property bool transitioning: false

    // Two overlapping images for crossfade
    Image {
        id: backImg
        anchors.fill: parent
        fillMode: Image.PreserveAspectCrop
        opacity: 0.0
        visible: opacity > 0
    }

    Image {
        id: frontImg
        anchors.fill: parent
        fillMode: Image.PreserveAspectCrop
        opacity: 1.0
        visible: true
    }

    // Show a background with optional transition
    function show(bgId, transition, duration, callback) {
        if (transitioning) {
            if (callback) callback();
            return;
        }

        var imageUrl = ResourceManager.getImage("bg:" + bgId);

        if (frontImg.source.toString() === "" || transition === "none") {
            frontImg.source = imageUrl;
            frontImg.opacity = 1.0;
            if (callback) callback();
            return;
        }

        transitioning = true;

        if (transition === "fade") {
            backImg.source = frontImg.source;
            backImg.opacity = 1.0;
            frontImg.source = imageUrl;
            frontImg.opacity = 0.0;

            fadeAnimTarget = frontImg;
            fadeAnimFrom = 0.0;
            fadeAnimTo = 1.0;
            fadeAnimDuration = (duration || 1.0) * 1000;
            fadeAnimCb = callback;
            fadeTimer.start();
        } else {
            frontImg.source = imageUrl;
            frontImg.opacity = 1.0;
            transitioning = false;
            if (callback) callback();
        }
    }

    // Manual fade animation using Timer (avoids NumberAnimation.callback issues)
    property var fadeAnimTarget: null
    property real fadeAnimFrom: 0
    property real fadeAnimTo: 1
    property real fadeAnimDuration: 1000
    property var fadeAnimCb: null
    property real fadeAnimElapsed: 0

    Timer {
        id: fadeTimer
        interval: 16
        repeat: true
        onTriggered: {
            if (!root.fadeAnimTarget) { stop(); return; }
            root.fadeAnimElapsed += 16;
            var p = Math.min(1.0, root.fadeAnimElapsed / root.fadeAnimDuration);
            root.fadeAnimTarget.opacity = root.fadeAnimFrom + (root.fadeAnimTo - root.fadeAnimFrom) * p;
            if (p >= 1.0) {
                stop();
                root.fadeAnimTarget.opacity = root.fadeAnimTo;
                backImg.opacity = 0.0;
                root.transitioning = false;
                if (root.fadeAnimCb) {
                    var cb = root.fadeAnimCb;
                    root.fadeAnimCb = null;
                    cb();
                }
            }
        }
    }

    // Background color fill while no image loaded
    Rectangle {
        id: bgFill
        anchors.fill: parent
        color: "#1a1a2e"
        z: -1
    }
}
