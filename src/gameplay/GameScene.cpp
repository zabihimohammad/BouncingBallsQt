#include "GameScene.h"
#include "../core/SoundManager.h"
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <cmath>

GameScene::GameScene(const QString& username, const QString& mode, QObject* parent)
        : QGraphicsScene(0, 0, 352, 600, parent),
          m_username(username),
          m_mode(mode) {

    // رنگ پس‌زمینه اختصاصی محیط نبرد
    setBackgroundBrush(QBrush(QColor(15, 23, 42)));

    initGame();
}

GameScene::~GameScene() {
    if (m_gameLoopTimer) {
        m_gameLoopTimer->stop();
    }
}

void GameScene::initGame() {
    // ۱. چیدمان اولیه مرحله کلاسیک
    m_grid.loadLevel(1);

    // ۲. ایجاد و افزودن خط نشانه‌گیری بازتابی
    m_aimLine = new AimLineItem(352, 600, GridManager::BALL_RADIUS);
    addItem(m_aimLine);

    // ۳. ایجاد و موقعیت‌دهی لوله پرتاب‌کننده توپ
    m_cannon = new CannonItem(352, 600);
    m_cannon->setPos(176, 545);
    addItem(m_cannon);

    // ۴. پر کردن خشاب اولیه کانن با رنگ‌های موجود در زمین
    prepareNextCannonBall();
    m_cannon->swapBalls();
    prepareNextCannonBall();

    // ۵. ایجاد آیتم گرافیکی برای نمایش گلوله در حال پرواز
    m_flyingBallItem = addEllipse(0, 0, GridManager::BALL_DIAMETER, GridManager::BALL_DIAMETER);
    m_flyingBallItem->setZValue(15);
    m_flyingBallItem->setVisible(false);

    // ۶. راه‌اندازی تایمر حلقه فیزیک بازی (۶۰ فریم بر ثانیه)
    m_gameLoopTimer = new QTimer(this);
    connect(m_gameLoopTimer, &QTimer::timeout, this, &GameScene::updateGameLoop);
    m_gameLoopTimer->start(16);

    // ۷. رندر اولیه شبکه گوی‌ها
    redrawGrid();
}

void GameScene::prepareNextCannonBall() {
    auto availableColors = m_grid.getRemainingColors();
    BallColor chosenColor = BallColor::Red;

    if (!availableColors.empty()) {
        int idx = QRandomGenerator::global()->bounded(static_cast<int>(availableColors.size()));
        chosenColor = availableColors[idx];
    } else {
        chosenColor = Ball::getRandomColor(5);
    }

    m_cannon->setNextBall(chosenColor, BallType::Regular);
}

void GameScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (m_isPaused) return;

    QPointF mousePos = event->scenePos();
    QPointF cannonPos = m_cannon->pos();

    // محاسبه زاویه پرتاب نسبت به موقعیت ماوس
    qreal angle = std::atan2(cannonPos.y() - mousePos.y(), mousePos.x() - cannonPos.x()) * 180.0 / M_PI;
    angle = std::clamp(angle, 15.0, 165.0);

    m_cannon->setAngle(angle);

    if (!m_isFlying) {
        m_aimLine->updateAim(cannonPos, angle);
    }

    QGraphicsScene::mouseMoveEvent(event);
}

void GameScene::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (m_isPaused || m_isFlying) return;

    if (event->button() == Qt::LeftButton) {
        fireBall();
    }

    QGraphicsScene::mousePressEvent(event);
}

void GameScene::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        emit pauseRequested();
    } else if (event->key() == Qt::Key_Space) {
        if (!m_isFlying) {
            m_cannon->swapBalls();
        }
    }
    QGraphicsScene::keyPressEvent(event);
}

void GameScene::fireBall() {
    m_isFlying = true;
    m_aimLine->clearAim();

    m_flyingColor = m_cannon->getCurrentColor();
    m_flyingType = m_cannon->getCurrentType();
    m_flyingPos = m_cannon->pos();

    // محاسبه بردار سرعت
    qreal rad = m_cannon->getAngle() * M_PI / 180.0;
    qreal speed = 15.0; // سرعت پیمایش گلوله در فریم
    m_flyingVel = QPointF(std::cos(rad) * speed, -std::sin(rad) * speed);

    // آماده‌سازی ظاهر گلوله پرنده
    m_flyingBallItem->setRect(-GridManager::BALL_RADIUS, -GridManager::BALL_RADIUS,
                              GridManager::BALL_DIAMETER, GridManager::BALL_DIAMETER);
    m_flyingBallItem->setPos(m_flyingPos);
    m_flyingBallItem->setPen(Qt::NoPen);
    m_flyingBallItem->setBrush(Ball::toQColor(m_flyingColor));
    m_flyingBallItem->setVisible(true);

    // به‌روزرسانی گلوله‌های لوله و خشاب
    m_cannon->setCurrentBall(m_cannon->getNextColor(), m_cannon->getNextType());
    prepareNextCannonBall();
    m_shotsFired++;

    SoundManager::instance().playShoot();
}

