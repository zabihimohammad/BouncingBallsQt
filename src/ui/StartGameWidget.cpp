#include "StartGameWidget.h"
#include "ThemeManager.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QRadialGradient>
#include <QLinearGradient>
#include <QRandomGenerator>
#include <cmath>
#include <algorithm>

// --- پیاده‌سازی متدهای ریاضی ۳ بعدی ---
Vector3D Vector3D::rotateX(qreal angle) const {
    qreal rad = angle * M_PI / 180.0;
    qreal c = std::cos(rad), s = std::sin(rad);
    return {x, y * c - z * s, y * s + z * c};
}
Vector3D Vector3D::rotateY(qreal angle) const {
    qreal rad = angle * M_PI / 180.0;
    qreal c = std::cos(rad), s = std::sin(rad);
    return {x * c + z * s, y, -x * s + z * c};
}
Vector3D Vector3D::rotateZ(qreal angle) const {
    qreal rad = angle * M_PI / 180.0;
    qreal c = std::cos(rad), s = std::sin(rad);
    return {x * c - y * s, x * s + y * c, z};
}
Vector3D Vector3D::normalized() const {
    qreal len = std::sqrt(x*x + y*y + z*z);
    if (len == 0) return {0,0,0};
    return {x/len, y/len, z/len};
}
Vector3D Vector3D::cross(const Vector3D& other) const {
    return {y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x};
}
qreal Vector3D::dot(const Vector3D& other) const {
    return x * other.x + y * other.y + z * other.z;
}

StartGameWidget::StartGameWidget(QWidget* parent) : QWidget(parent) {
    setMouseTracking(true);

    m_nameInput = new QLineEdit(this);
    m_nameInput->setPlaceholderText(">> ENTER OPERATIVE NAME <<");
    m_nameInput->setAlignment(Qt::AlignCenter);

    m_playBtn = new QPushButton("INITIATE LAUNCH SEQUENCE", this);
    m_playBtn->setEnabled(false);
    connect(m_playBtn, &QPushButton::clicked, this, &StartGameWidget::onPlayClicked);

    // دکمه بازگشت در پایین چپ
    m_backBtn = new QPushButton("◄ SYSTEM ABORT", this);
    connect(m_backBtn, &QPushButton::clicked, this, &StartGameWidget::backClicked);

    initEnvironment();

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &StartGameWidget::updateEngine);
    m_timer->start(16);
}

void StartGameWidget::initEnvironment() {
    auto rng = QRandomGenerator::global();
    
    // ۱. تولید ستارگان پس زمینه
    m_stars.clear();
    for (int i = 0; i < 300; ++i) {
        BgStar star;
        star.pos = QPointF(rng->bounded(4000) - 1000, rng->bounded(3000) - 500);
        star.size = 0.5 + rng->generateDouble() * 2.5;
        star.phase = rng->bounded(360) * M_PI / 180.0;
        star.speed = (rng->bounded(5) + 1) / 100.0;
        
        int r = rng->bounded(100);
        if (r < 15) star.color = QColor(0, 242, 254);
        else if (r < 30) star.color = QColor(255, 100, 200);
        else if (r < 40) star.color = QColor(160, 32, 240);
        else star.color = Qt::white;
        
        m_stars.append(star);
    }

    // ۲. تولید کره به صورت مش مثلثی (Geodesic-like Triangles)
    m_globeFaces.clear();
    int lats = 16;
    int lons = 32;
    
    auto getPoint = [&](int i, int j) -> Vector3D {
        qreal lat = M_PI * (-0.5 + (qreal)i / lats);
        qreal lon = 2 * M_PI * (qreal)j / lons;
        return {
            m_globeRadius * std::cos(lat) * std::cos(lon),
            m_globeRadius * std::sin(lat),
            m_globeRadius * std::cos(lat) * std::sin(lon)
        };
    };

    for (int i = 0; i < lats; ++i) {
        for (int j = 0; j < lons; ++j) {
            Vector3D p1 = getPoint(i, j);
            Vector3D p2 = getPoint(i, j + 1);
            Vector3D p3 = getPoint(i + 1, j);
            Vector3D p4 = getPoint(i + 1, j + 1);

            // محاسبه قاره بودن با نویز ساده سه بعدی
            auto isCont = [&](const Vector3D& center) -> bool {
                qreal noise = std::sin(center.x * 0.02) * std::cos(center.y * 0.02) + std::sin(center.z * 0.02);
                return noise > 0.4;
            };

            Vector3D c1 = {(p1.x+p2.x+p3.x)/3, (p1.y+p2.y+p3.y)/3, (p1.z+p2.z+p3.z)/3};
            Vector3D c2 = {(p2.x+p4.x+p3.x)/3, (p2.y+p4.y+p3.y)/3, (p2.z+p4.z+p3.z)/3};

            m_globeFaces.append({p1, p2, p3, c1, isCont(c1), 0.0});
            m_globeFaces.append({p2, p4, p3, c2, isCont(c2), 0.0});
        }
    }

    // ۳. پایگاه‌های عملیاتی
    m_sectors.clear();
    m_sectors.append({"CLASSIC", "SECTOR ALPHA [CLASSIC]", "Standard operational protocol.", QColor(0, 242, 254), {0, 0, m_globeRadius}, {0,0,0}, 1.0, false});
    m_sectors.append({"TIME_ATTACK", "SECTOR OMEGA [TIME ATTACK]", "Time-critical combat zone.", QColor(255, 60, 60), {m_globeRadius, 0, 0}, {0,0,0}, 1.0, false});
    m_sectors.append({"CHAOS", "SECTOR VOID [CHAOS]", "Unstable reality geometry.", QColor(160, 32, 240), {0, 0, -m_globeRadius}, {0,0,0}, 1.0, false});
    m_sectors.append({"ENDLESS", "SECTOR INFINITY [ENDLESS]", "Infinite depth survival.", QColor(0, 255, 128), {-m_globeRadius, 0, 0}, {0,0,0}, 1.0, false});
}

void StartGameWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    int w = width();
    int h = height();

    // UI در گوشه پایین چپ
    m_backBtn->setGeometry(20, h - 55, 160, 35);
    
    m_nameInput->setGeometry(w / 2 - 175, 50, 350, 45);
    m_playBtn->setGeometry(w / 2 - 200, h - 90, 400, 60);
}

void StartGameWidget::updateEngine() {
    m_time += 0.04;

    if (!m_isDragging && m_lockedSector == -1) {
        m_targetRotY += 0.2; // چرخش خودکار آرام
    }

    m_rotX += (m_targetRotX - m_rotX) * 0.1;
    m_rotY += (m_targetRotY - m_rotY) * 0.1;

    // آپدیت پیشرفت فرود
    if (m_lockedSector != -1) {
        if (m_dockingProgress < 1.0) m_dockingProgress += 0.025;
    } else {
        if (m_dockingProgress > 0.0) m_dockingProgress -= 0.04;
    }
    m_dockingProgress = std::clamp(m_dockingProgress, 0.0, 1.0);

    update3DTransforms();

    // بررسی Hover
    m_hoveredSector = -1;
    qreal cx = width() / 2.0;
    qreal cy = height() / 2.0;
    qreal perspective = 900.0;

    if (m_lockedSector == -1 && !m_isDragging) {
        for (int i = 0; i < m_sectors.size(); ++i) {
            auto& s = m_sectors[i];
            if (s.transformed.z > 0) {
                qreal scale = perspective / (perspective - s.transformed.z);
                QPointF screenPos(cx + s.transformed.x * scale, cy + s.transformed.y * scale);
                qreal dist = std::hypot(m_currentMousePos.x() - screenPos.x(), m_currentMousePos.y() - screenPos.y());
                if (dist < 45.0 * scale) {
                    m_hoveredSector = i;
                    break;
                }
            }
        }
    }

    for (int i = 0; i < m_sectors.size(); ++i) {
        qreal targetHover = (m_hoveredSector == i || m_lockedSector == i) ? 1.4 : 1.0;
        m_sectors[i].hoverScale += (targetHover - m_sectors[i].hoverScale) * 0.15;
    }

    // آپدیت ذرات (Particles)
    for (int i = m_particles.size() - 1; i >= 0; --i) {
        m_particles[i].pos.x += m_particles[i].velocity.x;
        m_particles[i].pos.y += m_particles[i].velocity.y;
        m_particles[i].pos.z += m_particles[i].velocity.z;
        m_particles[i].life -= 0.015;
        if (m_particles[i].life <= 0) {
            m_particles.removeAt(i);
        }
    }

    update();
}

void StartGameWidget::update3DTransforms() {
    for (auto& s : m_sectors) {
        s.transformed = s.localPos.rotateX(m_rotX).rotateY(m_rotY);
    }
}

