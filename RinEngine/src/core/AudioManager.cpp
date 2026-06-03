#include "AudioManager.h"
#include "ResourceManager.h"
#include "SettingsManager.h"
#include "Logger.h"
#include <QUrl>
#include <QFileInfo>

AudioManager* AudioManager::instance()
{
    static AudioManager inst;
    return &inst;
}

AudioManager::AudioManager(QObject *parent)
    : QObject(parent)
{
    // BGM channel — two players for crossfade
    m_bgmA = new QMediaPlayer(this);
    m_bgmB = new QMediaPlayer(this);
    m_outputA = new QAudioOutput(this);
    m_outputB = new QAudioOutput(this);
    m_bgmA->setAudioOutput(m_outputA);
    m_bgmB->setAudioOutput(m_outputB);
    m_outputA->setVolume(0.0f);
    m_outputB->setVolume(0.0f);

    // SE channel
    m_sePlayer = new QMediaPlayer(this);
    m_seOutput = new QAudioOutput(this);
    m_sePlayer->setAudioOutput(m_seOutput);

    // Voice channel
    m_voicePlayer = new QMediaPlayer(this);
    m_voiceOutput = new QAudioOutput(this);
    m_voicePlayer->setAudioOutput(m_voiceOutput);

    // Fade timer
    m_fadeTimer.setInterval(FADE_INTERVAL_MS);
    connect(&m_fadeTimer, &QTimer::timeout, this, &AudioManager::onFadeTick);

    // Connect to volume changes from settings
    auto *settings = SettingsManager::instance();
    connect(settings, &SettingsManager::bgmVolumeChanged,
            this, [this]() {
        m_bgmVolume = SettingsManager::instance()->bgmVolume() / 100.0;
        applyBgmVolume();
        emit bgmVolumeChanged();
    });
    connect(settings, &SettingsManager::seVolumeChanged,
            this, [this]() {
        m_seVolume = SettingsManager::instance()->seVolume() / 100.0;
        m_seOutput->setVolume(m_seVolume);
        emit seVolumeChanged();
    });
    connect(settings, &SettingsManager::voiceVolumeChanged,
            this, [this]() {
        m_voiceVolume = SettingsManager::instance()->voiceVolume() / 100.0;
        m_voiceOutput->setVolume(m_voiceVolume);
        emit voiceVolumeChanged();
    });

    // Init volumes from current settings
    m_bgmVolume = settings->bgmVolume() / 100.0;
    m_seVolume = settings->seVolume() / 100.0;
    m_voiceVolume = settings->voiceVolume() / 100.0;
    m_seOutput->setVolume(m_seVolume);
    m_voiceOutput->setVolume(m_voiceVolume);
}

QString AudioManager::resolveAudio(const QString &protocolPath) const
{
    QUrl url = ResourceManager::instance()->getAudio(protocolPath);
    if (url.isEmpty() || !url.isLocalFile()) {
        Logger::instance()->warn("AudioManager", "Cannot resolve audio: " + protocolPath);
        return QString();
    }
    QString path = url.toLocalFile();
    if (!QFileInfo::exists(path)) {
        Logger::instance()->warn("AudioManager", "Audio file not found: " + path);
        return QString();
    }
    return path;
}

double AudioManager::effectiveVolume(double scriptVolume) const
{
    return (scriptVolume / 100.0) * m_bgmVolume;
}

void AudioManager::applyBgmVolume()
{
    if (m_bgmActiveOutput && !m_fadeTimer.isActive()) {
        m_bgmActiveOutput->setVolume(effectiveVolume(m_currentBgmScriptVolume));
    }
}

// --- BGM ---

void AudioManager::playBgm(const QString &bgmId, double volume)
{
    QString path = resolveAudio("bgm:" + bgmId);
    if (path.isEmpty()) {
        Logger::instance()->warn("AudioManager", "BGM not found, skipping: " + bgmId);
        return;
    }

    Logger::instance()->info("AudioManager",
        "BGM play: " + bgmId + " vol=" + QString::number(volume));

    m_currentBgmId = bgmId;
    m_currentBgmScriptVolume = volume;

    // If nothing is playing, start immediately on the idle player
    if (!m_bgmActive) {
        // Use player A as initial
        m_bgmA->setSource(QUrl::fromLocalFile(path));
        m_bgmA->setLoops(QMediaPlayer::Infinite);
        m_bgmActive = m_bgmA;
        m_bgmActiveOutput = m_outputA;
        m_bgmActiveOutput->setVolume(0.0f);
        m_bgmA->play();
        m_bgmFading = nullptr;
        m_bgmFadingOutput = nullptr;
        // Fade in
        m_fadeStep = 0;
        m_fadeTimer.start();
    } else {
        // Crossfade: load on the inactive player, fade between them
        QMediaPlayer *inactive = (m_bgmActive == m_bgmA) ? m_bgmB : m_bgmA;
        QAudioOutput *inactiveOutput = (m_bgmActiveOutput == m_outputA) ? m_outputB : m_outputA;

        // Stop anything on the inactive player
        inactive->stop();

        // Start fade-out on current active player
        m_bgmFading = m_bgmActive;
        m_bgmFadingOutput = m_bgmActiveOutput;

        // Load new BGM on inactive player
        inactive->setSource(QUrl::fromLocalFile(path));
        inactive->setLoops(QMediaPlayer::Infinite);
        inactiveOutput->setVolume(0.0f);

        // Swap active to the new player
        m_bgmActive = inactive;
        m_bgmActiveOutput = inactiveOutput;
        m_bgmActive->play();

        // Start crossfade
        m_fadeStep = 0;
        m_fadeTimer.start();
    }

    emit bgmStarted(bgmId);
}

