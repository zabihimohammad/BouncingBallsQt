#include "../ui/ThemeManager.h"
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

    m_flyingBallItem = new BallItem(BallColor::None, BallType::Regular, BallColor::None, false, GridManager::BALL_RADIUS);
    m_flyingBallItem->setZValue(15);
    m_flyingBallItem->setVisible(false);
    addItem(m_flyingBallItem);

    m_particles.reserve(512);
    m_floatingTexts.reserve(64);
    m_shockwaves.reserve(32);
    m_laserBeams.reserve(16);

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
    m_skillPods.append({BallType::Laser, "LASER", "[2]", ThemeManager::instance().getPrimaryColor(), 2, QRectF(startX + (podW + gap), podY, podW, podH)});
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

void GameScene::activateSkill(int index) {
    armSkill(index);
}

void GameScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    QPointF mousePos = event->scenePos();
    m_mouseHoverPos = mousePos;
    QPointF cannonPos = m_cannon ? m_cannon->pos() : QPointF(500, 585);

    for (auto& node : m_satelliteNodes) {
        node.isHovered = (std::hypot(mousePos.x() - node.center.x(), mousePos.y() - node.center.y()) <= node.radius);
    }

    qreal angle = std::atan2(cannonPos.y() - mousePos.y(), mousePos.x() - cannonPos.x()) * 180.0 / M_PI;
    angle = std::clamp(angle, 15.0, 165.0);

    if (m_cannon) m_cannon->setAngle(angle);

    if (!m_isFlying && m_aimLine) {
        m_aimLine->updateAim(cannonPos, angle);
    }

    QGraphicsScene::mouseMoveEvent(event);
}

void GameScene::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    QPointF pos = event->scenePos();

    // ۱. کلیک روی دکمه‌های ماهواره‌ای
    for (const auto& node : m_satelliteNodes) {
        if (std::hypot(pos.x() - node.center.x(), pos.y() - node.center.y()) <= node.radius) {
            emit pauseRequested();
            return;
        }
    }

    // ۲. جابجایی تیر با کلیک راست (در صورت فعال بودن مهارت، اول لغو شود)
    if (event->button() == Qt::RightButton) {
        if (m_armedSkillIndex != -1) {
            disarmSkill();
        } else if (m_cannon) {
            m_cannon->swapBalls();
        }
        SoundManager::instance().playPop();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        // ۳. مسلح‌سازی مهارت‌ها از داک پایین با کلیک ماوس
        for (int i = 0; i < m_skillPods.size(); ++i) {
            if (m_skillPods[i].rect.contains(pos)) {
                armSkill(i);
                return;
            }
        }

        // ۴. کلیک روی توپ رزرو کانن
        if (m_cannon) {
            QPointF cannonLocal = m_cannon->mapFromScene(pos);
            if (std::hypot(cannonLocal.x() - (-65.0), cannonLocal.y() - 0.0) <= 28.0) {
                if (m_armedSkillIndex != -1) {
                    disarmSkill();
                } else {
                    m_cannon->swapBalls();
                }
                SoundManager::instance().playPop();
                return;
            }
        }

        // ۵. شلیک در میدان نبرد
        if (pos.x() >= PLAYFIELD_X && pos.x() <= PLAYFIELD_X + PLAYFIELD_W && pos.y() < 630.0) {
            if (!m_isPaused && !m_isFlying) {
                fireBall();
            }
        }
    }

    QGraphicsScene::mousePressEvent(event);
}

void GameScene::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        emit pauseRequested();
    } else if (event->key() == Qt::Key_Space) {
        if (!m_isFlying && m_cannon) {
            if (m_armedSkillIndex != -1) {
                disarmSkill();
            } else {
                m_cannon->swapBalls();
            }
            SoundManager::instance().playPop();
        }
    } else if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_4) {
        int idx = event->key() - Qt::Key_1;
        armSkill(idx);
    }
}

