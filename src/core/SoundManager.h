#pragma once
#include <QObject>

class SoundManager : public QObject {
    Q_OBJECT
public:
    static SoundManager& instance();

    void setSfxVolume(int volume);
    int getSfxVolume() const { return m_sfxVolume; }
    void setMusicVolume(int volume);
    int getMusicVolume() const { return m_musicVolume; }

    void playPop();
    void playShoot();
    void playGameOver();
    void playWin();

private:
    SoundManager(QObject* parent = nullptr);
    int m_sfxVolume = 80;
    int m_musicVolume = 50;
};
