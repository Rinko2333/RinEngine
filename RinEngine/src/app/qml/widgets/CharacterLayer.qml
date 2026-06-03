import QtQuick
import QtQuick.Controls
import RinEngine 1.0
import RinEngine.Core 1.0

/// CharacterLayer: 立绘显示层
/// 预声明5个槽位，避免动态创建的竞态条件
Item {
    id: root

    // Animation player instance
    AnimationPlayer {
        id: animPlayer
    }

    readonly property var positionX: ({
        "left":         102,
        "center-left":  358,
        "center":       640,
        "center-right": 922,
        "right":        1178
    })

    // z-order: rightmost on top so they overlap left characters
    readonly property var positionOrder: ({
        "left":         1,
        "center-left":  2,
        "center":       3,
        "center-right": 4,
        "right":        5
    })

    // Pre-declared slots — always exist, no race condition
    property list<Item> slotList: [slot0, slot1, slot2, slot3, slot4]

    // Slot 0
    Item {
        id: slot0
        property string positionName: ""
        property alias charImage: slotImg0
        width: parent ? parent.height * 0.85 : 400
        height: parent ? parent.height : 700
        visible: false
        y: parent ? parent.height : 700
        Image {
            id: slotImg0
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            width: parent.width; height: parent.height
            fillMode: Image.PreserveAspectFit
            opacity: 0.0
        }
    }

    // Slot 1
    Item {
        id: slot1
        property string positionName: ""
        property alias charImage: slotImg1
        width: parent ? parent.height * 0.85 : 400
        height: parent ? parent.height : 700
        visible: false
        y: parent ? parent.height : 700
        Image {
            id: slotImg1
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            width: parent.width; height: parent.height
            fillMode: Image.PreserveAspectFit
            opacity: 0.0
        }
    }

    // Slot 2
    Item {
        id: slot2
        property string positionName: ""
        property alias charImage: slotImg2
        width: parent ? parent.height * 0.85 : 400
        height: parent ? parent.height : 700
        visible: false
        y: parent ? parent.height : 700
        Image {
            id: slotImg2
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            width: parent.width; height: parent.height
            fillMode: Image.PreserveAspectFit
            opacity: 0.0
        }
    }

    // Slot 3
    Item {
        id: slot3
        property string positionName: ""
        property alias charImage: slotImg3
        width: parent ? parent.height * 0.85 : 400
        height: parent ? parent.height : 700
        visible: false
        y: parent ? parent.height : 700
        Image {
            id: slotImg3
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            width: parent.width; height: parent.height
            fillMode: Image.PreserveAspectFit
            opacity: 0.0
        }
    }

    // Slot 4
    Item {
        id: slot4
        property string positionName: ""
        property alias charImage: slotImg4
        width: parent ? parent.height * 0.85 : 400
        height: parent ? parent.height : 700
        visible: false
        y: parent ? parent.height : 700
        Image {
            id: slotImg4
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            width: parent.width; height: parent.height
            fillMode: Image.PreserveAspectFit
            opacity: 0.0
        }
    }

    // -- Slot searching --

    function slotAtPosition(posName) {
        for (var i = 0; i < 5; i++) {
            var s = slotList[i];
            if (!s) continue;
            if (s.positionName === posName && s.visible) return s;
        }
        return null;
    }

    function hiddenSlot() {
        for (var i = 0; i < 5; i++) {
            var s = slotList[i];
            if (!s) continue;
            if (!s.visible) return s;
        }
        return slotList[0]; // fallback: reuse first
    }

    // -- Public API --

    function show(charName, face, posName, effect, effectDuration, callback) {
        var xPos = positionX[posName] || 640;
        var imageUrl = ResourceManager.getImage("char:" + charName + "_" + face);

        var slot = slotAtPosition(posName);
        if (!slot) slot = hiddenSlot();
        if (!slot) {
            Logger.warn("CharacterLayer", "No slot available");
            if (callback) callback();
            return;
        }

        slot.positionName = posName;
        slot.visible = true;
        slot.z = positionOrder[posName] || slot.z;
        slot.x = xPos - slot.width / 2;

        if (imageUrl.toString() !== "") {
            slot.charImage.source = imageUrl;
        }

        // Entrance animation
        slot.y = parent.height;
        slot.charImage.opacity = 0.0;

        slideIn.startWith(slot);
        fadeIn.startWith(slot.charImage, callback, effect, effectDuration);
    }

    function hidePosition(posName, callback) {
        var slot = slotAtPosition(posName);
        if (slot) {
            fadeOut.startWith(slot.charImage, function() {
                slot.visible = false;
                slot.positionName = "";
                if (callback) callback();
            });
        } else if (callback) {
            callback();
        }
    }

    function moveTo(fromPos, toPos, duration, callback) {
        var slot = slotAtPosition(fromPos);
        if (slot) {
            var targetX = (positionX[toPos] || 640) - slot.width / 2;
            slot.positionName = toPos;
            slideX.startWith(slot, targetX, duration, callback);
        } else if (callback) {
            callback();
        }
    }

    // -- Reusable animations (procedurally created to work with dynamic targets) --

    property var slideInTgt: null
    property var slideInCb: null
    property var fadeInTgt: null
    property var fadeInCb: null
    property var fadeInEffect: ""
    property var fadeInDur: 0
    property var fadeOutTgt: null
    property var fadeOutCb: null
    property var slideXTgt: null
    property var slideXCb: null

    Timer {
        id: slideIn
        property var target: null
        property var cb: null
        interval: 16; repeat: true
        property real startY: 0
        property real elapsed: 0
        property real duration: 400
        function startWith(tgt) {
            target = tgt; cb = null;
            startY = tgt.y;
            elapsed = 0; duration = 400;
            restart();
        }
        onTriggered: {
            if (!target) { stop(); if (cb) cb(); return; }
            elapsed += 16;
            var p = Math.min(1.0, elapsed / duration);
            // easeOutCubic
            var ep = 1.0 - Math.pow(1.0 - p, 3);
            target.y = startY * (1.0 - ep);
            if (p >= 1.0) { stop(); target.y = 0; if (cb) cb(); }
        }
    }

    Timer {
        id: fadeIn
        property var target: null
        property var cb: null
        property string effect: ""
        property real effectDur: 0
        interval: 16; repeat: true
        property real startOp: 0
        property real elapsed: 0
        property real duration: 400
        function startWith(tgt, c, eff, ed) {
            target = tgt; cb = c; effect = eff; effectDur = ed;
            startOp = tgt.opacity;
            elapsed = 0; duration = 400;
            restart();
        }
        onTriggered: {
            if (!target) { stop(); return; }
            elapsed += 16;
            var p = Math.min(1.0, elapsed / duration);
            target.opacity = startOp + (1.0 - startOp) * p * p;
            if (p >= 1.0) {
                stop();
                target.opacity = 1.0;
                if (effect && effect !== "none") {
                    animPlayer.play(target, effect, effectDur || 0.3, cb);
                } else if (cb) {
                    cb();
                }
            }
        }
    }

    Timer {
        id: fadeOut
        property var target: null
        property var cb: null
        interval: 16; repeat: true
        property real startOp: 0
        property real elapsed: 0
        property real duration: 300
        function startWith(tgt, c) {
            target = tgt; cb = c;
            startOp = tgt.opacity;
            elapsed = 0; duration = 300;
            restart();
        }
        onTriggered: {
            if (!target) { stop(); return; }
            elapsed += 16;
            var p = Math.min(1.0, elapsed / duration);
            target.opacity = startOp * (1.0 - p * p);
            if (p >= 1.0) { stop(); target.opacity = 0.0; if (cb) cb(); }
        }
    }

    Timer {
        id: slideX
        property var target: null
        property var cb: null
        property real startX: 0
        property real endX: 0
        property real elapsed: 0
        property real duration: 500
        interval: 16; repeat: true
        function startWith(tgt, ex, dur, c) {
            target = tgt; cb = c;
            startX = tgt.x; endX = ex;
            elapsed = 0; duration = dur * 1000;
            restart();
        }
        onTriggered: {
            if (!target) { stop(); return; }
            elapsed += 16;
            var p = Math.min(1.0, elapsed / duration);
            var ep = p < 0.5 ? 2*p*p : -1+(4-2*p)*p;
            target.x = startX + (endX - startX) * ep;
            if (p >= 1.0) { stop(); target.x = endX; if (cb) cb(); }
        }
    }
}