void GameScene::fireBall() {
    m_isFlying = true;
    if (m_aimLine) m_aimLine->clearAim();
    m_cannon->triggerFireRecoil();

    m_flyingColor = m_cannon->getCurrentColor();
    m_flyingType = m_cannon->getCurrentType();
    m_flyingSecondaryColor = m_cannon->getCurrentSecondaryColor();
    m_flyingPos = m_cannon->pos();

    // ===> کسر مهمات مهارت فقط در لحظه شلیک واقعی <===
    if (m_armedSkillIndex != -1) {
        if (m_skillPods[m_armedSkillIndex].count > 0) {
            m_skillPods[m_armedSkillIndex].count--;
        }
        m_armedSkillIndex = -1;
        m_savedBaseColor = BallColor::None;
    }

    qreal rad = m_cannon->getAngle() * M_PI / 180.0;
    qreal speed = 16.0;
    m_flyingVel = QPointF(std::cos(rad) * speed, -std::sin(rad) * speed);

    m_flyingBallItem->setPos(m_flyingPos);
    m_flyingBallItem->updateData(m_flyingColor, m_flyingType, m_flyingSecondaryColor, false);
    m_flyingBallItem->setVisible(true);

    m_cannon->setCurrentBall(m_cannon->getNextColor(), m_cannon->getNextType(), m_cannon->getNextSecondaryColor());
    prepareNextCannonBall();
    m_shotsFired++;

    SoundManager::instance().playShoot();
}

void GameScene::updateGameLoop() {
    if (m_isPaused) return;
    m_time += 0.04;

    // --- Live HUD Telemetry Update ---
    m_gameplayTimeSeconds += 0.016;
    m_displayedScore += (m_score - m_displayedScore) * 0.15;
    if (std::abs(m_score - m_displayedScore) < 0.5) m_displayedScore = m_score;
    
    // Calculate Danger Level
    int lowestRow = 0;
    for (int r = GridManager::ROWS - 1; r >= 0; --r) {
        bool rowHasBall = false;
        int cols = (r % 2 == 0) ? GridManager::COLS_EVEN : GridManager::COLS_ODD;
        for (int c = 0; c < cols; ++c) {
            if (m_grid.isOccupied(r, c)) { rowHasBall = true; break; }
        }
        if (rowHasBall) { lowestRow = r; break; }
    }
    m_dangerLevel = static_cast<qreal>(lowestRow) / (GridManager::ROWS - 2.0);
    if (m_dangerLevel > 1.0) m_dangerLevel = 1.0;
    
    m_lowestGridY = lowestRow * GridManager::BALL_DIAMETER;
    m_ecgPhase += (m_dangerLevel > 0.6 ? 0.3 : 0.08);
    m_overdrivePhase += 0.1;
    // ---------------------------------

    // ۱. آپدیت پارتیکل‌ها و شوک‌ویوها
    for (auto& sw : m_shockwaves) {
        sw.radius += 8.0;
        sw.life -= 0.04;
    }
    m_shockwaves.erase(std::remove_if(m_shockwaves.begin(), m_shockwaves.end(),
                                     [](const Shockwave& sw) { return sw.life <= 0; }), m_shockwaves.end());

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
            spawnPopParticles(m_flyingPos, ThemeManager::instance().getPrimaryColor(), 6);
        } else if (m_flyingPos.x() >= rightWall) {
            m_flyingPos.setX(rightWall);
            m_flyingVel.setX(-m_flyingVel.x());
            SoundManager::instance().playBounce();
            spawnPopParticles(m_flyingPos, ThemeManager::instance().getPrimaryColor(), 6);
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
            m_trail.clear();
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
                        spawnPopParticles(pCenter, ThemeManager::instance().getPrimaryColor(), 16);
                        m_grid.removeBall(targetRow, colIdx);
                        m_score += 25;
                    }
                }
                spawnFloatingText(QPointF(PLAYFIELD_X + PLAYFIELD_W / 2.0, targetRow * 38.0 + 20.0),
                                  "LASER CLEARED!", ThemeManager::instance().getPrimaryColor());
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
        if (m_aimLine && m_cannon) {
            m_aimLine->updateAim(m_cannon->pos(), m_cannon->getAngle());
        }
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
        m_shockwaves.append({textPos, 20.0, 1.0, Ball::toQColor(color)});
        if (matches.size() >= 3) {
            emit shakeRequested(matches.size() * 3);
        }
        
        QString scoreStr = QString("+%1").arg(totalGained);
        if (m_comboStreak > 1) {
            scoreStr += QString(" (x%1 COMBO!)").arg(m_comboStreak);
        }
        spawnFloatingText(textPos, scoreStr, (m_comboStreak > 1) ? QColor(245, 158, 11) : ThemeManager::instance().getPrimaryColor());
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
        if (item->data(0).toString() == "grid_ball") {
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

                auto* ballItem = new BallItem(b, GridManager::BALL_RADIUS);
                ballItem->setPos(drawX + GridManager::BALL_RADIUS, drawY + GridManager::BALL_RADIUS);
                addItem(ballItem);
                ballItem->setZValue(1);
                ballItem->setData(0, "grid_ball");
            }
        }
    }
}

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
        painter->setPen(QPen(QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), alpha), 8.0, Qt::SolidLine, Qt::RoundCap));
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
    // Highly Translucent background so GameView wormhole is fully visible
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 0, 0));
    painter->drawRect(0, 0, SCENE_W, SCENE_H);
}

