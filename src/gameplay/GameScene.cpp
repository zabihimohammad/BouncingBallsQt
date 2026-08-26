#include "GameScene.h"
#include "../core/SoundManager.h"
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QRadialGradient>
#include <QLinearGradient>
#include <cmath>
#include <set>
#include <algorithm>

GameScene::GameScene(const QString& username, const QString& mode, int levelNumber, QObject* parent)
        : QGraphicsScene(0, 0, SCENE_W, SCENE_H, parent),
          m_username(username),
          m_mode(mode),
          m_levelNumber(levelNumber) {
    initGame();
}

GameScene::~GameScene() {
    if (m_gameLoopTimer) {
        m_gameLoopTimer->stop();
    }
}

void GameScene::initGame() {
    initSkillPods();

    // نودهای ماهواره‌ای کنترل در بالا سمت راست
    m_satelliteNodes.clear();
    m_satelliteNodes.append({"settings", "⚙ CONFIG", QPointF(885, 45), 22.0, false});
    m_satelliteNodes.append({"pause", "⏸ HOLD", QPointF(945, 45), 22.0, false});

    if (m_mode == "Random" || m_mode == "Endless" || m_mode == "CHAOS" || m_mode == "ENDLESS") {
        m_grid.generateRandomLevel();
    } else {
        m_grid.loadLevel(m_levelNumber);
    }

    m_aimLine = new AimLineItem(PLAYFIELD_X, PLAYFIELD_X + PLAYFIELD_W, SCENE_H, GridManager::BALL_RADIUS);
    addItem(m_aimLine);

    // قرارگیری لوله پرتاب در مرکز تقارن زمین (X = 500, Y = 585)
    m_cannon = new CannonItem(PLAYFIELD_W, SCENE_H);
    m_cannon->setPos(500.0, 585.0);
    addItem(m_cannon);

    prepareNextCannonBall();
    m_cannon->swapBalls();
    prepareNextCannonBall();

    m_flyingBallItem = addEllipse(0, 0, GridManager::BALL_DIAMETER, GridManager::BALL_DIAMETER);
    m_flyingBallItem->setZValue(15);
    m_flyingBallItem->setVisible(false);

    m_gameLoopTimer = new QTimer(this);
    connect(m_gameLoopTimer, &QTimer::timeout, this, &GameScene::updateGameLoop);
    m_gameLoopTimer->start(16);

    redrawGrid();
}

