#include "AdvancedSettingsWidget.h"
#include "ThemeManager.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>
#include <algorithm>
#include <QRandomGenerator>
#include <QRadialGradient>
#include <QLinearGradient>

AdvancedSettingsWidget::AdvancedSettingsWidget(QWidget* parent) : QWidget(parent) {
    setMouseTracking(true);
    initOrbit();
    
    m_tracks = {"CYBER NEON", "RETRO ARCADE", "SYNTHWAVE", "DEEP SPACE"};
    m_seismoWaveHistory.fill(0.0, 100);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &AdvancedSettingsWidget::updateFrame);
    m_timer->start(16); 
}

void AdvancedSettingsWidget::initOrbit() {
    m_nodes.clear();
    m_nodes.push_back({0.0,   "AUDIO\nLAUNCHER", ThemeManager::instance().getPrimaryColor(), QPointF(), 1.0, 0.0});
    m_nodes.push_back({90.0,  "GRAPHICS\nREACTOR", ThemeManager::instance().getSecondaryColor(), QPointF(), 1.0, 0.0});
    m_nodes.push_back({180.0, "HAPTICS\nSEISMOGRAPH", QColor(51, 255, 153), QPointF(), 1.0, 0.0});
    m_nodes.push_back({270.0, "DISPLAY\nGEARBOX", QColor(255, 204, 0), QPointF(), 1.0, 0.0});
}

void AdvancedSettingsWidget::generateStars(int w, int h) {
    m_bgStars.clear();
    m_constellationStars.clear();
    m_constellationPolys.clear();

    auto rng = QRandomGenerator::global();
    for(int i = 0; i < 280; ++i) {
        AdvBgStar s;
        s.pos = QPointF(rng->bounded(w), rng->bounded(h));
        s.size = 1.0 + rng->generateDouble() * 2.0;
        s.phase = rng->bounded(314) / 100.0;
        
        int r = rng->bounded(100);
        if (r < 20) s.color = ThemeManager::instance().getPrimaryColor();
        else if (r < 40) s.color = ThemeManager::instance().getSecondaryColor();
        else s.color = QColor(255, 255, 255, rng->bounded(80, 200));
        
        m_bgStars.append(s);
    }

    QPainterPath path;
    QFont font("Consolas", qMax(45, w / 16), QFont::Black);
    path.addText(0, 0, font, "CYBER BOUNCE");
    
    QRectF bounds = path.boundingRect();
    QTransform transform;
    transform.translate(w / 2.0 - bounds.width() / 2.0 - bounds.left(),
                        h / 3.8 - bounds.height() / 2.0 - bounds.top());
    QPainterPath translatedPath = transform.map(path);

    for (const QPolygonF& poly : translatedPath.toSubpathPolygons()) {
        QPolygonF sparsePoly;
        if (poly.isEmpty()) continue;
        sparsePoly.append(poly.first());
        for (int i = 1; i < poly.size(); ++i) {
            if (QLineF(sparsePoly.last(), poly[i]).length() > 20.0) sparsePoly.append(poly[i]);
        }
        if (poly.isClosed() && QLineF(sparsePoly.last(), sparsePoly.first()).length() > 20.0) {
            sparsePoly.append(sparsePoly.first());
        }
        m_constellationPolys.append(sparsePoly);

        for (const QPointF& pt : sparsePoly) {
            AdvBgStar s; 
            s.pos = pt; 
            s.size = 2.5 + rng->generateDouble() * 2.5;
            s.phase = rng->bounded(314) / 100.0; 
            s.color = QColor(0, 242, 254);
            m_constellationStars.append(s);
        }
    }
}

void AdvancedSettingsWidget::spawnParticles(const QPointF& pos, const QColor& color, int count, qreal speedMult) {
    auto rng = QRandomGenerator::global();
    for (int i = 0; i < count; ++i) {
        AdvParticle part;
        part.pos = pos;
        qreal angle = rng->bounded(360) * M_PI / 180.0;
        qreal speed = ((rng->bounded(40, 120)) / 10.0) * speedMult;
        part.velocity = QPointF(std::cos(angle) * speed, std::sin(angle) * speed);
        part.maxLife = (rng->bounded(50) + 50) / 100.0;
        part.life = part.maxLife;
        part.color = color;
        m_particles.append(part);
    }
}

void AdvancedSettingsWidget::resizeEvent(QResizeEvent* event) {
    int w = event->size().width();
    int h = event->size().height();
    generateStars(w, h);
    
    m_backButtonRect = QRectF(30, 30, 160, 45);

    m_cannonBase = QPointF(160, h - 100);
    m_trackCrystalPos = QPointF(160, h - 350);
    
    m_reactorCorePos = QPointF(w / 2.0, h - 220);
    m_capsules.clear();
    m_capsules.append({QPointF(w/2 - 250, h - 90), QPointF(w/2 - 250, h - 90), "LOW", QColor(51, 255, 153), 0});
    m_capsules.append({QPointF(w/2, h - 70), QPointF(w/2, h - 70), "MEDIUM", QColor(255, 204, 0), 1});
    m_capsules.append({QPointF(w/2 + 250, h - 90), QPointF(w/2 + 250, h - 90), "ULTRA", QColor(255, 51, 102), 2});

    m_anvilPos = QPointF(w / 2.0, h - 80);
    m_weightPos = QPointF(w / 2.0, h - 80 - m_shakeIntensity * 3.0); 

    m_gearCenter = QPointF(w / 2.0 - 100, h - 110);

    QWidget::resizeEvent(event);
}

