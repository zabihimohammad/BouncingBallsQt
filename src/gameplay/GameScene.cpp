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
    initSkills();
    if (m_mode == "Random" || m_mode == "Endless" || m_mode == "CHAOS" || m_mode == "ENDLESS") {
        m_grid.generateRandomLevel();
    } else {
        m_grid.loadLevel(m_levelNumber);
    }

    m_aimLine = new AimLineItem(PLAYFIELD_X, PLAYFIELD_X + PLAYFIELD_W, SCENE_H, GridManager::BALL_RADIUS);
    addItem(m_aimLine);

    m_cannon = new CannonItem(PLAYFIELD_W, SCENE_H);
    m_cannon->setPos(PLAYFIELD_X + (PLAYFIELD_W / 2.0), 545);
    addItem(m_cannon);

    prepareNextCannonBall();
    m_cannon->swapBalls();
    prepareNextCannonBall();

    m_flyingBallItem = new BallItem(BallColor::None, BallType::Regular, BallColor::None, false, GridManager::BALL_RADIUS);
    addItem(m_flyingBallItem);
    m_flyingBallItem->setZValue(15);
    m_flyingBallItem->setVisible(false);

    m_particles.reserve(512);
    m_floatingTexts.reserve(64);
    m_shockwaves.reserve(32);
    m_laserBeams.reserve(16);

    m_gameLoopTimer = new QTimer(this);
    connect(m_gameLoopTimer, &QTimer::timeout, this, &GameScene::updateGameLoop);
    m_gameLoopTimer->start(16);

    redrawGrid();
}

void GameScene::initSkills() {
    m_skills.clear();
    m_skills.append({BallType::Bomb, "BOMB", "Explodes 3x3 Area", QColor(231, 76, 60), 2, QRectF(595, 140, 185, 80)});
    m_skills.append({BallType::Laser, "LASER BEAM", "Clears Target Row", ThemeManager::instance().getPrimaryColor(), 2, QRectF(595, 235, 185, 80)});
    m_skills.append({BallType::Rainbow, "RAINBOW", "Wildcard Cascade", QColor(255, 204, 0), 2, QRectF(595, 330, 185, 80)});
    m_skills.append({BallType::DualColor, "DUAL ORB", "Splits Two Colors", QColor(165, 94, 234), 3, QRectF(595, 425, 185, 80)});
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

void GameScene::armSkill(int index) {
    if (index < 0 || index >= m_skills.size()) return;
    if (m_isPaused || m_isFlying) return;

    // ۱. اگر همین مهارت مسلح بود، با کلیک مجدد لغو شود
    if (m_armedSkillIndex == index) {
        disarmSkill();
        SoundManager::instance().playPop();
        update();
        return;
    }

    // ۲. بررسی موجودی کافی
    if (m_skills[index].count <= 0) return;

    // ۳. ذخیره رنگ عادی قبل از لود مهارت
    if (m_armedSkillIndex == -1) {
        m_savedBaseColor = m_cannon->getCurrentColor();
    }

    m_armedSkillIndex = index;
    const auto& skill = m_skills[index];

    if (skill.type == BallType::DualColor) {
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
        BallColor activeCol = (skill.type == BallType::Rainbow) ? BallColor::None :
                              (m_savedBaseColor != BallColor::None ? m_savedBaseColor : m_cannon->getCurrentColor());
        m_cannon->setCurrentBall(activeCol, skill.type);
    }

    SoundManager::instance().playShoot();
    spawnPopParticles(skill.rect.center(), skill.color, 14);
    update();
}

void GameScene::disarmSkill() {
    if (m_armedSkillIndex != -1) {
        BallColor restoreCol = (m_savedBaseColor != BallColor::None) ? m_savedBaseColor : BallColor::Red;
        m_cannon->setCurrentBall(restoreCol, BallType::Regular);
        m_armedSkillIndex = -1;
        m_savedBaseColor = BallColor::None;
    }
}

void GameScene::spawnPopParticles(const QPointF& pos, const QColor& color, int count) {
    for (int i = 0; i < count; ++i) {
        GameParticle p;
        p.pos = pos;
        qreal angle = QRandomGenerator::global()->bounded(360) * M_PI / 180.0;
        qreal speed = QRandomGenerator::global()->bounded(30, 110) / 10.0;
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
    QPointF pos = event->scenePos();
    m_mouseHoverPos = pos;

    if (!m_isPaused && !m_isFlying && m_cannon) {
        QPointF cannonCenter = m_cannon->pos();
        QPointF dir = pos - cannonCenter;
        qreal angle = std::atan2(-dir.y(), dir.x()) * 180.0 / M_PI;

        angle = std::clamp(angle, 10.0, 170.0);
        m_cannon->setAngle(angle);

        if (m_aimLine) {
            m_aimLine->updateAim(m_cannon->pos(), angle);
        }

        m_aimAngleTelemetry = angle;
        m_aimBouncesTelemetry = (std::abs(90 - angle) > 30) ? (std::abs(90 - angle) > 60 ? 2 : 1) : 0;
    }
    update();
    QGraphicsScene::mouseMoveEvent(event);
}

void GameScene::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    QPointF pos = event->scenePos();

    if (event->button() == Qt::LeftButton) {
        if (QRectF(12, 410, 195, 45).contains(pos)) {
            emit pauseRequested();
            return;
        }
    }

    if (m_isPaused || m_isFlying) return;

    if (event->button() == Qt::RightButton) {
        if (m_armedSkillIndex != -1) {
            disarmSkill();
        } else {
            m_cannon->swapBalls();
        }
        SoundManager::instance().playPop();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        for (int i = 0; i < m_skills.size(); ++i) {
            if (m_skills[i].rect.contains(pos)) {
                armSkill(i);
                return;
            }
        }

        QPointF cannonLocal = m_cannon->mapFromScene(pos);
        if (m_cannon->isNextBallClicked(cannonLocal)) {
            if (m_armedSkillIndex != -1) {
                disarmSkill();
            } else {
                m_cannon->swapBalls();
            }
            SoundManager::instance().playPop();
            return;
        }

        if (pos.x() >= PLAYFIELD_X && pos.x() <= PLAYFIELD_X + PLAYFIELD_W) {
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
            if (m_armedSkillIndex != -1) {
                disarmSkill();
            } else {
                m_cannon->swapBalls();
            }
            SoundManager::instance().playPop();
        }
    } else if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_4) {
        armSkill(event->key() - Qt::Key_1);
    }
}

