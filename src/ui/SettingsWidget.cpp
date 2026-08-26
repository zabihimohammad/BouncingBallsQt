#include "ThemeManager.h"
#include "SettingsWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QtMath>
#include <QRandomGenerator>
#include <QRadialGradient>
#include <QLinearGradient>
#include "../core/SoundManager.h"

SettingsWidget::SettingsWidget(QWidget* parent) : QWidget(parent) {
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus); // برای دریافت کلیدها
    
    initEnvironment();
    initTargets();

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &SettingsWidget::gameLoop);
    m_timer->start(16); // 60 FPS
}

void SettingsWidget::initEnvironment() {
    auto rng = QRandomGenerator::global();
    m_stars.clear();
    for (int i = 0; i < 150; ++i) {
        SettingsBgStar star;
        star.pos = QPointF(rng->bounded(3000), rng->bounded(2000));
        star.size = 1.0 + rng->generateDouble() * 2.0;
        star.phase = rng->bounded(360) * M_PI / 180.0;
        star.speed = (rng->bounded(5) + 1) / 50.0;
        
        int r = rng->bounded(100);
        if (r < 20) star.color = QColor(0, 242, 254);
        else if (r < 40) star.color = QColor(255, 0, 255);
        else star.color = Qt::white;
        
        m_stars.append(star);
    }
}

void SettingsWidget::initTargets() {
    m_targets.clear();
    auto addTarget = [&](TargetType type, const QString& label, const QColor& color, qreal radius) {
        SettingTarget t;
        t.type = type;
        t.label = label;
        t.color = color;
        t.radius = radius;
        m_targets.append(t);
    };

    addTarget(TargetType::VolUp, "VOL +", QColor(50, 255, 100), 55);
    addTarget(TargetType::VolDown, "VOL -", QColor(255, 50, 80), 55);
    addTarget(TargetType::Fullscreen, "DISPLAY", QColor(0, 200, 255), 60);
    
    addTarget(TargetType::ThemeNeon, "CYBER\nNEON", QColor(0, 242, 254), 45);
    addTarget(TargetType::ThemeCosmic, "COSMIC\nVOID", QColor(192, 132, 252), 45);
    addTarget(TargetType::ThemeSolar, "SOLAR\nFLARE", QColor(239, 68, 68), 45);
    addTarget(TargetType::ThemeMatrix, "MATRIX\nGREEN", QColor(16, 185, 129), 45);
    addTarget(TargetType::EMP, "SHOCKWAVE", QColor(255, 120, 0), 45);
    addTarget(TargetType::ProMode, "PRO MODE", QColor(0, 255, 100), 65);
    addTarget(TargetType::BackToMenu, "EXIT", QColor(200, 200, 200), 50);
}

void SettingsWidget::scatterTargets() {
    QPointF center(width() / 2.0, height() / 3.0);
    for (auto& t : m_targets) {
        t.pos = center;
        qreal angle = (QRandomGenerator::global()->bounded(360)) * M_PI / 180.0;
        qreal speed = (QRandomGenerator::global()->bounded(100, 300)) / 10.0;
        t.velocity = QPointF(std::cos(angle) * speed, std::sin(angle) * speed);
    }
}

