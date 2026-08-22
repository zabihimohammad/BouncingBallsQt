#include "GameOverDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

GameOverDialog::GameOverDialog(bool won, const QString& username, int score, QWidget* parent)
    : QDialog(parent) {
    setModal(true);
    setStyleSheet("background-color: #2c3e50; color: white;");

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(15);

    auto title = new QLabel(won ? "🎉 Victory! 🎉" : "💀 Game Over 💀", this);
    title->setStyleSheet(won ? "font-size: 22px; font-weight: bold; color: #2ecc71;"
                             : "font-size: 22px; font-weight: bold; color: #e74c3c;");
    layout->addWidget(title, 0, Qt::AlignCenter);

    auto info = new QLabel(QString("Player: %1\nScore: %2").arg(username).arg(score), this);
    info->setStyleSheet("font-size: 16px; color: #ecf0f1; text-align: center;");
    layout->addWidget(info, 0, Qt::AlignCenter);

    auto returnBtn = new QPushButton("Return to Main Menu", this);
    returnBtn->setStyleSheet("background-color: #2980b9; color: white; padding: 10px 20px; font-weight: bold; border-radius: 6px;");
    connect(returnBtn, &QPushButton::clicked, this, [this]() {
        emit returnToMenu();
        accept();
    });
    layout->addWidget(returnBtn);
}