void AdvancedSettingsWidget::updateFrame() {
    m_time += 0.04;
    m_currentScreenShake *= 0.88; 

    // آپدیت موج لرزه‌نگار
    qreal currentVal = (m_currentScreenShake > 0.5) 
                       ? ((QRandomGenerator::global()->bounded(200) - 100) / 100.0) * m_currentScreenShake
                       : (std::sin(m_time * 8.0) * 1.5);
    m_seismoWaveHistory.append(currentVal);
    if (m_seismoWaveHistory.size() > 120) m_seismoWaveHistory.removeFirst();

    if (!m_inSubMenu) {
        for (auto& node : m_nodes) {
            node.angle += m_orbitSpeed;
            if (node.angle >= 360.0) node.angle -= 360.0;
            if (node.angle < 0.0) node.angle += 360.0;
        }
        m_planetOffsetY = m_planetOffsetY * 0.9; 
        m_currentScale = m_currentScale * 0.9 + 1.0 * 0.1; 
    } else {
        qreal targetY = -(height() / 2.5) + 30; 
        m_planetOffsetY = m_planetOffsetY * 0.9 + targetY * 0.1;
        m_currentScale = m_currentScale * 0.9 + 0.4 * 0.1; 

        if (m_activeNodeIndex == 0) updateAudioLauncher();
        else if (m_activeNodeIndex == 1) updateGraphicsReactor();
        else if (m_activeNodeIndex == 2) updateHapticsSeismograph();
        else if (m_activeNodeIndex == 3) updateDisplayGearbox();
    }
    
    // آپدیت ذرات
    for (int i = m_particles.size() - 1; i >= 0; --i) {
        m_particles[i].pos += m_particles[i].velocity;
        m_particles[i].velocity.setY(m_particles[i].velocity.y() + 0.2); 
        m_particles[i].life -= 0.025; 
        if (m_particles[i].life <= 0) {
            m_particles.removeAt(i);
        }
    }

    calculate3D();
    update();
}

void AdvancedSettingsWidget::calculate3D() {
    int cx = width() / 2;
    int cy = (height() / 2) + m_planetOffsetY;
    qreal tiltRad = m_orbitTilt * M_PI / 180.0;

    for (auto& node : m_nodes) {
        qreal rad = node.angle * M_PI / 180.0;
        qreal x = m_orbitRadiusX * std::cos(rad);
        qreal y = m_orbitRadiusX * std::sin(rad);

        qreal projectedY = y * std::sin(tiltRad);
        node.zDepth = y * std::cos(tiltRad); 

        qreal focalLength = 1000.0;
        node.scale = focalLength / (focalLength - node.zDepth);
        node.projectedPos = QPointF(cx + x * node.scale * m_currentScale, cy + projectedY * node.scale * m_currentScale);
    }
}

// ================== آپدیت مینی‌گیم‌ها ==================

void AdvancedSettingsWidget::updateAudioLauncher() {
    if (m_audioBallFlying) {
        m_audioBallPos += m_audioBallVel;
        m_audioBallVel.setY(m_audioBallVel.y() + 0.6); 

        if (std::hypot(m_audioBallPos.x() - m_trackCrystalPos.x(), m_audioBallPos.y() - m_trackCrystalPos.y()) < 50.0) {
            m_currentTrackIndex = (m_currentTrackIndex + 1) % m_tracks.size(); 
            spawnParticles(m_audioBallPos, QColor(255, 51, 200), 40, 2.0);
            m_audioBallFlying = false; 
            
            SoundManager::instance().playMusic(m_tracks[m_currentTrackIndex]);
            SoundManager::instance().playPop();
            return; 
        }

        qreal groundY = height() - 100;
        if (m_audioBallPos.y() >= groundY) {
            m_audioBallPos.setY(groundY);
            m_audioBallFlying = false;

            qreal startX = 260;
            qreal endX = width() - 100;
            qreal hitX = m_audioBallPos.x() - startX;
            m_volume = qBound(0, (int)std::round((hitX / (endX - startX)) * 100.0), 100);

            SoundManager::instance().setMusicVolume(m_volume);
            SoundManager::instance().setSfxVolume(m_volume);

            spawnParticles(m_audioBallPos, QColor(0, 242, 254), 30, 1.5);
            SoundManager::instance().playBounce();
            m_currentScreenShake = 12.0; 
        }
    }
}

void AdvancedSettingsWidget::updateGraphicsReactor() {
    for (int i = 0; i < m_capsules.size(); ++i) {
        if (!m_isDraggingCapsule || m_draggedCapsuleIndex != i) {
            m_capsules[i].currentPos.setX(m_capsules[i].currentPos.x() * 0.9 + m_capsules[i].homePos.x() * 0.1);
            m_capsules[i].currentPos.setY(m_capsules[i].currentPos.y() * 0.9 + m_capsules[i].homePos.y() * 0.1);
        }
    }
    
    // اگر کیفیت Ultra است، ذرات گدازه رآکتور ساطع شوند
    if (m_graphicsQuality == 2 && QRandomGenerator::global()->bounded(10) < 3) {
        spawnParticles(m_reactorCorePos, QColor(255, 51, 102), 2, 1.5);
    }
}

void AdvancedSettingsWidget::updateHapticsSeismograph() {
    if (m_weightFalling) {
        m_weightPos.setY(m_weightPos.y() + m_weightVelY);
        m_weightVelY += 1.8; 
        
        if (m_weightPos.y() >= m_anvilPos.y() - 20) { 
            m_weightPos.setY(m_anvilPos.y() - 20);
            m_weightFalling = false;
            
            qreal dropHeight = (height() - 100) - m_dragPos.y();
            m_shakeIntensity = qBound(0, (int)(dropHeight / 3.0), 100);
            
            m_currentScreenShake = m_shakeIntensity * 0.6; 
            spawnParticles(m_weightPos, QColor(51, 255, 153), 40, 2.5);
            spawnParticles(m_weightPos, QColor(255, 200, 50), 20, 2.0); // جرقه‌های طلایی
            SoundManager::instance().playBounce();
        }
    } else if (!m_isDraggingWeight) {
        qreal targetY = m_anvilPos.y() - 20 - m_shakeIntensity * 3.0;
        m_weightPos.setY(m_weightPos.y() * 0.8 + targetY * 0.2);
    }
}

void AdvancedSettingsWidget::updateDisplayGearbox() {
    if (!m_isDraggingLever) {
        qreal targetAngle = -150.0 + m_currentFpsIndex * 40.0; 
        m_leverAngle = m_leverAngle * 0.8 + targetAngle * 0.2;
    }
    
    // آپدیت تاکومتر دور موتور بر اساس FPS
    qreal targetGauge = (m_currentFpsIndex / 3.0) * 220.0 - 110.0; // -110 to +110 deg
    if (m_currentFpsIndex == 3) targetGauge += std::sin(m_time * 20.0) * 5.0; // لرزش دور موتور ردلاین
    m_tachometerGaugeAngle += (targetGauge - m_tachometerGaugeAngle) * 0.15;
}

// ================== ماوس و رویدادها ==================

