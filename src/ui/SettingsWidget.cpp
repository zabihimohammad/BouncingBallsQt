#include "SettingsWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QtMath>
#include <QRandomGenerator>
#include "../core/SoundManager.h"

SettingsWidget::SettingsWidget(QWidget* parent) : QWidget(parent) {
    setMouseTracking(true);
    initTargets();

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &SettingsWidget::gameLoop);
    m_timer->start(16); // 60 FPS
}

void SettingsWidget::initTargets() {
    m_targets.clear();
    
    auto addTarget = [&](TargetType type, QString label, QColor color, qreal radius) {
        SettingTarget t;
        t.type = type; t.label = label; t.color = color;
        t.radius = radius;
        // در ابتدا همه در مرکز مخفی هستند تا زمان تغییر سایز اول فرا برسد
        t.pos = QPointF(-1000, -1000); 
        m_targets.append(t);
    };

    addTarget(TargetType::VolUp, "VOL +", QColor(0, 255, 120), 45);
    addTarget(TargetType::VolDown, "VOL -", QColor(255, 50, 80), 45);
    addTarget(TargetType::Fullscreen, "DISPLAY\nMODE", QColor(0, 200, 255), 55);
    
    addTarget(TargetType::ThemeNeon, "THEME:\nNEON", QColor(255, 0, 255), 50);
    addTarget(TargetType::ThemeArcade, "THEME:\nARCADE", QColor(255, 200, 0), 50);
    
    addTarget(TargetType::EMP, "SHOCKWAVE\n(TEST)", QColor(255, 120, 0), 40);
    addTarget(TargetType::BackToMenu, "EXIT TO\nMENU", QColor(200, 200, 200), 65);

    addTarget(TargetType::ProMode, "PRO MODE\nTERMINAL", QColor(0, 255, 100), 60);
}

void SettingsWidget::scatterTargets() {
    // پرتاب حباب‌ها از مرکز به اطراف در لحظه باز شدن صفحه
    QPointF center(width() / 2.0, height() / 3.0);
    for (auto& t : m_targets) {
        t.pos = center;
        qreal angle = (QRandomGenerator::global()->bounded(360)) * M_PI / 180.0;
        qreal speed = (QRandomGenerator::global()->bounded(100, 300)) / 10.0;
        t.velocity = QPointF(std::cos(angle) * speed, std::sin(angle) * speed);
    }
}

