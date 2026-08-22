#include "PauseDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

PauseDialog::PauseDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Paused");
    setModal(true);
    setStyleSheet("background-color: #2c3e50; color: white;");

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(15);

    auto label = new QLabel("Game Paused", this);
    label->setStyleSheet("font-size: 20px; font-weight: bold; color: #f1c40f;");
    layout->addWidget(label, 0, Qt::AlignCenter);

    auto resumeBtn = new QPushButton("Resume", this);
    resumeBtn->setStyleSheet("background-color: #27ae60; color: white; padding: 10px; font-weight: bold; border-radius: 6px;");
    connect(resumeBtn, &QPushButton::clicked, this, [this]() {
        emit resumeGame();
        accept();
    });
    layout->addWidget(resumeBtn);

    auto exitBtn = new QPushButton("Exit to Menu", this);
    exitBtn->setStyleSheet("background-color: #c0392b; color: white; padding: 10px; font-weight: bold; border-radius: 6px;");
    connect(exitBtn, &QPushButton::clicked, this, [this]() {
        emit exitToMenu();
        accept();
    });
    layout->addWidget(exitBtn);
}