void AudioManager::stopBgm(double fadeOutMs)
{
    if (!m_bgmActive) return;

    m_bgmFading = m_bgmActive;
    m_bgmFadingOutput = m_bgmActiveOutput;
    m_bgmActive = nullptr;
    m_bgmActiveOutput = nullptr;

    if (fadeOutMs <= 0) {
        m_bgmFading->stop();
        m_bgmFadingOutput->setVolume(0.0f);
        m_bgmFading = nullptr;
        m_bgmFadingOutput = nullptr;
        emit bgmStopped();
    } else {
        m_fadeStep = -FADE_STEPS; // negative means fade-to-stop
        m_fadeTimer.start();
    }
}

void AudioManager::onBgmStateChanged(QMediaPlayer::PlaybackState state)
{
    Q_UNUSED(state)
}

void AudioManager::onFadeTick()
{
    m_fadeStep++;

    if (m_bgmFading && m_fadeStep > FADE_STEPS) {
        // Crossfade complete: stop old player
        m_bgmFading->stop();
        m_bgmFadingOutput->setVolume(0.0f);
        m_bgmFading->setSource(QUrl()); // release source
        m_bgmFading = nullptr;
        m_bgmFadingOutput = nullptr;

        if (!m_bgmActive && m_fadeStep > 0) {
            // fade-to-stop completed
            m_fadeTimer.stop();
            m_fadeStep = 0;
            m_currentBgmId.clear();
            emit bgmStopped();
            return;
        }

        m_fadeTimer.stop();
        m_fadeStep = 0;
        return;
    }

    // Calculate volume factor for active (fading in) and fading (fading out)
    double fadeInFactor = qMin(1.0, (double)m_fadeStep / FADE_STEPS);
    double fadeOutFactor = 1.0 - fadeInFactor;

    double activeVol = effectiveVolume(m_currentBgmScriptVolume);

    if (m_bgmActiveOutput) {
        m_bgmActiveOutput->setVolume(activeVol * fadeInFactor);
    }
    if (m_bgmFadingOutput) {
        double fadingInitialVol = effectiveVolume(
            m_bgmFading == m_bgmA ? m_currentBgmScriptVolume : m_currentBgmScriptVolume);
        m_bgmFadingOutput->setVolume(fadingInitialVol * fadeOutFactor);
    }
}

// --- SE ---

void AudioManager::playSe(const QString &seId)
{
    QString path = resolveAudio("se:" + seId);
    if (path.isEmpty()) {
        Logger::instance()->warn("AudioManager", "SE not found, skipping: " + seId);
        return;
    }

    Logger::instance()->debug("AudioManager", "SE play: " + seId);
    m_seOutput->setVolume(m_seVolume);
    m_sePlayer->stop();
    m_sePlayer->setSource(QUrl::fromLocalFile(path));
    m_sePlayer->play();

    emit sePlayed(seId);
}

// --- Voice ---

void AudioManager::playVoice(const QString &voiceId)
{
    QString path = resolveAudio("voice:" + voiceId);
    if (path.isEmpty()) {
        Logger::instance()->warn("AudioManager", "Voice not found, skipping: " + voiceId);
        return;
    }

    Logger::instance()->debug("AudioManager", "Voice play: " + voiceId);
    m_voiceOutput->setVolume(m_voiceVolume);
    m_voicePlayer->stop();
    m_voicePlayer->setSource(QUrl::fromLocalFile(path));
    m_voicePlayer->play();

    emit voiceStarted(voiceId);
}

void AudioManager::stopVoice()
{
    m_voicePlayer->stop();
    emit voiceStopped();
}

// --- Volume Setters ---

void AudioManager::setBgmVolume(double vol)
{
    SettingsManager::instance()->setBgmVolume(vol);
}

void AudioManager::setSeVolume(double vol)
{
    SettingsManager::instance()->setSeVolume(vol);
}

void AudioManager::setVoiceVolume(double vol)
{
    SettingsManager::instance()->setVoiceVolume(vol);
}
