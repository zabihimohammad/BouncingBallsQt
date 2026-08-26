#include "GameOverDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <cmath>

GameOverDialog::GameOverDialog(bool won, const QString& username, int score, QWidget* parent)
    : QDialog(parent), m_won(won), m_username(username), m_score(score) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(500, 420);

    // Calculate stars: 1, 2, or 3
    if (m_won) {
        if (m_score >= 1800) m_stars = 3;
        else if (m_score >= 900) m_stars = 2;
        else m_stars = 1;
    } else {
        if (m_score >= 1200) m_stars = 1;
        else m_stars = 0;
    }

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(35, 30, 35, 30);
    mainLayout->setSpacing(12);

    // 1. Mission Title
    auto title = new QLabel(m_won ? "★ MISSION ACCOMPLISHED ★" : "⚡ SYSTEM FAILURE ⚡", this);
    title->setAlignment(Qt::AlignCenter);
    title->setFont(QFont("Segoe UI", 20, QFont::Black));
    title->setStyleSheet(m_won ? "color: #10b981; margin-top: 5px;" : "color: #ef4444; margin-top: 5px;");
    mainLayout->addWidget(title);

    // Subtitle rating
    QString subText = m_won ? (m_stars == 3 ? "PERFECT OPERATION" : (m_stars == 2 ? "EXCELLENT PERFORMANCE" : "MISSION CLEARED"))
                            : "DEFENSES OVERWHELMED";
    auto subLabel = new QLabel(subText, this);
    subLabel->setAlignment(Qt::AlignCenter);
    subLabel->setFont(QFont("Segoe UI", 9, QFont::Bold));
    subLabel->setStyleSheet(m_won ? "color: #34d399; letter-spacing: 2px;" : "color: #f87171; letter-spacing: 2px;");
    mainLayout->addWidget(subLabel);

    // Space for drawing the 3 Stars in paintEvent
    mainLayout->addSpacing(75);

    // 2. Score and Operative Details Card
    auto statsWidget = new QWidget(this);
    statsWidget->setStyleSheet("background-color: rgba(15, 23, 42, 180); border-radius: 8px; border: 1px solid rgba(56, 189, 248, 60);");
    auto statsLayout = new QVBoxLayout(statsWidget);
    statsLayout->setContentsMargins(15, 12, 15, 12);
    statsLayout->setSpacing(6);

    auto opLabel = new QLabel(QString("OPERATIVE: %1").arg(m_username), statsWidget);
    opLabel->setAlignment(Qt::AlignCenter);
    opLabel->setFont(QFont("Segoe UI", 10, QFont::Bold));
    opLabel->setStyleSheet("color: #94a3b8; border: none; background: transparent;");
    statsLayout->addWidget(opLabel);

    auto scoreLabel = new QLabel(QString("FINAL SCORE: %1").arg(m_score), statsWidget);
    scoreLabel->setAlignment(Qt::AlignCenter);
    scoreLabel->setFont(QFont("Consolas", 18, QFont::Bold));
    scoreLabel->setStyleSheet("color: #00f2fe; border: none; background: transparent;");
    statsLayout->addWidget(scoreLabel);

    mainLayout->addWidget(statsWidget);
    mainLayout->addStretch();

    // 3. Action Buttons
    auto btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    auto restartBtn = new QPushButton("REPLAY MISSION", this);
    restartBtn->setFont(QFont("Segoe UI", 10, QFont::Bold));
    restartBtn->setCursor(Qt::PointingHandCursor);
    restartBtn->setFixedHeight(44);
    restartBtn->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(16, 185, 129, 30);
            color: #10b981;
            border: 1.5px solid #10b981;
            border-radius: 6px;
        }
        QPushButton:hover {
            background-color: rgba(16, 185, 129, 60);
            color: #ffffff;
            border: 1.5px solid #34d399;
        }
    )");
    connect(restartBtn, &QPushButton::clicked, this, [this]() {
        emit restartGame();
        accept();
    });
    btnLayout->addWidget(restartBtn);

    auto returnBtn = new QPushButton("RETURN TO BASE", this);
    returnBtn->setFont(QFont("Segoe UI", 10, QFont::Bold));
    returnBtn->setCursor(Qt::PointingHandCursor);
    returnBtn->setFixedHeight(44);
    returnBtn->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(0, 242, 254, 25);
            color: #00f2fe;
            border: 1.5px solid #00f2fe;
            border-radius: 6px;
        }
        QPushButton:hover {
            background-color: rgba(0, 242, 254, 55);
            color: #ffffff;
            border: 1.5px solid #38bdf8;
        }
    )");
    connect(returnBtn, &QPushButton::clicked, this, [this]() {
        emit returnToMenu();
        accept();
    });
    btnLayout->addWidget(returnBtn);

    mainLayout->addLayout(btnLayout);
}