void StartGameWidget::emitThrusterSparks(const Vector3D& pos, const Vector3D& dir) {
    auto rng = QRandomGenerator::global();
    for (int i = 0; i < 4; ++i) {
        Particle3D p;
        p.pos = pos;
        
        // پخش شدن ذرات بر خلاف جهت حرکت (dir) به اضافه کمی تصادف
        Vector3D randomSpread = {
            (rng->bounded(60)-30)/10.0,
            (rng->bounded(60)-30)/10.0,
            (rng->bounded(60)-30)/10.0
        };
        p.velocity = { -dir.x * 5.0 + randomSpread.x, -dir.y * 5.0 + randomSpread.y, -dir.z * 5.0 + randomSpread.z };
        
        p.maxLife = (rng->bounded(100) + 50) / 100.0;
        p.life = p.maxLife;
        
        int r = rng->bounded(100);
        if (r < 20) p.color = QColor(255, 255, 255);
        else if (r < 50) p.color = QColor(0, 242, 254);
        else if (r < 80) p.color = QColor(255, 100, 50); // جرقه‌های آتشین
        else p.color = QColor(255, 200, 50);
        
        m_particles.append(p);
    }
}

void StartGameWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawCosmicNebula(painter);
    drawAtmosphere(painter);
    
    drawGeodesicGlobe(painter);
    drawSectorsAndBeacons(painter);
    
    if (m_dockingProgress > 0.0) {
        drawAdvancedShip(painter);
    }
    
    drawParticles(painter);

    drawScanlines(painter);
    drawCyberpunkHUD(painter);
    drawFloatingUI(painter);
}

void StartGameWidget::drawCosmicNebula(QPainter& painter) {
    int baseHue = ThemeManager::instance().getBaseHue();
    QColor bgTop = QColor::fromHsv(baseHue, 220, 16);
    QColor bgBottom = QColor::fromHsv(baseHue, 240, 6);
    
    QLinearGradient bgGrad(0, 0, 0, height());
    bgGrad.setColorAt(0.0, bgTop);
    bgGrad.setColorAt(1.0, bgBottom);
    painter.fillRect(rect(), bgGrad);

    // سحابی با رنگ‌های تم
    QRadialGradient neb1(width()*0.8, height()*0.2, 900);
    QColor nC1 = ThemeManager::instance().getSecondaryColor();
    nC1.setAlpha(45);
    neb1.setColorAt(0, nC1);
    neb1.setColorAt(1, Qt::transparent);
    painter.fillRect(rect(), neb1);
    
    QRadialGradient neb2(width()*0.2, height()*0.8, 1000);
    QColor nC2 = ThemeManager::instance().getPrimaryColor();
    nC2.setAlpha(35);
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
        c.setAlpha(int(currentBright * 220 + 35));
        painter.setPen(Qt::NoPen);
        painter.setBrush(c);
        painter.drawEllipse(star.pos, star.size, star.size);
    }
}

void StartGameWidget::drawAtmosphere(QPainter& painter) {
    qreal cx = width() / 2.0;
    qreal cy = height() / 2.0;
    
    QColor atmosColor = ThemeManager::instance().getPrimaryColor();
    if (m_lockedSector != -1) {
        atmosColor = m_sectors[m_lockedSector].color;
    }
    
    QRadialGradient rim(cx, cy, m_globeRadius * 1.3);
    rim.setColorAt(0.65, QColor(atmosColor.red(), atmosColor.green(), atmosColor.blue(), 10));
    rim.setColorAt(0.77, QColor(atmosColor.red(), atmosColor.green(), atmosColor.blue(), 150));
    rim.setColorAt(0.81, QColor(atmosColor.red(), atmosColor.green(), atmosColor.blue(), 40));
    rim.setColorAt(1.0, Qt::transparent);
    
    painter.setBrush(rim);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(cx, cy), m_globeRadius * 1.3, m_globeRadius * 1.3);
    
    // Shadow core
    QRadialGradient coreShadow(cx, cy, m_globeRadius);
    coreShadow.setColorAt(0.0, QColor(5, 10, 20, 220));
    coreShadow.setColorAt(0.7, QColor(0, 0, 0, 200));
    coreShadow.setColorAt(1.0, Qt::transparent);
    painter.setBrush(coreShadow);
    painter.drawEllipse(QPointF(cx, cy), m_globeRadius, m_globeRadius);
}

