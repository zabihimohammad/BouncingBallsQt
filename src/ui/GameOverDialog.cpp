#include "GameOverDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QRadialGradient>
#include <QLinearGradient>
#include <cmath>

GameOverDialog::GameOverDialog(bool won, const QString& username, int score, const QString& mode, QWidget* parent)
        : QDialog(parent), m_won(won), m_username(username), m_score(score), m_mode(mode) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(520, 430);

    bool isEndless = (m_mode.contains("Endless", Qt::CaseInsensitive));
    bool isTimeAttack = (m_mode.contains("Time", Qt::CaseInsensitive));

    // ارزیابی مدال در Time Attack
    if (isTimeAttack) {
        if (m_score >= 3200) m_medalTier = 3;       // Platinum
        else if (m_score >= 1800) m_medalTier = 2;  // Gold
        else if (m_score >= 800) m_medalTier = 1;   // Silver
        else m_medalTier = 0;                       // Bronze
    }
        // ارزیابی ستاره‌ها در کمپین
    else if (m_won && !isEndless) {
        if (m_score >= 1800) m_stars = 3;
        else if (m_score >= 900) m_stars = 2;
        else m_stars = 1;
    }

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(35, 25, 35, 25);
    mainLayout->setSpacing(10);

    // ۱. عنوان دیالوگ
    QString titleText;
    QString titleStyle;
    if (isTimeAttack) {
        titleText = "⏱ CHRONO OPERATION LOGGED ⏱";
        titleStyle = "color: #00f2fe; margin-top: 5px;";
    } else if (isEndless) {
        titleText = "✦ SURVIVAL ARCHIVE ✦";
        titleStyle = "color: #00f2fe; margin-top: 5px;";
    } else if (m_won) {
        titleText = "★ MISSION ACCOMPLISHED ★";
        titleStyle = "color: #10b981; margin-top: 5px;";
    } else {
        titleText = "⚡ SYSTEM FAILURE ⚡";
        titleStyle = "color: #ef4444; margin-top: 5px;";
    }

    auto title = new QLabel(titleText, this);
    title->setAlignment(Qt::AlignCenter);
    title->setFont(QFont("Segoe UI", 18, QFont::Black));
    title->setStyleSheet(titleStyle);
    mainLayout->addWidget(title);

    // ۲. زیرعنوان
    QString subText;
    QString subColorStyle;
    if (isTimeAttack) {
        QString medals[] = {"BRONZE CLASSIFICATION", "SILVER OPERATIVE", "GOLD CHRONO-MASTER", "✦ PLATINUM CYBER-GOD ✦"};
        subText = medals[m_medalTier];
        subColorStyle = (m_medalTier == 3) ? "color: #c084fc; letter-spacing: 2px;" : ((m_medalTier == 2) ? "color: #fbbf24; letter-spacing: 2px;" : "color: #38bdf8; letter-spacing: 2px;");
    } else if (isEndless) {
        subText = "ENDLESS DEFENSE LOGGED";
        subColorStyle = "color: #38bdf8; letter-spacing: 2px;";
    } else if (m_won) {
        subText = (m_stars == 3 ? "PERFECT OPERATION" : (m_stars == 2 ? "EXCELLENT PERFORMANCE" : "MISSION CLEARED"));
        subColorStyle = "color: #34d399; letter-spacing: 2px;";
    } else {
        subText = "PERIMETER OVERRUN";
        subColorStyle = "color: #f87171; letter-spacing: 2px;";
    }

    auto subLabel = new QLabel(subText, this);
    subLabel->setAlignment(Qt::AlignCenter);
    subLabel->setFont(QFont("Segoe UI", 9, QFont::Bold));
    subLabel->setStyleSheet(subColorStyle);
    mainLayout->addWidget(subLabel);

    // فاصله اختصاصی برای رندر مدال یا ستاره‌ها
    mainLayout->addSpacing(85);

    // ۳. کارت مشخصات و رکورد
    auto statsWidget = new QWidget(this);
    statsWidget->setStyleSheet("background-color: rgba(15, 23, 42, 180); border-radius: 8px; border: 1px solid rgba(56, 189, 248, 60);");
    auto statsLayout = new QVBoxLayout(statsWidget);
    statsLayout->setContentsMargins(15, 10, 15, 10);
    statsLayout->setSpacing(4);

    auto opLabel = new QLabel(QString("OPERATIVE: %1 | PROTOCOL: %2").arg(m_username).arg(m_mode.toUpper()), statsWidget);
    opLabel->setAlignment(Qt::AlignCenter);
    opLabel->setFont(QFont("Segoe UI", 9.5, QFont::Bold));
    opLabel->setStyleSheet("color: #94a3b8; border: none; background: transparent;");
    statsLayout->addWidget(opLabel);

    auto scoreLabel = new QLabel(QString("FINAL SCORE: %1 PTS").arg(m_score), statsWidget);
    scoreLabel->setAlignment(Qt::AlignCenter);
    scoreLabel->setFont(QFont("Consolas", 18, QFont::Bold));
    scoreLabel->setStyleSheet("color: #00f2fe; border: none; background: transparent;");
    statsLayout->addWidget(scoreLabel);

    mainLayout->addWidget(statsWidget);
    mainLayout->addStretch();

    // ۴. دکمه‌ها
    auto btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    auto restartBtn = new QPushButton("REDEPLOY", this);
    restartBtn->setFont(QFont("Segoe UI", 10, QFont::Bold));
    restartBtn->setCursor(Qt::PointingHandCursor);
    restartBtn->setFixedHeight(42);
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

    auto returnBtn = new QPushButton("COMMAND MENU", this);
    returnBtn->setFont(QFont("Segoe UI", 10, QFont::Bold));
    returnBtn->setCursor(Qt::PointingHandCursor);
    returnBtn->setFixedHeight(42);
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
    bool isEndless = (m_mode.contains("Endless", Qt::CaseInsensitive));
    bool isTimeAttack = (m_mode.contains("Time", Qt::CaseInsensitive));

    // ۱. پس‌زمینه شیشه‌ای تیره
    QLinearGradient bgGrad(0, 0, 0, height());
    bgGrad.setColorAt(0.0, QColor(10, 16, 30, 245));
    bgGrad.setColorAt(1.0, QColor(5, 8, 16, 250));
    painter.setBrush(bgGrad);

    QColor frameColor = (isTimeAttack || isEndless) ? QColor(0, 242, 254, 180) : (m_won ? QColor(16, 185, 129, 180) : QColor(239, 68, 68, 180));
    painter.setPen(QPen(frameColor, 2.0));
    painter.drawRoundedRect(dialogRect, 14, 14);

    // ۲. رسم المان میانی (مدال Time-Attack / نشان Endless / ستاره‌های Campaign)
    qreal startCenterX = width() / 2.0;
    qreal midY = 120.0;

    // الف) رندر مدال‌های ۴گانه در Time-Attack
    if (isTimeAttack) {
        painter.save();
        painter.translate(startCenterX, midY);

        QColor medalColors[] = {
                QColor(205, 127, 50),   // Bronze
                QColor(203, 213, 225),  // Silver
                QColor(245, 158, 11),   // Gold
                QColor(192, 132, 252)   // Platinum
        };

        QColor currentMedalColor = medalColors[m_medalTier];

        // هاله نورانی مدال
        QRadialGradient medalGlow(0, 0, 48.0);
        medalGlow.setColorAt(0.0, QColor(currentMedalColor.red(), currentMedalColor.green(), currentMedalColor.blue(), 160));
        medalGlow.setColorAt(1.0, Qt::transparent);
        painter.setBrush(medalGlow);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(0, 0), 48.0, 48.0);

        // حلقه بیرونی چرخ‌دنده‌ای / مکانیکی
        painter.setBrush(QColor(15, 23, 42, 230));
        painter.setPen(QPen(currentMedalColor, 2.5));
        painter.drawEllipse(QPointF(0, 0), 28.0, 28.0);

        // دیسک اصلی مدال
        QLinearGradient medalGrad(0, -22, 0, 22);
        medalGrad.setColorAt(0.0, currentMedalColor.lighter(150));
        medalGrad.setColorAt(0.5, currentMedalColor);
        medalGrad.setColorAt(1.0, currentMedalColor.darker(170));
        painter.setBrush(medalGrad);
        painter.setPen(QPen(Qt::white, 1.2));
        painter.drawEllipse(QPointF(0, 0), 22.0, 22.0);

        // نشان وسط مدال
        painter.setPen(Qt::white);
        if (m_medalTier == 3) {
            painter.setFont(QFont("Segoe UI", 16, QFont::Black));
            painter.drawText(QRectF(-20, -20, 40, 40), Qt::AlignCenter, "✦");
        } else {
            QString roman[] = {"I", "II", "III"};
            painter.setFont(QFont("Consolas", 12, QFont::Bold));
            painter.drawText(QRectF(-20, -20, 40, 40), Qt::AlignCenter, roman[m_medalTier]);
        }

        painter.restore();
    }
        // ب) نشان بی‌نهایت Endless
    else if (isEndless) {
        painter.save();
        painter.translate(startCenterX, midY + 2);
        painter.setPen(QPen(QColor(0, 242, 254, 220), 4.0, Qt::SolidLine, Qt::RoundCap));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(QPointF(-22, 0), 18, 14);
        painter.drawEllipse(QPointF(22, 0), 18, 14);

        painter.setPen(QPen(Qt::white, 1.5));
        painter.drawEllipse(QPointF(-22, 0), 8, 6);
        painter.drawEllipse(QPointF(22, 0), 8, 6);
        painter.restore();
    }
        // ج) ستاره‌های Campaign
    else {
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
                QRadialGradient starGlow(0, 0, rOuter * 1.5);
                starGlow.setColorAt(0.0, QColor(245, 158, 11, 160));
                starGlow.setColorAt(0.6, QColor(251, 191, 36, 80));
                starGlow.setColorAt(1.0, Qt::transparent);
                painter.setBrush(starGlow);
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(QPointF(0,0), rOuter * 1.5, rOuter * 1.5);

                QLinearGradient goldGrad(0, -rOuter, 0, rOuter);
                goldGrad.setColorAt(0.0, QColor(255, 255, 220));
                goldGrad.setColorAt(0.3, QColor(251, 191, 36));
                goldGrad.setColorAt(0.8, QColor(245, 158, 11));
                goldGrad.setColorAt(1.0, QColor(180, 83, 9));
                painter.setBrush(goldGrad);
                painter.setPen(QPen(QColor(254, 240, 138), 1.5));
                painter.drawPolygon(starPoly);
            } else {
                painter.setBrush(QColor(30, 41, 59, 140));
                painter.setPen(QPen(QColor(71, 85, 105, 180), 1.5));
                painter.drawPolygon(starPoly);
            }
            painter.restore();
        };

        draw5PointStar(QPointF(startCenterX - starSpacing, midY + 6), 20.0, 9.0, m_stars >= 1, 0.9);
        draw5PointStar(QPointF(startCenterX, midY - 4), 26.0, 11.5, m_stars >= 2, 1.1);
        draw5PointStar(QPointF(startCenterX + starSpacing, midY + 6), 20.0, 9.0, m_stars >= 3, 0.9);
    }
}