void AdvancedSettingsWidget::mouseMoveEvent(QMouseEvent* event) {
    m_mousePos = event->position();
    m_backHovered = m_backButtonRect.contains(m_mousePos);

    if (!m_inSubMenu) {
        qreal dx = m_mousePos.x() - width() / 2.0;
        m_orbitSpeed = (dx * 0.003);
        if (std::abs(m_orbitSpeed) < 0.1) m_orbitSpeed = 0.1;
    } 
    else if (m_activeNodeIndex == 0 && m_isDraggingCannon) {
        QLineF pullLine(m_cannonBase, m_mousePos);
        if (pullLine.length() > 220) {
            pullLine.setLength(220);
            m_dragPos = pullLine.p2();
        } else m_dragPos = m_mousePos;
    } 
    else if (m_activeNodeIndex == 1 && m_isDraggingCapsule) {
        m_capsules[m_draggedCapsuleIndex].currentPos = m_mousePos;
    }
    else if (m_activeNodeIndex == 2 && m_isDraggingWeight) {
        m_weightPos.setX(m_anvilPos.x()); 
        m_weightPos.setY(qMin(m_mousePos.y(), m_anvilPos.y() - 20)); 
    }
    else if (m_activeNodeIndex == 3 && m_isDraggingLever) {
        qreal angle = std::atan2(m_mousePos.y() - m_gearCenter.y(), m_mousePos.x() - m_gearCenter.x()) * 180.0 / M_PI;
        if (angle > 0) angle -= 360; 
        
        m_leverAngle = qBound(-150.0, angle, -30.0);
        m_currentFpsIndex = qBound(0, (int)std::round((m_leverAngle + 150.0) / 40.0), 3);
    }
}

void AdvancedSettingsWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton && m_inSubMenu) {
        m_inSubMenu = false;
        m_activeNodeIndex = -1;
        m_isDraggingCannon = m_isDraggingCapsule = m_isDraggingWeight = m_isDraggingLever = false;
        return;
    }

    if (event->button() == Qt::LeftButton) {
        if (m_backButtonRect.contains(m_mousePos)) {
            if (m_inSubMenu) {
                m_inSubMenu = false;
                m_activeNodeIndex = -1;
                m_isDraggingCannon = m_isDraggingCapsule = m_isDraggingWeight = m_isDraggingLever = false;
                SoundManager::instance().playPop();
            } else {
                SoundManager::instance().playPop();
                emit backClicked();
            }
            return;
        }

        if (!m_inSubMenu) {
            for (int i = 0; i < m_nodes.size(); ++i) {
                auto& node = m_nodes[i];
                if (std::hypot(m_mousePos.x() - node.projectedPos.x(), m_mousePos.y() - node.projectedPos.y()) < 45 * node.scale * m_currentScale && node.zDepth >= 0) {
                    m_inSubMenu = true;
                    m_activeNodeIndex = i;
                    m_audioBallFlying = false;
                    m_audioBallPos = m_cannonBase;
                    SoundManager::instance().playShoot();
                    break;
                }
            }
        } 
        else {
            m_hasInteracted[m_activeNodeIndex] = true; 

            if (m_activeNodeIndex == 0 && std::hypot(m_mousePos.x() - m_cannonBase.x(), m_mousePos.y() - m_cannonBase.y()) < 60) {
                m_isDraggingCannon = true;
                m_dragPos = m_mousePos;
            } 
            else if (m_activeNodeIndex == 1) {
                for (int i = 0; i < m_capsules.size(); ++i) {
                    if (std::hypot(m_mousePos.x() - m_capsules[i].currentPos.x(), m_mousePos.y() - m_capsules[i].currentPos.y()) < 45) {
                        m_isDraggingCapsule = true;
                        m_draggedCapsuleIndex = i;
                        break;
                    }
                }
            }
            else if (m_activeNodeIndex == 2 && std::hypot(m_mousePos.x() - m_weightPos.x(), m_mousePos.y() - m_weightPos.y()) < 65) {
                m_isDraggingWeight = true;
                m_weightFalling = false;
            }
            else if (m_activeNodeIndex == 3) {
                qreal rad = m_leverAngle * M_PI / 180.0;
                QPointF leverHandle = m_gearCenter + QPointF(std::cos(rad)*150, std::sin(rad)*150);
                if (std::hypot(m_mousePos.x() - leverHandle.x(), m_mousePos.y() - leverHandle.y()) < 65) {
                    m_isDraggingLever = true;
                }
            }
        }
    }
}

void AdvancedSettingsWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (m_isDraggingCannon) {
            m_isDraggingCannon = false;
            m_audioBallFlying = true;
            m_audioBallPos = m_cannonBase;
            m_audioBallVel = QPointF((m_cannonBase.x() - m_dragPos.x()) * 0.18, (m_cannonBase.y() - m_dragPos.y()) * 0.18); 
            spawnParticles(m_cannonBase, Qt::white, 10);
            SoundManager::instance().playShoot();
        }
        else if (m_isDraggingCapsule) {
            m_isDraggingCapsule = false;
            if (std::hypot(m_capsules[m_draggedCapsuleIndex].currentPos.x() - m_reactorCorePos.x(), m_capsules[m_draggedCapsuleIndex].currentPos.y() - m_reactorCorePos.y()) < 120) {
                m_graphicsQuality = m_capsules[m_draggedCapsuleIndex].qualityLevel;
                spawnParticles(m_reactorCorePos, m_capsules[m_draggedCapsuleIndex].color, 60, 2.5);
                SoundManager::instance().playPop();
                m_currentScreenShake = 18.0; 
            }
        }
        else if (m_isDraggingWeight) {
            m_isDraggingWeight = false;
            m_weightFalling = true;
            m_weightVelY = 0;
            m_dragPos = m_weightPos; 
        }
        else if (m_isDraggingLever) {
            m_isDraggingLever = false;
            qreal targetAngle = -150.0 + m_currentFpsIndex * 40.0;
            qreal rad = targetAngle * M_PI / 180.0;
            QPointF snapPos = m_gearCenter + QPointF(std::cos(rad)*150, std::sin(rad)*150);
            spawnParticles(snapPos, QColor(255, 204, 0), 20, 1.5);
            SoundManager::instance().playPop();
        }
    }
}

void AdvancedSettingsWidget::wheelEvent(QWheelEvent* event) {
    if (!m_inSubMenu) {
        qreal jump = (event->angleDelta().y() > 0) ? 10.0 : -10.0;
        for (auto& node : m_nodes) node.angle += jump;
    }
}

