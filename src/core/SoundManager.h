#pragma once
#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSoundEffect>
#include <QString>
#include <QMap>

class SoundManager : public QObject {
    Q_OBJECT
public:
    static SoundManager& instance();

    void setSfxVolume(int volume);
    int getSfxVolume() const { return m_sfxVolume; }
    void setMusicVolume(int volume);
    int getMusicVolume() const { return m_musicVolume; }

    void playMusic(const QString& trackName);
    void playPop();
    void playShoot();
    void playBounce();
    void playGameOver();
    void playWin();

private:
    SoundManager(QObject* parent = nullptr);
    QString resolveAudioPath(const QString& relativePath);
    void playSfx(const QString& filename);

    int m_sfxVolume = 80;
    int m_musicVolume = 50;

    QMediaPlayer* m_bgmPlayer;
    QAudioOutput* m_bgmAudioOutput;

    // نگهداری افکت‌ها با QSoundEffect به همراه fallback
    QMap<QString, QSoundEffect*> m_sfxEffects;
};