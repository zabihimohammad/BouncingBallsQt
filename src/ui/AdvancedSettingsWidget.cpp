#include "AdvancedSettingsWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>
#include <algorithm>
#include <QRandomGenerator>
#include <QApplication> 

AdvancedSettingsWidget::AdvancedSettingsWidget(QWidget* parent) : QWidget(parent) {
    setMouseTracking(true);
    initOrbit();
    
    m_tracks = {"CYBER NEON", "RETRO ARCADE", "SYNTHWAVE", "DEEP SPACE"};

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &AdvancedSettingsWidget::updateFrame);
    m_timer->start(16); 
}

void AdvancedSettingsWidget::initOrbit() {
    m_nodes.clear();
    m_nodes.push_back({0.0,   "AUDIO\nLAUNCHER", QColor(0, 242, 254), QPointF(), 1.0, 0.0});
    m_nodes.push_back({90.0,  "GRAPHICS\nREACTOR", QColor(255, 51, 102), QPointF(), 1.0, 0.0});
    m_nodes.push_back({180.0, "HAPTICS\nSEISMOGRAPH", QColor(51, 255, 153), QPointF(), 1.0, 0.0});
    m_nodes.push_back({270.0, "DISPLAY\nGEARBOX", QColor(255, 204, 0), QPointF(), 1.0, 0.0});
}

void AdvancedSettingsWidget::generateStars(int w, int h) {
    m_bgStars.clear();
    m_constellationStars.clear();
    m_constellationPolys.clear();

    for(int i = 0; i < 250; ++i) {
        Star s;
        s.pos = QPointF(QRandomGenerator::global()->bounded(w), QRandomGenerator::global()->bounded(h));
        s.size = QRandomGenerator::global()->bounded(1, 3);
        s.phase = QRandomGenerator::global()->bounded(314) / 100.0;
        s.color = QColor(255, 255, 255, QRandomGenerator::global()->bounded(50, 150));
        m_bgStars.append(s);
    }

    QPainterPath path;
    QFont font("Arial", qMax(50, w / 14), QFont::Black);
    path.addText(0, 0, font, "CYBER BOUNCE");
    
    QRectF bounds = path.boundingRect();
    QTransform transform;
    transform.translate(w / 2.0 - bounds.width() / 2.0 - bounds.left(),
                        h / 3.5 - bounds.height() / 2.0 - bounds.top());
    QPainterPath translatedPath = transform.map(path);

    for (const QPolygonF& poly : translatedPath.toSubpathPolygons()) {
        QPolygonF sparsePoly;
        if (poly.isEmpty()) continue;
        sparsePoly.append(poly.first());
        for (int i = 1; i < poly.size(); ++i) {
            if (QLineF(sparsePoly.last(), poly[i]).length() > 18.0) sparsePoly.append(poly[i]);
        }
        if (poly.isClosed() && QLineF(sparsePoly.last(), sparsePoly.first()).length() > 18.0) {
            sparsePoly.append(sparsePoly.first());
        }
        m_constellationPolys.append(sparsePoly);

        for (const QPointF& pt : sparsePoly) {
            Star s; s.pos = pt; s.size = QRandomGenerator::global()->bounded(2, 5);
            s.phase = QRandomGenerator::global()->bounded(314) / 100.0; s.color = QColor(0, 242, 254);
            m_constellationStars.append(s);
        }
    }
}

void AdvancedSettingsWidget::spawnParticles(const QPointF& pos, const QColor& color, int count, qreal speedMult) {
    for (int i = 0; i < count; ++i) {
        AdvParticle part;
        part.pos = pos;
        qreal angle = (QRandomGenerator::global()->bounded(360)) * M_PI / 180.0;
        qreal speed = ((QRandomGenerator::global()->bounded(40, 120)) / 10.0) * speedMult;
        part.velocity = QPointF(std::cos(angle) * speed, std::sin(angle) * speed);
        part.life = 1.0;
        part.color = color;
        m_particles.append(part);
    }
}