void StartGameWidget::drawGeodesicGlobe(QPainter& painter) {
    qreal cx = width() / 2.0;
    qreal cy = height() / 2.0;
    qreal perspective = 900.0;

    // موج رادار که روی محور Y میچرخد
    qreal pulsePhase = std::sin(m_time * 2.0) * m_globeRadius;

    // کپی چهره‌ها و اعمال ترانسفورم‌ها برای سورت کردن Z
    struct TransformedFace {
        Vector3D p1, p2, p3, c;
        bool isContinent;
    };
    QVector<TransformedFace> faces;
    faces.reserve(m_globeFaces.size());

    for (const auto& face : m_globeFaces) {
        faces.append({
            face.p1.rotateX(m_rotX).rotateY(m_rotY),
            face.p2.rotateX(m_rotX).rotateY(m_rotY),
            face.p3.rotateX(m_rotX).rotateY(m_rotY),
            face.center.rotateX(m_rotX).rotateY(m_rotY),
            face.isContinent
        });
    }

    // مرتب‌سازی برای نقاشی از عقب به جلو (Backface Culling جزئی)
    std::sort(faces.begin(), faces.end(), [](const TransformedFace& a, const TransformedFace& b) {
        return a.c.z < b.c.z;
    });

    QColor priColor = ThemeManager::instance().getPrimaryColor();

    for (const auto& f : faces) {
        // Culling بسیار دور یا نقاط نامعتبر
        if (f.c.z < -m_globeRadius * 0.9) continue;
        
        qreal s1 = perspective / (perspective - f.p1.z);
        qreal s2 = perspective / (perspective - f.p2.z);
        qreal s3 = perspective / (perspective - f.p3.z);

        QPointF sp1(cx + f.p1.x * s1, cy + f.p1.y * s1);
        QPointF sp2(cx + f.p2.x * s2, cy + f.p2.y * s2);
        QPointF sp3(cx + f.p3.x * s3, cy + f.p3.y * s3);

        QPolygonF poly; poly << sp1 << sp2 << sp3;

        bool isFront = f.c.z > 0;
        
        // محاسبه آلفا برای خطوط
        int lineAlpha = isFront ? int(30 + (f.c.z / m_globeRadius) * 100) : int(5);
        
        // پالس راداری (برجسته کردن نقاطی که در ارتفاع پالس هستند)
        if (isFront && std::abs(f.c.y - pulsePhase) < 40.0) {
            lineAlpha += 120;
        }
        lineAlpha = std::clamp(lineAlpha, 5, 255);

        painter.setPen(QPen(QColor(priColor.red(), priColor.green(), priColor.blue(), lineAlpha), isFront ? 1.0 : 0.5));
        
        // اگر قاره است، داخلش را با افکت نئونی رنگ کن
        if (f.isContinent) {
            int fillAlpha = isFront ? int(15 + (f.c.z / m_globeRadius) * 45) : 0;
            if (isFront && std::abs(f.c.y - pulsePhase) < 30.0) fillAlpha += 80;
            fillAlpha = std::clamp(fillAlpha, 0, 200);
            
            painter.setBrush(QColor(priColor.red(), priColor.green(), priColor.blue(), fillAlpha));
        } else {
            painter.setBrush(Qt::NoBrush);
        }
        
        painter.drawPolygon(poly);
        
        // رسم Node در مرکز بعضی قاره‌ها
        if (f.isContinent && isFront && ((int)f.p1.x % 3 == 0)) {
            qreal sc = perspective / (perspective - f.c.z);
            QPointF scp(cx + f.c.x * sc, cy + f.c.y * sc);
            int nodeAlpha = (std::abs(f.c.y - pulsePhase) < 40.0) ? 255 : 100;
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(255, 255, 255, nodeAlpha));
            painter.drawEllipse(scp, 2.0*sc, 2.0*sc);
        }
    }
}