void GameScene::initSkillPods() {
    m_skillPods.clear();
    qreal startX = PLAYFIELD_X + 14.0;
    qreal podW = 120.0;
    qreal podH = 50.0;
    qreal gap = 8.0;
    qreal podY = 638.0;

    m_skillPods.append({BallType::Bomb, "BOMB", "[1]", QColor(231, 76, 60), 2, QRectF(startX, podY, podW, podH)});
    m_skillPods.append({BallType::Laser, "LASER", "[2]", QColor(0, 210, 211), 2, QRectF(startX + (podW + gap), podY, podW, podH)});
    m_skillPods.append({BallType::Rainbow, "RAINBOW", "[3]", QColor(255, 204, 0), 2, QRectF(startX + 2 * (podW + gap), podY, podW, podH)});
    m_skillPods.append({BallType::DualColor, "DUAL ORB", "[4]", QColor(165, 94, 234), 3, QRectF(startX + 3 * (podW + gap), podY, podW, podH)});
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

void GameScene::syncCannonColorsWithGrid() {
    auto available = m_grid.getRemainingColors();
    if (available.empty()) return;

    auto isColorAlive = [&](BallColor c) {
        return std::find(available.begin(), available.end(), c) != available.end();
    };

    if (m_cannon->getCurrentType() == BallType::Regular && !isColorAlive(m_cannon->getCurrentColor())) {
        int idx = QRandomGenerator::global()->bounded(static_cast<int>(available.size()));
        m_cannon->setCurrentBall(available[idx], BallType::Regular);
    }

    if (m_cannon->getNextType() == BallType::Regular && !isColorAlive(m_cannon->getNextColor())) {
        int idx = QRandomGenerator::global()->bounded(static_cast<int>(available.size()));
        m_cannon->setNextBall(available[idx], BallType::Regular);
    }
}

void GameScene::spawnPopParticles(const QPointF& pos, const QColor& color, int count) {
    for (int i = 0; i < count; ++i) {
        GameParticle p;
        p.pos = pos;
        qreal angle = QRandomGenerator::global()->bounded(360) * M_PI / 180.0;
        qreal speed = QRandomGenerator::global()->bounded(30, 120) / 10.0;
        p.vel = QPointF(std::cos(angle) * speed, std::sin(angle) * speed);
        p.color = color;
        p.size = QRandomGenerator::global()->bounded(4, 9);
        p.life = 1.0;
        m_particles.append(p);
    }
}

void GameScene::spawnFloatingText(const QPointF& pos, const QString& text, const QColor& color) {
    FloatingScoreText ft;
    ft.pos = pos;
    ft.text = text;
    ft.color = color;
    ft.life = 1.0;
    m_floatingTexts.append(ft);
}

void GameScene::triggerLaserBeamEffect(int row) {
    if (row < 0 || row >= GridManager::ROWS) return;
    qreal beamY = GridManager::BALL_RADIUS + row * (GridManager::BALL_DIAMETER * 0.866025);
    m_laserBeams.append({beamY, 1.0});
}

void GameScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (m_isPaused) return;

    QPointF mousePos = event->scenePos();
    QPointF cannonPos = m_cannon->pos();

    for (auto& node : m_satelliteNodes) {
        node.isHovered = (std::hypot(mousePos.x() - node.center.x(), mousePos.y() - node.center.y()) <= node.radius);
    }

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

    QPointF pos = event->scenePos();

    // ۱. کلیک روی نودهای ماهواره‌ای بالا
    for (const auto& node : m_satelliteNodes) {
        if (std::hypot(pos.x() - node.center.x(), pos.y() - node.center.y()) <= node.radius) {
            SoundManager::instance().playPop();
            if (node.id == "pause" || node.id == "settings") {
                emit pauseRequested();
            }
            return;
        }
    }

    // ۲. جابجایی سریع با کلیک راست
    if (event->button() == Qt::RightButton) {
        m_cannon->swapBalls();
        SoundManager::instance().playPop();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        // ۳. انتخاب مهارت از داک افقی زیر توپ
        for (auto& pod : m_skillPods) {
            if (pod.rect.contains(pos)) {
                if (pod.count > 0) {
                    pod.count--;
                    if (pod.type == BallType::DualColor) {
                        auto rem = m_grid.getRemainingColors();
                        BallColor c1 = BallColor::Red;
                        BallColor c2 = BallColor::Blue;

                        if (rem.size() >= 2) {
                            int idx1 = QRandomGenerator::global()->bounded(static_cast<int>(rem.size()));
                            int idx2 = QRandomGenerator::global()->bounded(static_cast<int>(rem.size() - 1));
                            if (idx2 >= idx1) idx2++;
                            c1 = rem[idx1];
                            c2 = rem[idx2];
                        } else if (rem.size() == 1) {
                            c1 = rem[0];
                            c2 = (c1 == BallColor::Red) ? BallColor::Blue : BallColor::Red;
                        } else {
                            c1 = Ball::getRandomColor(5);
                            do { c2 = Ball::getRandomColor(5); } while (c2 == c1);
                        }
                        m_cannon->setCurrentBall(c1, BallType::DualColor, c2);
                    } else {
                        BallColor activeCol = (pod.type == BallType::Rainbow) ? BallColor::None : m_cannon->getCurrentColor();
                        m_cannon->setCurrentBall(activeCol, pod.type);
                        m_loadedSecondaryColor = BallColor::None;
                    }
                    SoundManager::instance().playShoot();
                    spawnPopParticles(pod.rect.center(), pod.color, 15);
                    update();
                }
                return;
            }
        }

        // ۴. بررسی کلیک روی گوی ذخیره کانن
        QPointF cannonLocal = m_cannon->mapFromScene(pos);
        if (std::hypot(cannonLocal.x() - (-65.0), cannonLocal.y() - 0.0) <= 28.0) {
            m_cannon->swapBalls();
            SoundManager::instance().playPop();
            return;
        }

        // ۵. شلیک در محفظه شلیک
        if (pos.x() >= PLAYFIELD_X && pos.x() <= PLAYFIELD_X + PLAYFIELD_W && pos.y() < 630.0) {
            fireBall();
        }
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
    } else if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_4) {
        int idx = event->key() - Qt::Key_1;
        if (idx >= 0 && idx < m_skillPods.size() && m_skillPods[idx].count > 0) {
            m_skillPods[idx].count--;
            if (m_skillPods[idx].type == BallType::DualColor) {
                auto rem = m_grid.getRemainingColors();
                BallColor c1 = rem.empty() ? BallColor::Red : rem[0];
                BallColor c2 = (rem.size() > 1) ? rem[1] : BallColor::Blue;
                m_cannon->setCurrentBall(c1, BallType::DualColor, c2);
            } else {
                BallColor activeCol = (m_skillPods[idx].type == BallType::Rainbow) ? BallColor::None : m_cannon->getCurrentColor();
                m_cannon->setCurrentBall(activeCol, m_skillPods[idx].type);
            }
            SoundManager::instance().playShoot();
            update();
        }
    }
    QGraphicsScene::keyPressEvent(event);
}

