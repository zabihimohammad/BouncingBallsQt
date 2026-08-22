#include "SettingsWidget.h"
#include "../core/SoundManager.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

SettingsWidget::SettingsWidget(QWidget* parent) : QWidget(parent) {
    auto layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(15);

    auto title = new QLabel("Settings", this);
    title->setStyleSheet("font-size: 22px; font-weight: bold; color: #ecf0f1;");
    layout->addWidget(title, 0, Qt::AlignCenter);

    auto sfxLabel = new QLabel("SFX Volume", this);
    sfxLabel->setStyleSheet("color: white;");
    layout->addWidget(sfxLabel);

    m_sfxSlider = new QSlider(Qt::Horizontal, this);
    m_sfxSlider->setRange(0, 100);
    m_sfxSlider->setValue(SoundManager::instance().getSfxVolume());
    connect(m_sfxSlider, &QSlider::valueChanged, [](int v) { SoundManager::instance().setSfxVolume(v); });
    layout->addWidget(m_sfxSlider);

    auto musicLabel = new QLabel("Music Volume", this);
    musicLabel->setStyleSheet("color: white;");
    layout->addWidget(musicLabel);

    m_musicSlider = new QSlider(Qt::Horizontal, this);
    m_musicSlider->setRange(0, 100);
    m_musicSlider->setValue(SoundManager::instance().getMusicVolume());
    connect(m_musicSlider, &QSlider::valueChanged, [](int v) { SoundManager::instance().setMusicVolume(v); });
    layout->addWidget(m_musicSlider);

    auto themeLabel = new QLabel("Theme", this);
    themeLabel->setStyleSheet("color: white;");
    layout->addWidget(themeLabel);

    m_themeCombo = new QComboBox(this);
    m_themeCombo->addItems({"Classic Dark", "Space Blue", "Forest Green"});
    layout->addWidget(m_themeCombo);

    auto backBtn = new QPushButton("Back", this);
    backBtn->setStyleSheet("QPushButton { background-color: #7f8c8d; color: white; font-size: 14px; "
                           "padding: 8px 20px; border-radius: 6px; } QPushButton:hover { background-color: #95a5a6; }");
    connect(backBtn, &QPushButton::clicked, this, &SettingsWidget::backClicked);
    layout->addWidget(backBtn, 0, Qt::AlignCenter);
}
