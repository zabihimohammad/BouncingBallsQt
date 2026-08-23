#include "MainMenuWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QRandomGenerator>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QFrame>
#include <cmath>

MainMenuWidget::MainMenuWidget(QWidget* parent) : QWidget(parent) {
    setMouseTracking(true); 
    setupUI();
    initBalls();
    initBubbles();

    m_animTimer = new QTimer(this);
    connect(m_animTimer, &QTimer::timeout, this, &MainMenuWidget::updateAnimation);
    m_animTimer->start(16);
}

void MainMenuWidget::initBalls() {
    m_balls.clear();
    QVector<QColor> colors = {
        QColor(255, 51, 102), QColor(0, 204, 255), 
        QColor(51, 255, 153), QColor(255, 204, 0), 
        QColor(153, 51, 255)
    };

    for (int i = 0; i < 40; ++i) { 
        MenuBall ball;
        ball.color = colors[i % colors.size()];
        
        if (i < 15) {
            ball.isBackground = false;
            ball.radius = QRandomGenerator::global()->bounded(25, 50);
        } else {
            ball.isBackground = true;
            ball.radius = QRandomGenerator::global()->bounded(8, 20);
        }
        
        // محاسبه جرم بر اساس مساحت توپ
        ball.mass = ball.radius * ball.radius; 
        m_balls.append(ball);
    }
    
    // شلیک اولیه (با فرض ابعاد حدودی 800x600)
    resetBallsToCorners(800, 600);
}

void MainMenuWidget::resetBallsToCorners(int w, int h) {
    if (w <= 0 || h <= 0) return;
    
    for (int i = 0; i < m_balls.size(); ++i) {
        int corner = i % 4;
        qreal startX = (corner == 0 || corner == 2) ? -50 : w + 50;
        qreal startY = (corner == 0 || corner == 1) ? -50 : h + 50;
        
        m_balls[i].pos = QPointF(startX + QRandomGenerator::global()->bounded(50), 
                                 startY + QRandomGenerator::global()->bounded(50));
        
        // شلیک به سمت مرکز صفحه
        qreal vx = (startX < 0) ? (QRandomGenerator::global()->bounded(300, 700) / 100.0) : -(QRandomGenerator::global()->bounded(300, 700) / 100.0);
        qreal vy = (startY < 0) ? (QRandomGenerator::global()->bounded(300, 700) / 100.0) : -(QRandomGenerator::global()->bounded(300, 700) / 100.0);
        
        m_balls[i].velocity = QPointF(vx, vy);
    }
}

void MainMenuWidget::initBubbles() {
    m_bubbles.clear();
    for(int i = 0; i < 60; ++i) {
        GasBubble b;
        b.pos = QPointF(QRandomGenerator::global()->bounded(2000), QRandomGenerator::global()->bounded(1500));
        b.speed = (QRandomGenerator::global()->bounded(20) + 15) / 100.0; 
        b.size = QRandomGenerator::global()->bounded(2, 5); 
        b.wobblePhase = QRandomGenerator::global()->bounded(314) / 100.0;
        m_bubbles.append(b);
    }
}

