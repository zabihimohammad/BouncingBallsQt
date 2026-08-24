#include "SoundManager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QDebug>

SoundManager& SoundManager::instance() {
    static SoundManager s_instance;
    return s_instance;
}

SoundManager::SoundManager(QObject* parent) : QObject(parent) {
    m_bgmPlayer = new QMediaPlayer(this);
    m_bgmAudioOutput = new QAudioOutput(this);
    m_bgmPlayer->setAudioOutput(m_bgmAudioOutput);
    m_bgmPlayer->setLoops(QMediaPlayer::Infinite);
    m_bgmAudioOutput->setVolume(m_musicVolume / 100.0f);

    // پیش‌بارگذاری افکت‌های صوتی
    QStringList effects = {"pop.wav", "shoot.wav", "bounce.wav", "win.wav", "gameover.wav"};
    for (const QString& sfx : effects) {
        QString fullPath = resolveAudioPath("assets/audio/sfx/" + sfx);
        if (QFileInfo::exists(fullPath)) {
            auto* effect = new QSoundEffect(this);
            effect->setSource(QUrl::fromLocalFile(fullPath));
            effect->setVolume(m_sfxVolume / 100.0f);
            m_sfxEffects[sfx] = effect;
        } else {
            qWarning() << "[SoundManager] SFX not found:" << fullPath;
        }
    }

    playMusic("CYBER NEON");
}

QString SoundManager::resolveAudioPath(const QString& relativePath) {
    // ۱. بررسی در مسیر اجرای باینری
    QString appDir = QCoreApplication::applicationDirPath();
    QString pathInApp = QDir(appDir).filePath(relativePath);
    if (QFileInfo::exists(pathInApp)) return pathInApp;

    // ۲. بررسی در پوشه والد (یک لایه عقب‌تر)
    QString parentDir = QDir(appDir + "/..").canonicalPath();
    QString pathInParent = QDir(parentDir).filePath(relativePath);
    if (QFileInfo::exists(pathInParent)) return pathInParent;

    // ۳. بررسی دایرکتوری فعلی ترمینال
    QString currentPath = QDir::current().filePath(relativePath);
    if (QFileInfo::exists(currentPath)) return currentPath;

    return relativePath;
}

void SoundManager::setSfxVolume(int volume) {
    m_sfxVolume = volume;
    float vol = volume / 100.0f;
    for (auto* effect : m_sfxEffects) {
        effect->setVolume(vol);
    }
}

void SoundManager::setMusicVolume(int volume) {
    m_musicVolume = volume;
    m_bgmAudioOutput->setVolume(volume / 100.0f);
}

void SoundManager::playMusic(const QString& trackName) {
    QString relPath = "assets/audio/track_laserpack.mp3";
    if (trackName.contains("SPACE") || trackName.contains("AMBIENT")) {
        relPath = "assets/audio/track_ambient.mp3";
    } else if (trackName.contains("SYNTHWAVE")) {
        relPath = "assets/audio/track_synthwave.mp3";
    } else if (trackName.contains("ARCADE")) {
        relPath = "assets/audio/track_arcade.mp3";
    }

    QString fullPath = resolveAudioPath(relPath);
    if (QFileInfo::exists(fullPath)) {
        m_bgmPlayer->setSource(QUrl::fromLocalFile(fullPath));
        m_bgmPlayer->play();
        qDebug() << "[SoundManager] Playing BGM:" << fullPath;
    } else {
        qWarning() << "[SoundManager] Music track not found:" << fullPath;
    }
}

void SoundManager::playSfx(const QString& filename) {
    if (m_sfxEffects.contains(filename)) {
        m_sfxEffects[filename]->play();
    }
}

void SoundManager::playPop() { playSfx("pop.wav"); }
void SoundManager::playShoot() { playSfx("shoot.wav"); }
void SoundManager::playBounce() { playSfx("bounce.wav"); }
void SoundManager::playGameOver() { playSfx("gameover.wav"); }
void SoundManager::playWin() { playSfx("win.wav"); }