void GameScene::drawForceFieldPlayfield(QPainter* painter) {
    QRectF playfieldRect(PLAYFIELD_X, 0, PLAYFIELD_W, 630.0);

    QColor priCol = ThemeManager::instance().getPrimaryColor();
    QLinearGradient glass(PLAYFIELD_X, 0, PLAYFIELD_X, 630.0);
    glass.setColorAt(0.0, QColor(priCol.red()/10, priCol.green()/10, priCol.blue()/10, 80));
    glass.setColorAt(0.5, QColor(0, 0, 0, 45));
    glass.setColorAt(1.0, QColor(priCol.red()/10, priCol.green()/10, priCol.blue()/10, 80));

    painter->setBrush(glass);
    painter->setPen(QPen(QColor(priCol.red(), priCol.green(), priCol.blue(), 120), 2.0));
    painter->drawRect(playfieldRect);

    // خطوط فوتونی دو طرف زمین بازی
    painter->setPen(QPen(QColor(priCol.red(), priCol.green(), priCol.blue(), 180), 3.0));
    painter->drawLine(PLAYFIELD_X, 0, PLAYFIELD_X, 630.0);
    painter->drawLine(PLAYFIELD_X + PLAYFIELD_W, 0, PLAYFIELD_X + PLAYFIELD_W, 630.0);

    // خط بحرانی قرمز در پایین زمین (Danger Line)
    qreal dangerY = 560.0;
    painter->setPen(QPen(QColor(239, 68, 68, int(150 + 60 * std::sin(m_time * 4.0))), 2.0, Qt::DashLine));
    painter->drawLine(PLAYFIELD_X + 10, dangerY, PLAYFIELD_X + PLAYFIELD_W - 10, dangerY);
}

