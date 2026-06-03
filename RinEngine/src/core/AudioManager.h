#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QTimer>
#include <QString>

class AudioManager : public QObject
{
    Q_OBJECT

public:
    static AudioManager* instance();

    Q_INVOKABLE void playBgm(const QString &bgmId, double volume);
    Q_INVOKABLE void stopBgm(double fadeOutMs = 500);
    Q_INVOKABLE void playSe(const QString &seId);
    Q_INVOKABLE void playVoice(const QString &voiceId);
    Q_INVOKABLE void stopVoice();

    Q_INVOKABLE void setBgmVolume(double vol);
    Q_INVOKABLE void setSeVolume(double vol);
    Q_INVOKABLE void setVoiceVolume(double vol);

    Q_INVOKABLE double bgmVolume() const { return m_bgmVolume; }
    Q_INVOKABLE double seVolume() const { return m_seVolume; }
    Q_INVOKABLE double voiceVolume() const { return m_voiceVolume; }

    QString currentBgmId() const { return m_currentBgmId; }

signals:
    void bgmStarted(const QString &bgmId);
    void bgmStopped();
    void sePlayed(const QString &seId);
    void voiceStarted(const QString &voiceId);
    void voiceStopped();
    void bgmVolumeChanged();
    void seVolumeChanged();
    void voiceVolumeChanged();

private slots:
    void onFadeTick();
    void onBgmStateChanged(QMediaPlayer::PlaybackState state);

private:
    explicit AudioManager(QObject *parent = nullptr);

    QString resolveAudio(const QString &protocolPath) const;
    void applyBgmVolume();  // apply settings volume to active BGM player
    void beginCrossfade(const QString &bgmId, double volume);
    void finishCrossfade();
    void swapPlayers();
    double effectiveVolume(double scriptVolume) const;

    // BGM: two players for crossfade
    QMediaPlayer *m_bgmA;
    QMediaPlayer *m_bgmB;
    QAudioOutput *m_outputA;
    QAudioOutput *m_outputB;
    // Which player is currently active (fading in / fully playing)
    // nullptr if no BGM playing; points to m_bgmA or m_bgmB
    QMediaPlayer *m_bgmActive = nullptr;
    QAudioOutput *m_bgmActiveOutput = nullptr;
    // Which player is fading out
    QMediaPlayer *m_bgmFading = nullptr;
    QAudioOutput *m_bgmFadingOutput = nullptr;

    // SE: single player (non-overlapping)
    QMediaPlayer *m_sePlayer;
    QAudioOutput *m_seOutput;

    // Voice: single player
    QMediaPlayer *m_voicePlayer;
    QAudioOutput *m_voiceOutput;

    // Crossfade timer
    QTimer m_fadeTimer;
    int m_fadeStep = 0;
    static constexpr int FADE_STEPS = 10;
    static constexpr int FADE_INTERVAL_MS = 50;

    // Volumes (0.0-1.0 normalized from settings 0-100)
    double m_bgmVolume = 0.8;
    double m_seVolume = 0.8;
    double m_voiceVolume = 0.8;

    // Currently playing BGM
    QString m_currentBgmId;
    double m_currentBgmScriptVolume = 80.0;
};

#endif // AUDIO_MANAGER_H