void GameScene::fireBall() {
    m_isFlying = true;
    m_aimLine->clearAim();
    m_cannon->triggerFireRecoil();

    m_flyingColor = m_cannon->getCurrentColor();
    m_flyingType = m_cannon->getCurrentType();
    m_flyingSecondaryColor = m_cannon->getCurrentSecondaryColor();
    m_flyingPos = m_cannon->pos();

    qreal rad = m_cannon->getAngle() * M_PI / 180.0;
    qreal speed = 16.0;
    m_flyingVel = QPointF(std::cos(rad) * speed, -std::sin(rad) * speed);

    m_flyingBallItem->setRect(-GridManager::BALL_RADIUS, -GridManager::BALL_RADIUS,
                              GridManager::BALL_DIAMETER, GridManager::BALL_DIAMETER);
    m_flyingBallItem->setPos(m_flyingPos);
    m_flyingBallItem->setPen(Qt::NoPen);

    if (m_flyingType == BallType::DualColor) {
        QLinearGradient dualGrad(-GridManager::BALL_RADIUS, 0, GridManager::BALL_RADIUS, 0);
        QColor c1 = Ball::toQColor(m_flyingColor);
        QColor c2 = Ball::toQColor(m_flyingSecondaryColor);
        dualGrad.setColorAt(0.0, c1);
        dualGrad.setColorAt(0.48, c1);
        dualGrad.setColorAt(0.52, c2);
        dualGrad.setColorAt(1.0, c2);
        m_flyingBallItem->setBrush(dualGrad);
    } else if (m_flyingType == BallType::Rainbow) {
        QLinearGradient rainbowGrad(-GridManager::BALL_RADIUS, -GridManager::BALL_RADIUS, GridManager::BALL_RADIUS, GridManager::BALL_RADIUS);
        rainbowGrad.setColorAt(0.00, QColor(235, 77, 75));
        rainbowGrad.setColorAt(0.20, QColor(249, 202, 36));
        rainbowGrad.setColorAt(0.40, QColor(106, 176, 76));
        rainbowGrad.setColorAt(0.60, QColor(0, 210, 211));
        rainbowGrad.setColorAt(0.80, QColor(72, 52, 212));
        rainbowGrad.setColorAt(1.00, QColor(190, 46, 221));
        m_flyingBallItem->setBrush(rainbowGrad);
    } else {
        QColor flyColor = Ball::toQColor(m_flyingColor);
        if (m_flyingType == BallType::Bomb) flyColor = QColor(231, 76, 60);
        else if (m_flyingType == BallType::Laser) flyColor = QColor(0, 210, 211);
        m_flyingBallItem->setBrush(flyColor);
    }

    m_flyingBallItem->setVisible(true);

    m_cannon->setCurrentBall(m_cannon->getNextColor(), m_cannon->getNextType(), m_cannon->getNextSecondaryColor());
    prepareNextCannonBall();
    m_shotsFired++;

    SoundManager::instance().playShoot();
}