void SettingsWidget::gameLoop() {
    if (width() < 100) return;
    m_time += 0.03;

    // شلیک ممتد
    if (m_isFiring) {
        if (m_framesSinceLastFire >= 4) { // رگبار سریع (هر 4 فریم)
            fireProjectile();
            m_framesSinceLastFire = 0;
            m_cannonHeat = qMin(1.0, m_cannonHeat + 0.15);
            m_cannonRecoil = 15.0; // انیمیشن لگد
        }
    }
    m_framesSinceLastFire++;
    
    // بازیابی لگد و خنک شدن لوله
    m_cannonRecoil += (0.0 - m_cannonRecoil) * 0.2;
    m_cannonHeat = qMax(0.0, m_cannonHeat - 0.02);

    // پیشروی گلیچ
    if (m_isGlitching) {
        m_glitchWave += 0.04;
        if (m_glitchWave > 1.2) m_isGlitching = false;
    }

    // آپدیت گلوله‌ها
    for (auto& p : m_projectiles) {
        p.pos += p.velocity;
        if (p.pos.x() < 0 || p.pos.x() > width() || p.pos.y() < 0 || p.pos.y() > height()) {
            p.active = false;
        }
    }
    m_projectiles.erase(std::remove_if(m_projectiles.begin(), m_projectiles.end(),
                        [](const SettingsProjectile& p) { return !p.active; }), m_projectiles.end());

    applyTargetRepulsion();
    
    for (int i = 0; i < m_targets.size(); ++i) {
        auto& t = m_targets[i];
        t.pos += t.velocity;
        t.velocity *= 0.98; // اصطکاک
        
        if (t.velocity.manhattanLength() < 0.6) {
            t.velocity.setX(t.velocity.x() > 0 ? 0.6 : -0.6);
            t.velocity.setY(t.velocity.y() > 0 ? 0.6 : -0.6);
        }

        // چرخش حلقه‌ها
        t.rotX = std::fmod(t.rotX + t.rotSpeedX, 360.0);
        t.rotY = std::fmod(t.rotY + t.rotSpeedY, 360.0);
        t.rotZ = std::fmod(t.rotZ + t.rotSpeedZ, 360.0);

        // بازیابی فیزیک ژله‌ای (Squash & Stretch)
        t.squash += (1.0 - t.squash) * 0.15;
        t.stretch += (1.0 - t.stretch) * 0.15;
        if (t.hitScale > 1.0) t.hitScale -= 0.05;

        // بررسی برخورد لیزر با درون‌ها
        for (auto& p : m_projectiles) {
            if (!p.active) continue;
            qreal dist = std::hypot(p.pos.x() - t.pos.x(), p.pos.y() - t.pos.y());
            if (dist < t.radius * t.hitScale) {
                p.active = false;
                triggerHitEffect(i, p.pos);
            }
        }
    }
    keepTargetsInBounds();

    // آپدیت ذرات
    for (auto& part : m_particles) {
        part.pos += part.velocity;
        part.life -= 0.02; 
    }
    m_particles.erase(std::remove_if(m_particles.begin(), m_particles.end(),
                      [](const SettingsParticle& part) { return part.life <= 0; }), m_particles.end());

    if (m_empActive) {
        m_empRadius += 40.0; 
        if (m_empRadius > 2500) m_empActive = false;
    }

    update();
}

void SettingsWidget::fireProjectile() {
    SettingsProjectile p;
    p.pos = QPointF(width() / 2.0, height() - 40.0);
    qreal speed = 40.0; 
    p.velocity = QPointF(std::cos(m_cannonAngle) * speed, std::sin(m_cannonAngle) * speed);
    p.color = m_cannonHeat > 0.6 ? QColor(255, 100, 50) : QColor(0, 242, 254); // لیزر داغ
    m_projectiles.append(p);
    
    spawnParticles(p.pos, p.color, 3);
    SoundManager::instance().playShoot();
}

void SettingsWidget::triggerHitEffect(int targetIndex, const QPointF& hitPos) {
    auto& t = m_targets[targetIndex];
    t.hitScale = 1.25;
    t.squash = 0.6;   // فشرده شدن
    t.stretch = 1.4;  // کشیده شدن
    
    // سرعت چرخش موقتاً زیاد می‌شود
    t.rotSpeedX += (t.rotSpeedX > 0 ? 5.0 : -5.0);
    t.rotSpeedY += (t.rotSpeedY > 0 ? 5.0 : -5.0);
    
    spawnParticles(hitPos, t.color, 15);
    handleTargetAction(t.type, t.pos);
}