// ================== رندرینگ ==================

void AdvancedSettingsWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.save();

    if (m_currentScreenShake > 0.5) {
        qreal ox = ((QRandomGenerator::global()->bounded(200) - 100) / 100.0) * m_currentScreenShake;
        qreal oy = ((QRandomGenerator::global()->bounded(200) - 100) / 100.0) * m_currentScreenShake;
        painter.translate(ox, oy);
    }

    drawNebulaAndAtmosphere(painter);

    int cx = width() / 2;
    int cy = (height() / 2) + m_planetOffsetY;
    qreal planetRadius = 160.0 * m_currentScale;
    qreal orbitW = m_orbitRadiusX * m_currentScale;
    qreal orbitH = orbitW * std::sin(m_orbitTilt * M_PI / 180.0);
    QRectF orbitRect(cx - orbitW, cy - orbitH, orbitW * 2, orbitH * 2);

    // نیمه پشتی مدار
    painter.setPen(QPen(QColor(0, 242, 254, 40), 2));
    painter.drawArc(orbitRect, 0 * 16, 180 * 16);

    for (const auto& node : m_nodes) {
        if (node.zDepth < 0) {
            qreal r = 28.0 * node.scale * m_currentScale;
            QRadialGradient nodeGrad(node.projectedPos, r, QPointF(node.projectedPos.x() - r*0.3, node.projectedPos.y() - r*0.3));
            nodeGrad.setColorAt(0.0, QColor(255, 255, 255, 100)); 
            nodeGrad.setColorAt(0.3, QColor(node.color.red(), node.color.green(), node.color.blue(), 120));
            nodeGrad.setColorAt(0.9, QColor(0, 0, 0, 200));
            painter.setPen(Qt::NoPen); 
            painter.setBrush(nodeGrad);
            painter.drawEllipse(node.projectedPos, r, r);
        }
    }

    // هسته کوانتومی مرکزی
    drawCentralQuantumCore(painter, cx, cy, planetRadius);

    // نیمه جلویی مدار
    painter.setPen(QPen(QColor(0, 242, 254, 180), 3));
    painter.drawArc(orbitRect, 180 * 16, 180 * 16);

    // نودهای جلویی
    for (const auto& node : m_nodes) {
        if (node.zDepth >= 0) {
            qreal r = 38.0 * node.scale * m_currentScale;
            QRadialGradient glow(node.projectedPos, r*1.8);
            glow.setColorAt(0.0, QColor(node.color.red(), node.color.green(), node.color.blue(), 150));
            glow.setColorAt(1.0, Qt::transparent);
            painter.setBrush(glow); 
            painter.drawEllipse(node.projectedPos, r*1.8, r*1.8);

            QRadialGradient nodeGrad(node.projectedPos, r, QPointF(node.projectedPos.x() - r*0.35, node.projectedPos.y() - r*0.35));
            nodeGrad.setColorAt(0.0, Qt::white); 
            nodeGrad.setColorAt(0.25, node.color);
            nodeGrad.setColorAt(0.8, node.color.darker(300));
            nodeGrad.setColorAt(1.0, Qt::black);
            
            painter.setPen(QPen(Qt::white, 1.5)); 
            painter.setBrush(nodeGrad);
            painter.drawEllipse(node.projectedPos, r, r);

            if (m_currentScale > 0.8) {
                painter.setPen(Qt::white);
                painter.setFont(QFont("Consolas", 12, QFont::Bold));
                painter.drawText(QRectF(node.projectedPos.x() - 120, node.projectedPos.y() - r - 45, 240, 40), 
                                 Qt::AlignCenter | Qt::AlignBottom, node.label);
            }
        }
    }

    // رندر مینی‌گیم فعال
    if (m_inSubMenu && m_currentScale < 0.5) {
        if (m_activeNodeIndex == 0) renderAudioLauncher(painter);
        else if (m_activeNodeIndex == 1) renderGraphicsReactor(painter);
        else if (m_activeNodeIndex == 2) renderHapticsSeismograph(painter);
        else if (m_activeNodeIndex == 3) renderDisplayGearbox(painter);
    }

    // رندر ذرات
    for (const auto& part : m_particles) {
        qreal ratio = part.life / part.maxLife;
        painter.setPen(Qt::NoPen);
        QColor c = part.color;
        c.setAlpha(int(ratio * 255));
        painter.setBrush(c);
        painter.drawEllipse(part.pos, 4.5 * ratio, 4.5 * ratio);
    }

    drawHUDTelemetry(painter);

    // دکمه بازگشت
    QColor priCol = ThemeManager::instance().getPrimaryColor();
    QColor secCol = ThemeManager::instance().getSecondaryColor();
    QColor btnColor = m_backHovered ? secCol : priCol;
    painter.setPen(QPen(btnColor, 2));
    painter.setBrush(m_backHovered ? QColor(secCol.red(), secCol.green(), secCol.blue(), 60) : QColor(priCol.red(), priCol.green(), priCol.blue(), 25));
    painter.drawRoundedRect(m_backButtonRect, 8, 8);

    painter.setPen(Qt::white);
    painter.setFont(QFont("Consolas", 12, QFont::Bold));
    QString btnText = m_inSubMenu ? "◄ RETURN" : "◄ MAIN MENU";
    painter.drawText(m_backButtonRect, Qt::AlignCenter, btnText);

    painter.restore(); 
}

