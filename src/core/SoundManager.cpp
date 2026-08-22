#include "SoundManager.h"
#include <QDebug>

SoundManager& SoundManager::instance() {
    static SoundManager s_instance;
    return s_instance;
}

SoundManager::SoundManager(QObject* parent) : QObject(parent) {}

void SoundManager::setSfxVolume(int volume) { m_sfxVolume = volume; }
void SoundManager::setMusicVolume(int volume) { m_musicVolume = volume; }

void SoundManager::playPop() {
    qDebug() << "[SFX] Pop played at volume" << m_sfxVolume;
}

void SoundManager::playShoot() {
    qDebug() << "[SFX] Shoot played at volume" << m_sfxVolume;
}

void SoundManager::playGameOver() {
    qDebug() << "[SFX] Game Over sound played";
}

void SoundManager::playWin() {
    qDebug() << "[SFX] Win victory sound played";
}