void StartGameWidget::drawSectorsAndBeacons(QPainter& painter) {
    qreal cx = width() / 2.0;
    qreal cy = height() / 2.0;
    qreal perspective = 900.0;

    QVector<int> order = {0, 1, 2, 3};
    std::sort(order.begin(), order.end(), [this](int a, int b) {
        return m_sectors[a].transformed.z < m_sectors[b].transformed.z;
    });

    for (int idx : order) {
        const auto& s = m_sectors[idx];
        if (s.transformed.z < -m_globeRadius * 0.5 && m_dockingProgress == 0.0) continue;
        
        qreal scale = perspective / (perspective - s.transformed.z);
        QPointF pos(cx + s.transformed.x * scale, cy + s.transformed.y * scale);
        
        bool isFront = (s.transformed.z > 0);
        qreal r = 25.0 * scale * s.hoverScale;
        
        // 1. Holo-Beacons (ستون‌های نوری عمودی)
        if (isFront && m_lockedSector != idx) {
            Vector3D normal = s.transformed.normalized();
            Vector3D endP = { s.transformed.x + normal.x * 200.0, 
                              s.transformed.y + normal.y * 200.0, 
                              s.transformed.z + normal.z * 200.0 };
            
            qreal endScale = perspective / (perspective - endP.z);
            QPointF endPos(cx + endP.x * endScale, cy + endP.y * endScale);
            
            QLinearGradient beaconGrad(pos, endPos);
            beaconGrad.setColorAt(0, QColor(s.color.red(), s.color.green(), s.color.blue(), 220));
            beaconGrad.setColorAt(1, Qt::transparent);
            painter.setPen(QPen(QBrush(beaconGrad), 4.0 * scale));
            painter.drawLine(pos, endPos);
            
            // حلقه‌های دیتای معلق روی ستون نور
            qreal dataRingPos = std::fmod(m_time * 50.0, 200.0);
            Vector3D ring3D = { s.transformed.x + normal.x * dataRingPos,
                                s.transformed.y + normal.y * dataRingPos,
                                s.transformed.z + normal.z * dataRingPos };
            qreal ringScale = perspective / (perspective - ring3D.z);
            QPointF ringScreen(cx + ring3D.x * ringScale, cy + ring3D.y * ringScale);
            painter.setPen(QPen(QColor(255, 255, 255, 150), 1.5 * ringScale));
            painter.drawEllipse(ringScreen, 15.0 * ringScale, 5.0 * ringScale); // زاویه پرسپکتیو فیک
        }

        // 2. Base Glow
        QRadialGradient glow(pos, r * 2.8);
        glow.setColorAt(0, QColor(s.color.red(), s.color.green(), s.color.blue(), isFront ? (idx == m_lockedSector ? 255 : 180) : 40));
        glow.setColorAt(1, Qt::transparent);
        painter.setBrush(glow);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(pos, r * 2.8, r * 2.8);

        // 3. Core
        painter.setBrush(QColor(255, 255, 255, isFront ? 255 : 80));
        painter.setPen(QPen(s.color, 3.0));
        painter.drawEllipse(pos, r*0.4, r*0.4);

        // 4. Target Lock HUD (حلقه‌های هولوگرافیک)
        if (idx == m_lockedSector) {
            painter.setBrush(Qt::NoBrush);
            painter.setPen(QPen(s.color, 2.5));
            // دایره داخلی
            painter.drawArc(QRectF(pos.x()-r*1.3, pos.y()-r*1.3, r*2.6, r*2.6), (int)(m_time * 300) % 5760, 1000);
            painter.drawArc(QRectF(pos.x()-r*1.3, pos.y()-r*1.3, r*2.6, r*2.6), (int)(m_time * 300 + 2880) % 5760, 1000);
            
            // دایره خارجی با خط چین
            QPen dashPen(Qt::white, 1.5);
            dashPen.setDashPattern({4, 4});
            painter.setPen(dashPen);
            painter.drawEllipse(pos, r*1.8, r*1.8);
            
            // براکت‌های قفل روی هدف
            painter.setPen(QPen(s.color, 3.0));
            qreal br = r * 2.2;
            qreal o = 10 * scale;
            painter.drawLine(pos + QPointF(-br, -br), pos + QPointF(-br+o, -br));
            painter.drawLine(pos + QPointF(-br, -br), pos + QPointF(-br, -br+o));
            
            painter.drawLine(pos + QPointF(br, -br), pos + QPointF(br-o, -br));
            painter.drawLine(pos + QPointF(br, -br), pos + QPointF(br, -br+o));
            
            painter.drawLine(pos + QPointF(-br, br), pos + QPointF(-br+o, br));
            painter.drawLine(pos + QPointF(-br, br), pos + QPointF(-br, br-o));
            
            painter.drawLine(pos + QPointF(br, br), pos + QPointF(br-o, br));
            painter.drawLine(pos + QPointF(br, br), pos + QPointF(br, br-o));
        }
    }
}