void GameScene::fireBall() {
    m_isFlying = true;
    m_aimLine->clearAim();
    m_cannon->triggerFireRecoil();

    m_flyingColor = m_cannon->getCurrentColor();
    m_flyingType = m_cannon->getCurrentType();
    m_flyingSecondaryColor = m_cannon->getCurrentSecondaryColor();
    m_flyingPos = m_cannon->pos();

    // کسر مهمات مهارت تنها در لحظه خروج تیر
    if (m_armedSkillIndex != -1) {
        if (m_skills[m_armedSkillIndex].count > 0) {
            m_skills[m_armedSkillIndex].count--;
        }
        m_armedSkillIndex = -1;
        m_savedBaseColor = BallColor::None;
    }

    qreal rad = m_cannon->getAngle() * M_PI / 180.0;
    qreal speed = 15.0;
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

    m_gameplayTimeSeconds += 0.016;
    m_displayedScore += (m_score - m_displayedScore) * 0.15;
    if (std::abs(m_score - m_displayedScore) < 0.5) m_displayedScore = m_score;

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

        // ۱. اثر بمب (انفجار شعاعی ۳x۳)
        if (type == BallType::Bomb) {
            auto exploded = m_grid.explodeBomb(r, c);
            for (const auto& p : exploded) {
                QPointF pCenter = QPointF(PLAYFIELD_X, 0) + m_grid.getCenterPos(p.first, p.second);
                spawnPopParticles(pCenter, QColor(231, 76, 60), 20);
                m_grid.removeBall(p.first, p.second);
                m_score += 30;
            }
            m_shockwaves.append({worldCenter, 25.0, 1.0, QColor(231, 76, 60)});
            emit shakeRequested(12);
            spawnFloatingText(worldCenter, "+BOOM 3x3!", QColor(231, 76, 60));
            SoundManager::instance().playPop();
            m_shotsHit++;
            m_comboStreak++;
            m_grid.damageNeighbors(r, c, true);
        }
            // ۲. اثر لیزر (پاکسازی سطر هدف)
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
            // ۳. اثر رنگین‌کمان (آبشار وایلدکارد)
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

            std::set<std::pair<int, int>> totalMatches;
            totalMatches.insert({r, c});

            for (BallColor col : touchedColors) {
                auto matches = m_grid.findMatches(r, c, col, BallType::Rainbow);
                for (const auto& mPos : matches) {
                    totalMatches.insert(mPos);
                }
            }

            int count = 0;
            for (const auto& p : totalMatches) {
                if (m_grid.isOccupied(p.first, p.second)) {
                    QPointF pCenter = QPointF(PLAYFIELD_X, 0) + m_grid.getCenterPos(p.first, p.second);
                    spawnPopParticles(pCenter, QColor(255, 204, 0), 16);
                    m_grid.removeBall(p.first, p.second);
                    count++;
                }
            }

            int pts = count * 35;
            m_score += pts;
            m_shockwaves.append({worldCenter, 20.0, 1.0, QColor(255, 204, 0)});
            spawnFloatingText(worldCenter, QString("+%1 RAINBOW CASCADE!").arg(pts), QColor(255, 204, 0));
            SoundManager::instance().playPop();
            m_shotsHit++;
            m_comboStreak++;
            m_grid.damageNeighbors(r, c, isKeyBall);
        }
            // ۴. تطبیق ۳تایی استاندارد یا توپ دورنگ
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

    drawPlayfieldFrame(painter);
    drawLeftHUD(painter);
    drawRightSkillPanel(painter);
}