void AdvancedSettingsWidget::resizeEvent(QResizeEvent* event) {
    int w = event->size().width();
    int h = event->size().height();
    generateStars(w, h);
    
    m_cannonBase = QPointF(150, h - 100);
    m_trackCrystalPos = QPointF(150, h - 350);
    
    m_reactorCorePos = QPointF(w / 2.0, h - 220);
    m_capsules.clear();
    m_capsules.append({QPointF(w/2 - 250, h - 100), QPointF(w/2 - 250, h - 100), "LOW", QColor(100, 255, 100), 0});
    m_capsules.append({QPointF(w/2, h - 80), QPointF(w/2, h - 80), "MEDIUM", QColor(255, 200, 0), 1});
    m_capsules.append({QPointF(w/2 + 250, h - 100), QPointF(w/2 + 250, h - 100), "ULTRA", QColor(255, 50, 100), 2});

    m_anvilPos = QPointF(w / 2.0, h - 80);
    m_weightPos = QPointF(w / 2.0, h - 80 - m_shakeIntensity * 3.0); 

    m_gearCenter = QPointF(w / 2.0, h - 100);

    QWidget::resizeEvent(event);
}

void AdvancedSettingsWidget::updateFrame() {
    m_time += 0.05;
    m_currentScreenShake *= 0.85; 

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
    
    for (auto& part : m_particles) {
        part.pos += part.velocity;
        part.velocity.setY(part.velocity.y() + 0.5); 
        part.life -= 0.03; 
    }
    m_particles.erase(std::remove_if(m_particles.begin(), m_particles.end(),
                      [](const AdvParticle& part) { return part.life <= 0; }), m_particles.end());

    calculate3D();
    update();
}