void StartGameWidget::drawAdvancedShip(QPainter& painter) {
    if (m_lockedSector == -1) return;
    
    qreal cx = width() / 2.0;
    qreal cy = height() / 2.0;
    qreal perspective = 900.0;
    
    const auto& targetSector = m_sectors[m_lockedSector];
    
    // مسیر فرود سفینه (از دوربین به سمت قطب، سپس به سمت ایستگاه)
    Vector3D shipPos;
    // انیمیشن نرم‌تر با EaseOut
    qreal ease = 1.0 - std::pow(1.0 - m_dockingProgress, 3.0);
    
    // سفینه از ارتفاع بالا در راستای نرمال پایگاه وارد می‌شود
    Vector3D normal = targetSector.transformed.normalized();
    qreal dist = 800.0 * (1.0 - ease); // از 800 واحدی نزدیک می‌شود
    
    shipPos = {
        targetSector.transformed.x + normal.x * dist,
        targetSector.transformed.y + normal.y * dist,
        targetSector.transformed.z + normal.z * dist
    };
    
    // تولید ذرات پیشران در خلاف جهت حرکت سفینه (جهت نورمال)
    if (m_dockingProgress > 0.05 && m_dockingProgress < 0.95) {
        emitThrusterSparks(shipPos, normal);
    }
    
    qreal scale = perspective / (perspective - shipPos.z);
    QPointF pos(cx + shipPos.x * scale, cy + shipPos.y * scale);
    qreal size = 30.0 * scale;
    
    painter.save();
    painter.translate(pos);
    
    // محاسبه زاویه سه بعدی چرخش سفینه. به سمت مرکز سیاره نگاه می‌کند.
    qreal angle = std::atan2(shipPos.y, shipPos.x) * 180 / M_PI;
    painter.rotate(angle - 90); 

    // رسم سفینه ۳ بعدی و پرجزئیات (Sci-Fi Fighter)
    QPolygonF wings, body, cockpit;
    
    wings << QPointF(0, size*0.5) 
          << QPointF(-size*1.2, -size*0.8) 
          << QPointF(0, -size*0.2) 
          << QPointF(size*1.2, -size*0.8);
          
    body << QPointF(0, size*1.5) // نوک بلند
         << QPointF(-size*0.4, -size*1.0) 
         << QPointF(size*0.4, -size*1.0);
         
    cockpit << QPointF(0, size*0.8)
            << QPointF(-size*0.25, size*0.2)
            << QPointF(size*0.25, size*0.2);

    painter.setBrush(QColor(20, 25, 40, 255));
    painter.setPen(QPen(QColor(100, 150, 200), 1.5 * scale));
    painter.drawPolygon(wings);
    
    painter.setBrush(QColor(30, 40, 60, 255));
    painter.setPen(QPen(Qt::white, 2.0 * scale));
    painter.drawPolygon(body);
    
    painter.setBrush(QColor(0, 242, 254, 200));
    painter.setPen(Qt::NoPen);
    painter.drawPolygon(cockpit);
    
    // نور موتورهای اصلی
    QRadialGradient engGlow(0, -size*1.0, size*0.8);
    engGlow.setColorAt(0, QColor(255, 255, 255, 255));
    engGlow.setColorAt(0.3, QColor(0, 242, 254, 200));
    engGlow.setColorAt(1, Qt::transparent);
    painter.setBrush(engGlow);
    painter.drawEllipse(QPointF(0, -size*1.0), size*1.5, size*1.5);

    painter.restore();
    
    // موج انفجار زمان فرود
    if (m_dockingProgress > 0.95) {
        qreal waveProgress = (m_dockingProgress - 0.95) * 20.0;
        qreal waveRadius = waveProgress * 300.0 * scale;
        
        QPen wavePen(targetSector.color, 6.0 * (1.0 - waveProgress));
        painter.setBrush(Qt::NoBrush);
        painter.setPen(wavePen);
        
        qreal tzScale = perspective / (perspective - targetSector.transformed.z);
        QPointF tPos(cx + targetSector.transformed.x * tzScale, cy + targetSector.transformed.y * tzScale);
        
        painter.drawEllipse(tPos, waveRadius, waveRadius * 0.4); 
    }
}

void StartGameWidget::drawParticles(QPainter& painter) {
    qreal cx = width() / 2.0;
    qreal cy = height() / 2.0;
    qreal perspective = 900.0;
    
    painter.setPen(Qt::NoPen);
    for (const auto& p : m_particles) {
        if (p.pos.z < -m_globeRadius) continue;
        
        qreal scale = perspective / (perspective - p.pos.z);
        QPointF sp(cx + p.pos.x * scale, cy + p.pos.y * scale);
        
        qreal lifeRatio = p.life / p.maxLife;
        qreal size = 4.0 * scale * lifeRatio;
        
        QColor c = p.color;
        c.setAlpha(int(255 * lifeRatio));
        
        painter.setBrush(c);
        painter.drawEllipse(sp, size, size);
    }
}

