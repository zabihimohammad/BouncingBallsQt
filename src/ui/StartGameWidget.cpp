#include "StartGameWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

StartGameWidget::StartGameWidget(QWidget* parent) : QWidget(parent) {
    auto layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(15);

    auto title = new QLabel("Select Mode & Player", this);
    title->setStyleSheet("font-size: 22px; font-weight: bold; color: #ecf0f1;");
    layout->addWidget(title, 0, Qt::AlignCenter);

    m_nameInput = new QLineEdit(this);
    m_nameInput->setPlaceholderText("Enter your username...");
    m_nameInput->setStyleSheet("padding: 10px; font-size: 14px; border-radius: 6px; min-width: 200px;");
    layout->addWidget(m_nameInput, 0, Qt::AlignCenter);

    m_modeCombo = new QComboBox(this);
    m_modeCombo->addItems({"Classic", "Time", "Random", "Endless"});
    m_modeCombo->setStyleSheet("padding: 8px; font-size: 14px; border-radius: 6px; min-width: 200px;");
    layout->addWidget(m_modeCombo, 0, Qt::AlignCenter);

    auto playBtn = new QPushButton("Play", this);
    playBtn->setStyleSheet("QPushButton { background-color: #27ae60; color: white; font-size: 16px; font-weight: bold; "
                           "padding: 10px 30px; border-radius: 8px; min-width: 150px; } "
                           "QPushButton:hover { background-color: #2ecc71; }");
    connect(playBtn, &QPushButton::clicked, this, [this]() {
        QString user = m_nameInput->text().trimmed();
        if (user.isEmpty()) user = "Player";
        emit launchGame(user, m_modeCombo->currentText());
    });
    layout->addWidget(playBtn, 0, Qt::AlignCenter);

    auto backBtn = new QPushButton("Back", this);
    backBtn->setStyleSheet("QPushButton { background-color: #7f8c8d; color: white; font-size: 14px; "
                           "padding: 8px 20px; border-radius: 6px; } QPushButton:hover { background-color: #95a5a6; }");
    connect(backBtn, &QPushButton::clicked, this, &StartGameWidget::backClicked);
    layout->addWidget(backBtn, 0, Qt::AlignCenter);
}