void GameScene::drawForeground(QPainter* painter, const QRectF& rect) {
    Q_UNUSED(rect);
    painter->setRenderHint(QPainter::Antialiasing);

    for (const auto& lb : m_laserBeams) {
        int alpha = int(lb.life * 255);
        painter->setPen(QPen(QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), alpha), 8.0, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(QPointF(PLAYFIELD_X + 10, lb.y), QPointF(PLAYFIELD_X + PLAYFIELD_W - 10, lb.y));

        painter->setPen(QPen(QColor(255, 255, 255, alpha), 3.0, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(QPointF(PLAYFIELD_X + 10, lb.y), QPointF(PLAYFIELD_X + PLAYFIELD_W - 10, lb.y));
    }

    if (m_trail.size() > 1) {
        QPainterPath trailPath;
        trailPath.moveTo(m_trail.first());
        for (int i = 1; i < m_trail.size(); ++i) {
            trailPath.lineTo(m_trail[i]);
        }
        painter->setPen(QPen(QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), 150), 18.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawPath(trailPath);
        painter->setPen(QPen(QColor(255, 255, 255, 200), 6.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawPath(trailPath);
    }

    for (const auto& sw : m_shockwaves) {
        int alpha = int(sw.life * 255);
        QColor col = sw.color;
        col.setAlpha(alpha);
        painter->setPen(QPen(col, 4.0));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(sw.pos, sw.radius, sw.radius);

        QColor colGlow = sw.color;
        colGlow.setAlpha(alpha / 2);
        painter->setPen(QPen(colGlow, 10.0));
        painter->drawEllipse(sw.pos, sw.radius, sw.radius);
    }

    for (const auto& p : m_particles) {
        int alpha = int(p.life * 255);
        QColor col = p.color;
        col.setAlpha(alpha);
        painter->setPen(Qt::NoPen);
        painter->setBrush(col);
        painter->drawEllipse(p.pos, p.size * p.life, p.size * p.life);
    }

    for (const auto& ft : m_floatingTexts) {
        int alpha = int(ft.life * 255);
        QColor textColor = ft.color;
        textColor.setAlpha(alpha);

        painter->setPen(QColor(0, 0, 0, alpha));
        painter->setFont(QFont("Segoe UI", 12, QFont::Black));
        painter->drawText(QRectF(ft.pos.x() - 100 + 1, ft.pos.y() - 15 + 1, 200, 30), Qt::AlignCenter, ft.text);

        painter->setPen(textColor);
        painter->drawText(QRectF(ft.pos.x() - 100, ft.pos.y() - 15, 200, 30), Qt::AlignCenter, ft.text);
    }
}

void GameScene::drawPlayfieldFrame(QPainter* painter) {
    QRectF playfieldRect(PLAYFIELD_X, 0, PLAYFIELD_W, SCENE_H);

    QLinearGradient glassGrad(PLAYFIELD_X, 0, PLAYFIELD_X, SCENE_H);
    QColor cBg = ThemeManager::instance().getPrimaryColor();
    glassGrad.setColorAt(0.0, QColor(cBg.red()/4, cBg.green()/4, cBg.blue()/4, 70));
    glassGrad.setColorAt(0.5, QColor(cBg.red()/8, cBg.green()/8, cBg.blue()/8, 45));
    glassGrad.setColorAt(1.0, QColor(cBg.red()/4, cBg.green()/4, cBg.blue()/4, 70));
    painter->setBrush(glassGrad);
    painter->setPen(Qt::NoPen);
    painter->drawRect(playfieldRect);

    painter->setPen(QPen(QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), 70), 8.0, Qt::SolidLine, Qt::FlatCap));
    painter->drawLine(PLAYFIELD_X, 0, PLAYFIELD_X, SCENE_H);
    painter->setPen(QPen(QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), 230), 3.0, Qt::SolidLine, Qt::FlatCap));
    painter->drawLine(PLAYFIELD_X, 0, PLAYFIELD_X, SCENE_H);
    painter->setPen(QPen(QColor(255, 255, 255, 200), 1.0, Qt::SolidLine, Qt::FlatCap));
    painter->drawLine(PLAYFIELD_X, 0, PLAYFIELD_X, SCENE_H);

    painter->setPen(QPen(QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), 180), 1.5));
    for (int y = 20; y < SCENE_H - 20; y += 25) {
        painter->drawLine(PLAYFIELD_X, y, PLAYFIELD_X + 6, y);
    }

    qreal rX = PLAYFIELD_X + PLAYFIELD_W;
    painter->setPen(QPen(QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), 70), 8.0, Qt::SolidLine, Qt::FlatCap));
    painter->drawLine(rX, 0, rX, SCENE_H);
    painter->setPen(QPen(QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), 230), 3.0, Qt::SolidLine, Qt::FlatCap));
    painter->drawLine(rX, 0, rX, SCENE_H);
    painter->setPen(QPen(QColor(255, 255, 255, 200), 1.0, Qt::SolidLine, Qt::FlatCap));
    painter->drawLine(rX, 0, rX, SCENE_H);

    for (int y = 20; y < SCENE_H - 20; y += 25) {
        painter->drawLine(rX - 6, y, rX, y);
    }

    painter->setPen(QPen(QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), 200), 2.0));
    painter->drawLine(PLAYFIELD_X, 2, rX, 2);

    painter->setPen(QPen(Qt::white, 2.5));
    int b = 15;
    painter->drawLine(PLAYFIELD_X, 0, PLAYFIELD_X + b, 0);
    painter->drawLine(PLAYFIELD_X, 0, PLAYFIELD_X, b);
    painter->drawLine(rX, 0, rX - b, 0);
    painter->drawLine(rX, 0, rX, b);
    painter->drawLine(PLAYFIELD_X, SCENE_H, PLAYFIELD_X + b, SCENE_H);
    painter->drawLine(PLAYFIELD_X, SCENE_H, PLAYFIELD_X, SCENE_H - b);
    painter->drawLine(rX, SCENE_H, rX - b, SCENE_H);
    painter->drawLine(rX, SCENE_H, rX, SCENE_H - b);

    qreal dangerY = SCENE_H - 120;
    if (m_dangerLevel > 0.65) {
        painter->setPen(QPen(QColor(239, 68, 68, 220), 2.0, Qt::DashLine));
    } else {
        painter->setPen(QPen(QColor(239, 68, 68, 120), 1.5, Qt::DashLine));
    }
    painter->drawLine(PLAYFIELD_X + 5, dangerY, rX - 5, dangerY);
}