void GameScene::updateGameLoop() {
    if (m_isPaused) return;
    m_time += 0.04;

    for (auto& p : m_particles) {
        p.pos += p.vel;
        p.vel.setY(p.vel.y() + 0.12);
        p.life -= 0.035;
    }
    m_particles.erase(std::remove_if(m_particles.begin(), m_particles.end(),
                                     [](const GameParticle& p) { return p.life <= 0; }), m_particles.end());

    for (auto& ft : m_floatingTexts) {
        ft.pos.setY(ft.pos.y() - 1.2);
        ft.life -= 0.025;
    }
    m_floatingTexts.erase(std::remove_if(m_floatingTexts.begin(), m_floatingTexts.end(),
                                         [](const FloatingScoreText& ft) { return ft.life <= 0; }), m_floatingTexts.end());

    for (auto& lb : m_laserBeams) {
        lb.life -= 0.05;
    }
    m_laserBeams.erase(std::remove_if(m_laserBeams.begin(), m_laserBeams.end(),
                                      [](const LaserRayEffect& lb) { return lb.life <= 0; }), m_laserBeams.end());

    if (m_isFlying) {
        m_flyingPos += m_flyingVel;

        qreal leftWall = PLAYFIELD_X + GridManager::BALL_RADIUS;
        qreal rightWall = PLAYFIELD_X + PLAYFIELD_W - GridManager::BALL_RADIUS;

        if (m_flyingPos.x() <= leftWall) {
            m_flyingPos.setX(leftWall);
            m_flyingVel.setX(-m_flyingVel.x());
            SoundManager::instance().playBounce();
            spawnPopParticles(m_flyingPos, QColor(0, 242, 254), 6);
        } else if (m_flyingPos.x() >= rightWall) {
            m_flyingPos.setX(rightWall);
            m_flyingVel.setX(-m_flyingVel.x());
            SoundManager::instance().playBounce();
            spawnPopParticles(m_flyingPos, QColor(0, 242, 254), 6);
        }

        bool collided = false;
        if (m_flyingPos.y() <= GridManager::BALL_RADIUS) {
            collided = true;
        } else {
            QPointF localGridPos(m_flyingPos.x() - PLAYFIELD_X, m_flyingPos.y());
            for (int r = 0; r < GridManager::ROWS; ++r) {
                int cols = (r % 2 == 0) ? GridManager::COLS_EVEN : GridManager::COLS_ODD;
                for (int c = 0; c < cols; ++c) {
                    if (m_grid.isOccupied(r, c)) {
                        QPointF center = m_grid.getCenterPos(r, c);
                        qreal dist = std::hypot(center.x() - localGridPos.x(), center.y() - localGridPos.y());
                        if (dist <= GridManager::BALL_DIAMETER * 0.88) {
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
            m_flyingBallItem->setVisible(false);
            QPointF localHitPos(m_flyingPos.x() - PLAYFIELD_X, m_flyingPos.y());
            snapBallToGrid(localHitPos, m_flyingColor, m_flyingType);
        } else {
            m_flyingBallItem->setPos(m_flyingPos);
        }
    }

    update();
}

bool GameScene::findBestSnapSlot(const QPointF& hitPos, int& outR, int& outC) {
    int approxR, approxC;
    m_grid.getGridCoords(hitPos, approxR, approxC);

    if (!m_grid.isOccupied(approxR, approxC)) {
        outR = approxR;
        outC = approxC;
        return true;
    }

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
        auto* newBall = new Ball(color, type, m_flyingSecondaryColor, r, c);
        bool isKeyBall = newBall->isKey();
        m_grid.setBall(r, c, newBall);

        QPointF worldCenter = QPointF(PLAYFIELD_X, 0) + m_grid.getCenterPos(r, c);

        if (type == BallType::Bomb) {
            auto exploded = m_grid.explodeBomb(r, c);
            for (const auto& p : exploded) {
                QPointF pCenter = QPointF(PLAYFIELD_X, 0) + m_grid.getCenterPos(p.first, p.second);
                spawnPopParticles(pCenter, QColor(231, 76, 60), 18);
                m_grid.removeBall(p.first, p.second);
                m_score += 30;
            }
            spawnFloatingText(worldCenter, "+BOOM 3x3!", QColor(231, 76, 60));
            SoundManager::instance().playPop();
            m_shotsHit++;
            m_comboStreak++;
            m_grid.damageNeighbors(r, c, true);
        }
        else if (type == BallType::Laser) {
            int targetRow = r - 1;
            if (targetRow >= 0) {
                triggerLaserBeamEffect(targetRow);
                int cols = (targetRow % 2 == 0) ? GridManager::COLS_EVEN : GridManager::COLS_ODD;
                for (int colIdx = 0; colIdx < cols; ++colIdx) {
                    if (m_grid.isOccupied(targetRow, colIdx)) {
                        QPointF pCenter = QPointF(PLAYFIELD_X, 0) + m_grid.getCenterPos(targetRow, colIdx);
                        spawnPopParticles(pCenter, QColor(0, 210, 211), 16);
                        m_grid.removeBall(targetRow, colIdx);
                        m_score += 25;
                    }
                }
                spawnFloatingText(QPointF(PLAYFIELD_X + PLAYFIELD_W / 2.0, targetRow * 38.0 + 20.0),
                                  "LASER CLEARED!", QColor(0, 210, 211));
            }
            m_grid.removeBall(r, c);
            SoundManager::instance().playPop();
            m_shotsHit++;
            m_comboStreak++;
        }
        else if (type == BallType::Rainbow) {
            std::set<BallColor> touchedColors;
            for (const auto& nb : m_grid.getNeighbors(r, c)) {
                if (m_grid.isOccupied(nb.first, nb.second)) {
                    Ball* nbBall = m_grid.getBall(nb.first, nb.second);
                    if (nbBall && !nbBall->isBlack() && !nbBall->isFrozen() && nbBall->getPrimaryColor() != BallColor::None) {
                        touchedColors.insert(nbBall->getPrimaryColor());
                    }
                }
            }

            std::set<std::pair<int, int>> totalRainbowMatches;
            totalRainbowMatches.insert({r, c});

            for (BallColor col : touchedColors) {
                auto matches = m_grid.findMatches(r, c, col, BallType::Rainbow);
                for (const auto& matchPos : matches) {
                    totalRainbowMatches.insert(matchPos);
                }
            }

            int poppedCount = 0;
            for (const auto& p : totalRainbowMatches) {
                if (m_grid.isOccupied(p.first, p.second)) {
                    QPointF pCenter = QPointF(PLAYFIELD_X, 0) + m_grid.getCenterPos(p.first, p.second);
                    spawnPopParticles(pCenter, QColor(255, 204, 0), 16);
                    m_grid.removeBall(p.first, p.second);
                    poppedCount++;
                }
            }

            int gained = poppedCount * 30;
            m_score += gained;
            spawnFloatingText(worldCenter, QString("+%1 RAINBOW CASCADE!").arg(gained), QColor(255, 204, 0));
            SoundManager::instance().playPop();
            m_shotsHit++;
            m_comboStreak++;
            m_grid.damageNeighbors(r, c, isKeyBall);
        }
        else {
            popMatches(r, c, color, type, m_flyingSecondaryColor);
            m_grid.damageNeighbors(r, c, isKeyBall);
        }

        checkFloatingBalls();
        redrawGrid();
        syncCannonColorsWithGrid();
        emit scoreChanged(m_score);

        if (m_grid.isBottomReached()) {
            SoundManager::instance().playGameOver();
            emit gameOver(m_score);
        } else if (m_grid.isCleared()) {
            SoundManager::instance().playWin();
            emit gameWon(m_score);
        }
    }
}

void GameScene::popMatches(int r, int c, BallColor color, BallType type, BallColor secColor) {
    auto matches = m_grid.findMatches(r, c, color, type, secColor);
    if (!matches.empty()) {
        SoundManager::instance().playPop();
        m_shotsHit++;
        m_comboStreak++;
        int comboBonus = m_comboStreak * 10;
        int totalGained = 0;

        for (const auto& p : matches) {
            QPointF pCenter = QPointF(PLAYFIELD_X, 0) + m_grid.getCenterPos(p.first, p.second);
            spawnPopParticles(pCenter, Ball::toQColor(m_grid.getBall(p.first, p.second)->getPrimaryColor()), 14);
            m_grid.removeBall(p.first, p.second);
            int pts = 20 + comboBonus;
            m_score += pts;
            totalGained += pts;
        }

        QPointF textPos = QPointF(PLAYFIELD_X, 0) + m_grid.getCenterPos(r, c);
        QString scoreStr = QString("+%1").arg(totalGained);
        if (m_comboStreak > 1) {
            scoreStr += QString(" (x%1 COMBO!)").arg(m_comboStreak);
        }
        spawnFloatingText(textPos, scoreStr, (m_comboStreak > 1) ? QColor(245, 158, 11) : QColor(0, 242, 254));
    } else {
        m_comboStreak = 0;
    }
}

void GameScene::checkFloatingBalls() {
    auto floating = m_grid.findFloatingBalls();
    if (!floating.empty()) {
        int dropScore = 0;
        for (const auto& p : floating) {
            QPointF pCenter = QPointF(PLAYFIELD_X, 0) + m_grid.getCenterPos(p.first, p.second);
            spawnPopParticles(pCenter, QColor(148, 163, 184), 16);
            m_grid.removeBall(p.first, p.second);
            m_score += 50;
            dropScore += 50;
        }
        spawnFloatingText(QPointF(PLAYFIELD_X + PLAYFIELD_W / 2.0, 300),
                          QString("+%1 FALL BONUS!").arg(dropScore), QColor(16, 185, 129));
    }
}

void GameScene::redrawGrid() {
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
                qreal drawX = PLAYFIELD_X + center.x() - GridManager::BALL_RADIUS;
                qreal drawY = center.y() - GridManager::BALL_RADIUS;

                QBrush ballBrush;
                if (b->getType() == BallType::DualColor) {
                    QLinearGradient dualGrad(drawX, drawY, drawX + GridManager::BALL_DIAMETER, drawY);
                    QColor c1 = Ball::toQColor(b->getPrimaryColor());
                    QColor c2 = Ball::toQColor(b->getSecondaryColor());
                    dualGrad.setColorAt(0.0, c1);
                    dualGrad.setColorAt(0.48, c1);
                    dualGrad.setColorAt(0.52, c2);
                    dualGrad.setColorAt(1.0, c2);
                    ballBrush = QBrush(dualGrad);
                } else if (b->getType() == BallType::Rainbow) {
                    QLinearGradient rainbowGrad(drawX, drawY, drawX + GridManager::BALL_DIAMETER, drawY + GridManager::BALL_DIAMETER);
                    rainbowGrad.setColorAt(0.00, QColor(235, 77, 75));
                    rainbowGrad.setColorAt(0.20, QColor(249, 202, 36));
                    rainbowGrad.setColorAt(0.40, QColor(106, 176, 76));
                    rainbowGrad.setColorAt(0.60, QColor(0, 210, 211));
                    rainbowGrad.setColorAt(0.80, QColor(72, 52, 212));
                    rainbowGrad.setColorAt(1.00, QColor(190, 46, 221));
                    ballBrush = QBrush(rainbowGrad);
                } else {
                    ballBrush = QBrush(b->getDisplayColor());
                }

                auto* ellipse = addEllipse(drawX, drawY,
                                           GridManager::BALL_DIAMETER,
                                           GridManager::BALL_DIAMETER,
                                           Qt::NoPen,
                                           ballBrush);
                ellipse->setZValue(1);

                if (b->isFrozen()) {
                    auto* iceOverlay = addEllipse(drawX + 4, drawY + 4,
                                                  GridManager::BALL_DIAMETER - 8,
                                                  GridManager::BALL_DIAMETER - 8,
                                                  QPen(QColor(255, 255, 255, 200), 2, Qt::DashLine),
                                                  Qt::NoBrush);
                    iceOverlay->setZValue(2);
                } else if (b->isMystery()) {
                    auto* txt = addText("?", QFont("Consolas", 13, QFont::Black));
                    txt->setDefaultTextColor(QColor(255, 255, 255, 220));
                    txt->setPos(drawX + 13, drawY + 8);
                    txt->setZValue(2);
                } else if (b->isKey()) {
                    auto* keyTxt = addText("★", QFont("Consolas", 12, QFont::Bold));
                    keyTxt->setDefaultTextColor(Qt::black);
                    keyTxt->setPos(drawX + 13, drawY + 9);
                    keyTxt->setZValue(2);
                }
            }
        }
    }
}

