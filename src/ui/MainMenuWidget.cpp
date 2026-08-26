#include "MainMenuWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QMouseEvent>
#include <cmath>
#include <algorithm>
#include "../core/SoundManager.h"

MainMenuWidget::MainMenuWidget(QWidget* parent) : QWidget(parent) {
    setMouseTracking(true); 
    setupUI();
    initBalls();
    initBubbles();
    initReactorNodes();

    m_animTimer = new QTimer(this);
    connect(m_animTimer, &QTimer::timeout, this, &MainMenuWidget::updateAnimation);
    m_animTimer->start(16);
}

void MainMenuWidget::initBalls() {
    m_balls.clear();
    QVector<QColor> colors = {
        QColor(0, 242, 254),   // Cyan
        QColor(255, 51, 102),  // Pink
        QColor(51, 255, 153),  // Emerald
        QColor(255, 204, 0),   // Gold
        QColor(153, 51, 255)   // Purple
    };

    for (int i = 0; i < 36; ++i) { 
        MenuBall ball;
        ball.color = colors[i % colors.size()];
        
        if (i < 14) {
            ball.isBackground = false;
            ball.radius = QRandomGenerator::global()->bounded(22, 38);
        } else {
            ball.isBackground = true;
            ball.radius = QRandomGenerator::global()->bounded(8, 16);
        }
        
        ball.mass = ball.radius * ball.radius; 
        m_balls.append(ball);
    }
    resetBallsToCorners(800, 600);
}

void MainMenuWidget::initBubbles() {
    m_bubbles.clear();
    for (int i = 0; i < 150; ++i) { 
        GasBubble b;
        b.pos = QPointF(QRandomGenerator::global()->bounded(3000), QRandomGenerator::global()->bounded(2000));
        b.zDepth = (QRandomGenerator::global()->bounded(100) / 100.0) * 2.5 + 0.5; // 0.5 (front) to 3.0 (back)
        b.size = QRandomGenerator::global()->bounded(2, 7) * (1.0 / b.zDepth); 
        b.speed = (QRandomGenerator::global()->bounded(20, 80)) / 100.0 * (1.0 / b.zDepth); 
        b.wobblePhase = QRandomGenerator::global()->bounded(314) / 100.0;
        m_bubbles.append(b);
    }
}

void MainMenuWidget::initReactorNodes() {
    m_reactors.clear();

    // 0: IGNITION (Main Game Launch)
    MenuReactorNode playNode;
    playNode.id = 0;
    playNode.label = "PLAY GAME";
    playNode.subLabel = "MISSION START";
    playNode.tag = "01 // LAUNCH";
    playNode.primaryColor = QColor(0, 242, 254);
    playNode.accentColor = QColor(255, 255, 255);
    playNode.radius = 55.0;
    m_reactors.append(playNode);

    // 1: SETTINGS (Configuration)
    MenuReactorNode setNode;
    setNode.id = 1;
    setNode.label = "SETTINGS";
    setNode.subLabel = "SYSTEM CONFIG";
    setNode.tag = "02 // MODULE";
    setNode.primaryColor = QColor(170, 70, 255);
    setNode.accentColor = QColor(0, 242, 254);
    setNode.radius = 42.0;
    m_reactors.append(setNode);

    // 2: DATABASE (Scoreboard)
    MenuReactorNode dbNode;
    dbNode.id = 2;
    dbNode.label = "LEADERBOARD";
    dbNode.subLabel = "HALL OF FAME";
    dbNode.tag = "03 // ARCHIVE";
    dbNode.primaryColor = QColor(255, 204, 0);
    dbNode.accentColor = QColor(255, 100, 50);
    dbNode.radius = 42.0;
    m_reactors.append(dbNode);

    // 3: EXIT (Abort)
    MenuReactorNode exitNode;
    exitNode.id = 3;
    exitNode.label = "EXIT";
    exitNode.subLabel = "ABORT SYSTEM";
    exitNode.tag = "04 // DISENGAGE";
    exitNode.primaryColor = QColor(255, 51, 102);
    exitNode.accentColor = QColor(255, 120, 150);
    exitNode.radius = 38.0;
    m_reactors.append(exitNode);

    // 4: HELP (Manual)
    MenuReactorNode helpNode;
    helpNode.id = 4;
    helpNode.label = "HELP";
    helpNode.subLabel = "FIELD MANUAL";
    helpNode.tag = "05 // ASSIST";
    helpNode.primaryColor = QColor(51, 255, 153);
    helpNode.accentColor = QColor(0, 255, 200);
    helpNode.radius = 38.0;
    m_reactors.append(helpNode);

    updateReactorLayout();
}