void SettingsWidget::handleTargetAction(TargetType type, const QPointF& hitPos) {
    switch (type) {
    case TargetType::VolUp: 
        m_volume = qMin(100, m_volume + 10); 
        SoundManager::instance().setMusicVolume(m_volume);
        SoundManager::instance().setSfxVolume(m_volume);
        SoundManager::instance().playPop();
        break;
            
    case TargetType::VolDown: 
        m_volume = qMax(0, m_volume - 10); 
        SoundManager::instance().setMusicVolume(m_volume);
        SoundManager::instance().setSfxVolume(m_volume);
        SoundManager::instance().playPop();
        break;
            
    case TargetType::Fullscreen: 
        m_fullscreen = !m_fullscreen; 
        emit fullscreenToggled(m_fullscreen); 
        SoundManager::instance().playPop();
        break;
            
    case TargetType::ThemeNeon: 
        ThemeManager::instance().setTheme(ThemeId::CyberNeon);
        m_currentTheme = "Cyber Neon";
        m_isGlitching = true;
        m_glitchWave = 0.0;
        triggerEMP(hitPos); 
        SoundManager::instance().playPop();
        break;
            
    case TargetType::ThemeCosmic: 
        ThemeManager::instance().setTheme(ThemeId::CosmicVoid);
        m_currentTheme = "Cosmic Void";
        m_isGlitching = true;
        m_glitchWave = 0.0;
        triggerEMP(hitPos); 
        SoundManager::instance().playPop();
        break;

    case TargetType::ThemeSolar: 
        ThemeManager::instance().setTheme(ThemeId::SolarFlare);
        m_currentTheme = "Solar Flare";
        m_isGlitching = true;
        m_glitchWave = 0.0;
        triggerEMP(hitPos); 
        SoundManager::instance().playPop();
        break;

    case TargetType::ThemeMatrix: 
        ThemeManager::instance().setTheme(ThemeId::MatrixGreen);
        m_currentTheme = "Matrix Green";
        m_isGlitching = true;
        m_glitchWave = 0.0;
        triggerEMP(hitPos); 
        SoundManager::instance().playPop();
        break;
            
    
            
    case TargetType::EMP: 
        triggerEMP(hitPos); 
        SoundManager::instance().playShoot(); 
        break;
            
    case TargetType::BackToMenu: 
        emit backClicked(); 
        SoundManager::instance().playPop();
        break;
            
    case TargetType::ProMode: 
        emit proModeClicked(); 
        SoundManager::instance().playPop();
        break;   
    }
}

void SettingsWidget::triggerEMP(const QPointF& center) {
    m_empActive = true;
    m_empRadius = 0.0;
    for (auto& t : m_targets) {
        qreal dx = t.pos.x() - center.x();
        qreal dy = t.pos.y() - center.y();
        qreal dist = std::hypot(dx, dy);
        if (dist > 0.1) {
            t.velocity.setX(t.velocity.x() + (dx / dist) * 30.0); 
            t.velocity.setY(t.velocity.y() + (dy / dist) * 30.0);
        }
    }
}

void SettingsWidget::spawnParticles(const QPointF& pos, const QColor& color, int count) {
    auto rng = QRandomGenerator::global();
    for (int i = 0; i < count; ++i) {
        SettingsParticle part;
        part.pos = pos;
        qreal angle = rng->bounded(360) * M_PI / 180.0;
        qreal speed = rng->bounded(40, 120) / 10.0;
        part.velocity = QPointF(std::cos(angle) * speed, std::sin(angle) * speed);
        part.maxLife = (rng->bounded(50) + 50) / 100.0;
        part.life = part.maxLife;
        part.color = color;
        m_particles.append(part);
    }
}