void GameOverDialog::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF dialogRect = this->rect().adjusted(4, 4, -4, -4);

    // 1. Dark Glass Panel Background
    QLinearGradient bgGrad(0, 0, 0, height());
    bgGrad.setColorAt(0.0, QColor(10, 16, 30, 245));
    bgGrad.setColorAt(1.0, QColor(5, 8, 16, 250));
    painter.setBrush(bgGrad);
    painter.setPen(QPen(m_won ? QColor(16, 185, 129, 180) : QColor(239, 68, 68, 180), 2.0));
    painter.drawRoundedRect(dialogRect, 14, 14);

    // Cyberpunk Corner Brackets
    painter.setPen(QPen(m_won ? QColor(52, 211, 153) : QColor(248, 113, 113), 2.5));
    int b = 14;
    // Top-Left
    painter.drawLine(dialogRect.left(), dialogRect.top() + b, dialogRect.left(), dialogRect.top());
    painter.drawLine(dialogRect.left(), dialogRect.top(), dialogRect.left() + b, dialogRect.top());
    // Top-Right
    painter.drawLine(dialogRect.right() - b, dialogRect.top(), dialogRect.right(), dialogRect.top());
    painter.drawLine(dialogRect.right(), dialogRect.top(), dialogRect.right(), dialogRect.top() + b);
    // Bottom-Left
    painter.drawLine(dialogRect.left(), dialogRect.bottom() - b, dialogRect.left(), dialogRect.bottom());
    painter.drawLine(dialogRect.left(), dialogRect.bottom(), dialogRect.left() + b, dialogRect.bottom());
    // Bottom-Right
    painter.drawLine(dialogRect.right() - b, dialogRect.bottom(), dialogRect.right(), dialogRect.bottom());
    painter.drawLine(dialogRect.right(), dialogRect.bottom(), dialogRect.right(), dialogRect.bottom() - b);

    // 2. Render the 3 Glowing Stars
    qreal startCenterX = width() / 2.0;
    qreal starY = 120.0;
    qreal starSpacing = 65.0;

    auto draw5PointStar = [&](QPointF center, qreal rOuter, qreal rInner, bool filled, qreal scale) {
        painter.save();
        painter.translate(center);
        painter.scale(scale, scale);

        QPolygonF starPoly;
        for (int i = 0; i < 10; ++i) {
            qreal r = (i % 2 == 0) ? rOuter : rInner;
            qreal angle = (i * 36.0 - 90.0) * M_PI / 180.0;
            starPoly << QPointF(r * std::cos(angle), r * std::sin(angle));
        }

        if (filled) {
            // Golden Halo
            QRadialGradient starGlow(0, 0, rOuter * 1.5);
            starGlow.setColorAt(0.0, QColor(245, 158, 11, 160));
            starGlow.setColorAt(0.6, QColor(251, 191, 36, 80));
            starGlow.setColorAt(1.0, Qt::transparent);
            painter.setBrush(starGlow);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(QPointF(0,0), rOuter * 1.5, rOuter * 1.5);

            // Radiant Gold Body
            QLinearGradient goldGrad(0, -rOuter, 0, rOuter);
            goldGrad.setColorAt(0.0, QColor(255, 255, 220));
            goldGrad.setColorAt(0.3, QColor(251, 191, 36));
            goldGrad.setColorAt(0.8, QColor(245, 158, 11));
            goldGrad.setColorAt(1.0, QColor(180, 83, 9));
            painter.setBrush(goldGrad);
            painter.setPen(QPen(QColor(254, 240, 138), 1.5));
            painter.drawPolygon(starPoly);
        } else {
            // Dark Wireframe Inactive Star
            painter.setBrush(QColor(30, 41, 59, 140));
            painter.setPen(QPen(QColor(71, 85, 105, 180), 1.5));
            painter.drawPolygon(starPoly);
        }

        painter.restore();
    };

    // Left Star (Index 1)
    draw5PointStar(QPointF(startCenterX - starSpacing, starY + 6), 20.0, 9.0, m_stars >= 1, 0.9);
    // Center Star (Index 2 - slightly larger & higher)
    draw5PointStar(QPointF(startCenterX, starY - 4), 26.0, 11.5, m_stars >= 2, 1.1);
    // Right Star (Index 3)
    draw5PointStar(QPointF(startCenterX + starSpacing, starY + 6), 20.0, 9.0, m_stars >= 3, 0.9);
}