void MainMenuWidget::updateReactorLayout() {
    if (m_reactors.size() < 5) return;

    qreal cx = width() / 2.0;
    qreal bottomY = height() - 150.0; // Moved significantly higher

    // چیدمان افقی ۵ تایی ارگونومیک
    m_reactors[4].basePos = QPointF(cx - 320.0, bottomY + 15.0); // HELP (Far Left)
    m_reactors[1].basePos = QPointF(cx - 160.0, bottomY + 5.0);  // SETTINGS (Center Left)
    m_reactors[0].basePos = QPointF(cx, bottomY - 15.0);         // IGNITION (Center - Largest)
    m_reactors[2].basePos = QPointF(cx + 160.0, bottomY + 5.0);  // DATABASE (Center Right)
    m_reactors[3].basePos = QPointF(cx + 320.0, bottomY + 15.0); // TERMINATE (Far Right)
}

void MainMenuWidget::resetBallsToCorners(int w, int h) {
    if (w <= 0 || h <= 0) return;
    for (int i = 0; i < m_balls.size(); ++i) {
        int corner = i % 4;
        qreal startX = (corner == 0 || corner == 2) ? -60 : w + 60;
        qreal startY = (corner == 0 || corner == 1) ? -60 : h + 60;
        
        m_balls[i].pos = QPointF(startX + QRandomGenerator::global()->bounded(60), 
                                 startY + QRandomGenerator::global()->bounded(60));
        qreal vx = (startX < 0) ? (QRandomGenerator::global()->bounded(250, 600) / 100.0) : -(QRandomGenerator::global()->bounded(250, 600) / 100.0);
        qreal vy = (startY < 0) ? (QRandomGenerator::global()->bounded(250, 600) / 100.0) : -(QRandomGenerator::global()->bounded(250, 600) / 100.0);
        
        m_balls[i].velocity = QPointF(vx, vy);
        m_balls[i].trail.clear();
    }
}

void MainMenuWidget::setupUI() {
}

void MainMenuWidget::resizeEvent(QResizeEvent* event) {
    int w = event->size().width();
    int h = event->size().height();
    resetBallsToCorners(w, h);
    updateReactorLayout();
    QWidget::resizeEvent(event);
}

void MainMenuWidget::mouseMoveEvent(QMouseEvent* event) {
    m_mousePos = event->position();
    QWidget::mouseMoveEvent(event);
}

void MainMenuWidget::mousePressEvent(QMouseEvent* event) {
    if (m_isTransitioning) return;

    if (event->button() == Qt::LeftButton) {
        // بررسی کلیک روی راکتورها
        for (int i = 0; i < m_reactors.size(); ++i) {
            qreal dist = std::hypot(m_mousePos.x() - m_reactors[i].currentPos.x(), 
                                    m_mousePos.y() - m_reactors[i].currentPos.y());
            if (dist <= m_reactors[i].radius * 1.3) {
                m_isTransitioning = true;
                m_transitionTarget = m_reactors[i].id;
                SoundManager::instance().playPop();
                return;
            }
        }
        triggerGravityExplosion(event->position());
    } else if (event->button() == Qt::RightButton) {
        m_isVortexActive = true;
    }
}

void MainMenuWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        m_isVortexActive = false;
        triggerGravityExplosion(m_mousePos);
    }
}

void MainMenuWidget::triggerGravityExplosion(const QPointF& center) {
    MenuShockwave sw;
    sw.pos = center;
    sw.radius = 10;
    sw.maxRadius = 550;
    sw.intensity = 1.0;
    sw.color = QColor(0, 242, 254);
    m_shockwaves.append(sw);

    for (auto& ball : m_balls) {
        qreal dx = ball.pos.x() - center.x();
        qreal dy = ball.pos.y() - center.y();
        qreal dist = std::hypot(dx, dy);
        
        if (dist < 450 && dist > 1) {
            qreal force = (450 - dist) / 14.0;
            ball.velocity.setX(ball.velocity.x() + (dx / dist) * force);
            ball.velocity.setY(ball.velocity.y() + (dy / dist) * force);
        }
    }
    SoundManager::instance().playShoot(); 
}

void MainMenuWidget::spawnCollisionEffects(const QPointF& pos, const QColor& color, qreal force) {
    if (force > 3.0) {
        MenuShockwave sw;
        sw.pos = pos;
        sw.radius = 5;
        sw.maxRadius = force * 14.0;
        sw.intensity = 0.8;
        sw.color = color;
        m_shockwaves.append(sw);
    }
    int count = qBound(3, int(force * 1.4), 12);
    auto rng = QRandomGenerator::global();
    for (int i = 0; i < count; ++i) {
        MenuParticle p;
        p.pos = pos;
        qreal angle = rng->bounded(360) * M_PI / 180.0;
        qreal speed = rng->bounded(20, int(force * 25 + 25)) / 10.0;
        p.velocity = QPointF(std::cos(angle) * speed, std::sin(angle) * speed);
        p.maxLife = rng->bounded(30, 70) / 100.0;
        p.life = p.maxLife;
        p.color = color;
        p.size = rng->bounded(2, 5);
        m_particles.append(p);
    }
}