void StartGameWidget::drawCyberpunkHUD(QPainter& painter) {
    // فونت دیجیتالی مینیمال
    QFont font("Consolas", 11, QFont::Bold);
    font.setLetterSpacing(QFont::AbsoluteSpacing, 1.5);
    painter.setFont(font);
    
    QColor priCol = ThemeManager::instance().getPrimaryColor();

    // اطلاعات تلماتری (گوشه بالا چپ)
    painter.setPen(priCol);
    painter.drawText(30, 40, "[ TELEMETRY // ORBITAL LINK ]");
    
    painter.setPen(Qt::white);
    painter.drawText(30, 65, QString("LATITUDE:  %1°").arg(m_rotX, 7, 'f', 2, QChar('0')));
    painter.drawText(30, 85, QString("LONGITUDE: %1°").arg(std::fmod(m_rotY, 360.0), 7, 'f', 2, QChar('0')));
    painter.drawText(30, 105, QString("SYNC RATE: %1 THz").arg(14.32 + std::sin(m_time)*0.1, 5, 'f', 2));
    
    // خطوط گرافیکی دور HUD
    QColor lineCol = priCol;
    lineCol.setAlpha(100);
    painter.setPen(QPen(lineCol, 2.0));
    painter.drawLine(20, 25, 20, 120);
    painter.drawLine(20, 25, 50, 25);
    painter.drawLine(20, 120, 50, 120);

    // وضعیت سیستم (گوشه بالا راست)
    QString sysStatus = (m_lockedSector == -1) ? "SCANNING TARGETS" : "DOCKING IN PROGRESS";
    if (m_dockingProgress == 1.0) sysStatus = "SYSTEM LOCKED & READY";
    
    int tx = width() - 280;
    painter.setPen(priCol);
    painter.drawText(tx, 40, "[ MISSION STATUS ]");
    
    painter.setPen((m_lockedSector != -1) ? QColor(255, 60, 60) : Qt::white);
    painter.drawText(tx, 65, sysStatus);
    
    painter.setPen(QPen(lineCol, 2.0));
    painter.drawLine(width() - 20, 25, width() - 20, 120);
    painter.drawLine(width() - 20, 25, width() - 50, 25);
    painter.drawLine(width() - 20, 120, width() - 50, 120);
}

void StartGameWidget::drawScanlines(QPainter& painter) {
    // خطوط اسکن‌لاین ضخیم‌تر و بهتر
    painter.setPen(QPen(QColor(0, 0, 0, 30), 1.0));
    for (int y = 0; y < height(); y += 3) {
        painter.drawLine(0, y, width(), y);
    }
    
    // افکت Vignette (تاریکی گوشه‌ها)
    QRadialGradient vig(width()/2.0, height()/2.0, std::hypot(width()/2.0, height()/2.0));
    vig.setColorAt(0.5, Qt::transparent);
    vig.setColorAt(1.0, QColor(0, 0, 0, 180));
    painter.fillRect(rect(), vig);
}