void SettingsWidget::applyTargetRepulsion() {
    for (int i = 0; i < m_targets.size(); ++i) {
        for (int j = i + 1; j < m_targets.size(); ++j) {
            auto& t1 = m_targets[i];
            auto& t2 = m_targets[j];
            qreal dx = t2.pos.x() - t1.pos.x();
            qreal dy = t2.pos.y() - t1.pos.y();
            qreal dist = std::hypot(dx, dy);
            qreal minDist = (t1.radius + t2.radius) * 1.3; 

            if (dist < minDist && dist > 0.1) {
                qreal force = (minDist - dist) * 0.08;
                t1.velocity.setX(t1.velocity.x() - (dx / dist) * force);
                t1.velocity.setY(t1.velocity.y() - (dy / dist) * force);
                t2.velocity.setX(t2.velocity.x() + (dx / dist) * force);
                t2.velocity.setY(t2.velocity.y() + (dy / dist) * force);
            }
        }
    }
}

void SettingsWidget::keepTargetsInBounds() {
    int w = width();
    int h = height();
    for (auto& t : m_targets) {
        if (t.pos.x() - t.radius < 50) { t.pos.setX(t.radius + 50); t.velocity.setX(std::abs(t.velocity.x())); }
        if (t.pos.x() + t.radius > w - 50) { t.pos.setX(w - t.radius - 50); t.velocity.setX(-std::abs(t.velocity.x())); }
        
        if (t.pos.y() - t.radius < 120) { t.pos.setY(t.radius + 120); t.velocity.setY(std::abs(t.velocity.y())); }
        if (t.pos.y() + t.radius > h - 180) { t.pos.setY(h - t.radius - 180); t.velocity.setY(-std::abs(t.velocity.y())); }
    }
}

// ================= ورودی کاربر =================

void SettingsWidget::mouseMoveEvent(QMouseEvent* event) {
    m_mousePos = event->position();
    qreal dx = m_mousePos.x() - (width() / 2.0);
    qreal dy = m_mousePos.y() - (height() - 40.0);
    m_cannonAngle = std::atan2(dy, dx);
}

void SettingsWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        // ابتدا بررسی کنیم آیا مستقیم روی گوی کلیک شده است (تعامل دوگانه)
        bool directHit = false;
        for (int i = 0; i < m_targets.size(); ++i) {
            auto& t = m_targets[i];
            if (std::hypot(event->position().x() - t.pos.x(), event->position().y() - t.pos.y()) < t.radius) {
                triggerHitEffect(i, event->position());
                directHit = true;
                break;
            }
        }
        
        if (!directHit) {
            m_isFiring = true;
            m_framesSinceLastFire = 4; // شلیک آنی در فریم اول
        }
    }
}

void SettingsWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_isFiring = false;
    }
}

void SettingsWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Space) {
        m_isFiring = true;
        m_framesSinceLastFire = 4;
    } else if (event->key() == Qt::Key_Right || event->key() == Qt::Key_Up) {
        handleTargetAction(TargetType::VolUp, QPointF(width()/2.0, height()/2.0));
    } else if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Down) {
        handleTargetAction(TargetType::VolDown, QPointF(width()/2.0, height()/2.0));
    }
}

void SettingsWidget::keyReleaseEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Space) {
        m_isFiring = false;
    }
}

void SettingsWidget::resizeEvent(QResizeEvent* event) {
    if (m_firstShow && width() > 100) {
        scatterTargets(); 
        m_firstShow = false;
    }
    QWidget::resizeEvent(event);
}

// ================= رندرینگ شاهکار =================

void SettingsWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawNebulaBackground(painter);

    // موج انفجار EMP
    if (m_empActive) {
        painter.setPen(QPen(QColor(0, 242, 254, qMax(0, 150 - int(m_empRadius / 15.0))), 8));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(QPointF(width()/2, height()/2), m_empRadius, m_empRadius);
    }

    // درون‌های شناور
    for (const auto& t : m_targets) {
        drawTargetDrone(painter, t);
    }

    drawLaserProjectiles(painter);
    drawParticles(painter);
    drawAdvancedCannon(painter);
    
    if (m_isGlitching) {
        drawGlitchSweep(painter);
    }

    drawHUD(painter);
}