// =========================================================================
// رندرینگ کنسول هولوگرافیک (Background & Foreground Layers)
// =========================================================================
void GameScene::drawBackground(QPainter* painter, const QRectF& rect) {
    Q_UNUSED(rect);
    painter->setRenderHint(QPainter::Antialiasing);

    drawDeepSpaceNebula(painter);
    drawForceFieldPlayfield(painter);
    drawTelemetryGlassHUD(painter);
    drawSatelliteControls(painter);
    drawOrbitalSkillPods(painter);
}

void GameScene::drawForeground(QPainter* painter, const QRectF& rect) {
    Q_UNUSED(rect);
    painter->setRenderHint(QPainter::Antialiasing);

    // پرتو لیزر افقی
    for (const auto& lb : m_laserBeams) {
        int alpha = int(lb.life * 255);
        painter->setPen(QPen(QColor(0, 210, 211, alpha), 8.0, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(QPointF(PLAYFIELD_X + 10, lb.y), QPointF(PLAYFIELD_X + PLAYFIELD_W - 10, lb.y));
        painter->setPen(QPen(QColor(255, 255, 255, alpha), 3.0, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(QPointF(PLAYFIELD_X + 10, lb.y), QPointF(PLAYFIELD_X + PLAYFIELD_W - 10, lb.y));
    }

    // ذرات معلق
    for (const auto& p : m_particles) {
        int alpha = int(p.life * 255);
        QColor col = p.color;
        col.setAlpha(alpha);
        painter->setPen(Qt::NoPen);
        painter->setBrush(col);
        painter->drawEllipse(p.pos, p.size * p.life, p.size * p.life);
    }

    // اعداد شناور
    for (const auto& ft : m_floatingTexts) {
        int alpha = int(ft.life * 255);
        QColor textColor = ft.color;
        textColor.setAlpha(alpha);
        painter->setPen(QColor(0, 0, 0, alpha));
        painter->setFont(QFont("Consolas", 13, QFont::Bold));
        painter->drawText(QRectF(ft.pos.x() - 120 + 1, ft.pos.y() - 15 + 1, 240, 30), Qt::AlignCenter, ft.text);
        painter->setPen(textColor);
        painter->drawText(QRectF(ft.pos.x() - 120, ft.pos.y() - 15, 240, 30), Qt::AlignCenter, ft.text);
    }
}

void GameScene::drawDeepSpaceNebula(QPainter* painter) {
    painter->fillRect(0, 0, SCENE_W, SCENE_H, QColor(4, 7, 14));

    QRadialGradient neb1(SCENE_W * 0.2, SCENE_H * 0.3, 700);
    neb1.setColorAt(0, QColor(0, 242, 254, 25));
    neb1.setColorAt(1, Qt::transparent);
    painter->fillRect(0, 0, SCENE_W, SCENE_H, neb1);

    QRadialGradient neb2(SCENE_W * 0.8, SCENE_H * 0.7, 750);
    neb2.setColorAt(0, QColor(255, 51, 102, 20));
    neb2.setColorAt(1, Qt::transparent);
    painter->fillRect(0, 0, SCENE_W, SCENE_H, neb2);
}

void GameScene::drawForceFieldPlayfield(QPainter* painter) {
    QRectF fieldRect(PLAYFIELD_X, 0, PLAYFIELD_W, 630.0);

    // محفظه شفاف شلیک
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(10, 16, 28, 200));
    painter->drawRect(fieldRect);

    // ستون‌های لیزری چپ و راست میدان نیرو
    qreal pulse = std::sin(m_time * 4.0) * 0.3 + 0.7;
    QPen borderPen(QColor(0, 242, 254, int(160 * pulse)), 3.0);
    painter->setPen(borderPen);
    painter->drawLine(QPointF(PLAYFIELD_X, 0), QPointF(PLAYFIELD_X, 630.0));
    painter->drawLine(QPointF(PLAYFIELD_X + PLAYFIELD_W, 0), QPointF(PLAYFIELD_X + PLAYFIELD_W, 630.0));

    // هاله ستون‌های کناری
    QLinearGradient glowLeft(PLAYFIELD_X - 15, 0, PLAYFIELD_X + 15, 0);
    glowLeft.setColorAt(0.0, Qt::transparent);
    glowLeft.setColorAt(0.5, QColor(0, 242, 254, 30));
    glowLeft.setColorAt(1.0, Qt::transparent);
    painter->fillRect(QRectF(PLAYFIELD_X - 15, 0, 30, 630), glowLeft);

    QLinearGradient glowRight(PLAYFIELD_X + PLAYFIELD_W - 15, 0, PLAYFIELD_X + PLAYFIELD_W + 15, 0);
    glowRight.setColorAt(0.0, Qt::transparent);
    glowRight.setColorAt(0.5, QColor(0, 242, 254, 30));
    glowRight.setColorAt(1.0, Qt::transparent);
    painter->fillRect(QRectF(PLAYFIELD_X + PLAYFIELD_W - 15, 0, 30, 630), glowRight);

    // افق بحرانی (Danger Line)
    qreal dangerY = GridManager::BALL_RADIUS + (GridManager::ROWS - 1) * (GridManager::BALL_DIAMETER * 0.866025);
    painter->setPen(QPen(QColor(239, 68, 68, 140), 1.5, Qt::DashLine));
    painter->drawLine(QPointF(PLAYFIELD_X, dangerY), QPointF(PLAYFIELD_X + PLAYFIELD_W, dangerY));
}

void GameScene::drawTelemetryGlassHUD(QPainter* painter) {
    QRectF hudRect(20, 20, 195, 240);

    QLinearGradient glass(hudRect.topLeft(), hudRect.bottomRight());
    glass.setColorAt(0.0, QColor(255, 255, 255, 15));
    glass.setColorAt(0.5, QColor(10, 16, 30, 220));
    glass.setColorAt(1.0, QColor(5, 8, 18, 240));

    painter->setBrush(glass);
    painter->setPen(QPen(QColor(0, 242, 254, 120), 1.5));
    painter->drawRoundedRect(hudRect, 10, 10);

    // تک‌آرت گوشه‌ها
    painter->setPen(QPen(Qt::white, 2.5));
    painter->drawLine(hudRect.topLeft() + QPointF(12, 0), hudRect.topLeft());
    painter->drawLine(hudRect.topLeft(), hudRect.topLeft() + QPointF(0, 12));

    painter->setPen(QColor(0, 242, 254));
    painter->setFont(QFont("Consolas", 8, QFont::Bold));
    painter->drawText(hudRect.adjusted(12, 10, -12, 0), "// TELEMETRY LINK");

    painter->setPen(Qt::white);
    painter->setFont(QFont("Consolas", 12, QFont::Bold));
    painter->drawText(hudRect.adjusted(12, 26, -12, 0), m_username);

    auto drawRow = [&](int yOff, QString label, QString val, QColor valCol) {
        painter->setPen(QColor(148, 163, 184));
        painter->setFont(QFont("Consolas", 8));
        painter->drawText(QRectF(32, yOff, 170, 15), label);

        painter->setPen(valCol);
        painter->setFont(QFont("Consolas", 14, QFont::Bold));
        painter->drawText(QRectF(32, yOff + 13, 170, 24), val);
    };

    drawRow(68, "SECTOR SCORE", QString::number(m_score), QColor(0, 242, 254));
    drawRow(115, "STREAK COMBO", QString("%1x").arg(m_comboStreak), QColor(245, 158, 11));

    int acc = (m_shotsFired > 0) ? (m_shotsHit * 100 / m_shotsFired) : 100;
    drawRow(162, "BEAM ACCURACY", QString("%1%").arg(acc), QColor(16, 185, 129));
    drawRow(208, "MISSION PROTOCOL", m_mode.toUpper(), QColor(255, 204, 0));
}

void GameScene::drawSatelliteControls(QPainter* painter) {
    for (const auto& node : m_satelliteNodes) {
        painter->save();
        painter->translate(node.center);

        qreal r = node.radius;
        if (node.isHovered) r *= 1.15;

        // هاله نوری
        QRadialGradient glow(0, 0, r * 1.5);
        glow.setColorAt(0.0, QColor(0, 242, 254, node.isHovered ? 140 : 40));
        glow.setColorAt(1.0, Qt::transparent);
        painter->setBrush(glow);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0, 0), r * 1.5, r * 1.5);

        // حلقه بیرونی چرخشی
        painter->save();
        painter->rotate(m_time * 20.0);
        painter->setPen(QPen(QColor(0, 242, 254, node.isHovered ? 255 : 120), 1.5, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(QPointF(0, 0), r * 1.2, r * 1.2);
        painter->restore();

        // هسته دکمه
        painter->setBrush(QColor(15, 23, 42, 220));
        painter->setPen(QPen(node.isHovered ? Qt::white : QColor(0, 242, 254), 1.5));
        painter->drawEllipse(QPointF(0, 0), r, r);

        painter->setPen(Qt::white);
        painter->setFont(QFont("Consolas", 8, QFont::Bold));
        painter->drawText(QRectF(-r, -r, r * 2, r * 2), Qt::AlignCenter, node.label);

        painter->restore();
    }
}

void GameScene::drawOrbitalSkillPods(QPainter* painter) {
    // کادر نگهدارنده داک افقی سلاح‌ها
    QRectF dockFrame(PLAYFIELD_X, 630.0, PLAYFIELD_W, 62.0);
    painter->setBrush(QColor(10, 16, 28, 230));
    painter->setPen(QPen(QColor(0, 242, 254, 80), 1.5));
    painter->drawRoundedRect(dockFrame, 8, 8);

    for (const auto& pod : m_skillPods) {
        bool available = pod.count > 0;

        QColor borderColor = available ? pod.color : QColor(60, 70, 85);
        painter->setPen(QPen(borderColor, 1.5));
        painter->setBrush(available ? QColor(pod.color.red(), pod.color.green(), pod.color.blue(), 25) : QColor(15, 20, 30, 150));
        painter->drawRoundedRect(pod.rect, 6, 6);

        // آیکون دایره‌ای
        painter->setPen(Qt::NoPen);
        painter->setBrush(available ? pod.color : QColor(60, 70, 85));
        painter->drawEllipse(pod.rect.left() + 8, pod.rect.top() + 11, 26, 26);

        // عنوان و کلید سریع
        painter->setPen(available ? Qt::white : QColor(120, 130, 145));
        painter->setFont(QFont("Consolas", 9, QFont::Bold));
        painter->drawText(QRectF(pod.rect.left() + 38, pod.rect.top() + 7, 75, 18), pod.name);

        painter->setPen(QColor(148, 163, 184));
        painter->setFont(QFont("Consolas", 7));
        painter->drawText(QRectF(pod.rect.left() + 38, pod.rect.top() + 26, 75, 16), QString("KEY %1").arg(pod.hotkey));

        // شمارنده باقیمانده
        QRectF countBadge(pod.rect.right() - 20, pod.rect.top() + 6, 16, 16);
        painter->setBrush(available ? QColor(0, 242, 254) : QColor(80, 90, 100));
        painter->drawRoundedRect(countBadge, 3, 3);

        painter->setPen(Qt::black);
        painter->setFont(QFont("Consolas", 8, QFont::Bold));
        painter->drawText(countBadge, Qt::AlignCenter, QString::number(pod.count));
    }
}

void GameScene::pauseGame() {
    m_isPaused = true;
    if (m_aimLine) m_aimLine->clearAim();
}

void GameScene::resumeGame() {
    m_isPaused = false;
}