void StartGameWidget::drawFloatingUI(QPainter& painter) {
    qreal cx = width() / 2.0;
    qreal cy = height() / 2.0;

    QColor priCol = ThemeManager::instance().getPrimaryColor();

    // خط اتصال از کره به Name Input
    QColor dashCol = priCol;
    dashCol.setAlpha(150);
    painter.setPen(QPen(dashCol, 1.5, Qt::DashLine));
    painter.drawLine(QPointF(cx, cy - m_globeRadius), QPointF(cx, 95));
    painter.setBrush(priCol);
    painter.drawEllipse(QPointF(cx, cy - m_globeRadius), 4, 4);
    painter.drawEllipse(QPointF(cx, 95), 4, 4);

    // پنل هولوگرافیک اطلاعات ایستگاه
    if (m_lockedSector != -1 && m_dockingProgress > 0.5) {
        const auto& s = m_sectors[m_lockedSector];
        qreal alphaAnim = (m_dockingProgress - 0.5) * 2.0; // 0 to 1
        
        QRectF panelRect(cx - 220, height() - 260, 440, 150);
        
        QLinearGradient pGrad(panelRect.topLeft(), panelRect.bottomRight());
        pGrad.setColorAt(0, QColor(10, 15, 30, int(220 * alphaAnim)));
        pGrad.setColorAt(1, QColor(0, 5, 10, int(180 * alphaAnim)));
        
        painter.setBrush(pGrad);
        QColor borderColor = s.color;
        borderColor.setAlpha(int(255 * alphaAnim));
        painter.setPen(QPen(borderColor, 2));
        painter.drawRect(panelRect);
        
        // تک‌آرت گوشه‌ها
        painter.setPen(QPen(QColor(255,255,255,int(255*alphaAnim)), 3));
        painter.drawLine(panelRect.topLeft(), panelRect.topLeft() + QPointF(20, 0));
        painter.drawLine(panelRect.topLeft(), panelRect.topLeft() + QPointF(0, 20));
        painter.drawLine(panelRect.bottomRight(), panelRect.bottomRight() - QPointF(20, 0));
        painter.drawLine(panelRect.bottomRight(), panelRect.bottomRight() - QPointF(0, 20));
        
        painter.setPen(borderColor);
        painter.setFont(QFont("Segoe UI", 18, QFont::Black));
        painter.drawText(panelRect.adjusted(20, 20, -20, -20), Qt::AlignTop | Qt::AlignLeft, s.name);
        
        painter.setPen(QColor(255,255,255,int(255*alphaAnim)));
        painter.setFont(QFont("Segoe UI", 12));
        painter.drawText(panelRect.adjusted(20, 55, -20, -20), Qt::AlignTop | Qt::AlignLeft, s.description);
        
        QColor alertColor = QColor(255, 60, 60, int(255 * alphaAnim * (std::sin(m_time*8.0)*0.5+0.5)));
        painter.setPen(alertColor);
        painter.setFont(QFont("Consolas", 11, QFont::Bold));
        painter.drawText(panelRect.adjusted(20, 110, -20, -20), Qt::AlignTop | Qt::AlignLeft, ">> AUTHORIZATION GRANTED. READY FOR LAUNCH.");
        
        // خط اتصال از پنل به دکمه پرتاب
        painter.setPen(QPen(borderColor, 1.5, Qt::DashLine));
        painter.drawLine(panelRect.bottomLeft() + QPointF(40, 0), QPointF(cx - 150, height() - 90));
        painter.drawLine(panelRect.bottomRight() - QPointF(40, 0), QPointF(cx + 150, height() - 90));
        
        // افکت حلقه‌های خطر (Hazard Rings) زیر دکمه پرتاب
        if (m_dockingProgress > 0.9) {
            painter.setBrush(Qt::NoBrush);
            painter.setPen(QPen(QColor(239, 68, 68, int(100 * (std::sin(m_time*5.0)+1)/2)), 3.0));
            QRectF btnRect(cx - 200, height() - 90, 400, 60);
            painter.drawRoundedRect(btnRect.adjusted(-10, -10, 10, 10), 20, 20);
            painter.setPen(QPen(QColor(239, 68, 68, 50), 1.0));
            painter.drawRoundedRect(btnRect.adjusted(-20, -20, 20, 20), 25, 25);
        }
    }
    
    // راهنمای استفاده
    if (m_lockedSector == -1) {
        painter.setPen(QColor(255, 255, 255, int(150 + 100 * std::sin(m_time * 3.0))));
        painter.setFont(QFont("Consolas", 12, QFont::Bold));
        painter.drawText(QRectF(0, height() - 80, width(), 30), Qt::AlignCenter, "[ DRAG GLOBE TO ROTATE. CLICK SECTOR TO DOCK ]");
    }
}

void StartGameWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_lastMousePos = event->position();
        
        if (m_hoveredSector != -1) {
            if (m_lockedSector == m_hoveredSector) {
                m_lockedSector = -1;
                m_playBtn->setEnabled(false);
            } else {
                m_lockedSector = m_hoveredSector;
                m_playBtn->setEnabled(true);
                
                m_targetRotX = 0;
                if (m_lockedSector == 0) m_targetRotY = 0.0;
                if (m_lockedSector == 1) m_targetRotY = -90.0;
                if (m_lockedSector == 2) m_targetRotY = 180.0;
                if (m_lockedSector == 3) m_targetRotY = 90.0;
            }
        } else {
            m_lockedSector = -1;
            m_playBtn->setEnabled(false);
        }
    }
}

void StartGameWidget::mouseMoveEvent(QMouseEvent* event) {
    m_currentMousePos = event->position();
    
    if (m_isDragging && m_lockedSector == -1) {
        QPointF delta = m_currentMousePos - m_lastMousePos;
        m_targetRotX -= delta.y() * 0.5;
        m_targetRotY += delta.x() * 0.5;
        
        m_targetRotX = std::clamp(m_targetRotX, -70.0, 70.0);
        
        m_lastMousePos = m_currentMousePos;
    }
    
    if (m_hoveredSector != -1 || m_lockedSector != -1) {
        setCursor(Qt::CrossCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }
}

void StartGameWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
    }
}

void StartGameWidget::wheelEvent(QWheelEvent* event) {
    if (event->angleDelta().y() > 0) {
        m_globeRadius += 10;
    } else {
        m_globeRadius -= 10;
    }
    m_globeRadius = std::clamp(m_globeRadius, 150.0, 450.0);
    initEnvironment();
}

void StartGameWidget::onPlayClicked() {
    QString user = m_nameInput->text().trimmed();
    if (user.isEmpty()) user = "OPERATIVE_X";
    
    if (m_lockedSector != -1) {
        QString selectedMode = m_sectors[m_lockedSector].id;
        emit launchGame(user, selectedMode);
    }
}