void SettingsWidget::gameLoop() {
    if (width() < 100) return; // جلوگیری از باگ فشردگی قبل از رندر کامل

    // آپدیت گلوله‌ها
    for (auto& p : m_projectiles) {
        p.pos += p.velocity;
        if (p.pos.x() < 0 || p.pos.x() > width() || p.pos.y() < 0 || p.pos.y() > height()) {
            p.active = false;
        }
    }
    m_projectiles.erase(std::remove_if(m_projectiles.begin(), m_projectiles.end(),
                        [](const Projectile& p) { return !p.active; }), m_projectiles.end());

    // فیزیک و آپدیت حباب‌ها
    applyTargetRepulsion(); // جلوگیری از در هم رفتن حباب‌ها
    
    for (int i = 0; i < m_targets.size(); ++i) {
        auto& t = m_targets[i];
        t.pos += t.velocity;
        
        t.velocity *= 0.98; // اصطکاک
        
        // حرکت شناور ملایم همیشگی
        if (t.velocity.manhattanLength() < 0.6) {
            t.velocity.setX(t.velocity.x() > 0 ? 0.6 : -0.6);
            t.velocity.setY(t.velocity.y() > 0 ? 0.6 : -0.6);
        }

        if (t.hitScale > 1.0) t.hitScale -= 0.04;

        for (auto& p : m_projectiles) {
            if (!p.active) continue;
            qreal dist = std::sqrt(std::pow(p.pos.x() - t.pos.x(), 2) + std::pow(p.pos.y() - t.pos.y(), 2));
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
        part.life -= 0.03; 
    }
    m_particles.erase(std::remove_if(m_particles.begin(), m_particles.end(),
                      [](const Particle& part) { return part.life <= 0; }), m_particles.end());

    // آپدیت موج
    if (m_empActive) {
        m_empRadius += 30.0; 
        if (m_empRadius > 2000) m_empActive = false;
    }

    update();
}

void SettingsWidget::applyTargetRepulsion() {
    // سیستم دافعه مغناطیسی بین حباب‌ها تا همیشه فاصله داشته باشند
    for (int i = 0; i < m_targets.size(); ++i) {
        for (int j = i + 1; j < m_targets.size(); ++j) {
            auto& t1 = m_targets[i];
            auto& t2 = m_targets[j];
            qreal dx = t2.pos.x() - t1.pos.x();
            qreal dy = t2.pos.y() - t1.pos.y();
            qreal dist = std::sqrt(dx*dx + dy*dy);
            qreal minDist = (t1.radius + t2.radius) * 1.2; // 20% فاصله امن

            if (dist < minDist && dist > 0.1) {
                qreal force = (minDist - dist) * 0.05;
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

void SettingsWidget::triggerHitEffect(int targetIndex, const QPointF& hitPos) {
    auto& t = m_targets[targetIndex];
    t.hitScale = 1.3; 
    spawnParticles(hitPos, t.color, 20);

    switch (t.type) {
    case TargetType::VolUp: 
        m_volume = qMin(100, m_volume + 10); 
        // اعمال ولوم واقعی
        SoundManager::instance().setMusicVolume(m_volume);
        SoundManager::instance().setSfxVolume(m_volume);
        SoundManager::instance().playPop();
        break;
            
    case TargetType::VolDown: 
        m_volume = qMax(0, m_volume - 10); 
        // اعمال ولوم واقعی
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
        m_currentTheme = "Cyber Neon"; 
        triggerEMP(t.pos); 
        SoundManager::instance().playPop();
        break;
            
    case TargetType::ThemeArcade: 
        m_currentTheme = "Retro Arcade"; 
        triggerEMP(t.pos); 
        SoundManager::instance().playPop();
        break;
            
    case TargetType::EMP: 
        triggerEMP(t.pos); 
        SoundManager::instance().playShoot(); // صدای متفاوت برای EMP
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
        qreal dist = std::sqrt(dx*dx + dy*dy);
        if (dist > 0.1) {
            t.velocity.setX(t.velocity.x() + (dx / dist) * 25.0); 
            t.velocity.setY(t.velocity.y() + (dy / dist) * 25.0);
        }
    }
}

void SettingsWidget::spawnParticles(const QPointF& pos, const QColor& color, int count) {
    for (int i = 0; i < count; ++i) {
        Particle part;
        part.pos = pos;
        qreal angle = (QRandomGenerator::global()->bounded(360)) * M_PI / 180.0;
        qreal speed = (QRandomGenerator::global()->bounded(40, 100)) / 10.0;
        part.velocity = QPointF(std::cos(angle) * speed, std::sin(angle) * speed);
        part.life = 1.0;
        part.color = color;
        m_particles.append(part);
    }
}

void SettingsWidget::mouseMoveEvent(QMouseEvent* event) {
    m_mousePos = event->position();
    qreal dx = m_mousePos.x() - (width() / 2.0);
    qreal dy = m_mousePos.y() - (height() - 40.0);
    m_cannonAngle = std::atan2(dy, dx);
}

void SettingsWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        Projectile p;
        p.pos = QPointF(width() / 2.0, height() - 40.0);
        qreal speed = 35.0; 
        p.velocity = QPointF(std::cos(m_cannonAngle) * speed, std::sin(m_cannonAngle) * speed);
        m_projectiles.append(p);
        spawnParticles(p.pos, QColor(0, 242, 254), 3); // جرقه لوله شلیک
    }
}

void SettingsWidget::resizeEvent(QResizeEvent* event) {
    if (m_firstShow && width() > 100) {
        scatterTargets(); // انفجار اولیه هنگام باز شدن صفحه!
        m_firstShow = false;
    }
    QWidget::resizeEvent(event);
}

void SettingsWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // ۱. پس‌زمینه
    QLinearGradient bgGrad(0, 0, 0, height());
    if (m_currentTheme == "Cyber Neon") {
        bgGrad.setColorAt(0.0, QColor(6, 9, 15)); bgGrad.setColorAt(1.0, QColor(2, 3, 5));
    } else {
        bgGrad.setColorAt(0.0, QColor(20, 5, 10)); bgGrad.setColorAt(1.0, QColor(5, 1, 2));
    }
    painter.fillRect(rect(), bgGrad);

    // ۲. موج انفجار
    if (m_empActive) {
        painter.setPen(QPen(QColor(0, 242, 254, qMax(0, 150 - int(m_empRadius / 15.0))), 6));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(QPointF(width()/2, height()/2), m_empRadius, m_empRadius);
    }

    // ۳. رابط هولوگرامی بالا (HUD)
    painter.setPen(QPen(QColor(255, 255, 255, 30), 1));
    painter.drawRect(20, 20, width() - 40, 60);
    
    painter.setPen(QColor(0, 242, 254, 200));
    painter.setFont(QFont("Arial", 14, QFont::Bold));
    QString statusText = QString("SYSTEM OVERRIDE | VOL: %1% | DISPLAY: %2 | THEME: %3")
                         .arg(m_volume)
                         .arg(m_fullscreen ? "FULLSCREEN" : "WINDOWED")
                         .arg(m_currentTheme.toUpper());
    painter.drawText(QRect(0, 20, width(), 60), Qt::AlignCenter, statusText);

    // ۴. رسم حباب‌ها (هولوگرامی شیشه‌ای)
    for (const auto& t : m_targets) {
        qreal currentRadius = t.radius * t.hitScale;
        
        // حلقه بیرونی درخشان
        painter.setPen(QPen(t.color, 3));
        painter.setBrush(QColor(t.color.red(), t.color.green(), t.color.blue(), 40));
        painter.drawEllipse(t.pos, currentRadius, currentRadius);
        
        // هسته شیشه‌ای داخلی
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255, 15));
        painter.drawEllipse(t.pos, currentRadius * 0.8, currentRadius * 0.8);

        // سایه متن برای خوانایی
        painter.setPen(QColor(0, 0, 0, 150));
        painter.setFont(QFont("Arial", 10, QFont::Black));
        painter.drawText(QRectF(t.pos.x() - currentRadius + 2, t.pos.y() - currentRadius + 2, currentRadius*2, currentRadius*2), Qt::AlignCenter, t.label);
        
        // متن اصلی
        painter.setPen(Qt::white);
        painter.drawText(QRectF(t.pos.x() - currentRadius, t.pos.y() - currentRadius, currentRadius*2, currentRadius*2), Qt::AlignCenter, t.label);
    }

    // ۵. خط نشانه‌گیر لیزری (Laser Sight)
    painter.setPen(QPen(QColor(0, 242, 254, 80), 2, Qt::DashLine));
    painter.drawLine(QPointF(width() / 2.0, height() - 40.0), m_mousePos);

    // ۶. رسم گلوله‌های لیزری (Motion Blur effect)
    painter.setPen(QPen(QColor(0, 242, 254), 4, Qt::SolidLine, Qt::RoundCap));
    for (const auto& p : m_projectiles) {
        painter.drawLine(p.pos, p.pos - (p.velocity * 1.5));
    }

    // ۷. رسم ذرات
    for (const auto& part : m_particles) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(part.color.red(), part.color.green(), part.color.blue(), int(part.life * 255)));
        painter.drawEllipse(part.pos, 4, 4);
    }

    // ۸. توپخانه سایبرپانک
    painter.save();
    painter.translate(width() / 2.0, height() - 40.0);
    painter.rotate(qRadiansToDegrees(m_cannonAngle));
    
    // لوله توپ
    painter.setBrush(QColor(15, 20, 30));
    painter.setPen(QPen(QColor(0, 242, 254), 3));
    painter.drawRect(0, -12, 50, 24);
    
    // بدنه چرخشی
    painter.setBrush(QColor(30, 40, 60));
    painter.drawEllipse(QPointF(0, 0), 25, 25);
    // هسته نئونی
    painter.setBrush(QColor(0, 242, 254));
    painter.drawEllipse(QPointF(0, 0), 10, 10);
    
    painter.restore();
}