#include "MainMenuWidget.h"
#include <QLabel>

MainMenuWidget::MainMenuWidget(QWidget* parent) : QWidget(parent) {
    auto layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(20);

    auto title = new QLabel("🔵 Bouncing Balls 🔴", this);
    title->setStyleSheet("font-size: 28px; font-weight: bold; color: #ecf0f1; margin-bottom: 20px;");
    layout->addWidget(title, 0, Qt::AlignCenter);

    QString btnStyle = "QPushButton { background-color: #2980b9; color: white; font-size: 16px; font-weight: bold; "
                       "padding: 12px 30px; border-radius: 8px; min-width: 180px; } "
                       "QPushButton:hover { background-color: #3498db; }";

    auto startBtn = new QPushButton("Start Game", this);
    startBtn->setStyleSheet(btnStyle);
    connect(startBtn, &QPushButton::clicked, this, &MainMenuWidget::startGameClicked);
    layout->addWidget(startBtn, 0, Qt::AlignCenter);

    auto scoreBtn = new QPushButton("Scoreboard", this);
    scoreBtn->setStyleSheet(btnStyle);
    connect(scoreBtn, &QPushButton::clicked, this, &MainMenuWidget::scoreboardClicked);
    layout->addWidget(scoreBtn, 0, Qt::AlignCenter);

    auto settingsBtn = new QPushButton("Settings", this);
    settingsBtn->setStyleSheet(btnStyle);
    connect(settingsBtn, &QPushButton::clicked, this, &MainMenuWidget::settingsClicked);
    layout->addWidget(settingsBtn, 0, Qt::AlignCenter);

    auto exitBtn = new QPushButton("Exit", this);
    exitBtn->setStyleSheet("QPushButton { background-color: #c0392b; color: white; font-size: 16px; font-weight: bold; "
                           "padding: 12px 30px; border-radius: 8px; min-width: 180px; } "
                           "QPushButton:hover { background-color: #e74c3c; }");
    connect(exitBtn, &QPushButton::clicked, this, &MainMenuWidget::exitClicked);
    layout->addWidget(exitBtn, 0, Qt::AlignCenter);
}