void AdvancedSettingsWidget::drawNebulaAndAtmosphere(QPainter& painter) {
    int baseHue = ThemeManager::instance().getBaseHue();
    QLinearGradient bgGrad(0, 0, width(), height());
    bgGrad.setColorAt(0.0, QColor::fromHsv(baseHue, 220, 15));
    bgGrad.setColorAt(1.0, QColor::fromHsv(baseHue, 240, 6));
    painter.fillRect(rect(), bgGrad);

    // سحابی‌های رنگی
    QRadialGradient neb1(width()*0.2, height()*0.3, 800);
    QColor n1 = ThemeManager::instance().getPrimaryColor();
    n1.setAlpha(35);
    neb1.setColorAt(0, n1);
    neb1.setColorAt(1, Qt::transparent);
    painter.fillRect(rect(), neb1);
    
    QRadialGradient neb2(width()*0.8, height()*0.7, 900);
    QColor n2 = ThemeManager::instance().getSecondaryColor();
    n2.setAlpha(30);
    neb2.setColorAt(0, n2);
    neb2.setColorAt(1, Qt::transparent);
    painter.fillRect(rect(), neb2);

    // ستاره‌ها
    for (const auto& s : m_bgStars) {
        int alpha = 60 + 195 * std::abs(std::sin(m_time + s.phase));
        QColor sc = s.color;
        sc.setAlpha(alpha);
        painter.setPen(Qt::NoPen);
        painter.setBrush(sc);
        painter.drawEllipse(s.pos, s.size, s.size);
    }

    // خطوط صورت فلکی
    int constAlpha = m_inSubMenu ? 25 : 120; 
    QColor constCol = ThemeManager::instance().getPrimaryColor();
    painter.setPen(QPen(QColor(constCol.red(), constCol.green(), constCol.blue(), constAlpha / 2), 1.5, Qt::DashLine)); 
    for (const QPolygonF& poly : m_constellationPolys) painter.drawPolyline(poly); 
    
    for (const auto& s : m_constellationStars) {
        int alpha = (constAlpha / 2) + (constAlpha) * std::abs(std::sin(m_time * 2.5 + s.phase));
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(constCol.red(), constCol.green(), constCol.blue(), alpha));
        painter.drawEllipse(s.pos, s.size, s.size);
    }
}

void AdvancedSettingsWidget::drawCentralQuantumCore(QPainter& painter, int cx, int cy, qreal radius) {
    QColor priCol = ThemeManager::instance().getPrimaryColor();
    QColor secCol = ThemeManager::instance().getSecondaryColor();

    // ۱. هاله اتمسفر دور سیاره مرکزی
    qreal haloRadius = radius * 1.4;
    QRadialGradient atmosphere(cx, cy, haloRadius);
    atmosphere.setColorAt(0.70, Qt::transparent);
    atmosphere.setColorAt(0.85, QColor(priCol.red(), priCol.green(), priCol.blue(), int(120 * m_currentScale)));
    atmosphere.setColorAt(1.0, Qt::transparent);
    painter.setBrush(atmosphere);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(cx, cy), haloRadius, haloRadius);

    // ۲. کره جامد هسته کوانتومی
    QRadialGradient planetGrad(QPointF(cx, cy), radius, QPointF(cx - radius*0.35, cy - radius*0.35));
    planetGrad.setColorAt(0.0, Qt::white);      
    planetGrad.setColorAt(0.15, priCol);      
    planetGrad.setColorAt(0.55, QColor(priCol.red()/6, priCol.green()/6, priCol.blue()/6));         
    planetGrad.setColorAt(1.0, QColor(2, 4, 8));           
    painter.setBrush(planetGrad);
    painter.drawEllipse(QPointF(cx, cy), radius, radius);

    // ۳. حلقه‌های ژیروسکوپ چرخان دور هسته
    painter.save();
    painter.translate(cx, cy);
    painter.setBrush(Qt::NoBrush);
    
    // حلقه ژیروسکوپ اول
    painter.save();
    painter.rotate(m_time * 35.0);
    painter.scale(1.0, 0.35);
    painter.setPen(QPen(QColor(priCol.red(), priCol.green(), priCol.blue(), int(180 * m_currentScale)), 2.5));
    painter.drawEllipse(QPointF(0,0), radius*1.15, radius*1.15);
    painter.restore();
    
    // حلقه ژیروسکوپ دوم معکوس
    painter.save();
    painter.rotate(-m_time * 45.0);
    painter.scale(0.35, 1.0);
    painter.setPen(QPen(QColor(secCol.red(), secCol.green(), secCol.blue(), int(180 * m_currentScale)), 2.5));
    painter.drawEllipse(QPointF(0,0), radius*1.2, radius*1.2);
    painter.restore();

    // حلقه ژیروسکوپ سوم مایل
    painter.save();
    painter.rotate(m_time * 25.0 + 45.0);
    painter.scale(0.7, 0.7);
    painter.setPen(QPen(QColor(255, 255, 255, int(160 * m_currentScale)), 2.0, Qt::DashLine));
    painter.drawEllipse(QPointF(0,0), radius*1.3, radius*1.3);
    painter.restore();

    painter.restore();
}

void AdvancedSettingsWidget::drawHUDTelemetry(QPainter& painter) {
    QColor priCol = ThemeManager::instance().getPrimaryColor();
    painter.setFont(QFont("Consolas", 10, QFont::Bold));
    painter.setPen(priCol);

    // گوشه بالا راست
    int rx = width() - 250;
    painter.drawText(rx, 40, "[ QUANTUM TELEMETRY ]");
    painter.setPen(Qt::white);
    painter.drawText(rx, 65, QString("GPU ENGINE: %1").arg(m_graphicsQuality == 2 ? "ULTRA (OC)" : m_graphicsQuality == 1 ? "OPTIMAL" : "SAVER"));
    painter.drawText(rx, 85, QString("TARGET FPS: %1").arg(m_fpsOptions[m_currentFpsIndex] == 999 ? "UNLOCKED" : QString::number(m_fpsOptions[m_currentFpsIndex])));
    painter.drawText(rx, 105, QString("SEISMO LOAD: %1%").arg(m_shakeIntensity));

    // خطوط اسکن‌لاین محو
    painter.setPen(QPen(QColor(0, 0, 0, 35), 1.0));
    for (int y = 0; y < height(); y += 3) {
        painter.drawLine(0, y, width(), y);
    }
}

void AdvancedSettingsWidget::drawTeslaLightning(QPainter& painter, const QPointF& start, const QPointF& end, const QColor& color) {
    auto rng = QRandomGenerator::global();
    QVector<QPointF> points;
    points.append(start);

    int segments = 8;
    for (int i = 1; i < segments; ++i) {
        qreal t = (qreal)i / segments;
        QPointF mid = start * (1.0 - t) + end * t;
        mid += QPointF(rng->bounded(40) - 20, rng->bounded(40) - 20);
        points.append(mid);
    }
    points.append(end);

    // هاله صاعقه
    painter.setPen(QPen(QColor(color.red(), color.green(), color.blue(), 120), 5.0, Qt::SolidLine, Qt::RoundCap));
    for (int i = 0; i < points.size() - 1; ++i) painter.drawLine(points[i], points[i+1]);

    // هسته سفید صاعقه
    painter.setPen(QPen(Qt::white, 2.0, Qt::SolidLine, Qt::RoundCap));
    for (int i = 0; i < points.size() - 1; ++i) painter.drawLine(points[i], points[i+1]);
}

