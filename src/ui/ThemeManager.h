#pragma once
#include <QString>

class ThemeManager {
public:
    static QString getMasterStyleSheet() {
        return R"(
            QWidget {
                background-color: #0F172A;
                color: #F8FAFC;
                font-family: 'Segoe UI', 'Ubuntu', sans-serif;
                font-size: 14px;
            }

            /* Neon Glass Card / Group Box */
            QFrame#cardFrame {
                background-color: #1E293B;
                border: 1px solid #334155;
                border-radius: 16px;
            }

            /* Buttons */
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #0284C7, stop:1 #0369A1);
                color: #FFFFFF;
                font-size: 15px;
                font-weight: bold;
                padding: 12px 24px;
                border: 1px solid #38BDF8;
                border-radius: 12px;
                min-width: 160px;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #38BDF8, stop:1 #0284C7);
                border: 2px solid #E0F2FE;
            }
            QPushButton:pressed {
                background-color: #0C4A6E;
            }
            QPushButton#dangerBtn {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #DC2626, stop:1 #991B1B);
                border: 1px solid #F87171;
            }
            QPushButton#dangerBtn:hover {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #EF4444, stop:1 #DC2626);
                border: 2px solid #FEE2E2;
            }

            /* Input Fields */
            QLineEdit {
                background-color: #1E293B;
                color: #38BDF8;
                border: 2px solid #334155;
                border-radius: 10px;
                padding: 10px 14px;
                font-size: 15px;
                font-weight: bold;
            }
            QLineEdit:focus {
                border: 2px solid #38BDF8;
                background-color: #0F172A;
            }

            /* ComboBox */
            QComboBox {
                background-color: #1E293B;
                border: 2px solid #334155;
                border-radius: 10px;
                padding: 8px 16px;
                color: #F8FAFC;
                font-weight: bold;
            }
            QComboBox:hover {
                border: 2px solid #38BDF8;
            }
            QComboBox QAbstractItemView {
                background-color: #1E293B;
                border: 1px solid #38BDF8;
                selection-background-color: #0284C7;
                color: #FFFFFF;
            }

            /* Sliders */
            QSlider::groove:horizontal {
                height: 8px;
                background: #334155;
                border-radius: 4px;
            }
            QSlider::sub-page:horizontal {
                background: #38BDF8;
                border-radius: 4px;
            }
            QSlider::handle:horizontal {
                background: #FFFFFF;
                border: 2px solid #38BDF8;
                width: 20px;
                margin-top: -6px;
                margin-bottom: -6px;
                border-radius: 10px;
            }

            /* Table Widget */
            QTableWidget {
                background-color: #1E293B;
                border: 1px solid #334155;
                border-radius: 12px;
                gridline-color: #334155;
                color: #F8FAFC;
            }
            QHeaderView::section {
                background-color: #0F172A;
                color: #38BDF8;
                font-weight: bold;
                padding: 8px;
                border: none;
                border-bottom: 2px solid #38BDF8;
            }
        )";
    }
};