void MainMenuWidget::setupUI() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 40);

    // نوار وضعیت
    auto topLayout = new QHBoxLayout();
    auto versionLabel = new QLabel("v1.3.0 | Physics Edition", this);
    versionLabel->setStyleSheet("color: rgba(255, 255, 255, 100); font-size: 13px; font-weight: bold; background: transparent;");
    
    auto hudLabel = new QLabel("👤 PLAYER 1   |   🏆 BEST: 12,400   |   🪙 350", this);
    hudLabel->setStyleSheet("background: rgba(15, 23, 42, 0.7); color: #00F2FE; padding: 10px 20px; border-radius: 12px; border: 1px solid rgba(0, 242, 254, 0.3); font-weight: bold; font-size: 14px;");
    
    topLayout->addWidget(versionLabel);
    topLayout->addStretch();
    topLayout->addWidget(hudLabel);
    mainLayout->addLayout(topLayout);

    mainLayout->addStretch(1);

    // عنوان بازی (دینامیک)
    m_titleLabel = new QLabel("CYBER BOUNCE", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setMinimumSize(0, 0); 
    
    auto shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(25);
    shadow->setColor(QColor(0, 242, 254, 120)); 
    shadow->setOffset(0, 0);
    m_titleLabel->setGraphicsEffect(shadow);
    
    mainLayout->addWidget(m_titleLabel);
    mainLayout->addStretch(1);

    // دکمه‌ها
    auto bottomLayout = new QHBoxLayout();
    auto glassPanel = new QFrame(this);
    glassPanel->setStyleSheet("QFrame { background: rgba(15, 23, 42, 0.65); border: 1px solid rgba(255, 255, 255, 0.15); border-radius: 20px; }");
    
    m_buttonsLayout = new QBoxLayout(QBoxLayout::LeftToRight, glassPanel);
    m_buttonsLayout->setContentsMargins(30, 20, 30, 20);
    m_buttonsLayout->setSpacing(20);
    m_buttonsLayout->setAlignment(Qt::AlignCenter);

    QString btnStyle = R"(
        QPushButton {
            background: rgba(255, 255, 255, 0.05);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 12px;
            padding: 12px 25px; 
            font-size: 18px; 
            font-weight: bold;
            color: #E2E8F0; 
            min-width: 150px;
        }
        QPushButton:hover {
            background: rgba(0, 242, 254, 0.15);
            border: 1px solid #00F2FE;
            color: #00F2FE; 
        }
    )";
    QString playStyle = btnStyle + "QPushButton { border: 1px solid rgba(0, 242, 254, 0.5); background: rgba(0, 242, 254, 0.1); color: #00F2FE; font-size: 20px; }";

    auto settingsBtn = new QPushButton("⚙ Settings", glassPanel);
    settingsBtn->setStyleSheet(btnStyle);
    connect(settingsBtn, &QPushButton::clicked, this, &MainMenuWidget::settingsClicked);

    auto startBtn = new QPushButton("🎮 PLAY GAME", glassPanel);
    startBtn->setStyleSheet(playStyle);
    connect(startBtn, &QPushButton::clicked, this, &MainMenuWidget::startGameClicked);

    auto scoreBtn = new QPushButton("🏆 Leaderboard", glassPanel);
    scoreBtn->setStyleSheet(btnStyle);
    connect(scoreBtn, &QPushButton::clicked, this, &MainMenuWidget::scoreboardClicked);

    auto exitBtn = new QPushButton("✖ Exit", glassPanel);
    exitBtn->setStyleSheet(btnStyle);
    connect(exitBtn, &QPushButton::clicked, this, &MainMenuWidget::exitClicked);

    m_buttonsLayout->addWidget(settingsBtn);
    m_buttonsLayout->addWidget(startBtn);
    m_buttonsLayout->addWidget(scoreBtn);
    m_buttonsLayout->addWidget(exitBtn);

    bottomLayout->addStretch();
    bottomLayout->addWidget(glassPanel);
    bottomLayout->addStretch();

    mainLayout->addLayout(bottomLayout);
}

void MainMenuWidget::resizeEvent(QResizeEvent* event) {
    int w = event->size().width();
    int h = event->size().height();

    // تنظیم سایز فونت به صورت کاملاً پویا بر اساس عرض صفحه
    int fontSize = qBound(35, w / 15, 100);
    m_titleLabel->setStyleSheet(QString("QLabel { background: transparent; color: #FFFFFF; font-size: %1px; font-weight: 900; letter-spacing: 8px; }").arg(fontSize));

    if (w < 850) {
        m_buttonsLayout->setDirection(QBoxLayout::TopToBottom);
    } else {
        m_buttonsLayout->setDirection(QBoxLayout::LeftToRight);
    }
    
    // شلیک مجدد توپ‌ها از گوشه‌ها هنگام تغییر سایز پنجره
    resetBallsToCorners(w, h);
    
    QWidget::resizeEvent(event);
}

void MainMenuWidget::mouseMoveEvent(QMouseEvent* event) {
    m_mousePos = event->position();
    QWidget::mouseMoveEvent(event);
}

