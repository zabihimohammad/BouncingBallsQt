#pragma once
#include <QWidget>
#include <QSlider>
#include <QComboBox>

class SettingsWidget : public QWidget {
    Q_OBJECT
public:
    SettingsWidget(QWidget* parent = nullptr);

signals:
    void backClicked();

private:
    QSlider* m_sfxSlider;
    QSlider* m_musicSlider;
    QComboBox* m_themeCombo;
};