void AdvancedSettingsWidget::updateAudioLauncher() {
    if (m_audioBallFlying) {
        m_audioBallPos += m_audioBallVel;
        m_audioBallVel.setY(m_audioBallVel.y() + 0.6); 

        // شلیک به کریستال تغییر آهنگ
        if (std::hypot(m_audioBallPos.x() - m_trackCrystalPos.x(), m_audioBallPos.y() - m_trackCrystalPos.y()) < 45.0) {
            m_currentTrackIndex = (m_currentTrackIndex + 1) % m_tracks.size(); 
            spawnParticles(m_audioBallPos, QColor(255, 51, 200), 40, 1.5);
            m_audioBallFlying = false; 
            
            // تغییر آهنگ واقعی
            SoundManager::instance().playMusic(m_tracks[m_currentTrackIndex]);
            SoundManager::instance().playPop();
            return; 
        }

        // فرود روی خط‌کش تنظیم ولوم
        qreal groundY = height() - 100;
        if (m_audioBallPos.y() >= groundY) {
            m_audioBallPos.setY(groundY);
            m_audioBallFlying = false;

            qreal startX = 250;
            qreal endX = width() - 100;
            qreal hitX = m_audioBallPos.x() - startX;
            m_volume = qBound(0, (int)std::round((hitX / (endX - startX)) * 100.0), 100);

            // اعمال ولوم واقعی
            SoundManager::instance().setMusicVolume(m_volume);
            SoundManager::instance().setSfxVolume(m_volume);

            spawnParticles(m_audioBallPos, QColor(0, 242, 254), 25);
            SoundManager::instance().playBounce();
            m_currentScreenShake = 10.0; 
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
}

void AdvancedSettingsWidget::updateHapticsSeismograph() {
    if (m_weightFalling) {
        m_weightPos.setY(m_weightPos.y() + m_weightVelY);
        m_weightVelY += 1.5; 
        
        if (m_weightPos.y() >= m_anvilPos.y() - 20) { 
            m_weightPos.setY(m_anvilPos.y() - 20); // استفاده از setY صحیح
            m_weightFalling = false;
            
            qreal dropHeight = (height() - 100) - m_dragPos.y();
            m_shakeIntensity = qBound(0, (int)(dropHeight / 3.0), 100);
            
            m_currentScreenShake = m_shakeIntensity * 0.5; 
            spawnParticles(m_weightPos, QColor(51, 255, 153), 30, 2.0);
            QApplication::beep();
        }
    } else if (!m_isDraggingWeight) {
        qreal targetY = m_anvilPos.y() - 20 - m_shakeIntensity * 3.0;
        m_weightPos.setY(m_weightPos.y() * 0.8 + targetY * 0.2);
    }
}

void AdvancedSettingsWidget::updateDisplayGearbox() {
    if (!m_isDraggingLever) {
        // حرکت نرم به سمت زاویه مورد نظر (-150 تا -30)
        qreal targetAngle = -150.0 + m_currentFpsIndex * 40.0; 
        m_leverAngle = m_leverAngle * 0.8 + targetAngle * 0.2;
    }
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

void AdvancedSettingsWidget::mouseMoveEvent(QMouseEvent* event) {
    m_mousePos = event->position();
    if (!m_inSubMenu) {
        qreal dx = m_mousePos.x() - width() / 2.0;
        m_orbitSpeed = (dx * 0.003);
        if (std::abs(m_orbitSpeed) < 0.1) m_orbitSpeed = 0.1;
    } 
    else if (m_activeNodeIndex == 0 && m_isDraggingCannon) {
        QLineF pullLine(m_cannonBase, m_mousePos);
        if (pullLine.length() > 200) {
            pullLine.setLength(200);
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
        // محاسبه زاویه چرخش در نیمه بالایی (از -150 تا -30)
        qreal angle = std::atan2(m_mousePos.y() - m_gearCenter.y(), m_mousePos.x() - m_gearCenter.x()) * 180.0 / M_PI;
        if (angle > 0) angle -= 360; // محدود کردن به مقادیر منفی در نیمه بالایی صفحه
        
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
        if (!m_inSubMenu) {
            for (int i = 0; i < m_nodes.size(); ++i) {
                auto& node = m_nodes[i];
                if (std::hypot(m_mousePos.x() - node.projectedPos.x(), m_mousePos.y() - node.projectedPos.y()) < 40 * node.scale * m_currentScale && node.zDepth >= 0) {
                    m_inSubMenu = true;
                    m_activeNodeIndex = i;
                    m_audioBallFlying = false;
                    m_audioBallPos = m_cannonBase;
                    break;
                }
            }
        } 
        else {
            m_hasInteracted[m_activeNodeIndex] = true; 

            if (m_activeNodeIndex == 0 && std::hypot(m_mousePos.x() - m_cannonBase.x(), m_mousePos.y() - m_cannonBase.y()) < 50) {
                m_isDraggingCannon = true;
                m_dragPos = m_mousePos;
            } 
            else if (m_activeNodeIndex == 1) {
                for (int i = 0; i < m_capsules.size(); ++i) {
                    if (std::hypot(m_mousePos.x() - m_capsules[i].currentPos.x(), m_mousePos.y() - m_capsules[i].currentPos.y()) < 40) {
                        m_isDraggingCapsule = true;
                        m_draggedCapsuleIndex = i;
                        break;
                    }
                }
            }
            else if (m_activeNodeIndex == 2 && std::hypot(m_mousePos.x() - m_weightPos.x(), m_mousePos.y() - m_weightPos.y()) < 60) {
                m_isDraggingWeight = true;
                m_weightFalling = false;
            }
            else if (m_activeNodeIndex == 3) {
                qreal rad = m_leverAngle * M_PI / 180.0;
                QPointF leverHandle = m_gearCenter + QPointF(std::cos(rad)*150, std::sin(rad)*150);
                if (std::hypot(m_mousePos.x() - leverHandle.x(), m_mousePos.y() - leverHandle.y()) < 60) {
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
            spawnParticles(m_cannonBase, Qt::white, 5);
        }
        else if (m_isDraggingCapsule) {
            m_isDraggingCapsule = false;
            if (std::hypot(m_capsules[m_draggedCapsuleIndex].currentPos.x() - m_reactorCorePos.x(), m_capsules[m_draggedCapsuleIndex].currentPos.y() - m_reactorCorePos.y()) < 100) {
                m_graphicsQuality = m_capsules[m_draggedCapsuleIndex].qualityLevel;
                spawnParticles(m_reactorCorePos, m_capsules[m_draggedCapsuleIndex].color, 50, 2.0);
                QApplication::beep();
                m_currentScreenShake = 15.0; 
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
            spawnParticles(snapPos, QColor(255, 204, 0), 15);
            QApplication::beep();
        }
    }
}

void AdvancedSettingsWidget::wheelEvent(QWheelEvent* event) {
    if (!m_inSubMenu) {
        qreal jump = (event->angleDelta().y() > 0) ? 10.0 : -10.0;
        for (auto& node : m_nodes) node.angle += jump;
    }
}

void AdvancedSettingsWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.save();

    if (m_currentScreenShake > 0.5) {
        qreal ox = (QRandomGenerator::global()->bounded(200) - 100) / 100.0 * m_currentScreenShake;
        qreal oy = (QRandomGenerator::global()->bounded(200) - 100) / 100.0 * m_currentScreenShake;
        painter.translate(ox, oy);
    }

    painter.fillRect(rect(), QColor(3, 5, 8));

    painter.setPen(Qt::NoPen);
    for (const auto& s : m_bgStars) {
        int alpha = 50 + 155 * std::abs(std::sin(m_time + s.phase));
        painter.setBrush(QColor(s.color.red(), s.color.green(), s.color.blue(), alpha));
        painter.drawEllipse(s.pos, s.size, s.size);
    }

    int constAlpha = m_inSubMenu ? 20 : 100; 
    painter.setPen(QPen(QColor(0, 242, 254, constAlpha / 2), 1, Qt::DashLine)); 
    for (const QPolygonF& poly : m_constellationPolys) painter.drawPolyline(poly); 
    
    painter.setPen(Qt::NoPen);
    for (const auto& s : m_constellationStars) {
        int alpha = (constAlpha / 2) + (constAlpha) * std::abs(std::sin(m_time * 2.0 + s.phase));
        painter.setBrush(QColor(s.color.red(), s.color.green(), s.color.blue(), alpha));
        painter.drawEllipse(s.pos, s.size, s.size);
    }

    int cx = width() / 2;
    int cy = (height() / 2) + m_planetOffsetY;
    qreal planetRadius = 160.0 * m_currentScale;
    qreal orbitW = m_orbitRadiusX * m_currentScale;
    qreal orbitH = orbitW * std::sin(m_orbitTilt * M_PI / 180.0);
    QRectF orbitRect(cx - orbitW, cy - orbitH, orbitW * 2, orbitH * 2);

    painter.setPen(QPen(QColor(0, 242, 254, 40), 2));
    painter.drawArc(orbitRect, 0 * 16, 180 * 16);

    for (const auto& node : m_nodes) {
        if (node.zDepth < 0) {
            qreal r = 25.0 * node.scale * m_currentScale;
            QRadialGradient nodeGrad(node.projectedPos, r, QPointF(node.projectedPos.x() - r*0.3, node.projectedPos.y() - r*0.3));
            nodeGrad.setColorAt(0.0, QColor(255, 255, 255, 80)); 
            nodeGrad.setColorAt(0.2, QColor(node.color.red(), node.color.green(), node.color.blue(), 100));
            nodeGrad.setColorAt(0.8, QColor(0, 0, 0, 180));
            painter.setPen(Qt::NoPen); painter.setBrush(nodeGrad);
            painter.drawEllipse(node.projectedPos, r, r);
        }
    }

    QRadialGradient planetGrad(QPointF(cx, cy), planetRadius, QPointF(cx - planetRadius*0.35, cy - planetRadius*0.35));
    planetGrad.setColorAt(0.0, QColor(255, 255, 255));      
    planetGrad.setColorAt(0.15, QColor(60, 150, 200));      
    planetGrad.setColorAt(0.6, QColor(15, 35, 60));         
    planetGrad.setColorAt(1.0, QColor(2, 5, 10));           
    painter.setPen(Qt::NoPen); painter.setBrush(planetGrad);
    painter.drawEllipse(QPointF(cx, cy), planetRadius, planetRadius);
    
    QRadialGradient atmosphere(cx, cy, planetRadius*1.15);
    atmosphere.setColorAt(0.85, QColor(0, 242, 254, int(50 * m_currentScale))); 
    atmosphere.setColorAt(1.0, QColor(0, 0, 0, 0));
    painter.setBrush(atmosphere);
    painter.drawEllipse(QPointF(cx, cy), planetRadius*1.15, planetRadius*1.15);

    painter.setPen(QPen(QColor(0, 242, 254, 150), 3));
    painter.drawArc(orbitRect, 180 * 16, 180 * 16);

    for (const auto& node : m_nodes) {
        if (node.zDepth >= 0) {
            qreal r = 35.0 * node.scale * m_currentScale;
            QRadialGradient glow(node.projectedPos, r*1.6);
            glow.setColorAt(0.0, QColor(node.color.red(), node.color.green(), node.color.blue(), 120));
            glow.setColorAt(1.0, QColor(0, 0, 0, 0));
            painter.setBrush(glow); painter.drawEllipse(node.projectedPos, r*1.6, r*1.6);

            QRadialGradient nodeGrad(node.projectedPos, r, QPointF(node.projectedPos.x() - r*0.35, node.projectedPos.y() - r*0.35));
            nodeGrad.setColorAt(0.0, QColor(255, 255, 255)); 
            nodeGrad.setColorAt(0.2, node.color);
            nodeGrad.setColorAt(0.7, node.color.darker(300));
            nodeGrad.setColorAt(1.0, QColor(0, 0, 0));
            
            painter.setPen(Qt::NoPen); painter.setBrush(nodeGrad);
            painter.drawEllipse(node.projectedPos, r, r);

            if (m_currentScale > 0.8) {
                painter.setPen(Qt::white);
                painter.setFont(QFont("Arial", 11, QFont::Bold));
                painter.drawText(QRectF(node.projectedPos.x() - 100, node.projectedPos.y() - r - 45, 200, 40), 
                                 Qt::AlignCenter | Qt::AlignBottom, node.label);
            }
        }
    }

    if (m_inSubMenu && m_currentScale < 0.5) {
        if (m_activeNodeIndex == 0) renderAudioLauncher(painter);
        else if (m_activeNodeIndex == 1) renderGraphicsReactor(painter);
        else if (m_activeNodeIndex == 2) renderHapticsSeismograph(painter);
        else if (m_activeNodeIndex == 3) renderDisplayGearbox(painter);
    }

    for (const auto& part : m_particles) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(part.color.red(), part.color.green(), part.color.blue(), int(part.life * 255)));
        painter.drawEllipse(part.pos, 4, 4);
    }

    painter.restore(); 
}

// ==========================================
// 0. AUDIO LAUNCHER
// ==========================================
void AdvancedSettingsWidget::renderAudioLauncher(QPainter& painter) {
    qreal groundY = height() - 100;
    qreal startX = 250;
    qreal endX = width() - 100;

    painter.setPen(QColor(0, 242, 254));
    painter.setFont(QFont("Arial", 28, QFont::Black));
    painter.drawText(QRectF(0, height()/2 - 50, width(), 50), Qt::AlignCenter, QString("VOLUME: %1%").arg(m_volume));

    qreal hoverY = std::sin(m_time * 3.0) * 10.0;
    QPointF orbPos = m_trackCrystalPos + QPointF(0, hoverY);
    
    QRadialGradient trackGlow(orbPos, 60.0);
    trackGlow.setColorAt(0.0, QColor(255, 51, 200, 150));
    trackGlow.setColorAt(1.0, QColor(0, 0, 0, 0));
    painter.setPen(Qt::NoPen);
    painter.setBrush(trackGlow);
    painter.drawEllipse(orbPos, 60, 60);

    QPolygonF crystalTop, crystalBottom;
    qreal crW = 25.0, crH = 40.0;
    qreal rot = m_time * 2.0;
    QPointF topP = orbPos + QPointF(0, -crH);
    QPointF botP = orbPos + QPointF(0, crH);
    QPointF mid1 = orbPos + QPointF(std::cos(rot)*crW, std::sin(rot)*5.0);
    QPointF mid2 = orbPos + QPointF(std::cos(rot + M_PI)*crW, std::sin(rot + M_PI)*5.0);

    painter.setPen(QPen(QColor(255, 150, 255), 2));
    painter.setBrush(QColor(255, 51, 200, 200));
    crystalTop << topP << mid1 << mid2;
    painter.drawPolygon(crystalTop);
    crystalBottom << botP << mid1 << mid2;
    painter.drawPolygon(crystalBottom);

    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(QRectF(orbPos.x() - 150, orbPos.y() - 90, 300, 30), Qt::AlignCenter, m_tracks[m_currentTrackIndex]);

    if (!m_hasInteracted[0]) {
        qreal t = std::fmod(m_time, 3.0); 
        QPointF ghostPos = m_cannonBase + ((t < 1.0) ? QPointF(-80*t, 80*t) : QPointF(-80, 80));
        if (t < 1.5) {
            painter.setPen(QPen(QColor(255, 255, 255, 100), 2, Qt::DashLine));
            painter.drawLine(m_cannonBase, ghostPos);
            painter.setBrush(QColor(255, 255, 255, 50));
            painter.drawEllipse(ghostPos, 15, 15);
            painter.drawText(QRectF(ghostPos.x() - 150, ghostPos.y() + 20, 200, 30), Qt::AlignRight, "PULL TO SHOOT");
        }
    }

    painter.setPen(QPen(QColor(255, 255, 255, 80), 2, Qt::DashLine));
    painter.drawLine(QPointF(startX, groundY), QPointF(endX, groundY));
    for (int i = 0; i <= 100; i += 25) {
        qreal tx = startX + (i / 100.0) * (endX - startX);
        painter.setPen(QPen(QColor(255, 255, 255, 150), 2));
        painter.drawLine(QPointF(tx, groundY - 10), QPointF(tx, groundY + 10));
        painter.setPen((i <= m_volume) ? QColor(0, 242, 254) : QColor(255, 255, 255, 100));
        painter.setFont(QFont("Arial", 10, QFont::Bold));
        painter.drawText(QRectF(tx - 20, groundY + 15, 40, 20), Qt::AlignCenter, QString::number(i));
    }
    qreal curX = startX + (m_volume / 100.0) * (endX - startX);
    painter.setPen(QPen(QColor(0, 242, 254, 150), 4));
    painter.drawLine(QPointF(startX, groundY), QPointF(curX, groundY));

    painter.setPen(QPen(QColor(100, 110, 130), 6, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(m_cannonBase, QPointF(m_cannonBase.x(), groundY)); 
    QPointF drawBall = m_cannonBase;
    if (m_isDraggingCannon) {
        drawBall = m_dragPos;
        painter.setPen(QPen(QColor(255, 51, 102, 200), 3));
        painter.drawLine(m_cannonBase, m_dragPos);
    } else if (m_audioBallFlying) drawBall = m_audioBallPos;

    QRadialGradient bg(drawBall, 18.0, drawBall - QPointF(5,5));
    bg.setColorAt(0, Qt::white); bg.setColorAt(0.3, QColor(0, 242, 254)); bg.setColorAt(1, QColor(0, 40, 80));
    painter.setPen(Qt::NoPen); painter.setBrush(bg);
    painter.drawEllipse(drawBall, 18.0, 18.0);
}

// ==========================================
// 1. GRAPHICS REACTOR
// ==========================================
void AdvancedSettingsWidget::renderGraphicsReactor(QPainter& painter) {
    QString qLabel = (m_graphicsQuality == 0) ? "LOW" : (m_graphicsQuality == 1) ? "MEDIUM" : "ULTRA";
    painter.setPen(QColor(255, 51, 102));
    painter.setFont(QFont("Arial", 28, QFont::Black));
    painter.drawText(QRectF(0, height()/2 - 50, width(), 50), Qt::AlignCenter, QString("GRAPHICS: %1").arg(qLabel));

    qreal coreRadius = 80.0;
    QColor coreColor = (m_graphicsQuality == 0) ? QColor(0, 255, 100) : (m_graphicsQuality == 1) ? QColor(255, 200, 0) : QColor(255, 50, 100);
    qreal pulse = 1.0 + 0.05 * std::sin(m_time * 4.0);
    
    painter.save();
    painter.translate(m_reactorCorePos);
    
    painter.save();
    painter.rotate(m_time * 30.0);
    painter.setPen(QPen(QColor(255, 255, 255, 100), 2, Qt::DashLine));
    painter.drawEllipse(QPointF(0, 0), coreRadius*1.3, coreRadius*1.3);
    painter.restore();
    
    painter.save();
    painter.rotate(-m_time * 40.0);
    painter.setPen(QPen(coreColor, 4, Qt::DotLine));
    painter.drawEllipse(QPointF(0, 0), coreRadius*1.1, coreRadius*1.1);
    painter.restore();

    QRadialGradient coreGrad(0, 0, coreRadius * pulse);
    coreGrad.setColorAt(0.0, Qt::white);
    coreGrad.setColorAt(0.2, coreColor);
    coreGrad.setColorAt(1.0, Qt::transparent);
    painter.setPen(Qt::NoPen);
    painter.setBrush(coreGrad);
    painter.drawEllipse(QPointF(0, 0), coreRadius * pulse, coreRadius * pulse);
    
    painter.restore(); 

    if (!m_hasInteracted[1]) {
        qreal t = std::fmod(m_time, 2.0); 
        QPointF ghostPos = m_capsules[2].homePos * (1.0 - t) + m_reactorCorePos * t;
        painter.setPen(QPen(Qt::white, 2, Qt::DashLine));
        painter.drawEllipse(ghostPos, 20, 20);
        painter.setFont(QFont("Arial", 11, QFont::Bold));
        painter.drawText(QRectF(ghostPos.x() - 100, ghostPos.y() + 30, 200, 30), Qt::AlignCenter, "DRAG IN");
    }

    for (const auto& cap : m_capsules) {
        QRectF cRect(cap.currentPos.x() - 20, cap.currentPos.y() - 40, 40, 80);

        painter.setPen(QPen(QColor(255,255,255,100), 1));
        painter.setBrush(QColor(10, 15, 25, 150));
        painter.drawRoundedRect(cRect, 10, 10);

        QRectF energyRect(cRect.x() + 4, cRect.y() + 20, cRect.width() - 8, cRect.height() - 24);
        QLinearGradient eGrad(energyRect.topLeft(), energyRect.bottomLeft());
        eGrad.setColorAt(0, cap.color);
        eGrad.setColorAt(1, cap.color.darker(300));
        painter.setPen(Qt::NoPen);
        painter.setBrush(eGrad);
        painter.drawRoundedRect(energyRect, 5, 5);

        painter.setBrush(QColor(200, 200, 200));
        painter.drawRoundedRect(cRect.x() + 10, cRect.y() - 5, 20, 10, 3, 3);

        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 9, QFont::Bold));
        painter.drawText(QRectF(cRect.x() - 10, cRect.y() - 25, cRect.width() + 20, 20), Qt::AlignCenter, cap.label);
    }
}

// ==========================================
// 2. HAPTICS SEISMOGRAPH
// ==========================================
void AdvancedSettingsWidget::renderHapticsSeismograph(QPainter& painter) {
    painter.setPen(QColor(51, 255, 153));
    painter.setFont(QFont("Arial", 28, QFont::Black));
    painter.drawText(QRectF(0, height()/2 - 50, width(), 50), Qt::AlignCenter, QString("SHAKE: %1%").arg(m_shakeIntensity));

    QRectF baseRect(m_anvilPos.x() - 100, m_anvilPos.y(), 200, 30);
    QLinearGradient bGrad(baseRect.topLeft(), baseRect.bottomLeft());
    bGrad.setColorAt(0, QColor(60, 70, 80));
    bGrad.setColorAt(1, QColor(20, 25, 30));
    painter.setPen(QPen(QColor(51, 255, 153), 2));
    painter.setBrush(bGrad);
    painter.drawRoundedRect(baseRect, 10, 10);

    painter.setPen(Qt::NoPen);
    QRadialGradient coreGlow(m_anvilPos.x(), m_anvilPos.y() + 15, 60);
    coreGlow.setColorAt(0, QColor(51, 255, 153, 150));
    coreGlow.setColorAt(1, Qt::transparent);
    painter.setBrush(coreGlow);
    painter.drawEllipse(QPointF(m_anvilPos.x(), m_anvilPos.y() + 15), 60, 20);

    painter.setPen(QPen(QColor(51, 255, 153, 100), 4));
    painter.drawLine(m_weightPos, QPointF(m_anvilPos.x(), m_anvilPos.y()));

    QRectF wRect(m_weightPos.x() - 35, m_weightPos.y() - 50, 70, 50);
    painter.setPen(QPen(QColor(200, 200, 200), 2));
    QLinearGradient wGrad(wRect.topLeft(), wRect.bottomRight());
    wGrad.setColorAt(0, QColor(40, 40, 40));
    wGrad.setColorAt(1, QColor(10, 10, 10));
    painter.setBrush(wGrad);
    painter.drawRoundedRect(wRect, 8, 8);

    QRectF plasma(wRect.x() + 10, wRect.y() + 10, wRect.width() - 20, wRect.height() - 20);
    painter.setBrush(QColor(51, 255, 153));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(plasma, 4, 4);

    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 12, QFont::Black));
    painter.drawText(plasma, Qt::AlignCenter, "KG");

    if (!m_hasInteracted[2]) {
        qreal t = std::fmod(m_time, 2.0); 
        QPointF ghostPos = m_anvilPos + QPointF(0, -200 * t);
        painter.setPen(QPen(Qt::white, 2, Qt::DashLine));
        painter.drawEllipse(ghostPos, 20, 20);
        painter.setFont(QFont("Arial", 11, QFont::Bold));
        painter.drawText(QRectF(ghostPos.x() + 30, ghostPos.y() - 10, 150, 30), Qt::AlignLeft, "DRAG UP & DROP");
    }
}

// ==========================================
// 3. DISPLAY GEARBOX
// ==========================================
void AdvancedSettingsWidget::renderDisplayGearbox(QPainter& painter) {
    QString fpsTxt = (m_currentFpsIndex == 3) ? "MAX" : QString("%1 FPS").arg(m_fpsOptions[m_currentFpsIndex]);
    painter.setPen(QColor(255, 204, 0));
    painter.setFont(QFont("Arial", 28, QFont::Black));
    painter.drawText(QRectF(0, height()/2 - 50, width(), 50), Qt::AlignCenter, QString("FRAME RATE: %1").arg(fpsTxt));

    painter.save();
    painter.translate(m_gearCenter);

    // ریل قوسی‌شکل (بخش بالایی چرخ‌دنده از زاویه ۳۰ تا ۱۵۰ درجه در Qt که معادل -۱۵۰ تا -۳۰ ریاضی است)
    QRectF trackRect(-150, -150, 300, 300);
    painter.setPen(QPen(QColor(20, 20, 25), 40, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(trackRect, 30 * 16, 120 * 16); 

    // کادر نئونی دور ریل قوسی
    painter.setPen(QPen(QColor(255, 204, 0, 100), 2, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(trackRect.adjusted(-20, -20, 20, 20), 30 * 16, 120 * 16);
    painter.drawArc(trackRect.adjusted(20, 20, -20, -20), 30 * 16, 120 * 16);

    // چرخ دنده مهندسی
    painter.save();
    painter.rotate(m_leverAngle);
    painter.setPen(QPen(QColor(255, 204, 0), 2));
    painter.setBrush(QColor(30, 30, 35));
    painter.drawEllipse(QPointF(0,0), 70, 70); 
    for(int i=0; i<8; i++) {
        painter.rotate(45);
        painter.drawRoundedRect(-15, -85, 30, 20, 5, 5); 
    }
    painter.setBrush(QColor(15, 15, 20));
    painter.drawEllipse(QPointF(0,0), 40, 40); 
    painter.restore();

    // نقاط قفل شدن روی مدار
    for(int i=0; i<4; i++) {
        qreal ang = -150.0 + i*40.0;
        
        painter.save();
        painter.rotate(ang);
        painter.setPen(QPen(QColor(255,255,255,150), 2, Qt::DashLine));
        painter.drawLine(85, 0, 160, 0); 
        painter.restore();

        qreal rad = ang * M_PI / 180.0;
        QPointF pt(std::cos(rad)*180, std::sin(rad)*180);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 10, QFont::Bold));
        QString txt = (i==3) ? "MAX" : QString::number(m_fpsOptions[i]);
        painter.drawText(QRectF(pt.x()-30, pt.y()-15, 60, 30), Qt::AlignCenter, txt);
    }

    // اهرم سه‌بعدی
    painter.save();
    painter.rotate(m_leverAngle);
    painter.setPen(QPen(QColor(200, 200, 200), 10, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(0, 0, 150, 0); 
    
    painter.setPen(Qt::NoPen);
    QRadialGradient handleGrad(150, 0, 20);
    handleGrad.setColorAt(0, Qt::white);
    handleGrad.setColorAt(0.3, QColor(255, 204, 0));
    handleGrad.setColorAt(1, QColor(50, 40, 0));
    painter.setBrush(handleGrad);
    painter.drawEllipse(QPointF(150, 0), 20, 20);
    painter.restore();

    if (!m_hasInteracted[3]) {
        qreal t = std::fmod(m_time, 2.0); 
        qreal gAng = -150.0 + (t * 120.0);
        qreal gRad = gAng * M_PI / 180.0;
        QPointF ghostPos = QPointF(std::cos(gRad)*150, std::sin(gRad)*150);
        
        painter.setPen(QPen(Qt::white, 2, Qt::DashLine));
        painter.drawEllipse(ghostPos, 20, 20);
        painter.setFont(QFont("Arial", 11, QFont::Bold));
        painter.drawText(QRectF(ghostPos.x() - 75, ghostPos.y() - 40, 150, 30), Qt::AlignCenter, "PULL LEVER");
    }

    painter.restore(); 
}