#include "GameScene.h"
#include "../core/SoundManager.h"
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <cmath>
#include <algorithm>

GameScene::GameScene(const QString& username, const QString& mode, QObject* parent)
        : QGraphicsScene(0, 0, SCENE_W, SCENE_H, parent),
          m_username(username),
          m_mode(mode) {
    initGame();
}

GameScene::~GameScene() {
    if (m_gameLoopTimer) {
        m_gameLoopTimer->stop();
    }
}

void GameScene::initGame() {
    initSkills();
    m_grid.loadLevel(1);

    m_aimLine = new AimLineItem(PLAYFIELD_X, PLAYFIELD_X + PLAYFIELD_W, SCENE_H, GridManager::BALL_RADIUS);
    addItem(m_aimLine);

    m_cannon = new CannonItem(PLAYFIELD_W, SCENE_H);
    m_cannon->setPos(PLAYFIELD_X + (PLAYFIELD_W / 2.0), 545);
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

void GameScene::initSkills() {
    m_skills.clear();
    m_skills.append({BallType::Bomb, "BOMB", "Explodes 3x3 Area", QColor(231, 76, 60), 2, QRectF(595, 140, 185, 80)});
    m_skills.append({BallType::Laser, "LASER BEAM", "Clears Top Row", QColor(0, 210, 211), 2, QRectF(595, 235, 185, 80)});
    m_skills.append({BallType::Rainbow, "RAINBOW", "Wildcard Match", QColor(255, 204, 0), 2, QRectF(595, 330, 185, 80)});
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
    if (m_isPaused) return;

    QPointF mousePos = event->scenePos();
    QPointF cannonPos = m_cannon->pos();

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

    if (event->button() == Qt::RightButton) {
        m_cannon->swapBalls();
        SoundManager::instance().playPop();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        for (auto& skill : m_skills) {
            if (skill.rect.contains(pos)) {
                if (skill.count > 0) {
                    skill.count--;
                    if (skill.type == BallType::DualColor) {
                        auto rem = m_grid.getRemainingColors();
                        BallColor c1 = BallColor::Red;
                        BallColor c2 = BallColor::Blue;

                        if (rem.size() >= 2) {
                            // انتخاب دو رنگ کاملاً تصادفی و غیرتکراری از بین رنگ‌های فعال زمین
                            int idx1 = QRandomGenerator::global()->bounded(static_cast<int>(rem.size()));
                            int idx2 = QRandomGenerator::global()->bounded(static_cast<int>(rem.size() - 1));
                            if (idx2 >= idx1) idx2++; // تضمین متفاوت بودن دو اندیس

                            c1 = rem[idx1];
                            c2 = rem[idx2];
                        } else if (rem.size() == 1) {
                            c1 = rem[0];
                            // اگر فقط یک رنگ در زمین بود، رنگ دوم یک رنگ مکمل متفاوت باشد
                            c2 = (c1 == BallColor::Red) ? BallColor::Blue : BallColor::Red;
                        } else {
                            c1 = Ball::getRandomColor(5);
                            do {
                                c2 = Ball::getRandomColor(5);
                            } while (c2 == c1);
                        }

                        m_cannon->setCurrentBall(c1, BallType::DualColor, c2);
                    } else {
                        BallColor activeCol = (skill.type == BallType::Rainbow) ? BallColor::None : m_cannon->getCurrentColor();
                        m_cannon->setCurrentBall(activeCol, skill.type);
                        m_loadedSecondaryColor = BallColor::None;
                    }
                    SoundManager::instance().playShoot();
                    spawnPopParticles(skill.rect.center(), skill.color, 15);
                    update();
                }
                return;
            }
        }

        QPointF cannonLocal = m_cannon->mapFromScene(pos);
        if (m_cannon->isNextBallClicked(cannonLocal)) {
            m_cannon->swapBalls();
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
            m_cannon->swapBalls();
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
    qreal speed = 15.0;
    m_flyingVel = QPointF(std::cos(rad) * speed, -std::sin(rad) * speed);

    m_flyingBallItem->setRect(-GridManager::BALL_RADIUS, -GridManager::BALL_RADIUS,
                              GridManager::BALL_DIAMETER, GridManager::BALL_DIAMETER);
    m_flyingBallItem->setPos(m_flyingPos);
    m_flyingBallItem->setPen(Qt::NoPen);

    // گرادیان دوتکه برای پرتابه در حال پرواز
    if (m_flyingType == BallType::DualColor) {
        QLinearGradient dualGrad(-GridManager::BALL_RADIUS, 0, GridManager::BALL_RADIUS, 0);
        QColor c1 = Ball::toQColor(m_flyingColor);
        QColor c2 = Ball::toQColor(m_flyingSecondaryColor);
        dualGrad.setColorAt(0.0, c1);
        dualGrad.setColorAt(0.48, c1);
        dualGrad.setColorAt(0.52, c2);
        dualGrad.setColorAt(1.0, c2);
        m_flyingBallItem->setBrush(dualGrad);
    } else {
        QColor flyColor = Ball::toQColor(m_flyingColor);
        if (m_flyingType == BallType::Rainbow) flyColor = QColor(255, 204, 0);
        else if (m_flyingType == BallType::Bomb) flyColor = QColor(231, 76, 60);
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

    // ۱. آپدیت پارتیکل‌ها
    for (auto& p : m_particles) {
        p.pos += p.vel;
        p.vel.setY(p.vel.y() + 0.12); // گرانش جزئی
        p.life -= 0.035;
    }
    m_particles.erase(std::remove_if(m_particles.begin(), m_particles.end(),
                                     [](const GameParticle& p) { return p.life <= 0; }), m_particles.end());

    // ۲. آپدیت متن‌های شناور
    for (auto& ft : m_floatingTexts) {
        ft.pos.setY(ft.pos.y() - 1.2);
        ft.life -= 0.025;
    }
    m_floatingTexts.erase(std::remove_if(m_floatingTexts.begin(), m_floatingTexts.end(),
                                         [](const FloatingScoreText& ft) { return ft.life <= 0; }), m_floatingTexts.end());

    // ۳. آپدیت پرتو لیزر
    for (auto& lb : m_laserBeams) {
        lb.life -= 0.05;
    }
    m_laserBeams.erase(std::remove_if(m_laserBeams.begin(), m_laserBeams.end(),
                                      [](const LaserRayEffect& lb) { return lb.life <= 0; }), m_laserBeams.end());

    // ۴. فیزیک حرکت گلوله
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
        else {
            // ارسال دقیق هر دو رنگ به الگوریتم مچینگ
            popMatches(r, c, color, type, m_flyingSecondaryColor);
        }

        m_grid.unlockNeighbors(r, c);
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

void GameScene::popMatches(int r, int c, BallColor color, BallType type) {
    auto matches = m_grid.findMatches(r, c, color, type);
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
                } else {
                    ballBrush = QBrush(b->getDisplayColor());
                }

                auto* ellipse = addEllipse(drawX, drawY,
                                           GridManager::BALL_DIAMETER,
                                           GridManager::BALL_DIAMETER,
                                           Qt::NoPen,
                                           ballBrush);
                ellipse->setZValue(1);
            }
        }
    }
}

// =========================================================================
// ترسیم لایه پس‌زمینه و جلوه‌های پیش‌زمینه (Foreground VFX)
// =========================================================================
void GameScene::drawBackground(QPainter* painter, const QRectF& rect) {
    Q_UNUSED(rect);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->fillRect(0, 0, SCENE_W, SCENE_H, QColor(8, 12, 22));

    drawPlayfieldFrame(painter);
    drawLeftHUD(painter);
    drawRightSkillPanel(painter);
}

void GameScene::drawForeground(QPainter* painter, const QRectF& rect) {
    Q_UNUSED(rect);
    painter->setRenderHint(QPainter::Antialiasing);

    // ۱. رسم پرتو لیزری نئونی افقی
    for (const auto& lb : m_laserBeams) {
        int alpha = int(lb.life * 255);
        painter->setPen(QPen(QColor(0, 210, 211, alpha), 8.0, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(QPointF(PLAYFIELD_X + 10, lb.y), QPointF(PLAYFIELD_X + PLAYFIELD_W - 10, lb.y));

        painter->setPen(QPen(QColor(255, 255, 255, alpha), 3.0, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(QPointF(PLAYFIELD_X + 10, lb.y), QPointF(PLAYFIELD_X + PLAYFIELD_W - 10, lb.y));
    }

    // ۲. رسم ذرات انفجاری نئونی
    for (const auto& p : m_particles) {
        int alpha = int(p.life * 255);
        QColor col = p.color;
        col.setAlpha(alpha);

        painter->setPen(Qt::NoPen);
        painter->setBrush(col);
        painter->drawEllipse(p.pos, p.size * p.life, p.size * p.life);
    }

    // ۳. رسم اعداد و عبارات امتیاز شناور (Combat Text)
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
    painter->setPen(QPen(QColor(56, 189, 248, 60), 2));
    painter->setBrush(QColor(15, 23, 42, 220));
    painter->drawRect(playfieldRect);

    qreal dangerY = GridManager::BALL_RADIUS + (GridManager::ROWS - 1) * (GridManager::BALL_DIAMETER * 0.866025);
    painter->setPen(QPen(QColor(239, 68, 68, 120), 1.5, Qt::DashLine));
    painter->drawLine(QPointF(PLAYFIELD_X, dangerY), QPointF(PLAYFIELD_X + PLAYFIELD_W, dangerY));
}

void GameScene::drawLeftHUD(QPainter* painter) {
    QRectF profileCard(15, 20, 195, 100);
    painter->setPen(QPen(QColor(56, 189, 248, 80), 1));
    painter->setBrush(QColor(15, 23, 42, 180));
    painter->drawRoundedRect(profileCard, 12, 12);

    painter->setPen(QColor(148, 163, 184));
    painter->setFont(QFont("Segoe UI", 9, QFont::Bold));
    painter->drawText(profileCard.adjusted(12, 12, -12, 0), "OPERATIVE PROFILE");

    painter->setPen(Qt::white);
    painter->setFont(QFont("Segoe UI", 13, QFont::Bold));
    painter->drawText(profileCard.adjusted(12, 35, -12, 0), m_username);

    painter->setPen(QColor(0, 242, 254));
    painter->setFont(QFont("Segoe UI", 10));
    painter->drawText(profileCard.adjusted(12, 65, -12, 0), QString("MODE: %1").arg(m_mode.toUpper()));

    QRectF scoreCard(15, 135, 195, 300);
    painter->setPen(QPen(QColor(56, 189, 248, 80), 1));
    painter->setBrush(QColor(15, 23, 42, 180));
    painter->drawRoundedRect(scoreCard, 12, 12);

    auto drawStat = [&](int yOff, QString label, QString val, QColor valCol) {
        painter->setPen(QColor(148, 163, 184));
        painter->setFont(QFont("Segoe UI", 9, QFont::Bold));
        painter->drawText(QRectF(27, yOff, 170, 20), label);

        painter->setPen(valCol);
        painter->setFont(QFont("Segoe UI", 16, QFont::Black));
        painter->drawText(QRectF(27, yOff + 18, 170, 30), val);
    };

    drawStat(155, "CURRENT SCORE", QString::number(m_score), QColor(0, 242, 254));
    drawStat(225, "COMBO STREAK", QString("%1x").arg(m_comboStreak), QColor(245, 158, 11));

    int acc = (m_shotsFired > 0) ? (m_shotsHit * 100 / m_shotsFired) : 100;
    drawStat(295, "HIT ACCURACY", QString("%1%").arg(acc), QColor(16, 185, 129));
    drawStat(365, "SHOTS FIRED", QString::number(m_shotsFired), QColor(241, 245, 249));

    QRectF hintCard(15, 450, 195, 130);
    painter->setPen(QPen(QColor(255, 255, 255, 30), 1));
    painter->setBrush(QColor(15, 23, 42, 120));
    painter->drawRoundedRect(hintCard, 10, 10);

    painter->setPen(QColor(148, 163, 184));
    painter->setFont(QFont("Segoe UI", 8, QFont::Bold));
    painter->drawText(hintCard.adjusted(10, 10, -10, -10),
                      "COMMAND CONTROLS:\n\n"
                      "• [R-CLICK] / [SPACE]:\n  Swap Ammo\n"
                      "• [RIGHT PANEL]:\n  Equip Special Skills\n"
                      "• [ESC]: Pause Menu");
}

void GameScene::drawRightSkillPanel(QPainter* painter) {
    painter->setPen(QColor(0, 242, 254));
    painter->setFont(QFont("Segoe UI", 12, QFont::Black));
    painter->drawText(QRectF(595, 25, 185, 30), Qt::AlignCenter, "TACTICAL ARSENAL");

    painter->setPen(QColor(148, 163, 184));
    painter->setFont(QFont("Segoe UI", 8));
    painter->drawText(QRectF(595, 50, 185, 40), Qt::AlignCenter, "Click to load skill\ninto cannon");

    for (const auto& skill : m_skills) {
        bool available = skill.count > 0;
        QColor cardBorder = available ? skill.color : QColor(71, 85, 105);
        painter->setPen(QPen(cardBorder, 1.5));
        painter->setBrush(available ? QColor(15, 23, 42, 200) : QColor(15, 23, 42, 100));
        painter->drawRoundedRect(skill.rect, 10, 10);

        painter->setPen(Qt::NoPen);
        painter->setBrush(available ? skill.color : QColor(71, 85, 105));
        painter->drawEllipse(skill.rect.left() + 15, skill.rect.top() + 25, 28, 28);

        painter->setPen(available ? Qt::white : QColor(148, 163, 184));
        painter->setFont(QFont("Segoe UI", 10, QFont::Bold));
        painter->drawText(QRectF(skill.rect.left() + 52, skill.rect.top() + 16, 125, 20), skill.title);

        painter->setPen(QColor(148, 163, 184));
        painter->setFont(QFont("Segoe UI", 8));
        painter->drawText(QRectF(skill.rect.left() + 52, skill.rect.top() + 38, 125, 18), skill.desc);

        QRectF badge(skill.rect.right() - 28, skill.rect.top() + 8, 20, 20);
        painter->setBrush(available ? QColor(0, 242, 254) : QColor(100, 116, 139));
        painter->drawRoundedRect(badge, 5, 5);

        painter->setPen(Qt::black);
        painter->setFont(QFont("Segoe UI", 9, QFont::Black));
        painter->drawText(badge, Qt::AlignCenter, QString::number(skill.count));
    }
}

void GameScene::pauseGame() {
    m_isPaused = true;
    if (m_aimLine) m_aimLine->clearAim();
}

void GameScene::resumeGame() {
    m_isPaused = false;
}