void MainMenuWidget::updateAnimation() {
    m_time += 0.025;

    // به‌روزرسانی نودهای راکتور (میکروانیمیشن‌ها، شناوری، هاور)
    m_hoveredReactorIdx = -1;
    for (int i = 0; i < m_reactors.size(); ++i) {
        auto& r = m_reactors[i];
        r.spinAngle += (1.0 + r.hover * 2.0);
        r.pulsePhase += 0.04;

        // شناوری آرام و موزون
        qreal floatOffset = std::sin(m_time * 2.0 + i * 1.2) * 5.0;
        r.currentPos = r.basePos + QPointF(0, floatOffset);

        // تشخیص هاور ماوس
        qreal distToMouse = std::hypot(m_mousePos.x() - r.currentPos.x(), m_mousePos.y() - r.currentPos.y());
        bool isHovered = (distToMouse <= r.radius * 1.3) && !m_isTransitioning;
        
        if (isHovered) {
            m_hoveredReactorIdx = i;
            r.hover = std::min(1.0, r.hover + 0.1);
        } else {
            r.hover = std::max(0.0, r.hover - 0.08);
        }
    }

    // اگر در حال ترنزیشن وارپ هستیم
    if (m_isTransitioning) {
        m_transitionProgress += 0.025;
        if (m_transitionProgress >= 1.0) {
            m_isTransitioning = false;
            m_transitionProgress = 0.0;
            
            // ریست کردن وضعیت کامل برای بازگشت بعدی
            resetBallsToCorners(width(), height());
            initBubbles();
            m_particles.clear();
            m_shockwaves.clear();
            
            if (m_transitionTarget == 0) emit startGameClicked();
            else if (m_transitionTarget == 1) emit settingsClicked();
            else if (m_transitionTarget == 2) emit scoreboardClicked();
            else if (m_transitionTarget == 3) emit exitClicked();
            else if (m_transitionTarget == 4) emit helpClicked();
            return;
        }

        QPointF center(width() / 2.0, height() / 2.0);
        qreal warpForce = 1.0 + (m_transitionProgress * 20.0); 
        
        for (auto& b : m_bubbles) {
            QPointF dir = b.pos - center;
            qreal dist = std::hypot(dir.x(), dir.y());
            if (dist > 0 && dist < 3000) {
                b.pos += (dir / dist) * (0.5 * warpForce);
            }
        }
        for (auto& ball : m_balls) {
            QPointF dir = ball.pos - center;
            qreal dist = std::hypot(dir.x(), dir.y());
            if (dist > 0 && dist < 3000) {
                ball.velocity += (dir / dist) * (0.8 * warpForce);
                qreal speed = std::hypot(ball.velocity.x(), ball.velocity.y());
                if (speed > 120.0) ball.velocity = (ball.velocity / speed) * 120.0;
                ball.pos += ball.velocity;
            }
        }
        update();
        return; 
    }

    // به‌روزرسانی حباب‌های پس‌زمینه
    for (auto& b : m_bubbles) {
        b.pos.setY(b.pos.y() - b.speed);
        b.pos.setX(b.pos.x() + std::sin(m_time * 1.5 + b.wobblePhase) * (0.6 / b.zDepth)); 
        if (b.pos.y() < -20) {
            b.pos.setY(height() + 40);
            b.pos.setX(QRandomGenerator::global()->bounded(std::max(1, width())));
        }
    }

    // فیزیک توپ‌ها نسبت به ماوس و انحراف ملایم در اطراف راکتورها
    for (auto& ball : m_balls) {
        qreal dx = ball.pos.x() - m_mousePos.x();
        qreal dy = ball.pos.y() - m_mousePos.y();
        qreal dist = std::hypot(dx, dy);
        
        if (m_isVortexActive) {
            if (dist > 20) {
                qreal force = 140.0 / dist; 
                ball.velocity.setX(ball.velocity.x() - (dx / dist) * force);
                ball.velocity.setY(ball.velocity.y() - (dy / dist) * force);
                qreal tanForce = 70.0 / dist;
                ball.velocity.setX(ball.velocity.x() - (dy / dist) * tanForce);
                ball.velocity.setY(ball.velocity.y() + (dx / dist) * tanForce);
            }
        } else {
            qreal repelRadius = ball.isBackground ? 100.0 : 200.0;
            if (dist < repelRadius && dist > 0) {
                qreal force = (repelRadius - dist) / repelRadius * 1.2;
                ball.velocity.setX(ball.velocity.x() + (dx / dist) * force);
                ball.velocity.setY(ball.velocity.y() + (dy / dist) * force);
            }
        }

        // انحراف ملایم در اطراف راکتورهای دکمه تا متن دکمه‌ها پوشانده نشود
        for (const auto& r : m_reactors) {
            qreal rdx = ball.pos.x() - r.currentPos.x();
            qreal rdy = ball.pos.y() - r.currentPos.y();
            qreal rdist = std::hypot(rdx, rdy);
            qreal protectRadius = r.radius + ball.radius + 20.0;
            if (rdist < protectRadius && rdist > 0) {
                qreal defl = (protectRadius - rdist) / protectRadius * 2.0;
                ball.velocity.setX(ball.velocity.x() + (rdx / rdist) * defl);
                ball.velocity.setY(ball.velocity.y() + (rdy / rdist) * defl);
            }
        }
    }

    // برخوردهای الاستیک توپ‌ها با یکدیگر
    for (int i = 0; i < m_balls.size(); ++i) {
        for (int j = i + 1; j < m_balls.size(); ++j) {
            MenuBall& b1 = m_balls[i];
            MenuBall& b2 = m_balls[j];
            if (b1.isBackground != b2.isBackground) continue;

            qreal dx = b2.pos.x() - b1.pos.x();
            qreal dy = b2.pos.y() - b1.pos.y();
            qreal dist = std::hypot(dx, dy);
            qreal minDist = b1.radius + b2.radius;

            if (dist < minDist && dist > 0) {
                qreal overlap = 0.5 * (minDist - dist);
                qreal nx = dx / dist;
                qreal ny = dy / dist;
                b1.pos.setX(b1.pos.x() - nx * overlap);
                b1.pos.setY(b1.pos.y() - ny * overlap);
                b2.pos.setX(b2.pos.x() + nx * overlap);
                b2.pos.setY(b2.pos.y() + ny * overlap);

                qreal kx = b1.velocity.x() - b2.velocity.x();
                qreal ky = b1.velocity.y() - b2.velocity.y();
                qreal impactForce = std::abs(nx * kx + ny * ky); 
                
                qreal p = 2.0 * (nx * kx + ny * ky) / (b1.mass + b2.mass);
                b1.velocity.setX(b1.velocity.x() - p * b2.mass * nx);
                b1.velocity.setY(b1.velocity.y() - p * b2.mass * ny);
                b2.velocity.setX(b2.velocity.x() + p * b1.mass * nx);
                b2.velocity.setY(b2.velocity.y() + p * b1.mass * ny);
                
                if (impactForce > 2.0) {
                    QPointF midPoint = (b1.pos + b2.pos) / 2.0;
                    spawnCollisionEffects(midPoint, (impactForce > 4.5) ? QColor(255,255,255) : b1.color, impactForce);
                    if (impactForce > 7.0 && !b1.isBackground) SoundManager::instance().playBounce();
                }
            }
        }
    }

    // حرکت و ردپای توپ‌ها
    for (auto& ball : m_balls) {
        ball.velocity *= (m_isVortexActive ? 0.96 : 0.992);

        if (std::abs(ball.velocity.x()) < 0.1) ball.velocity.setX(ball.velocity.x() > 0 ? 0.1 : -0.1);
        if (std::abs(ball.velocity.y()) < 0.1) ball.velocity.setY(ball.velocity.y() > 0 ? 0.1 : -0.1);

        ball.pos += ball.velocity;

        if (!ball.isBackground) {
            ball.trail.push_front(ball.pos);
            if (ball.trail.size() > 8) ball.trail.pop_back(); 
        }

        if (ball.pos.x() - ball.radius < 0) { ball.pos.setX(ball.radius); ball.velocity.setX(std::abs(ball.velocity.x())); }
        else if (ball.pos.x() + ball.radius > width()) { ball.pos.setX(width() - ball.radius); ball.velocity.setX(-std::abs(ball.velocity.x())); }

        if (ball.pos.y() - ball.radius < 0) { ball.pos.setY(ball.radius); ball.velocity.setY(std::abs(ball.velocity.y())); }
        else if (ball.pos.y() + ball.radius > height()) { ball.pos.setY(height() - ball.radius); ball.velocity.setY(-std::abs(ball.velocity.y())); }
    }
    
    // به‌روزرسانی ذرات و امواج
    for (int i = m_particles.size() - 1; i >= 0; --i) {
        m_particles[i].pos += m_particles[i].velocity;
        m_particles[i].velocity *= 0.95; 
        m_particles[i].life -= 0.025;
        if (m_particles[i].life <= 0) m_particles.removeAt(i);
    }
    for (int i = m_shockwaves.size() - 1; i >= 0; --i) {
        m_shockwaves[i].radius += (m_shockwaves[i].maxRadius - m_shockwaves[i].radius) * 0.15;
        m_shockwaves[i].intensity -= 0.04;
        if (m_shockwaves[i].intensity <= 0) m_shockwaves.removeAt(i);
    }

    update(); 
}

void MainMenuWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_isTransitioning) {
        drawHyperdriveTransition(painter);
        return; 
    }

    drawNebulaBackground(painter);
    drawConstellationWeb(painter);
    drawEffects(painter);
    drawPlasmaOrbsAndTrails(painter);
    drawHolographicTitle(painter);
    drawReactorNodes(painter); 
}

void MainMenuWidget::drawHyperdriveTransition(QPainter& painter) {
    painter.fillRect(rect(), QColor(5, 8, 15));
    QPointF center(width() / 2.0, height() / 2.0);
    
    for (const auto& b : m_bubbles) {
        QPointF dir = b.pos - center;
        qreal dist = std::hypot(dir.x(), dir.y());
        if (dist == 0) continue;
        
        qreal stretch = dist * m_transitionProgress * 0.7;
        QPointF endPos = b.pos + (dir / dist) * stretch;
        
        painter.setPen(QPen(QColor(255, 255, 255, 160), b.size * 1.4, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(b.pos, endPos);
    }
    
    for (const auto& ball : m_balls) {
        QPointF dir = ball.pos - center;
        qreal dist = std::hypot(dir.x(), dir.y());
        if (dist == 0) continue;
        
        qreal stretch = dist * m_transitionProgress * 1.0;
        QPointF endPos = ball.pos + (dir / dist) * stretch;
        
        QColor col = ball.color;
        col.setAlpha(180);
        painter.setPen(QPen(col, ball.radius * 0.4, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(ball.pos, endPos);
    }
    
    int whiteAlpha = qBound(0, int(std::pow(m_transitionProgress, 3.5) * 255), 255);
    painter.fillRect(rect(), QColor(255, 255, 255, whiteAlpha));
}

void MainMenuWidget::drawNebulaBackground(QPainter& painter) {
    painter.fillRect(rect(), QColor(6, 9, 18));
    
    if (m_isVortexActive) {
        QRadialGradient vortex(m_mousePos, 320);
        vortex.setColorAt(0.0, QColor(153, 51, 255, 70));
        vortex.setColorAt(0.5, QColor(0, 242, 254, 30));
        vortex.setColorAt(1.0, Qt::transparent);
        painter.fillRect(rect(), vortex);
    } else {
        QRadialGradient neb1(width() * 0.3, height() * 0.4, height() * 0.7);
        neb1.setColorAt(0.0, QColor(0, 242, 254, 18));
        neb1.setColorAt(1.0, Qt::transparent);
        painter.fillRect(rect(), neb1);

        QRadialGradient neb2(width() * 0.7, height() * 0.6, height() * 0.7);
        neb2.setColorAt(0.0, QColor(255, 51, 102, 14));
        neb2.setColorAt(1.0, Qt::transparent);
        painter.fillRect(rect(), neb2);
    }

    painter.setPen(Qt::NoPen);
    for (const auto& b : m_bubbles) {
        int alpha = qBound(5, int(45 / b.zDepth), 255);
        painter.setBrush(QColor(255, 255, 255, alpha)); 
        painter.drawEllipse(b.pos, b.size, b.size);
    }
}

void MainMenuWidget::drawConstellationWeb(QPainter& painter) {
    for (int i = 0; i < m_balls.size(); ++i) {
        for (int j = i + 1; j < m_balls.size(); ++j) {
            const auto& b1 = m_balls[i];
            const auto& b2 = m_balls[j];
            
            if (b1.isBackground != b2.isBackground) continue;
            
            qreal dist = std::hypot(b2.pos.x() - b1.pos.x(), b2.pos.y() - b1.pos.y());
            qreal maxDist = b1.isBackground ? 90.0 : 180.0;
            
            if (dist < maxDist) {
                qreal alpha = (1.0 - (dist / maxDist));
                QColor webColor = b1.color; 
                webColor.setAlpha(int(alpha * 100));
                painter.setPen(QPen(webColor, 1.2));
                painter.drawLine(b1.pos, b2.pos);
            }
        }
    }
}

void MainMenuWidget::drawPlasmaOrbsAndTrails(QPainter& painter) {
    // رسم دنباله‌های نوری ملایم
    for (const auto& ball : m_balls) {
        if (ball.isBackground || ball.trail.size() < 2) continue; 
        
        qreal speed = ball.velocity.manhattanLength();
        if (speed < 1.0) continue; 
        
        for (int i = 0; i < ball.trail.size() - 1; ++i) {
            qreal t = 1.0 - ((qreal)i / (ball.trail.size() - 1)); 
            qreal w = ball.radius * 0.5 * t; 
            int alpha = int(100 * t); 
            
            QPen trailPen(QColor(ball.color.red(), ball.color.green(), ball.color.blue(), alpha));
            trailPen.setWidthF(w);
            trailPen.setCapStyle(Qt::RoundCap);
            painter.setPen(trailPen);
            painter.drawLine(ball.trail[i], ball.trail[i+1]);
        }
    }

    // رسم گوی‌های کریستالی با جزئیات بالا
    for (const auto& ball : m_balls) {
        if (ball.isBackground) {
            QColor col = ball.color;
            col.setAlpha(110);
            painter.setPen(Qt::NoPen);
            painter.setBrush(col);
            painter.drawEllipse(ball.pos, ball.radius, ball.radius);
        } else {
            // هاله کریستالی (Outer Aura)
            QColor auraCol = ball.color;
            auraCol.setAlpha(55);
            painter.setPen(Qt::NoPen);
            painter.setBrush(auraCol);
            painter.drawEllipse(ball.pos, ball.radius * 1.35, ball.radius * 1.35);
            
            // هسته سه‌بعدی و بازتاب نور
            QRadialGradient core(ball.pos - QPointF(ball.radius * 0.3, ball.radius * 0.3), ball.radius * 1.2);
            core.setColorAt(0.0, Qt::white); 
            core.setColorAt(0.25, ball.color.lighter(130)); 
            core.setColorAt(0.7, ball.color); 
            core.setColorAt(1.0, ball.color.darker(180)); 
            
            painter.setBrush(core);
            painter.setPen(QPen(QColor(255, 255, 255, 90), 1.5));
            painter.drawEllipse(ball.pos, ball.radius, ball.radius);

            // رفلکس شیشه‌ای هلالی (Glass Specular)
            painter.setBrush(QColor(255, 255, 255, 140));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(ball.pos - QPointF(ball.radius * 0.35, ball.radius * 0.35), ball.radius * 0.3, ball.radius * 0.18);
        }
    }
}

void MainMenuWidget::drawEffects(QPainter& painter) {
    painter.setBrush(Qt::NoBrush);
    for (const auto& sw : m_shockwaves) {
        QPen swPen(QColor(sw.color.red(), sw.color.green(), sw.color.blue(), int(sw.intensity * 240)));
        swPen.setWidthF(1.5 + sw.intensity * 3.5);
        painter.setPen(swPen);
        painter.drawEllipse(sw.pos, sw.radius, sw.radius);
    }
    
    painter.setPen(Qt::NoPen);
    for (const auto& p : m_particles) {
        qreal ratio = p.life / p.maxLife;
        QColor c = p.color;
        c.setAlpha(int(ratio * 255));
        painter.setBrush(c);
        painter.save();
        painter.translate(p.pos);
        painter.rotate(std::atan2(p.velocity.y(), p.velocity.x()) * 180.0 / M_PI);
        painter.drawEllipse(QRectF(0, -p.size/2.0, p.size * 2.5, p.size));
        painter.restore();
    }
}

void MainMenuWidget::drawHolographicTitle(QPainter& painter) {
    QString text = "CYBER BOUNCE";
    QFont font("Consolas", qBound(42, width() / 17, 100), QFont::Black);
    font.setLetterSpacing(QFont::AbsoluteSpacing, 12);
    
    QFontMetrics fm(font);
    QRectF textRect = fm.boundingRect(text);
    QPointF centerPos(width() / 2.0 - textRect.width() / 2.0, height() * 0.28);
    
    qreal glitchOffset = 0.0;
    if (std::sin(m_time * 12.0) > 0.88) {
        glitchOffset = (QRandomGenerator::global()->bounded(16) - 8);
    }

    painter.setFont(font);
    
    painter.setPen(QColor(0, 0, 0, 160));
    painter.drawText(centerPos + QPointF(4, 4), text);
    
    painter.setPen(QColor(0, 242, 254, 180));
    painter.drawText(centerPos + QPointF(-3 + glitchOffset, 0), text);
    
    painter.setPen(QColor(255, 51, 102, 180));
    painter.drawText(centerPos + QPointF(3 - glitchOffset, 0), text);
    
    painter.setPen(Qt::white);
    painter.drawText(centerPos, text);
    
    // کادربندی HUD لیزری
    painter.setPen(QPen(QColor(0, 242, 254, 140), 1.5));
    qreal lineY1 = centerPos.y() - textRect.height() + 12;
    qreal lineY2 = centerPos.y() + 18;
    
    painter.drawLine(centerPos.x() - 35, lineY1, centerPos.x() - 15, lineY1);
    painter.drawLine(centerPos.x() + textRect.width() + 15, lineY1, centerPos.x() + textRect.width() + 35, lineY1);
    painter.drawLine(centerPos.x() - 35, lineY2, centerPos.x() - 15, lineY2);
    painter.drawLine(centerPos.x() + textRect.width() + 15, lineY2, centerPos.x() + textRect.width() + 35, lineY2);
    
    painter.setFont(QFont("Consolas", 12, QFont::Bold));
    painter.setPen(QColor(255, 255, 255, 110));
    painter.drawText(QRectF(0, lineY2 + 8, width(), 26), Qt::AlignCenter, "N E X T - G E N   P H Y S I C S   E N G I N E");
}

void MainMenuWidget::drawReactorNodes(QPainter& painter) {
    for (int i = 0; i < m_reactors.size(); ++i) {
        const auto& r = m_reactors[i];
        QPointF p = r.currentPos;
        qreal rEff = r.radius * (1.0 + r.hover * 0.15);

        painter.save();

        // ۱. پرتو لنگر نوری از پایین صفحه (Docking / Anchor Beam)
        QLinearGradient beam(p.x(), height(), p.x(), p.y());
        beam.setColorAt(0.0, QColor(r.primaryColor.red(), r.primaryColor.green(), r.primaryColor.blue(), 5));
        beam.setColorAt(1.0, QColor(r.primaryColor.red(), r.primaryColor.green(), r.primaryColor.blue(), int(40 + r.hover * 60)));
        painter.setPen(QPen(beam, 2.0, Qt::DashLine));
        painter.drawLine(p.x(), height(), p.x(), p.y() + rEff);

        // ۲. هاله میدان گرانش (Outer Gravitational Field Aura)
        QRadialGradient gravField(p, rEff * 2.2);
        gravField.setColorAt(0.0, QColor(r.primaryColor.red(), r.primaryColor.green(), r.primaryColor.blue(), int(45 + r.hover * 85)));
        gravField.setColorAt(0.7, QColor(r.primaryColor.red(), r.primaryColor.green(), r.primaryColor.blue(), int(15 + r.hover * 30)));
        gravField.setColorAt(1.0, Qt::transparent);
        painter.setPen(Qt::NoPen);
        painter.setBrush(gravField);
        painter.drawEllipse(p, rEff * 2.2, rEff * 2.2);

        // ۳. حلقه‌های مکانیکی و HUD تکنولوژیک چرخان دور راکتور
        painter.translate(p);
        
        // حلقه بیرونی تکه‌تکه (Segmented Outer Ring)
        painter.save();
        painter.rotate(r.spinAngle * 0.6);
        painter.setPen(QPen(QColor(r.primaryColor.red(), r.primaryColor.green(), r.primaryColor.blue(), int(120 + r.hover * 135)), 2.0));
        painter.setBrush(Qt::NoBrush);
        for (int a = 0; a < 4; ++a) {
            painter.drawArc(QRectF(-rEff * 1.35, -rEff * 1.35, rEff * 2.7, rEff * 2.7), a * 90 * 16 + 15 * 16, 60 * 16);
        }
        painter.restore();

        // حلقه داخلی معکوس با نشانگرهای زاویه (Inner Gyro Ring)
        painter.save();
        painter.rotate(-r.spinAngle * 1.2);
        painter.setPen(QPen(QColor(255, 255, 255, int(80 + r.hover * 120)), 1.5));
        painter.setBrush(Qt::NoBrush);
        painter.drawArc(QRectF(-rEff * 1.15, -rEff * 1.15, rEff * 2.3, rEff * 2.3), 0, 360 * 16);
        for (int tick = 0; tick < 8; ++tick) {
            qreal rad = tick * M_PI / 4.0;
            painter.drawLine(std::cos(rad) * rEff * 1.1, std::sin(rad) * rEff * 1.1,
                             std::cos(rad) * rEff * 1.22, std::sin(rad) * rEff * 1.22);
        }
        painter.restore();

        // ۴. هسته مرکزی راکتور (Spherical Quantum Core)
        QRadialGradient coreGrad(0, 0, rEff);
        coreGrad.setColorAt(0.0, Qt::white);
        coreGrad.setColorAt(0.35, r.primaryColor.lighter(120));
        coreGrad.setColorAt(0.85, r.primaryColor.darker(160));
        coreGrad.setColorAt(1.0, QColor(5, 8, 15));
        
        painter.setBrush(coreGrad);
        painter.setPen(QPen(QColor(255, 255, 255, int(150 + r.hover * 105)), 2.0));
        painter.drawEllipse(QPointF(0, 0), rEff, rEff);

        // ۵. آیکون و میکروانیمیشن داخلی منحصر‌به‌فرد برای هر دکمه
        painter.setPen(QPen(Qt::white, 2.0));
        if (r.id == 0) { // IGNITION -> فلش‌های پرتاب ۳ لایه تپنده
            qreal pulse = std::sin(m_time * 6.0) * 4.0;
            QPolygonF chevron;
            chevron << QPointF(-14, 10 + pulse) << QPointF(0, -12 + pulse) << QPointF(14, 10 + pulse)
                    << QPointF(0, 2 + pulse);
            painter.setBrush(QColor(255, 255, 255, int(180 + r.hover * 75)));
            painter.drawPolygon(chevron);
        } else if (r.id == 1) { // SETTINGS -> چرخ‌دنده هولوگرافیک چرخان
            painter.save();
            painter.rotate(m_time * 40.0);
            painter.setBrush(Qt::NoBrush);
            painter.setPen(QPen(Qt::white, 2.0));
            painter.drawEllipse(QPointF(0, 0), 14, 14);
            for (int t = 0; t < 6; ++t) {
                qreal rad = t * M_PI / 3.0;
                painter.drawLine(std::cos(rad) * 11, std::sin(rad) * 11,
                                 std::cos(rad) * 19, std::sin(rad) * 19);
            }
            painter.restore();
        } else if (r.id == 2) { // DATABASE -> مکعب هولوگرافیک / تسراکت
            painter.save();
            painter.rotate(m_time * 30.0);
            painter.setBrush(Qt::NoBrush);
            painter.setPen(QPen(Qt::white, 1.8));
            painter.drawRect(QRectF(-11, -11, 22, 22));
            painter.drawRect(QRectF(-6, -6, 12, 12));
            painter.drawLine(-11, -11, -6, -6);
            painter.drawLine(11, -11, 6, -6);
            painter.drawLine(11, 11, 6, 6);
            painter.drawLine(-11, 11, -6, 6);
            painter.restore();
        } else if (r.id == 3) { // EXIT -> علامت هشدار و پرتال خروج
            painter.setBrush(Qt::NoBrush);
            painter.setPen(QPen(Qt::white, 2.2));
            painter.drawLine(-12, -12, 12, 12);
            painter.drawLine(12, -12, -12, 12);
            painter.drawEllipse(QPointF(0, 0), 16, 16);
        }

        painter.resetTransform();

        // ۶. بج متنی هولوگرافیک زیر راکتور (Cyberpunk Label Badge)
        QRectF badgeRect(p.x() - 85, p.y() + rEff + 14, 170, 44);
        
        // پس‌زمینه پنل متن
        QColor badgeBg = QColor(10, 15, 25, int(180 + r.hover * 55));
        painter.setBrush(badgeBg);
        painter.setPen(QPen(QColor(r.primaryColor.red(), r.primaryColor.green(), r.primaryColor.blue(), int(100 + r.hover * 155)), 1.5));
        painter.drawRoundedRect(badgeRect, 6, 6);

        // خطوط تزیینی گوشه‌ها
        painter.setPen(QPen(Qt::white, 2.0));
        painter.drawLine(badgeRect.left(), badgeRect.top() + 6, badgeRect.left(), badgeRect.top());
        painter.drawLine(badgeRect.left(), badgeRect.top(), badgeRect.left() + 6, badgeRect.top());
        painter.drawLine(badgeRect.right(), badgeRect.top() + 6, badgeRect.right(), badgeRect.top());
        painter.drawLine(badgeRect.right(), badgeRect.top(), badgeRect.right() - 6, badgeRect.top());

        // برچسب شماره نود
        painter.setFont(QFont("Consolas", 8, QFont::Bold));
        painter.setPen(QColor(r.primaryColor.red(), r.primaryColor.green(), r.primaryColor.blue(), 220));
        painter.drawText(QRectF(badgeRect.left(), badgeRect.top() + 3, badgeRect.width(), 12), Qt::AlignCenter, r.tag);

        // عنوان اصلی
        painter.setFont(QFont("Consolas", 12, QFont::Black));
        painter.setPen(r.hover > 0.1 ? Qt::white : QColor(220, 230, 240));
        painter.drawText(QRectF(badgeRect.left(), badgeRect.top() + 16, badgeRect.width(), 16), Qt::AlignCenter, r.label);

        // زیرعنوان فرعی
        painter.setFont(QFont("Consolas", 7, QFont::Normal));
        painter.setPen(QColor(150, 170, 190, 180));
        painter.drawText(QRectF(badgeRect.left(), badgeRect.top() + 30, badgeRect.width(), 12), Qt::AlignCenter, r.subLabel);

        painter.restore();
    }
}