void SettingsWidget::drawNebulaBackground(QPainter& painter) {
    int baseHue = ThemeManager::instance().getBaseHue();
    QColor bgTop = QColor::fromHsv(baseHue, 220, 18);
    QColor bgBottom = QColor::fromHsv(baseHue, 240, 6);
    
    QLinearGradient bgGrad(0, 0, 0, height());
    bgGrad.setColorAt(0.0, bgTop); 
    bgGrad.setColorAt(1.0, bgBottom);
    painter.fillRect(rect(), bgGrad);

    // سحابی مرکزی با رنگ تم
    QRadialGradient neb1(width() * 0.35, height() * 0.45, height() * 0.8);
    QColor nC1 = ThemeManager::instance().getPrimaryColor(); 
    nC1.setAlpha(45);
    neb1.setColorAt(0, nC1);
    neb1.setColorAt(1, Qt::transparent);
    painter.fillRect(rect(), neb1);

    QRadialGradient neb2(width() * 0.7, height() * 0.55, height() * 0.7);
    QColor nC2 = ThemeManager::instance().getSecondaryColor(); 
    nC2.setAlpha(40);
    neb2.setColorAt(0, nC2);
    neb2.setColorAt(1, Qt::transparent);
    painter.fillRect(rect(), neb2);

    for (int i = 0; i < m_stars.size(); ++i) {
        auto& star = m_stars[i];
        qreal currentBright = (std::sin(m_time * star.speed + star.phase) + 1.0) / 2.0;
        QColor c;
        if (i % 3 == 0) c = ThemeManager::instance().getPrimaryColor();
        else if (i % 3 == 1) c = ThemeManager::instance().getSecondaryColor();
        else c = Qt::white;
        c.setAlpha(int(currentBright * 200 + 40));
        painter.setPen(Qt::NoPen);
        painter.setBrush(c);
        painter.drawEllipse(star.pos, star.size, star.size);
    }
}

void SettingsWidget::drawTargetDrone(QPainter& painter, const SettingTarget& t) {
    painter.save();
    painter.translate(t.pos);
    
    // اعمال فیزیک لهیدگی در جهت حرکت
    qreal angle = std::atan2(t.velocity.y(), t.velocity.x()) * 180 / M_PI;
    painter.rotate(angle);
    painter.scale(t.stretch * t.hitScale, t.squash * t.hitScale);
    painter.rotate(-angle); // برگرداندن چرخش برای رسم محتوا
    
    qreal r = t.radius;

    // هاله نوری درون (Glow)
    QRadialGradient glow(0, 0, r * 1.5);
    glow.setColorAt(0, QColor(t.color.red(), t.color.green(), t.color.blue(), 100));
    glow.setColorAt(1, Qt::transparent);
    painter.setBrush(glow);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(0,0), r*1.5, r*1.5);

    // رسم حلقه‌های مداری سه بعدی فیک
    painter.setBrush(Qt::NoBrush);
    
    // حلقه اول
    painter.save();
    painter.rotate(t.rotX);
    painter.scale(1.0, 0.3);
    painter.setPen(QPen(QColor(255, 255, 255, 120), 2.0));
    painter.drawEllipse(QPointF(0,0), r, r);
    painter.restore();
    
    // حلقه دوم
    painter.save();
    painter.rotate(t.rotY);
    painter.scale(0.3, 1.0);
    painter.setPen(QPen(t.color, 3.0));
    painter.drawEllipse(QPointF(0,0), r, r);
    painter.restore();

    // هسته اصلی شیشه‌ای
    painter.setPen(QPen(t.color, 2.0));
    painter.setBrush(QColor(t.color.red(), t.color.green(), t.color.blue(), 30));
    painter.drawEllipse(QPointF(0,0), r*0.8, r*0.8);

    drawDroneVisuals(painter, t, r);

    // سایه متن برای خوانایی
    painter.setPen(QColor(0, 0, 0, 200));
    painter.setFont(QFont("Consolas", 10, QFont::Bold));
    painter.drawText(QRectF(-r, r*0.3, r*2, r), Qt::AlignCenter, t.label);
    
    painter.setPen(Qt::white);
    painter.drawText(QRectF(-r, r*0.3 - 2, r*2, r), Qt::AlignCenter, t.label);

    painter.restore();
}

