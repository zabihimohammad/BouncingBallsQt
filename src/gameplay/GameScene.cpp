#include "GameScene.h"
#include "../core/SoundManager.h"
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <cmath>

GameScene::GameScene(const QString& username, const QString& mode, QObject* parent)
    : QGraphicsScene(0, 0, 352, 600, parent), m_username(username), m_mode(mode) {
    setBackgroundBrush(QBrush(QColor(30, 39, 46)));

    if (m_mode == "Random" || m_mode == "Endless") {
        m_grid.generateRandomLevel();
    } else {
        m_grid.loadLevel(1);
    }

    m_aimLine = new AimLineItem(352, 600);
    addItem(m_aimLine);

    m_cannon = new CannonItem(352, 600);
    m_cannon->setPos(176, 550);
    addItem(m_cannon);

    m_gameLoopTimer = new QTimer(this);
    connect(m_gameLoopTimer, &QTimer::timeout, this, &GameScene::updateGameLoop);
    m_gameLoopTimer->start(16);

    if (m_mode == "Time") {
        m_countdownTimer = new QTimer(this);
        connect(m_countdownTimer, &QTimer::timeout, this, &GameScene::onCountdownTimer);
        m_countdownTimer->start(1000);
    }

    if (m_mode == "Endless") {
        m_rowPushTimer = new QTimer(this);
        connect(m_rowPushTimer, &QTimer::timeout, this, &GameScene::onRowTimer);
        m_rowPushTimer->start(15000);
    }

    redrawGrid();
}

void GameScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (m_isPaused) return;
    QPointF mousePos = event->scenePos();
    QPointF cannonPos = m_cannon->pos();
    qreal angle = std::atan2(cannonPos.y() - mousePos.y(), mousePos.x() - cannonPos.x()) * 180.0 / M_PI;
    if (angle < 15) angle = 15;
    if (angle > 165) angle = 165;

    m_cannon->setAngle(angle);
    m_aimLine->updateAim(cannonPos, angle);
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
    m_flyingColor = m_cannon->getCurrentBall();
    m_flyingPos = m_cannon->pos();

    qreal rad = m_cannon->getAngle() * M_PI / 180.0;
    qreal speed = 14.0;
    m_flyingVel = QPointF(std::cos(rad) * speed, -std::sin(rad) * speed);

    auto remaining = m_grid.getRemainingColors();
    BallColor nextCol = remaining.empty() ? Ball::getRandomColor(5) : remaining[QRandomGenerator::global()->bounded((int)remaining.size())];
    m_cannon->setCurrentBall(m_cannon->getNextBall());
    m_cannon->setNextBall(nextCol);
    m_shotsFired++;

    SoundManager::instance().playShoot();
}

void GameScene::updateGameLoop() {
    if (m_isPaused) return;

    if (m_isFlying) {
        m_flyingPos += m_flyingVel;
        // Wall Bounce
        if (m_flyingPos.x() <= 22 || m_flyingPos.x() >= 352 - 22) {
            m_flyingVel.setX(-m_flyingVel.x());
        }

        // Top hit or ball collision check
        bool collided = false;
        if (m_flyingPos.y() <= 22) {
            collided = true;
        } else {
            for (int r = 0; r < GridManager::ROWS; ++r) {
                int cols = (r % 2 == 0) ? GridManager::COLS_EVEN : GridManager::COLS_ODD;
                for (int c = 0; c < cols; ++c) {
                    if (m_grid.isOccupied(r, c)) {
                        QPointF center = m_grid.getCenterPos(r, c);
                        qreal dist = std::hypot(center.x() - m_flyingPos.x(), center.y() - m_flyingPos.y());
                        if (dist < GridManager::BALL_DIAMETER * 0.9) {
                            collided = true;
                            break;
                        }
                    }
                }
                if (collided) break;
            }
        }

        if (collided) {
            m_isFlying = false;
            snapBallToGrid(m_flyingPos, m_flyingColor);
        }
    }
    update();
}

void GameScene::snapBallToGrid(const QPointF& pos, BallColor color) {
    int r, c;
    m_grid.getGridCoords(pos, r, c);
    m_grid.setBall(r, c, new Ball(color, r, c));
    popMatches(r, c, color);
    checkFloatingBalls();
    redrawGrid();

    if (m_grid.isBottomReached()) {
        SoundManager::instance().playGameOver();
        emit gameOver(m_score);
    } else if (m_grid.isCleared()) {
        SoundManager::instance().playWin();
        emit gameWon(m_score);
    }
}

void GameScene::popMatches(int r, int c, BallColor color) {
    auto matches = m_grid.findMatches(r, c, color);
    if (!matches.empty()) {
        SoundManager::instance().playPop();
        for (auto p : matches) {
            m_grid.removeBall(p.first, p.second);
            m_score += 20;
        }
        emit scoreChanged(m_score);
    }
}

void GameScene::checkFloatingBalls() {
    auto floating = m_grid.findFloatingBalls();
    for (auto p : floating) {
        m_grid.removeBall(p.first, p.second);
        m_score += 40;
    }
    if (!floating.empty()) {
        emit scoreChanged(m_score);
    }
}

void GameScene::redrawGrid() {
    // Clear old ball items except cannon and aim
    for (auto item : items()) {
        if (item != m_cannon && item != m_aimLine) {
            removeItem(item);
            delete item;
        }
    }

    for (int r = 0; r < GridManager::ROWS; ++r) {
        int cols = (r % 2 == 0) ? GridManager::COLS_EVEN : GridManager::COLS_ODD;
        for (int c = 0; c < cols; ++c) {
            if (m_grid.isOccupied(r, c)) {
                QPointF center = m_grid.getCenterPos(r, c);
                auto ellipse = addEllipse(center.x() - 20, center.y() - 20, 40, 40,
                    Qt::NoPen, QBrush(Ball::toQColor(m_grid.getBall(r, c)->getColor())));
                ellipse->setZValue(1);
            }
        }
    }
}

void GameScene::onRowTimer() {
    m_grid.addRowFromTop();
    redrawGrid();
    if (m_grid.isBottomReached()) {
        emit gameOver(m_score);
    }
}

void GameScene::onCountdownTimer() {
    m_timeLeft--;
    emit timeChanged(m_timeLeft);
    if (m_timeLeft <= 0) {
        m_countdownTimer->stop();
        emit gameOver(m_score);
    }
}

void GameScene::pauseGame() { m_isPaused = true; }
void GameScene::resumeGame() { m_isPaused = false; }
