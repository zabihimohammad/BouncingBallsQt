#pragma once
#include <QString>
#include <QColor>
#include <QApplication>

enum class ThemeId {
    CyberNeon,
    CosmicVoid,
    SolarFlare,
    MatrixGreen
};

class ThemeManager {
private:
    ThemeId m_currentTheme = ThemeId::CyberNeon;
    ThemeManager() {}

public:
    static ThemeManager& instance() {
        static ThemeManager instance;
        return instance;
    }

    void setTheme(ThemeId theme) {
        if (m_currentTheme == theme) return;
        m_currentTheme = theme;
        if (qApp) {
            qApp->setStyleSheet(getMasterStyleSheet());
        }
    }

    ThemeId currentTheme() const { return m_currentTheme; }

    QString themeName() const {
        switch (m_currentTheme) {
            case ThemeId::CyberNeon: return "CYBER NEON";
            case ThemeId::CosmicVoid: return "COSMIC VOID";
            case ThemeId::SolarFlare: return "SOLAR FLARE";
            case ThemeId::MatrixGreen: return "MATRIX GREEN";
        }
        return "CYBER NEON";
    }

    QColor getPrimaryColor() const {
        switch (m_currentTheme) {
            case ThemeId::CyberNeon: return QColor(0, 242, 254);   // Cyan
            case ThemeId::CosmicVoid: return QColor(192, 132, 252);  // Neon Purple
            case ThemeId::SolarFlare: return QColor(239, 68, 68);    // Crimson Red
            case ThemeId::MatrixGreen: return QColor(16, 185, 129);  // Emerald Green
        }
        return QColor(0, 242, 254);
    }

    QColor getSecondaryColor() const {
        switch (m_currentTheme) {
            case ThemeId::CyberNeon: return QColor(0, 114, 255);   // Deep Blue
            case ThemeId::CosmicVoid: return QColor(245, 158, 11); // Gold
            case ThemeId::SolarFlare: return QColor(249, 115, 22); // Orange
            case ThemeId::MatrixGreen: return QColor(34, 197, 94); // Bright Green
        }
        return QColor(0, 114, 255);
    }

    int getBaseHue() const {
        switch (m_currentTheme) {
            case ThemeId::CyberNeon: return 200;   // Cyan/Blue
            case ThemeId::CosmicVoid: return 275;  // Purple/Violet
            case ThemeId::SolarFlare: return 5;    // Red/Orange
            case ThemeId::MatrixGreen: return 145; // Green
        }
        return 200;
    }

    QString getMasterStyleSheet() const {
        QString primaryHex = getPrimaryColor().name();
        QString secondaryHex = getSecondaryColor().name();
        QString bgHex = "#0F172A"; 
        QString cardBg = "#1E293B";
        
        if (m_currentTheme == ThemeId::CosmicVoid) { bgHex = "#150C25"; cardBg = "#22153B"; }
        else if (m_currentTheme == ThemeId::SolarFlare) { bgHex = "#1A0808"; cardBg = "#2A1212"; }
        else if (m_currentTheme == ThemeId::MatrixGreen) { bgHex = "#051A10"; cardBg = "#0C2B1C"; }

        return QString(R"(
            QWidget {
                background-color: %3;
                color: #F8FAFC;
                font-family: 'Segoe UI', 'Ubuntu', sans-serif;
                font-size: 14px;
            }
            QFrame#cardFrame, QFrame#scoreFrame, QFrame#targetFrame {
                background-color: %4;
                border: 1px solid %2;
                border-radius: 16px;
            }
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 %1, stop:1 %2);
                color: #FFFFFF;
                font-size: 15px;
                font-weight: bold;
                padding: 12px 24px;
                border: 1px solid %1;
                border-radius: 12px;
                min-width: 160px;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 %2, stop:1 %1);
                border: 2px solid #FFFFFF;
            }
            QPushButton:pressed {
                background-color: %2;
            }
            QLineEdit, QComboBox, QTableWidget {
                background-color: %4;
                border: 1px solid %2;
                color: #F8FAFC;
                border-radius: 8px;
                padding: 8px;
            }
            QLineEdit:focus, QComboBox:hover {
                border: 1px solid %1;
            }
            QSlider::sub-page:horizontal {
                background: %1;
                border-radius: 4px;
            }
            QSlider::handle:horizontal {
                background: #FFFFFF;
                border: 2px solid %1;
                width: 20px;
                margin-top: -6px;
                margin-bottom: -6px;
                border-radius: 10px;
            }
        )").arg(primaryHex, secondaryHex, bgHex, cardBg);
    }
};
