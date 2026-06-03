import QtQuick
import QtMultimedia
import RinEngine.Core 1.0

Item {
    id: root
    visible: false
    enabled: visible

    Rectangle {
        anchors.fill: parent
        color: "#000000"
    }

    VideoOutput {
        id: videoOut
        anchors.fill: parent
    }

    MediaPlayer {
        id: mediaPlayer
        audioOutput: AudioOutput {}
        videoOutput: videoOut

        onPlaybackStateChanged: {
            if (playbackState === MediaPlayer.StoppedState && root.visible) {
                finish();
            }
        }

        onErrorOccurred: (error, errorString) => {
            Logger.warn("VideoPlayer", "Error: " + errorString);
            finish();
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: skip()
    }

    function play(videoId) {
        var url = ResourceManager.getVideo("video:" + videoId);
        if (!url || url.toString() === "") {
            Logger.warn("VideoPlayer", "Video not found: " + videoId);
            ScriptRunner.advance();
            return;
        }
        Logger.info("VideoPlayer", "Playing: " + videoId);
        root.visible = true;
        root.enabled = true;
        mediaPlayer.source = url;
        mediaPlayer.play();
    }

    function skip() {
        if (!root.visible) return;
        mediaPlayer.stop();
        finish();
    }

    function finish() {
        root.visible = false;
        root.enabled = false;
        mediaPlayer.source = "";
        if (ScriptRunner.isRunning) ScriptRunner.advance();
    }
}