void GameScene::drawLeftHUD(QPainter* painter) {
    qint64 ms = QTime::currentTime().msecsSinceStartOfDay();
    qreal t = (ms % 600000) / 1000.0;

    QRectF profileCard(12, 12, 195, 90);
    painter->setBrush(QColor(10, 16, 28, 145));
    painter->setPen(QPen(QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), 80), 1.2));
    painter->drawRoundedRect(profileCard, 8, 8);

    painter->setPen(QPen(m_dangerLevel > 0.7 ? QColor(239, 68, 68) : QColor(16, 185, 129), 1.5));
    QPainterPath ecgPath;
    qreal startX = 18;
    qreal ecgY = 26;
    ecgPath.moveTo(startX, ecgY);
    for (int i = 0; i < 180; i += 5) {
        qreal x = startX + i;
        qreal modPhase = std::fmod(m_ecgPhase - (i * 0.02), 2.0 * M_PI);
        qreal yOff = 0;
        if (modPhase > 0 && modPhase < 1.0) {
            yOff = -12 * std::sin(modPhase * M_PI);
        } else if (modPhase > 1.0 && modPhase < 1.5) {
            yOff = 6 * std::sin((modPhase - 1.0) * M_PI * 2);
        }
        ecgPath.lineTo(x, ecgY + yOff);
    }
    painter->drawPath(ecgPath);

    painter->setPen(Qt::white);
    painter->setFont(QFont("Segoe UI", 11, QFont::Black));
    painter->drawText(QRectF(20, 38, 110, 20), m_username);

    QString rankTitle = "RANK: RECRUIT";
    QColor rankColor = QColor(148, 163, 184);
    if (m_score >= 3000) { rankTitle = "RANK: CYBER-ACE"; rankColor = QColor(168, 85, 247); }
    else if (m_score >= 1500) { rankTitle = "RANK: SHARPSHOOTER"; rankColor = QColor(245, 158, 11); }
    else if (m_score >= 500) { rankTitle = "RANK: SPECIALIST"; rankColor = ThemeManager::instance().getPrimaryColor(); }

    painter->setPen(rankColor);
    painter->setFont(QFont("Segoe UI", 7, QFont::Bold));
    painter->drawText(QRectF(20, 58, 120, 16), rankTitle);

    int totalSec = static_cast<int>(m_gameplayTimeSeconds);
    int mins = totalSec / 60;
    int secs = totalSec % 60;
    int frac = static_cast<int>((m_gameplayTimeSeconds - totalSec) * 10);
    QString timeStr = QString("%1:%2.%3").arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0')).arg(frac);

    painter->setPen(ThemeManager::instance().getPrimaryColor());
    painter->setFont(QFont("Consolas", 10, QFont::Bold));
    painter->drawText(QRectF(118, 42, 80, 20), Qt::AlignRight, timeStr);

    painter->setBrush(QColor(16, 185, 129));
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QPointF(142, 70), 3.5, 3.5);
    painter->setPen(QColor(16, 185, 129));
    painter->setFont(QFont("Segoe UI", 7, QFont::Bold));
    painter->drawText(QRectF(150, 62, 50, 18), "ONLINE");

    QRectF statsCard(12, 108, 195, 295);
    QColor statsBorder = (m_comboStreak >= 3) ? QColor(245, 158, 11, 160) : ((m_dangerLevel > 0.7) ? QColor(239, 68, 68, 160) : QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), 70));
    painter->setBrush(QColor(10, 16, 28, 145));
    painter->setPen(QPen(statsBorder, 1.2));
    painter->drawRoundedRect(statsCard, 8, 8);

    QString scoreStr = QString("%1").arg(static_cast<int>(m_displayedScore), 6, 10, QChar('0'));
    painter->setPen(QColor(148, 163, 184));
    painter->setFont(QFont("Segoe UI", 7, QFont::Bold));
    painter->drawText(QRectF(22, 116, 100, 15), "LIVE SCORE");

    painter->setPen((m_comboStreak >= 3) ? QColor(245, 158, 11) : ThemeManager::instance().getPrimaryColor());
    painter->setFont(QFont("Consolas", 18, QFont::Bold));
    painter->drawText(QRectF(22, 130, 170, 26), scoreStr);

    painter->setPen(QColor(148, 163, 184));
    painter->setFont(QFont("Segoe UI", 7, QFont::Bold));
    painter->drawText(QRectF(22, 162, 170, 14), QString("OVERDRIVE STREAK (x%1)").arg(m_comboStreak));

    int activeBars = std::min(m_comboStreak, 5);
    for (int b = 0; b < 5; ++b) {
        QRectF barRect(22 + b * 34, 178, 30, 8);
        if (b < activeBars) {
            QColor barCol = (activeBars >= 5) ? QColor(239, 68, 68) : ((activeBars >= 3) ? QColor(245, 158, 11) : ThemeManager::instance().getPrimaryColor());
            painter->setBrush(barCol);
            painter->setPen(Qt::NoPen);
            painter->drawRoundedRect(barRect, 2, 2);
        } else {
            painter->setBrush(QColor(30, 41, 59, 150));
            painter->setPen(QPen(QColor(71, 85, 105, 100), 1));
            painter->drawRoundedRect(barRect, 2, 2);
        }
    }

    int acc = m_shotsFired > 0 ? (m_shotsHit * 100 / m_shotsFired) : 0;
    QColor accCol = acc >= 60 ? QColor(16, 185, 129) : (acc >= 35 ? ThemeManager::instance().getPrimaryColor() : QColor(239, 68, 68));

    QRectF arcRect(24, 202, 50, 50);
    painter->setPen(QPen(QColor(30, 41, 59, 180), 4.5, Qt::SolidLine, Qt::RoundCap));
    painter->setBrush(Qt::NoBrush);
    painter->drawArc(arcRect, 225 * 16, -270 * 16);
    painter->setPen(QPen(accCol, 4.5, Qt::SolidLine, Qt::RoundCap));
    qreal spanAngle = -(acc / 100.0) * 270.0;
    painter->drawArc(arcRect, 225 * 16, int(spanAngle * 16));

    painter->setPen(accCol);
    painter->setFont(QFont("Consolas", 10, QFont::Bold));
    painter->drawText(arcRect, Qt::AlignCenter, QString("%1%").arg(acc));

    painter->setPen(QColor(148, 163, 184));
    painter->setFont(QFont("Segoe UI", 7.5, QFont::Bold));
    painter->drawText(QRectF(84, 210, 110, 16), "TARGET ACCURACY");
    painter->setFont(QFont("Consolas", 9));
    painter->setPen(Qt::white);
    painter->drawText(QRectF(84, 228, 110, 18), QString("HITS: %1 / %2").arg(m_shotsHit).arg(m_shotsFired));

    painter->setPen(QColor(148, 163, 184));
    painter->setFont(QFont("Segoe UI", 7, QFont::Bold));
    painter->drawText(QRectF(22, 264, 170, 14), "GRID SPECTRUM RADAR");

    auto dist = m_grid.getColorDistribution();
    int totalBalls = 0;
    for (const auto& pair : dist) totalBalls += pair.second;

    QRectF spectrumBar(22, 280, 175, 7);
    painter->setBrush(QColor(15, 23, 42));
    painter->setPen(QPen(QColor(56, 189, 248, 50), 1));
    painter->drawRoundedRect(spectrumBar, 2, 2);

    if (totalBalls > 0) {
        qreal curX = 22;
        for (const auto& pair : dist) {
            qreal w = (static_cast<qreal>(pair.second) / totalBalls) * 175.0;
            if (w > 1.0) {
                painter->setBrush(Ball::toQColor(pair.first));
                painter->setPen(Qt::NoPen);
                painter->drawRect(QRectF(curX, 280, w, 7));
                curX += w;
            }
        }
    }

    painter->setPen(QColor(148, 163, 184));
    painter->setFont(QFont("Consolas", 7.5));
    QString telemetry = QString("TRAJ: %1° | BNC: %2 | SECTOR: 01").arg(m_aimAngleTelemetry, 0, 'f', 1).arg(m_aimBouncesTelemetry);
    painter->drawText(QRectF(22, 305, 175, 18), telemetry);

    if (m_dangerLevel > 0.7 && std::fmod(t * 3.0, 1.0) < 0.5) {
        painter->setPen(QColor(239, 68, 68));
        painter->setFont(QFont("Segoe UI", 7.5, QFont::Bold));
        painter->drawText(QRectF(12, 375, 195, 18), Qt::AlignCenter, "⚠️ PERIMETER BREACH ⚠️");
    }

    QRectF menuBtn(12, 410, 195, 45);
    bool isHovered = menuBtn.contains(m_mouseHoverPos);
    painter->setBrush(isHovered ? QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), 45) : QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), 25));
    painter->setPen(QPen(isHovered ? QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), 255) : QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), 180), isHovered ? 2.0 : 1.5));
    QPolygonF btnPoly;
    btnPoly << QPointF(12, 420) << QPointF(22, 410) << QPointF(197, 410)
            << QPointF(207, 420) << QPointF(207, 445) << QPointF(197, 455)
            << QPointF(22, 455) << QPointF(12, 445);
    painter->drawPolygon(btnPoly);

    painter->setPen(Qt::white);
    painter->setFont(QFont("Segoe UI", 10.5, QFont::Bold));
    painter->drawText(menuBtn, Qt::AlignCenter, "[ ≡ TACTICAL MENU ]");
}