void GameScene::drawTelemetryGlassHUD(QPainter* painter) {
    QRectF hudRect(20, 20, 195, 260);

    QLinearGradient glass(hudRect.topLeft(), hudRect.bottomRight());
    glass.setColorAt(0.0, QColor(10, 16, 32, 220));
    glass.setColorAt(1.0, QColor(5, 8, 18, 240));

    QColor priCol = ThemeManager::instance().getPrimaryColor();
    painter->setBrush(glass);
    painter->setPen(QPen(QColor(priCol.red(), priCol.green(), priCol.blue(), 120), 1.5));
    painter->drawRoundedRect(hudRect, 10, 10);

    // تک‌آرت گوشه‌ها
    painter->setPen(QPen(Qt::white, 2.5));
    painter->drawLine(hudRect.topLeft() + QPointF(12, 0), hudRect.topLeft());
    painter->drawLine(hudRect.topLeft(), hudRect.topLeft() + QPointF(0, 12));

    painter->setPen(priCol);
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

    drawRow(68, "SECTOR SCORE", QString::number(int(m_displayedScore)), priCol);
    drawRow(115, "STREAK COMBO", QString("%1x").arg(m_comboStreak), QColor(245, 158, 11));

    int acc = (m_shotsFired > 0) ? (m_shotsHit * 100 / m_shotsFired) : 100;
    drawRow(162, "BEAM ACCURACY", QString("%1%").arg(acc), QColor(16, 185, 129));
    drawRow(208, "MISSION PROTOCOL", m_mode.toUpper(), QColor(255, 204, 0));
}

void GameScene::drawSatelliteControls(QPainter* painter) {
    QColor priCol = ThemeManager::instance().getPrimaryColor();
    for (const auto& node : m_satelliteNodes) {
        painter->save();
        painter->translate(node.center);

        qreal r = node.radius;
        if (node.isHovered) r *= 1.15;

        // هاله نوری
        QRadialGradient glow(0, 0, r * 1.5);
        glow.setColorAt(0.0, QColor(priCol.red(), priCol.green(), priCol.blue(), node.isHovered ? 140 : 40));
        glow.setColorAt(1.0, Qt::transparent);
        painter->setBrush(glow);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0, 0), r * 1.5, r * 1.5);

        // حلقه بیرونی چرخشی
        painter->save();
        painter->rotate(m_time * 20.0);
        painter->setPen(QPen(QColor(priCol.red(), priCol.green(), priCol.blue(), node.isHovered ? 255 : 120), 1.5, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(QPointF(0, 0), r * 1.2, r * 1.2);
        painter->restore();

        // هسته دکمه
        painter->setBrush(QColor(15, 23, 42, 220));
        painter->setPen(QPen(node.isHovered ? Qt::white : priCol, 1.5));
        painter->drawEllipse(QPointF(0, 0), r, r);

        painter->setPen(Qt::white);
        painter->setFont(QFont("Consolas", 8, QFont::Bold));
        painter->drawText(QRectF(-r, -r, r * 2, r * 2), Qt::AlignCenter, node.label);

        painter->restore();
    }
}