void MainMenuWidget::updateAnimation() {
    m_time += 0.05;

    // ۱. آپدیت حباب‌های گاز
    for (auto& b : m_bubbles) {
        b.pos.setY(b.pos.y() - b.speed);
        b.pos.setX(b.pos.x() + std::sin(m_time + b.wobblePhase) * 0.4); 
        if (b.pos.y() < -10) {
            b.pos.setY(std::max(1000, height()) + 10);
            b.pos.setX(QRandomGenerator::global()->bounded(std::max(1, width())));
        }
    }

    // ۲. فیزیک برخورد توپ‌ها با یکدیگر (Elastic Collision)
    for (int i = 0; i < m_balls.size(); ++i) {
        for (int j = i + 1; j < m_balls.size(); ++j) {
            MenuBall& b1 = m_balls[i];
            MenuBall& b2 = m_balls[j];

            // فقط توپ‌های هم‌لایه با هم برخورد کنند
            if (b1.isBackground != b2.isBackground) continue;

            qreal dx = b2.pos.x() - b1.pos.x();
            qreal dy = b2.pos.y() - b1.pos.y();
            qreal dist = std::sqrt(dx*dx + dy*dy);
            qreal minDist = b1.radius + b2.radius;

            if (dist < minDist && dist > 0) {
                // رفع همپوشانی (جلوگیری از گیر کردن توپ‌ها در هم)
                qreal overlap = 0.5 * (minDist - dist);
                qreal nx = dx / dist;
                qreal ny = dy / dist;
                b1.pos.setX(b1.pos.x() - nx * overlap);
                b1.pos.setY(b1.pos.y() - ny * overlap);
                b2.pos.setX(b2.pos.x() + nx * overlap);
                b2.pos.setY(b2.pos.y() + ny * overlap);

                // محاسبه سرعت‌های جدید بر اساس تکانه و وزن
                qreal kx = b1.velocity.x() - b2.velocity.x();
                qreal ky = b1.velocity.y() - b2.velocity.y();
                qreal p = 2.0 * (nx * kx + ny * ky) / (b1.mass + b2.mass);

                b1.velocity.setX(b1.velocity.x() - p * b2.mass * nx);
                b1.velocity.setY(b1.velocity.y() - p * b2.mass * ny);
                b2.velocity.setX(b2.velocity.x() + p * b1.mass * nx);
                b2.velocity.setY(b2.velocity.y() + p * b1.mass * ny);
            }
        }
    }

    // ۳. آپدیت موقعیت و دافعه موس
    for (auto& ball : m_balls) {
        qreal dx = ball.pos.x() - m_mousePos.x();
        qreal dy = ball.pos.y() - m_mousePos.y();
        qreal dist = std::sqrt(dx*dx + dy*dy);
        
        qreal repelRadius = ball.isBackground ? 100.0 : 200.0;
        
        if (dist < repelRadius && dist > 0) {
            qreal force = (repelRadius - dist) / repelRadius;
            qreal multiplier = ball.isBackground ? 1.0 : 2.0;
            ball.velocity.setX(ball.velocity.x() + (dx / dist) * force * multiplier);
            ball.velocity.setY(ball.velocity.y() + (dy / dist) * force * multiplier);
        }

        ball.velocity.setX(ball.velocity.x() * 0.99); // اصطکاک ملایم‌تر
        ball.velocity.setY(ball.velocity.y() * 0.99);

        if (std::abs(ball.velocity.x()) < 0.2) ball.velocity.setX(ball.velocity.x() > 0 ? 0.2 : -0.2);
        if (std::abs(ball.velocity.y()) < 0.2) ball.velocity.setY(ball.velocity.y() > 0 ? 0.2 : -0.2);

        ball.pos += ball.velocity;

        if (ball.pos.x() - ball.radius < 0) { ball.pos.setX(ball.radius); ball.velocity.setX(std::abs(ball.velocity.x())); }
        else if (ball.pos.x() + ball.radius > width()) { ball.pos.setX(width() - ball.radius); ball.velocity.setX(-std::abs(ball.velocity.x())); }

        if (ball.pos.y() - ball.radius < 0) { ball.pos.setY(ball.radius); ball.velocity.setY(std::abs(ball.velocity.y())); }
        else if (ball.pos.y() + ball.radius > height()) { ball.pos.setY(height() - ball.radius); ball.velocity.setY(-std::abs(ball.velocity.y())); }
    }
    
    update(); 
}

void MainMenuWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QLinearGradient bgGrad(0, 0, 0, height());
    bgGrad.setColorAt(0.0, QColor(8, 12, 22)); 
    bgGrad.setColorAt(1.0, QColor(2, 4, 8));   
    painter.fillRect(rect(), bgGrad);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 30)); 
    for (const auto& b : m_bubbles) {
        painter.drawEllipse(b.pos, b.size, b.size);
    }

    for (const auto& ball : m_balls) {
        QRadialGradient ballGrad(ball.pos.x() - ball.radius * 0.3, ball.pos.y() - ball.radius * 0.3, ball.radius * 1.2);
        
        if (ball.isBackground) {
            ballGrad.setColorAt(0.0, QColor(ball.color.red(), ball.color.green(), ball.color.blue(), 100));
            ballGrad.setColorAt(1.0, QColor(ball.color.red(), ball.color.green(), ball.color.blue(), 20));
        } else {
            ballGrad.setColorAt(0.0, QColor(255, 255, 255, 255)); 
            ballGrad.setColorAt(0.3, QColor(ball.color.red(), ball.color.green(), ball.color.blue(), 220));
            ballGrad.setColorAt(1.0, QColor(ball.color.red(), ball.color.green(), ball.color.blue(), 60)); 
        }

        painter.setBrush(ballGrad);
        painter.drawEllipse(ball.pos, ball.radius, ball.radius);
    }
}