// ================== 0. AUDIO LAUNCHER ==================
void AdvancedSettingsWidget::renderAudioLauncher(QPainter& painter) {
    qreal groundY = height() - 100;
    qreal startX = 260;
    qreal endX = width() - 100;
    QColor priCol = ThemeManager::instance().getPrimaryColor();
    QColor secCol = ThemeManager::instance().getSecondaryColor();

    painter.setPen(priCol);
    painter.setFont(QFont("Consolas", 26, QFont::Bold));
    painter.drawText(QRectF(0, height()/2 - 60, width(), 50), Qt::AlignCenter, QString("VOLUME: %1%").arg(m_volume));

    // کریستال موسیقی ۳ بعدی
    qreal hoverY = std::sin(m_time * 3.0) * 12.0;
    QPointF orbPos = m_trackCrystalPos + QPointF(0, hoverY);
    
    // امواج صوتی اطراف کریستال
    qreal soundPulse = std::fmod(m_time * 40.0, 70.0);
    painter.setPen(QPen(QColor(secCol.red(), secCol.green(), secCol.blue(), int(255 * (1.0 - soundPulse/70.0))), 2.0));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(orbPos, soundPulse, soundPulse);

    QRadialGradient trackGlow(orbPos, 70.0);
    trackGlow.setColorAt(0.0, QColor(secCol.red(), secCol.green(), secCol.blue(), 160));
    trackGlow.setColorAt(1.0, Qt::transparent);
    painter.setPen(Qt::NoPen);
    painter.setBrush(trackGlow);
    painter.drawEllipse(orbPos, 70, 70);

    QPolygonF crystalTop, crystalBottom;
    qreal crW = 28.0, crH = 45.0;
    qreal rot = m_time * 2.5;
    QPointF topP = orbPos + QPointF(0, -crH);
    QPointF botP = orbPos + QPointF(0, crH);
    QPointF mid1 = orbPos + QPointF(std::cos(rot)*crW, std::sin(rot)*8.0);
    QPointF mid2 = orbPos + QPointF(std::cos(rot + M_PI)*crW, std::sin(rot + M_PI)*8.0);

    painter.setPen(QPen(Qt::white, 2));
    painter.setBrush(QColor(secCol.red(), secCol.green(), secCol.blue(), 220));
    crystalTop << topP << mid1 << mid2;
    painter.drawPolygon(crystalTop);
    crystalBottom << botP << mid1 << mid2;
    painter.drawPolygon(crystalBottom);

    painter.setPen(Qt::white);
    painter.setFont(QFont("Consolas", 13, QFont::Bold));
    painter.drawText(QRectF(orbPos.x() - 150, orbPos.y() - 95, 300, 30), Qt::AlignCenter, m_tracks[m_currentTrackIndex]);

    // مسیر پیش‌بینی لیزری (Trajectory Arc)
    if (m_isDraggingCannon) {
        QPointF vel = QPointF((m_cannonBase.x() - m_dragPos.x()) * 0.18, (m_cannonBase.y() - m_dragPos.y()) * 0.18);
        QPointF simPos = m_cannonBase;
        QPointF simVel = vel;
        
        painter.setPen(QPen(QColor(priCol.red(), priCol.green(), priCol.blue(), 180), 2.0, Qt::DotLine));
        for (int step = 0; step < 40; ++step) {
            QPointF nextPos = simPos + simVel;
            simVel.setY(simVel.y() + 0.6);
            painter.drawLine(simPos, nextPos);
            simPos = nextPos;
            if (simPos.y() >= groundY) {
                painter.setBrush(QColor(priCol.red(), priCol.green(), priCol.blue(), 150));
                painter.drawEllipse(simPos, 6, 6);
                break;
            }
        }
    }

    // ریل اسلایدر ولوم
    painter.setPen(QPen(QColor(255, 255, 255, 80), 2, Qt::DashLine));
    painter.drawLine(QPointF(startX, groundY), QPointF(endX, groundY));
    for (int i = 0; i <= 100; i += 25) {
        qreal tx = startX + (i / 100.0) * (endX - startX);
        painter.setPen(QPen(QColor(255, 255, 255, 150), 2));
        painter.drawLine(QPointF(tx, groundY - 10), QPointF(tx, groundY + 10));
        painter.setPen((i <= m_volume) ? priCol : QColor(255, 255, 255, 100));
        painter.setFont(QFont("Consolas", 11, QFont::Bold));
        painter.drawText(QRectF(tx - 25, groundY + 15, 50, 20), Qt::AlignCenter, QString::number(i));
    }
    qreal curX = startX + (m_volume / 100.0) * (endX - startX);
    painter.setPen(QPen(QColor(priCol.red(), priCol.green(), priCol.blue(), 200), 5));
    painter.drawLine(QPointF(startX, groundY), QPointF(curX, groundY));

    // پایه و توپ پرتاب
    painter.setPen(QPen(QColor(60, 75, 100), 8, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(m_cannonBase, QPointF(m_cannonBase.x(), groundY)); 
    QPointF drawBall = m_cannonBase;
    if (m_isDraggingCannon) {
        drawBall = m_dragPos;
        painter.setPen(QPen(QColor(secCol.red(), secCol.green(), secCol.blue(), 220), 4));
        painter.drawLine(m_cannonBase, m_dragPos);
    } else if (m_audioBallFlying) drawBall = m_audioBallPos;

    QRadialGradient bg(drawBall, 20.0, drawBall - QPointF(5,5));
    bg.setColorAt(0, Qt::white); 
    bg.setColorAt(0.3, priCol); 
    bg.setColorAt(1, QColor(priCol.red()/4, priCol.green()/4, priCol.blue()/4));
    painter.setPen(Qt::NoPen); 
    painter.setBrush(bg);
    painter.drawEllipse(drawBall, 20.0, 20.0);
}

// ================== 1. GRAPHICS REACTOR ==================
void AdvancedSettingsWidget::renderGraphicsReactor(QPainter& painter) {
    QString qLabel = (m_graphicsQuality == 0) ? "LOW [EFFICIENCY]" : (m_graphicsQuality == 1) ? "MEDIUM [BALANCED]" : "ULTRA [OVERCLOCKED]";
    painter.setPen(QColor(255, 51, 102));
    painter.setFont(QFont("Consolas", 26, QFont::Bold));
    painter.drawText(QRectF(0, height()/2 - 60, width(), 50), Qt::AlignCenter, QString("GRAPHICS: %1").arg(qLabel));

    qreal coreRadius = 85.0;
    QColor coreColor = (m_graphicsQuality == 0) ? QColor(51, 255, 153) : (m_graphicsQuality == 1) ? QColor(255, 204, 0) : QColor(255, 51, 102);
    qreal pulse = 1.0 + 0.08 * std::sin(m_time * (m_graphicsQuality == 2 ? 10.0 : 4.0));
    
    // صاعقه‌های تسلا اگر کپسول در حال درگ است
    if (m_isDraggingCapsule) {
        drawTeslaLightning(painter, m_reactorCorePos, m_capsules[m_draggedCapsuleIndex].currentPos, coreColor);
    }

    painter.save();
    painter.translate(m_reactorCorePos);
    
    // توربین بیرونی
    painter.save();
    painter.rotate(m_time * (m_graphicsQuality == 2 ? 80.0 : 30.0));
    painter.setPen(QPen(QColor(255, 255, 255, 120), 2, Qt::DashLine));
    painter.drawEllipse(QPointF(0, 0), coreRadius*1.3, coreRadius*1.3);
    painter.restore();
    
    // پره‌های داخلی
    painter.save();
    painter.rotate(-m_time * (m_graphicsQuality == 2 ? 120.0 : 45.0));
    painter.setPen(QPen(coreColor, 5, Qt::DotLine));
    painter.drawEllipse(QPointF(0, 0), coreRadius*1.1, coreRadius*1.1);
    painter.restore();

    QRadialGradient coreGrad(0, 0, coreRadius * pulse);
    coreGrad.setColorAt(0.0, Qt::white);
    coreGrad.setColorAt(0.25, coreColor);
    coreGrad.setColorAt(1.0, Qt::transparent);
    painter.setPen(Qt::NoPen);
    painter.setBrush(coreGrad);
    painter.drawEllipse(QPointF(0, 0), coreRadius * pulse, coreRadius * pulse);
    
    painter.restore(); 

    // کپسول‌های سوخت گرافیکی
    for (const auto& cap : m_capsules) {
        QRectF cRect(cap.currentPos.x() - 25, cap.currentPos.y() - 45, 50, 90);

        painter.setPen(QPen(QColor(255,255,255,120), 1.5));
        painter.setBrush(QColor(10, 15, 30, 200));
        painter.drawRoundedRect(cRect, 12, 12);

        QRectF energyRect(cRect.x() + 5, cRect.y() + 22, cRect.width() - 10, cRect.height() - 28);
        QLinearGradient eGrad(energyRect.topLeft(), energyRect.bottomLeft());
        eGrad.setColorAt(0, cap.color);
        eGrad.setColorAt(1, cap.color.darker(350));
        painter.setPen(Qt::NoPen);
        painter.setBrush(eGrad);
        painter.drawRoundedRect(energyRect, 6, 6);

        painter.setBrush(QColor(220, 220, 220));
        painter.drawRoundedRect(cRect.x() + 12, cRect.y() - 6, 26, 12, 4, 4);

        painter.setPen(Qt::white);
        painter.setFont(QFont("Consolas", 10, QFont::Bold));
        painter.drawText(QRectF(cRect.x() - 15, cRect.y() - 30, cRect.width() + 30, 20), Qt::AlignCenter, cap.label);
    }
}

// ================== 2. HAPTICS SEISMOGRAPH ==================
void AdvancedSettingsWidget::renderHapticsSeismograph(QPainter& painter) {
    painter.setPen(QColor(51, 255, 153));
    painter.setFont(QFont("Consolas", 26, QFont::Bold));
    painter.drawText(QRectF(0, height()/2 - 60, width(), 50), Qt::AlignCenter, QString("IMPACT FORCE: %1%").arg(m_shakeIntensity));

    // صفحه مانیتور اسیلوسکوپ / زلزله‌نگار دیجیتال
    QRectF seismoRect(width()/2.0 - 180, height()/2.0 + 10, 360, 90);
    painter.setBrush(QColor(5, 15, 10, 220));
    painter.setPen(QPen(QColor(51, 255, 153, 150), 2));
    painter.drawRoundedRect(seismoRect, 8, 8);

    // گرید مانیتور
    painter.setPen(QPen(QColor(51, 255, 153, 30), 1));
    for (int x = seismoRect.left(); x < seismoRect.right(); x += 30) painter.drawLine(x, seismoRect.top(), x, seismoRect.bottom());
    for (int y = seismoRect.top(); y < seismoRect.bottom(); y += 20) painter.drawLine(seismoRect.left(), y, seismoRect.right(), y);

    // رسم موج متحرک زلزله‌نگار
    QPainterPath wavePath;
    qreal midY = seismoRect.center().y();
    qreal stepX = seismoRect.width() / (qreal)m_seismoWaveHistory.size();
    
    wavePath.moveTo(seismoRect.left(), midY);
    for (int i = 0; i < m_seismoWaveHistory.size(); ++i) {
        qreal px = seismoRect.left() + i * stepX;
        qreal py = midY + m_seismoWaveHistory[i] * 1.5;
        py = qBound(seismoRect.top() + 5, py, seismoRect.bottom() - 5);
        wavePath.lineTo(px, py);
    }
    painter.setPen(QPen(QColor(51, 255, 153), 2.0));
    painter.drawPath(wavePath);

    // سندان و جک‌های هیدرولیک
    QRectF baseRect(m_anvilPos.x() - 110, m_anvilPos.y(), 220, 35);
    QLinearGradient bGrad(baseRect.topLeft(), baseRect.bottomLeft());
    bGrad.setColorAt(0, QColor(70, 80, 95));
    bGrad.setColorAt(1, QColor(20, 25, 35));
    painter.setPen(QPen(QColor(51, 255, 153), 2));
    painter.setBrush(bGrad);
    painter.drawRoundedRect(baseRect, 10, 10);

    // جک‌های هیدرولیک کناری
    painter.setPen(QPen(QColor(150, 160, 180), 8, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(baseRect.bottomLeft() + QPointF(20, 0), baseRect.bottomLeft() + QPointF(20, 40));
    painter.drawLine(baseRect.bottomRight() + QPointF(-20, 0), baseRect.bottomRight() + QPointF(-20, 40));

    // کابل وزنه‌ی سقوط
    painter.setPen(QPen(QColor(51, 255, 153, 120), 4));
    painter.drawLine(m_weightPos, QPointF(m_anvilPos.x(), m_anvilPos.y()));

    // وزنه سنگین
    QRectF wRect(m_weightPos.x() - 40, m_weightPos.y() - 55, 80, 55);
    painter.setPen(QPen(Qt::white, 2));
    QLinearGradient wGrad(wRect.topLeft(), wRect.bottomRight());
    wGrad.setColorAt(0, QColor(50, 55, 65));
    wGrad.setColorAt(1, QColor(15, 20, 25));
    painter.setBrush(wGrad);
    painter.drawRoundedRect(wRect, 10, 10);

    QRectF plasma(wRect.x() + 10, wRect.y() + 10, wRect.width() - 20, wRect.height() - 20);
    painter.setBrush(QColor(51, 255, 153));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(plasma, 5, 5);

    painter.setPen(Qt::black);
    painter.setFont(QFont("Consolas", 13, QFont::Black));
    painter.drawText(plasma, Qt::AlignCenter, "HEAVY");
}

// ================== 3. DISPLAY GEARBOX ==================
void AdvancedSettingsWidget::renderDisplayGearbox(QPainter& painter) {
    QString fpsTxt = (m_currentFpsIndex == 3) ? "UNLIMITED (999 FPS)" : QString("%1 FPS").arg(m_fpsOptions[m_currentFpsIndex]);
    painter.setPen(QColor(255, 204, 0));
    painter.setFont(QFont("Consolas", 26, QFont::Bold));
    painter.drawText(QRectF(0, height()/2 - 60, width(), 50), Qt::AlignCenter, QString("FRAME RATE: %1").arg(fpsTxt));

    // ۱. تاکومتر / گیج دور موتور هولوگرافیک سمت راست
    QPointF gaugeCenter(width()/2.0 + 170, height() - 140);
    qreal gaugeRadius = 75.0;
    
    painter.setBrush(QColor(15, 20, 30, 220));
    painter.setPen(QPen(QColor(255, 204, 0, 150), 3));
    painter.drawEllipse(gaugeCenter, gaugeRadius, gaugeRadius);

    // قوس ردلاین (Redline)
    painter.setPen(QPen(QColor(255, 51, 102), 6, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(QRectF(gaugeCenter.x() - gaugeRadius + 10, gaugeCenter.y() - gaugeRadius + 10, (gaugeRadius-10)*2, (gaugeRadius-10)*2), -30 * 16, 50 * 16);

    // عقربه تاکومتر
    painter.save();
    painter.translate(gaugeCenter);
    painter.rotate(m_tachometerGaugeAngle);
    painter.setPen(QPen((m_currentFpsIndex == 3) ? QColor(255, 51, 102) : QColor(255, 204, 0), 4, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(0, 0, 0, -gaugeRadius + 15);
    painter.setBrush(Qt::white);
    painter.drawEllipse(QPointF(0, 0), 8, 8);
    painter.restore();

    painter.setPen((m_currentFpsIndex == 3) ? QColor(255, 51, 102) : Qt::white);
    painter.setFont(QFont("Consolas", 11, QFont::Bold));
    painter.drawText(QRectF(gaugeCenter.x() - 50, gaugeCenter.y() + 20, 100, 30), Qt::AlignCenter, (m_currentFpsIndex == 3) ? "REDLINE!" : "RPM TACHO");

    // ۲. شیار دنده و اهرم مکانیکی
    painter.save();
    painter.translate(m_gearCenter);

    QRectF trackRect(-150, -150, 300, 300);
    painter.setPen(QPen(QColor(20, 25, 35), 45, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(trackRect, 30 * 16, 120 * 16); 

    painter.setPen(QPen(QColor(255, 204, 0, 150), 2, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(trackRect.adjusted(-22, -22, 22, 22), 30 * 16, 120 * 16);
    painter.drawArc(trackRect.adjusted(22, 22, -22, -22), 30 * 16, 120 * 16);

    // چرخ‌دنده مرکزی
    painter.save();
    painter.rotate(m_leverAngle);
    painter.setPen(QPen(QColor(255, 204, 0), 2));
    painter.setBrush(QColor(35, 40, 50));
    painter.drawEllipse(QPointF(0,0), 75, 75); 
    for(int i=0; i<8; i++) {
        painter.rotate(45);
        painter.drawRoundedRect(-16, -90, 32, 22, 5, 5); 
    }
    painter.setBrush(QColor(15, 20, 25));
    painter.drawEllipse(QPointF(0,0), 45, 45); 
    painter.restore();

    // LED های وضعیت دنده
    for(int i=0; i<4; i++) {
        qreal ang = -150.0 + i*40.0;
        qreal rad = ang * M_PI / 180.0;
        QPointF pt(std::cos(rad)*185, std::sin(rad)*185);
        
        bool isActive = (i == m_currentFpsIndex);
        painter.setPen(Qt::NoPen);
        painter.setBrush(isActive ? QColor(255, 204, 0) : QColor(255, 255, 255, 60));
        painter.drawEllipse(pt, 6, 6);

        painter.setPen(isActive ? QColor(255, 204, 0) : Qt::white);
        painter.setFont(QFont("Consolas", 11, QFont::Bold));
        QString txt = (i==3) ? "MAX" : QString::number(m_fpsOptions[i]);
        painter.drawText(QRectF(pt.x()-35, pt.y()-25, 70, 25), Qt::AlignCenter, txt);
    }

    // دسته دنده
    painter.save();
    painter.rotate(m_leverAngle);
    painter.setPen(QPen(QColor(200, 210, 225), 12, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(0, 0, 150, 0); 
    
    painter.setPen(Qt::NoPen);
    QRadialGradient handleGrad(150, 0, 22);
    handleGrad.setColorAt(0, Qt::white);
    handleGrad.setColorAt(0.35, QColor(255, 204, 0));
    handleGrad.setColorAt(1, QColor(60, 45, 0));
    painter.setBrush(handleGrad);
    painter.drawEllipse(QPointF(150, 0), 22, 22);
    painter.restore();

    painter.restore(); 
}