void GameScene::drawOrbitalSkillPods(QPainter* painter) {
    QRectF dockFrame(PLAYFIELD_X, 630.0, PLAYFIELD_W, 62.0);
    painter->setBrush(QColor(10, 16, 28, 230));
    QColor priCol = ThemeManager::instance().getPrimaryColor();
    painter->setPen(QPen(QColor(priCol.red(), priCol.green(), priCol.blue(), 80), 1.5));
    painter->drawRoundedRect(dockFrame, 8, 8);

    for (int i = 0; i < m_skillPods.size(); ++i) {
        const auto& pod = m_skillPods[i];
        bool available = pod.count > 0;
        bool isArmed = (m_armedSkillIndex == i);

        QColor borderColor = isArmed ? QColor(255, 255, 255) : (available ? pod.color : QColor(60, 70, 85));
        qreal borderWidth = isArmed ? 2.5 : 1.5;

        // پس‌زمینه درخشان‌تر در صورت مسلح بودن
        int bgAlpha = isArmed ? int(60 + 30 * std::sin(m_time * 8.0)) : (available ? 25 : 10);
        painter->setPen(QPen(borderColor, borderWidth));
        painter->setBrush(available ? QColor(pod.color.red(), pod.color.green(), pod.color.blue(), bgAlpha) : QColor(15, 20, 30, 150));
        painter->drawRoundedRect(pod.rect, 6, 6);

        // آیکون دایره‌ای
        painter->setPen(Qt::NoPen);
        painter->setBrush(available ? pod.color : QColor(60, 70, 85));
        painter->drawEllipse(pod.rect.left() + 8, pod.rect.top() + 11, 26, 26);

        // عنوان
        painter->setPen(isArmed ? priCol : (available ? Qt::white : QColor(120, 130, 145)));
        painter->setFont(QFont("Consolas", 9, QFont::Bold));
        painter->drawText(QRectF(pod.rect.left() + 38, pod.rect.top() + 7, 75, 18), pod.name);

        // کلید سریع یا وضعیت ARMED
        painter->setPen(isArmed ? QColor(255, 204, 0) : QColor(148, 163, 184));
        painter->setFont(QFont("Consolas", 7, isArmed ? QFont::Bold : QFont::Normal));
        QString subText = isArmed ? "● ARMED" : QString("KEY %1").arg(pod.hotkey);
        painter->drawText(QRectF(pod.rect.left() + 38, pod.rect.top() + 26, 75, 16), subText);

        // شمارنده باقیمانده
        QRectF countBadge(pod.rect.right() - 20, pod.rect.top() + 6, 16, 16);
        painter->setBrush(isArmed ? QColor(255, 204, 0) : (available ? priCol : QColor(80, 90, 100)));
        painter->drawRoundedRect(countBadge, 3, 3);

        painter->setPen(Qt::black);
        painter->setFont(QFont("Consolas", 8, QFont::Bold));
        painter->drawText(countBadge, Qt::AlignCenter, QString::number(pod.count));
    }
}

void GameScene::armSkill(int podIndex) {
    if (podIndex < 0 || podIndex >= m_skillPods.size()) return;

    // ۱. اگر همین مهارت در حال حاضر مسلح است، با کلیک مجدد لغو شود (Toggle Off)
    if (m_armedSkillIndex == podIndex) {
        disarmSkill();
        SoundManager::instance().playPop();
        update();
        return;
    }

    // ۲. بررسی داشتن موجودی کافی
    if (m_skillPods[podIndex].count <= 0) return;

    // ۳. اگر قبلاً مهارتی مسلح نبوده، رنگ توپ عادی فعلی ذخیره شود
    if (m_armedSkillIndex == -1 && m_cannon) {
        m_savedBaseColor = m_cannon->getCurrentColor();
    }

    m_armedSkillIndex = podIndex;
    const auto& pod = m_skillPods[podIndex];

    // ۴. لود کردن مهارت در کانن بدون کسر کردن count
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
        if (m_cannon) m_cannon->setCurrentBall(c1, BallType::DualColor, c2);
    } else {
        BallColor activeCol = (pod.type == BallType::Rainbow) ? BallColor::None :
                              (m_savedBaseColor != BallColor::None ? m_savedBaseColor : (m_cannon ? m_cannon->getCurrentColor() : BallColor::Red));
        if (m_cannon) m_cannon->setCurrentBall(activeCol, pod.type);
        m_loadedSecondaryColor = BallColor::None;
    }

    SoundManager::instance().playShoot();
    spawnPopParticles(pod.rect.center(), pod.color, 12);
    update();
}

void GameScene::disarmSkill() {
    if (m_armedSkillIndex != -1) {
        BallColor restoreCol = (m_savedBaseColor != BallColor::None) ? m_savedBaseColor : BallColor::Red;
        if (m_cannon) m_cannon->setCurrentBall(restoreCol, BallType::Regular);
        m_armedSkillIndex = -1;
        m_savedBaseColor = BallColor::None;
    }
}

void GameScene::pauseGame() {
    m_isPaused = true;
    if (m_aimLine) m_aimLine->clearAim();
}

void GameScene::resumeGame() {
    m_isPaused = false;
}