void SettingsWidget::drawDroneVisuals(QPainter& painter, const SettingTarget& t, qreal r) {
    // ویژوال‌های زنده داخل گوی
    if (t.type == TargetType::VolUp || t.type == TargetType::VolDown) {
        // اکولایزر ولوم
        int bars = 10;
        int activeBars = (m_volume * bars) / 100;
        qreal barW = (r * 1.2) / bars;
        qreal startX = -r * 0.6;
        for(int i=0; i<bars; ++i) {
            qreal barH = (std::sin(m_time * 5.0 + i) * 0.5 + 0.5) * (r * 0.5) + (r * 0.2);
            QRectF barRect(startX + i*barW + 1, -barH*0.5 - 10, barW - 2, barH);
            if (i < activeBars) {
                painter.fillRect(barRect, t.color);
            } else {
                painter.fillRect(barRect, QColor(255,255,255,30));
            }
        }
    } 
    else if (t.type == TargetType::Fullscreen) {
        // آیکون مانیتور
        painter.setPen(QPen(Qt::white, 2.0));
        painter.setBrush(m_fullscreen ? QColor(0, 242, 254, 100) : Qt::NoBrush);
        painter.drawRect(QRectF(-r*0.4, -r*0.5, r*0.8, r*0.6));
        painter.drawLine(QPointF(0, r*0.1), QPointF(0, r*0.3));
        painter.drawLine(QPointF(-r*0.2, r*0.3), QPointF(r*0.2, r*0.3));
    }
    else if (t.type == TargetType::ThemeNeon || t.type == TargetType::ThemeCosmic || t.type == TargetType::ThemeSolar || t.type == TargetType::ThemeMatrix) {
        // پرتال مینیاتوری چرخشی
        int arcRot = int(m_time * 100) % 5760;
        painter.setPen(QPen(t.color, 4.0));
        painter.drawArc(QRectF(-r*0.4, -r*0.4, r*0.8, r*0.8), arcRot, 2880);
    }
}

