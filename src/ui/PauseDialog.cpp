#include "PauseDialog.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include "ThemeManager.h"

PauseDialog::PauseDialog(QWidget* parent) : QDialog(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(400, 500);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(20);

    auto* title = new QLabel("SYSTEM PAUSED", this);
    title->setAlignment(Qt::AlignCenter);
    title->setFont(QFont("Segoe UI", 24, QFont::Black));
    layout->addWidget(title);

    auto createBtn = [this](const QString& text, const QString& icon = "") {
        auto* btn = new QPushButton(icon + " " + text, this);
        btn->setFont(QFont("Segoe UI", 12, QFont::Bold));
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(50);
        
        auto* shadow = new QGraphicsDropShadowEffect(this);
        shadow->setBlurRadius(15);
        shadow->setColor(ThemeManager::instance().getPrimaryColor());
        shadow->setOffset(0, 0);
        btn->setGraphicsEffect(shadow);
        return btn;
    };

    auto* btnResume = createBtn("RESUME ENGAGEMENT");
    auto* btnRestart = createBtn("RESTART MISSION");
    auto* btnSave = createBtn("SAVE STATE");
    btnSave->setDisabled(true);
    btnSave->setToolTip("Module offline. Awaiting Phase 3 authorization.");
    auto* btnSettings = createBtn("QUICK SETTINGS");
    auto* btnExit = createBtn("ABORT TO MAIN MENU");

    layout->addWidget(btnResume);
    layout->addWidget(btnRestart);
    layout->addWidget(btnSave);
    layout->addWidget(btnSettings);
    layout->addStretch();
    layout->addWidget(btnExit);

    connect(btnResume, &QPushButton::clicked, this, [this]() {
        emit resumeGame();
        accept();
    });
    connect(btnRestart, &QPushButton::clicked, this, [this]() {
        emit restartGame();
        accept();
    });
    connect(btnSettings, &QPushButton::clicked, this, [this]() {
        emit openSettings();
    });
    connect(btnExit, &QPushButton::clicked, this, [this]() {
        emit exitToMenu();
        accept();
    });
}

void PauseDialog::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF rect = this->rect().adjusted(5, 5, -5, -5);

    QColor pri = ThemeManager::instance().getPrimaryColor();
    
    // Glass panel background
    painter.setBrush(QColor(10, 15, 30, 230));
    painter.setPen(QPen(QColor(pri.red(), pri.green(), pri.blue(), 150), 2));
    painter.drawRoundedRect(rect, 15, 15);

    // Scanlines effect
    painter.setPen(QPen(QColor(255, 255, 255, 5), 1));
    for (int y = 0; y < height(); y += 4) {
        painter.drawLine(0, y, width(), y);
    }
}