void GameScene::drawRightSkillPanel(QPainter* painter) {
    qint64 ms = QTime::currentTime().msecsSinceStartOfDay();
    qreal t = (ms % 600000) / 1000.0;

    QRectF headerCard(590, 12, 198, 65);
    painter->setBrush(QColor(10, 16, 28, 145));
    painter->setPen(QPen(QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), 80), 1.2));
    painter->drawRoundedRect(headerCard, 8, 8);

    painter->setPen(QPen(QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), 160), 1.5));
    for (int i = 0; i < 20; ++i) {
        qreal h = 4 + (QRandomGenerator::global()->bounded(16)) * (m_comboStreak > 1 ? 1.4 : 1.0);
        painter->drawLine(600 + i * 8, 40, 600 + i * 8, 40 - h);
    }

    painter->setPen(ThemeManager::instance().getPrimaryColor());
    painter->setFont(QFont("Segoe UI", 11, QFont::Black));
    painter->drawText(QRectF(590, 46, 198, 22), Qt::AlignCenter, "CYBER-ARSENAL");

    for (int i = 0; i < m_skills.size(); ++i) {
        const auto& skill = m_skills[i];
        bool available = skill.count > 0;
        bool isArmed = (m_armedSkillIndex == i);
        bool isHovered = skill.rect.contains(m_mouseHoverPos);

        QColor cardBorder = isArmed ? QColor(255, 255, 255) : (available ? (isHovered ? skill.color.lighter(130) : skill.color) : QColor(71, 85, 105));
        qreal bWidth = isArmed ? 2.5 : (isHovered ? 2.0 : (available ? 1.4 : 1.0));

        painter->setPen(QPen(cardBorder, bWidth));
        QLinearGradient cardGrad(skill.rect.topLeft(), skill.rect.bottomRight());
        int alphaTop = isArmed ? int(90 + 35 * std::sin(t * 8.0)) : (available ? (isHovered ? 75 : 45) : 15);
        cardGrad.setColorAt(0.0, available ? QColor(cardBorder.red(), cardBorder.green(), cardBorder.blue(), alphaTop) : QColor(10, 16, 28, 110));
        cardGrad.setColorAt(1.0, QColor(10, 16, 28, isHovered ? 175 : 140));
        painter->setBrush(cardGrad);
        painter->drawRoundedRect(skill.rect, 8, 8);

        QPointF hexCenter(skill.rect.left() + 30, skill.rect.top() + 32);
        qreal hexRadius = 22.0;

        if (available) {
            BallColor previewSec = (skill.type == BallType::DualColor) ? BallColor::Purple : BallColor::None;
            BallColor previewMain = (skill.type == BallType::DualColor) ? BallColor::Red : BallColor::None;
            BallItem::paintBall(painter, hexCenter, 14.0, previewMain, skill.type, previewSec, false);
        } else {
            QPolygonF hex;
            for (int h = 0; h < 6; ++h) {
                qreal a = h * M_PI / 3.0 + (M_PI / 2.0);
                hex << QPointF(hexCenter.x() + std::cos(a) * hexRadius, hexCenter.y() + std::sin(a) * hexRadius);
            }
            painter->setBrush(QColor(15, 20, 30, 140));
            painter->setPen(QPen(QColor(71, 85, 105), 1, Qt::DashLine));
            painter->drawPolygon(hex);

            painter->setPen(QColor(239, 68, 68, 150));
            painter->setFont(QFont("Segoe UI", 6, QFont::Bold));
            painter->drawText(QRectF(hexCenter.x() - 15, hexCenter.y() - 10, 30, 20), Qt::AlignCenter, "EMPTY");
        }

        painter->setPen(isArmed ? QColor(0, 242, 254) : (available ? Qt::white : QColor(148, 163, 184)));
        painter->setFont(QFont("Segoe UI", 9, QFont::Bold));
        painter->drawText(QRectF(skill.rect.left() + 60, skill.rect.top() + 16, 95, 20), skill.title);

        painter->setPen(isArmed ? QColor(255, 204, 0) : (available ? QColor(148, 163, 184) : QColor(71, 85, 105)));
        painter->setFont(QFont("Segoe UI", 7.5, isArmed ? QFont::Bold : QFont::Normal));
        QString subDesc = isArmed ? "● ARMED // CLICK TO CANCEL" : skill.desc;
        painter->drawText(QRectF(skill.rect.left() + 60, skill.rect.top() + 36, 120, 20), subDesc);

        QRectF keyBadge(skill.rect.right() - 48, skill.rect.top() + 8, 18, 16);
        painter->setBrush(QColor(15, 23, 42, 200));
        painter->setPen(QPen(isArmed ? QColor(255, 204, 0) : (available ? ThemeManager::instance().getPrimaryColor() : QColor(71, 85, 105)), 1.0));
        painter->drawRoundedRect(keyBadge, 3, 3);
        painter->setPen(isArmed ? QColor(255, 204, 0) : (available ? ThemeManager::instance().getPrimaryColor() : QColor(148, 163, 184)));
        painter->setFont(QFont("Consolas", 8, QFont::Bold));
        painter->drawText(keyBadge, Qt::AlignCenter, QString::number(i + 1));

        if (available) {
            painter->setBrush(isArmed ? QColor(255, 204, 0) : cardBorder);
            painter->setPen(Qt::NoPen);
            QPolygonF badgePoly;
            badgePoly << QPointF(skill.rect.right() - 25, skill.rect.top() + 8)
                      << QPointF(skill.rect.right() - 10, skill.rect.top() + 8)
                      << QPointF(skill.rect.right() - 5, skill.rect.top() + 16)
                      << QPointF(skill.rect.right() - 10, skill.rect.top() + 24)
                      << QPointF(skill.rect.right() - 25, skill.rect.top() + 24)
                      << QPointF(skill.rect.right() - 30, skill.rect.top() + 16);
            painter->drawPolygon(badgePoly);

            painter->setPen(Qt::black);
            painter->setFont(QFont("Segoe UI", 9, QFont::Bold));
            painter->drawText(QRectF(skill.rect.right() - 30, skill.rect.top() + 8, 25, 16), Qt::AlignCenter, QString::number(skill.count));
        }
    }
}

void GameScene::pauseGame() {
    m_isPaused = true;
}

void GameScene::resumeGame() {
    m_isPaused = false;
}