void SettingsWidget::drawLaserProjectiles(QPainter& painter) {
    for (const auto& p : m_projectiles) {
        // لیزر ضخیم و نورانی
        painter.setPen(QPen(QColor(255, 255, 255, 200), 5, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(p.pos, p.pos - (p.velocity * 1.5));
        
        painter.setPen(QPen(p.color, 12, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(p.pos, p.pos - (p.velocity * 1.0));
    }
}

void SettingsWidget::drawParticles(QPainter& painter) {
    for (const auto& part : m_particles) {
        qreal ratio = part.life / part.maxLife;
        painter.setPen(Qt::NoPen);
        QColor c = part.color;
        c.setAlpha(int(ratio * 255));
        painter.setBrush(c);
        painter.drawEllipse(part.pos, 4.0 * ratio, 4.0 * ratio);
    }
}

void SettingsWidget::drawAdvancedCannon(QPainter& painter) {
    painter.save();
    // پایه توپ
    painter.translate(width() / 2.0, height() - 40.0);
    
    // اتصال لوله با احتساب لگد (Recoil)
    painter.rotate(qRadiansToDegrees(m_cannonAngle));
    painter.translate(-m_cannonRecoil, 0); // لگد به عقب
    
    // رنگ لوله بر اساس حرارت
    QColor barrelColor(15, 20, 30);
    QColor heatColor = QColor(255, 50, 0);
    
    painter.setBrush(barrelColor);
    painter.setPen(QPen(ThemeManager::instance().getPrimaryColor(), 3));
    painter.drawRect(0, -14, 60, 28);
    
    // درخشش حرارتی سر لوله
    if (m_cannonHeat > 0) {
        QLinearGradient hG(0, 0, 60, 0);
        hG.setColorAt(0, Qt::transparent);
        hG.setColorAt(1, QColor(heatColor.red(), heatColor.green(), heatColor.blue(), int(m_cannonHeat * 255)));
        painter.setBrush(hG);
        painter.setPen(Qt::NoPen);
        painter.drawRect(0, -14, 60, 28);
    }
    
    // هسته چرخان
    painter.translate(m_cannonRecoil, 0); // برگرداندن لگد برای پایه
    painter.rotate((m_isFiring ? m_time * 1000 : 0)); // چرخش هسته هنگام شلیک
    
    painter.setBrush(QColor(30, 40, 60));
    painter.setPen(QPen(Qt::white, 2));
    painter.drawEllipse(QPointF(0, 0), 30, 30);
    
    // نماد انرژی داخل توپ
    painter.setBrush(m_cannonHeat > 0.7 ? heatColor : ThemeManager::instance().getPrimaryColor());
    painter.drawPolygon(QPolygonF() << QPointF(0, -15) << QPointF(15, 15) << QPointF(-15, 15));
    
    painter.restore();

    // خط نشانه‌گیر لیزری
    if (!m_isFiring) {
        QColor aimCol = ThemeManager::instance().getPrimaryColor();
        aimCol.setAlpha(80);
        painter.setPen(QPen(aimCol, 2, Qt::DashLine));
        painter.drawLine(QPointF(width() / 2.0, height() - 40.0), m_mousePos);
    }
}

void SettingsWidget::drawHUD(QPainter& painter) {
    // خطوط اسکن‌لاین
    painter.setPen(QPen(QColor(0, 0, 0, 40), 1.0));
    for (int y = 0; y < height(); y += 3) {
        painter.drawLine(0, y, width(), y);
    }

    // رابط هولوگرامی بالا
    QRectF hudRect(20, 20, width() - 40, 70);
    QColor hudBg = ThemeManager::instance().getPrimaryColor();
    hudBg.setAlpha(20);
    painter.setBrush(hudBg);
    
    QColor hudBorder = ThemeManager::instance().getPrimaryColor();
    hudBorder.setAlpha(150);
    painter.setPen(QPen(hudBorder, 2));
    painter.drawRect(hudRect);
    
    // براکت‌های گوشه
    painter.setPen(QPen(Qt::white, 3));
    painter.drawLine(hudRect.topLeft(), hudRect.topLeft() + QPointF(15, 0));
    painter.drawLine(hudRect.topLeft(), hudRect.topLeft() + QPointF(0, 15));
    
    painter.setPen(ThemeManager::instance().getPrimaryColor());
    painter.setFont(QFont("Consolas", 16, QFont::Bold));
    QString statusText = QString("[ SYSTEM OVERRIDE ]  VOL: %1%  |  DISP: %2  |  THEME: %3")
                         .arg(m_volume)
                         .arg(m_fullscreen ? "FULL" : "WIN")
                         .arg(ThemeManager::instance().themeName());
    painter.drawText(hudRect, Qt::AlignCenter, statusText);
}

void SettingsWidget::drawGlitchSweep(QPainter& painter) {
    // موج تغییر تم که از بالا به پایین میاد
    qreal yPos = height() * m_glitchWave;
    
    // خط نورانی اصلی
    painter.setPen(QPen(Qt::white, 5));
    painter.drawLine(0, yPos, width(), yPos);
    
    // هاله اطراف خط
    QLinearGradient g(0, yPos - 50, 0, yPos + 50);
    g.setColorAt(0, Qt::transparent);
    g.setColorAt(0.5, QColor(0, 255, 120, 150));
    g.setColorAt(1, Qt::transparent);
    painter.fillRect(0, yPos - 50, width(), 100, g);
    
    // شبکه‌ی هولوگرافیک دیجیتال (Grid) بالای خط
    painter.setPen(QPen(QColor(0, 255, 120, 40), 1));
    for (int y = 0; y < yPos; y += 40) {
        painter.drawLine(0, y, width(), y);
    }
    for (int x = 0; x < width(); x += 40) {
        painter.drawLine(x, 0, x, yPos);
    }
}