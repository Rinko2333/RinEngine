import QtQuick
import QtQuick.Controls
import RinEngine 1.0
import RinEngine.Core 1.0

Window {
    id: root
    title: "Rin"

    readonly property int gameWidth: 1280
    readonly property int gameHeight: 720

    property real scaleFactor: 1.0
    property real offsetX: 0
    property real offsetY: 0

    function recalcLayout() {
        var sx = root.width / gameWidth;
        var sy = root.height / gameHeight;
        scaleFactor = Math.min(sx, sy);
        offsetX = (root.width - gameWidth * scaleFactor) / 2;
        offsetY = (root.height - gameHeight * scaleFactor) / 2;
    }

    width: 1280
    height: 720
    minimumWidth: 640
    minimumHeight: 360
    visible: true
    color: "#000000"

    onWidthChanged: recalcLayout()
    onHeightChanged: recalcLayout()

    Rectangle { x: 0; y: 0;
        width: root.offsetX; height: root.height
        color: "#000000"; visible: root.offsetX > 0 }
    Rectangle { x: root.width - root.offsetX; y: 0;
        width: root.offsetX; height: root.height
        color: "#000000"; visible: root.offsetX > 0 }
    Rectangle { x: 0; y: 0;
        width: root.width; height: root.offsetY
        color: "#000000"; visible: root.offsetY > 0 }
    Rectangle { x: 0; y: root.height - root.offsetY;
        width: root.width; height: root.offsetY
        color: "#000000"; visible: root.offsetY > 0 }

    Item {
        id: gameCanvas
        x: root.offsetX; y: root.offsetY
        width: gameWidth; height: gameHeight
        transform: Scale {
            origin.x: 0; origin.y: 0
            xScale: root.scaleFactor; yScale: root.scaleFactor
        }

        StackView {
            id: sceneStack
            anchors.fill: parent
            initialItem: titleScreenComponent
        }
    }

    Component {
        id: titleScreenComponent
        TitleScreen {
            width: gameWidth
            height: gameHeight
            onStartGame: {
                ScriptRunner.loadScript("scripts/demo/script.json");
                sceneStack.push(gameScreenComponent);
            }
            onContinueGame: {
                sceneStack.push(gameScreenComponent);
            }
            onLoadGame: {
                ScriptRunner.loadScript("scripts/demo/script.json");
                sceneStack.push(gameScreenComponent, { "openLoadScreen": true });
            }
            onOpenSettings: { settingsOverlay.open(); }
            onOpenCgGallery: { sceneStack.push(cgGalleryComponent); }
            onOpenMusicRoom: { sceneStack.push(musicRoomComponent); }
            onQuitGame: { Qt.quit(); }
        }
    }

    Component {
        id: cgGalleryComponent
        CGGalleryScreen {
            width: gameWidth
            height: gameHeight
            onBack: { sceneStack.pop(); }
        }
    }

    Component {
        id: musicRoomComponent
        MusicRoomScreen {
            width: gameWidth
            height: gameHeight
            onBack: { sceneStack.pop(); }
        }
    }

    Component {
        id: gameScreenComponent
        Item {
            id: gameScreen
            width: gameWidth
            height: gameHeight

            BackgroundLayer { id: bgLayer; anchors.fill: parent }
            CharacterLayer { id: charLayer; anchors.fill: parent }

            ChoiceBox { id: choiceBox; anchors.fill: parent; z: 20 }

            DialogueBox {
                id: dialogueBox
                x: 0; y: gameHeight * 0.72
                width: gameWidth; height: gameHeight * 0.28; z: 10
            }

            Connections {
                target: ScriptRunner

                function onShowBackground(bgId, tr, dur) {
                    bgLayer.show(bgId, tr, dur, function() { ScriptRunner.advance(); });
                }
                function onShowCharacter(n, f, p, e, ed) {
                    charLayer.show(n, f, p, e, ed, function() { ScriptRunner.advance(); });
                }
                function onHideCharacter(p) {
                    charLayer.hidePosition(p, function() { ScriptRunner.advance(); });
                }
                function onMoveCharacter(f, t, d) {
                    charLayer.moveTo(f, t, d, function() { ScriptRunner.advance(); });
                }
                function onShowDialogue(s, t) {
                    dialogueBox.showText(s, t, function() {});
                }
                function onShowChoices(o) { choiceBox.show(o); }
                function onHideChoices() { choiceBox.hide(); }
                function onPlayBgm(i, v) { AudioManager.playBgm(i, v); }
                function onPlaySe(i) { AudioManager.playSe(i); }
                function onPlayVoice(i) { AudioManager.playVoice(i); }
                function onPlayVideo(i) { videoPlayer.play(i); }
                function onScriptEnded() {
                    Logger.info("Game", "Script finished.");
                    Qt.callLater(function() { sceneStack.pop(); });
                }
                function onRestoreBackground(bgId) {
                    bgLayer.show(bgId, "none", 0, function() {});
                }
                function onRestoreCharacter(n, f, p) {
                    charLayer.show(n, f, p, "none", 0, function() {});
                }
                function onRestoreDialogue(s, t) {
                    dialogueBox.showText(s, t, function() {});
                    dialogueBox.finishTyping();
                }
                function onClearDialogue() {
                    dialogueBox.clear();
                }
            }

            MouseArea {
                anchors.fill: parent
                z: 5
                enabled: !choiceBox.visible && !systemMenu.visible && !saveLoadScreen.visible && !historyScreen.visible && !settingsOverlay.visible && !videoPlayer.visible
                onClicked: {
                    if (dialogueBox.isTyping) dialogueBox.finishTyping();
                    else if (dialogueBox.textComplete && ScriptRunner.isRunning) ScriptRunner.advance();
                }
            }

            SystemMenu {
                id: systemMenu; anchors.fill: parent; z: 50
                function show() { visible = true; enabled = true; }
                function hide() { visible = false; enabled = false; }
                function toggle() { visible ? hide() : show(); }

                onResumeGame: { hide(); gameScreen.forceActiveFocus(); }
                onSaveGame: { hide(); saveLoadScreen.saveMode = true; saveLoadScreen.show(); gameScreen.forceActiveFocus(); }
                onLoadGame: { hide(); saveLoadScreen.saveMode = false; saveLoadScreen.show(); gameScreen.forceActiveFocus(); }
                onOpenSettings: { hide(); settingsOverlay.open(); gameScreen.forceActiveFocus(); }
                onReturnToTitle: { ScriptRunner.stop(); sceneStack.pop(); }
                onQuitGame: Qt.quit()
            }

            SaveLoadScreen {
                id: saveLoadScreen; anchors.fill: parent; z: 60
                function show() { visible = true; enabled = true; }
                function hide() { visible = false; enabled = false; }
                onClosed: { 
                    hide(); 
                    if (openLoadScreen && !ScriptRunner.isRunning) {
                        sceneStack.pop();
                    } else {
                        gameScreen.forceActiveFocus(); 
                    }
                }
            }

            HistoryScreen {
                id: historyScreen; anchors.fill: parent; z: 60
                function show() { visible = true; enabled = true; }
                function hide() { visible = false; enabled = false; }
                onClosed: { hide(); gameScreen.forceActiveFocus(); }
            }

            VideoPlayer {
                id: videoPlayer; anchors.fill: parent; z: 70
            }

            // Start script once the game screen and all Connections exist
            property bool openLoadScreen: false

            Component.onCompleted: {
                if (GameStateManager.hasPendingLoad()) {
                    dialogueBox.clear();
                    GameStateManager.executePendingLoad();
                } else if (openLoadScreen) {
                    saveLoadScreen.saveMode = false;
                    saveLoadScreen.show();
                } else {
                    ScriptRunner.start();
                }
            }

            focus: true
            Keys.onSpacePressed: {
                if (dialogueBox.isTyping) dialogueBox.finishTyping();
                else if (dialogueBox.textComplete && ScriptRunner.isRunning) ScriptRunner.advance();
            }
            Keys.onEscapePressed: {
                if (videoPlayer.visible) { videoPlayer.skip(); }
                else if (settingsOverlay.visible) { settingsOverlay.hide(); gameScreen.forceActiveFocus(); }
                else if (historyScreen.visible) { historyScreen.hide(); gameScreen.forceActiveFocus(); }
                else if (saveLoadScreen.visible) { saveLoadScreen.hide(); gameScreen.forceActiveFocus(); }
                else if (systemMenu.visible) { systemMenu.hide(); gameScreen.forceActiveFocus(); }
                else { systemMenu.show(); }
            }
            Keys.onPressed: (event) => {
                if (event.key === Qt.Key_F5) { GameStateManager.quickSave(); event.accepted = true; }
                else if (event.key === Qt.Key_F7) {
                    if (GameStateManager.slotExists(98)) GameStateManager.quickLoad();
                    event.accepted = true;
                }
                else if (event.key === Qt.Key_H || event.key === Qt.Key_Up) {
                    historyScreen.show(); event.accepted = true;
                }
            }
        }
    }

    SettingsScreen {
        id: settingsOverlay; width: gameWidth; height: gameHeight; z: 100; visible: false; enabled: visible
        function open() { visible = true; enabled = true; }
        function hide() { visible = false; enabled = false; }
        onClosed: { hide(); if (sceneStack.currentItem) sceneStack.currentItem.forceActiveFocus(); }
        onFullscreenToggled: (fs) => {
            root.visibility = fs ? Window.FullScreen : Window.Windowed;
        }
    }

    Component.onCompleted: {
        recalcLayout();
        Logger.info("RinEngine", "RinEngine v0.1.0 starting...");
    }
}