void GameScene::updateGameLoop() {
    if (m_isPaused || !m_isFlying) return;

    m_flyingPos += m_flyingVel;

    // ۱. بازتاب از دیواره‌های چپ و راست
    if (m_flyingPos.x() <= GridManager::BALL_RADIUS) {
        m_flyingPos.setX(GridManager::BALL_RADIUS);
        m_flyingVel.setX(-m_flyingVel.x());
    } else if (m_flyingPos.x() >= 352.0 - GridManager::BALL_RADIUS) {
        m_flyingPos.setX(352.0 - GridManager::BALL_RADIUS);
        m_flyingVel.setX(-m_flyingVel.x());
    }

    // ۲. بررسی برخورد با سقف یا توپ‌های چیده شده در ماتریس
    bool collided = false;

    // برخورد با سقف
    if (m_flyingPos.y() <= GridManager::BALL_RADIUS) {
        collided = true;
    } else {
        // برخورد با توپ‌های موجود در زمین
        for (int r = 0; r < GridManager::ROWS; ++r) {
            int cols = (r % 2 == 0) ? GridManager::COLS_EVEN : GridManager::COLS_ODD;
            for (int c = 0; c < cols; ++c) {
                if (m_grid.isOccupied(r, c)) {
                    QPointF center = m_grid.getCenterPos(r, c);
                    qreal dist = std::hypot(center.x() - m_flyingPos.x(), center.y() - m_flyingPos.y());

                    // آستانه برخورد شعاعی دو دایره (با ضریب ایمنی ۰.۸۸ برای اسنپ طبیعی)
                    if (dist <= GridManager::BALL_DIAMETER * 0.88) {
                        collided = true;
                        break;
                    }
                }
            }
            if (collided) break;
        }
    }

    // ۳. متوقف کردن پرواز و جایگذاری در شبکه
    if (collided) {
        m_isFlying = false;
        m_flyingBallItem->setVisible(false);
        snapBallToGrid(m_flyingPos, m_flyingColor, m_flyingType);
    } else {
        m_flyingBallItem->setPos(m_flyingPos);
    }

    update();
}

bool GameScene::findBestSnapSlot(const QPointF& hitPos, int& outR, int& outC) {
    int approxR, approxC;
    m_grid.getGridCoords(hitPos, approxR, approxC);

    // اگر مستقیم در یک خانه خالی بود
    if (!m_grid.isOccupied(approxR, approxC)) {
        outR = approxR;
        outC = approxC;
        return true;
    }

    // در غیر این صورت نزدیک‌ترین خانه خالی مجاور را پیدا می‌کنیم
    qreal minDistance = 1e9;
    bool found = false;

    for (int r = 0; r < GridManager::ROWS; ++r) {
        int cols = (r % 2 == 0) ? GridManager::COLS_EVEN : GridManager::COLS_ODD;
        for (int c = 0; c < cols; ++c) {
            if (!m_grid.isOccupied(r, c)) {
                QPointF center = m_grid.getCenterPos(r, c);
                qreal dist = std::hypot(center.x() - hitPos.x(), center.y() - hitPos.y());
                if (dist < minDistance) {
                    minDistance = dist;
                    outR = r;
                    outC = c;
                    found = true;
                }
            }
        }
    }
    return found;
}

void GameScene::snapBallToGrid(const QPointF& hitPos, BallColor color, BallType type) {
    int r = 0, c = 0;
    if (findBestSnapSlot(hitPos, r, c)) {
        auto* newBall = new Ball(color, type, BallColor::None, r, c);
        m_grid.setBall(r, c, newBall);

        // اجرای الگوریتم ترکیدن و ریزش
        popMatches(r, c, color, type);
        checkFloatingBalls();
        redrawGrid();

        // بررسی شرایط پایان بازی
        if (m_grid.isBottomReached()) {
            SoundManager::instance().playGameOver();
            emit gameOver(m_score);
        } else if (m_grid.isCleared()) {
            SoundManager::instance().playWin();
            emit gameWon(m_score);
        }
    }
}

void GameScene::popMatches(int r, int c, BallColor color, BallType type) {
    auto matches = m_grid.findMatches(r, c, color, type);
    if (!matches.empty()) {
        SoundManager::instance().playPop();
        for (const auto& p : matches) {
            m_grid.removeBall(p.first, p.second);
            m_score += 20; // ۲۰ امتیاز به ازای هر توپ در زنجیره
        }
        emit scoreChanged(m_score);
    }
}

void GameScene::checkFloatingBalls() {
    auto floating = m_grid.findFloatingBalls();
    if (!floating.empty()) {
        for (const auto& p : floating) {
            m_grid.removeBall(p.first, p.second);
            m_score += 40; // ۴۰ امتیاز جایزه به ازای هر توپ افتاده
        }
        emit scoreChanged(m_score);
    }
}

void GameScene::redrawGrid() {
    // پاک کردن گوی‌های قبلی شبکه بدون حذف لوله، خط نشانه و گلوله متحرک
    for (auto* item : items()) {
        if (item != m_cannon && item != m_aimLine && item != m_flyingBallItem) {
            removeItem(item);
            delete item;
        }
    }

    for (int r = 0; r < GridManager::ROWS; ++r) {
        int cols = (r % 2 == 0) ? GridManager::COLS_EVEN : GridManager::COLS_ODD;
        for (int c = 0; c < cols; ++c) {
            if (m_grid.isOccupied(r, c)) {
                const Ball* b = m_grid.getBall(r, c);
                QPointF center = m_grid.getCenterPos(r, c);

                auto* ellipse = addEllipse(center.x() - GridManager::BALL_RADIUS,
                                           center.y() - GridManager::BALL_RADIUS,
                                           GridManager::BALL_DIAMETER,
                                           GridManager::BALL_DIAMETER,
                                           Qt::NoPen,
                                           QBrush(b->getDisplayColor()));
                ellipse->setZValue(1);
            }
        }
    }
}

void GameScene::pauseGame() {
    m_isPaused = true;
    if (m_aimLine) m_aimLine->clearAim();
}

void GameScene::resumeGame() {
    m_isPaused = false;
}