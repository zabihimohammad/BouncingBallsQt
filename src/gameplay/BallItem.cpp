#include "../ui/ThemeManager.h"
#include "BallItem.h"
#include <QPainterPath>
#include <QTime>
#include <QRandomGenerator>
#include <cmath>

BallItem::BallItem(BallColor color, BallType type, BallColor secColor, bool locked, qreal radius, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_color(color), m_type(type), m_secColor(secColor), m_isLocked(locked), m_radius(radius) {
}

BallItem::BallItem(const Ball* ball, qreal radius, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_radius(radius) {
    if (ball) {
        m_color = ball->getPrimaryColor();
        m_type = ball->getType();
        m_secColor = ball->getSecondaryColor();
        m_isLocked = ball->isLocked();
    } else {
        m_color = BallColor::None;
        m_type = BallType::Regular;
        m_secColor = BallColor::None;
        m_isLocked = false;
    }
}

QRectF BallItem::boundingRect() const {
    // Increased bounding rect for outer glowing coronas and lightning sparks
    return QRectF(-m_radius - 8, -m_radius - 8, (m_radius + 8) * 2, (m_radius + 8) * 2);
}

void BallItem::updateData(BallColor color, BallType type, BallColor secColor, bool locked) {
    m_color = color;
    m_type = type;
    m_secColor = secColor;
    m_isLocked = locked;
    update();
}

void BallItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    paintBall(painter, QPointF(0, 0), m_radius, m_color, m_type, m_secColor, m_isLocked);
}

void BallItem::paintBall(QPainter* painter, const QPointF& center, qreal radius, BallColor color, BallType type, BallColor secColor, bool locked) {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->translate(center);

    qint64 ms = QTime::currentTime().msecsSinceStartOfDay();
    qreal t = (ms % 600000) / 1000.0;
    qreal breath = (std::sin(t * 4.0) + 1.0) * 0.5;

    // =========================================================================
    // ⚡ 1. HYPER-CHARGED PHOTON LASER BALL (Color-Independent Radiant Super-Orb)
    // =========================================================================
    if (type == BallType::Laser) {
        // Outer Volumetric Glowing Halo / Corona (Layer 1)
        QRadialGradient outerGlow(0, 0, radius * 1.45);
        outerGlow.setColorAt(0.0, QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), int(180 + breath * 75)));
        outerGlow.setColorAt(0.5, QColor(ThemeManager::instance().getSecondaryColor().red(), ThemeManager::instance().getSecondaryColor().green(), ThemeManager::instance().getSecondaryColor().blue(), int(80 + breath * 40)));
        outerGlow.setColorAt(1.0, QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), 0));
        painter->setBrush(outerGlow);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0,0), radius * 1.45, radius * 1.45);

        // Photonic Core Sphere
        QRadialGradient laserCore(0, 0, radius);
        laserCore.setColorAt(0.0, QColor(255, 255, 255));
        laserCore.setColorAt(0.3, QColor(180, 255, 255));
        laserCore.setColorAt(0.7, ThemeManager::instance().getPrimaryColor());
        laserCore.setColorAt(1.0, ThemeManager::instance().getSecondaryColor());
        painter->setBrush(laserCore);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0,0), radius, radius);

        // Dual Chromium Heavy Electromagnetic Emitter Caps
        painter->setPen(QPen(QColor(15, 23, 42), 1.5));
        painter->setBrush(QColor(226, 232, 240));
        painter->drawRoundedRect(QRectF(-radius - 2, -6, 5, 12), 2, 2);
        painter->drawRoundedRect(QRectF(radius - 3, -6, 5, 12), 2, 2);

        // Gyroscopic Fast-Rotating Focus Rings
        painter->save();
        painter->setBrush(Qt::NoBrush);
        painter->rotate(t * 120.0);
        painter->setPen(QPen(QColor(255, 255, 255, 200), 1.5, Qt::DashLine));
        painter->drawEllipse(QPointF(0,0), radius * 0.75, radius * 0.35);
        painter->rotate(t * -240.0);
        painter->setPen(QPen(QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), 220), 1.5));
        painter->drawEllipse(QPointF(0,0), radius * 0.35, radius * 0.75);
        painter->restore();

        // Piercing Hyper-Laser Horizontal Ray with vibrating plasma core
        qreal vib = std::sin(t * 40.0) * 1.2;
        painter->setPen(QPen(QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), 200), 8.0, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(QPointF(-radius + 2, vib), QPointF(radius - 2, vib));
        painter->setPen(QPen(QColor(255, 255, 255, 255), 3.0, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(QPointF(-radius + 2, vib), QPointF(radius - 2, vib));

        // Random Electric Plasma Sparks
        painter->setPen(QPen(QColor(255, 255, 255, 220), 1.2));
        for (int i = 0; i < 3; ++i) {
            qreal a = (i * 2.09) + t * 5.0;
            qreal sx = std::cos(a) * (radius * 0.7);
            qreal sy = std::sin(a) * (radius * 0.7);
            painter->drawLine(QPointF(sx, sy), QPointF(sx + (std::sin(t * 20.0 + i) * 5), sy + (std::cos(t * 20.0 + i) * 5)));
        }

        // Specular Glint
        QRadialGradient glint(-radius * 0.35, -radius * 0.35, radius * 0.5);
        glint.setColorAt(0.0, QColor(255, 255, 255, 240));
        glint.setColorAt(0.4, QColor(255, 255, 255, 80));
        glint.setColorAt(1.0, QColor(255, 255, 255, 0));
        painter->setBrush(glint);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(-radius * 0.35, -radius * 0.35), radius * 0.45, radius * 0.35);

        painter->restore();
        return;
    }

    // =========================================================================
    // 💣 2. APOCALYPTIC THERMO-NUCLEAR BOMB (Color-Independent Molten Core)
    // =========================================================================
    if (type == BallType::Bomb) {
        // Dark Obsidian / Tungsten Hull
        QRadialGradient bombHull(0, 0, radius);
        bombHull.setColorAt(0.0, QColor(60, 20, 20));
        bombHull.setColorAt(0.6, QColor(30, 20, 25));
        bombHull.setColorAt(1.0, QColor(10, 10, 15));
        painter->setBrush(bombHull);
        painter->setPen(QPen(QColor(245, 158, 11), 1.2));
        painter->drawEllipse(QPointF(0,0), radius, radius);

        // Molten Fissures / Glowing Cracks
        painter->setPen(QPen(QColor(255, 140, 0, int(180 + breath * 75)), 1.5));
        painter->drawLine(QPointF(-radius * 0.6, -radius * 0.3), QPointF(0, 0));
        painter->drawLine(QPointF(radius * 0.6, -radius * 0.4), QPointF(0, 0));
        painter->drawLine(QPointF(-radius * 0.3, radius * 0.6), QPointF(0, 0));
        painter->drawLine(QPointF(radius * 0.5, radius * 0.5), QPointF(0, 0));

        // Industrial Yellow/Black Caution Hazard Equator Belt
        painter->setPen(QPen(QColor(245, 158, 11), 3.0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(QPointF(0,0), radius * 0.8, radius * 0.8);

        // Pulsing Molten Thermo-Nuclear Core
        qreal coreR = radius * (0.38 + breath * 0.15);
        QRadialGradient fireCore(0, 0, coreR);
        fireCore.setColorAt(0.0, QColor(255, 255, 255));
        fireCore.setColorAt(0.25, QColor(255, 220, 50));
        fireCore.setColorAt(0.6, QColor(239, 68, 68));
        fireCore.setColorAt(1.0, QColor(185, 28, 28));
        painter->setBrush(fireCore);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0,0), coreR, coreR);

        // Radiating Warning Shockwaves (( ☣ ))
        qreal waveR = std::fmod(t * 28.0, radius * 1.3);
        int waveA = int((1.0 - (waveR / (radius * 1.3))) * 220);
        painter->setPen(QPen(QColor(239, 68, 68, waveA), 1.8));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(QPointF(0,0), waveR, waveR);

        // Blinking Red Digital Beacon LED
        if (std::fmod(t * 5.0, 1.0) < 0.5) {
            painter->setBrush(QColor(255, 0, 0));
            painter->setPen(QPen(Qt::white, 0.8));
            painter->drawEllipse(QPointF(0, -radius * 0.55), 3.0, 3.0);
        }

        // Orbiting Fire Embers
        for (int i = 0; i < 4; ++i) {
            qreal spkA = t * 6.5 + (i * 1.57);
            qreal spkR = radius * 0.7;
            painter->setBrush(QColor(255, 200, 0));
            painter->setPen(Qt::NoPen);
            painter->drawEllipse(QPointF(std::cos(spkA)*spkR, std::sin(spkA)*spkR), 2.0, 2.0);
        }

        painter->restore();
        return;
    }

    // =========================================================================
    // 🌈 3. COSMIC CHROMATIC HYPER-PRISM (Rainbow Wildcard)
    // =========================================================================
    if (type == BallType::Rainbow) {
        qreal hue = std::fmod(t * 0.15, 1.0);
        QColor prismColor = QColor::fromHsvF(hue, 0.85, 0.95);

        // Prismatic Rainbow Corona
        QRadialGradient rainbowHalo(0, 0, radius * 1.35);
        rainbowHalo.setColorAt(0.0, QColor::fromHsvF(std::fmod(hue + 0.5, 1.0), 0.9, 1.0, (150.0 + breath * 80.0) / 255.0));
        rainbowHalo.setColorAt(0.7, prismColor);
        rainbowHalo.setColorAt(1.0, Qt::transparent);
        painter->setBrush(rainbowHalo);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0,0), radius * 1.35, radius * 1.35);

        // Cosmic Crystal Body
        QRadialGradient crystalBody(0, 0, radius);
        crystalBody.setColorAt(0.0, QColor(255, 255, 255));
        crystalBody.setColorAt(0.4, prismColor.lighter(130));
        crystalBody.setColorAt(0.85, prismColor);
        crystalBody.setColorAt(1.0, prismColor.darker(200));
        painter->setBrush(crystalBody);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0,0), radius, radius);

        // Low-Poly Diamond Facet Cuts
        painter->setPen(QPen(QColor(255, 255, 255, 160), 1.0));
        painter->setBrush(Qt::NoBrush);
        for (int i = 0; i < 6; ++i) {
            qreal a = i * M_PI / 3.0;
            painter->drawLine(QPointF(0,0), QPointF(std::cos(a) * radius, std::sin(a) * radius));
        }

        // Rotating 4-Point Holographic Star ✦
        painter->save();
        painter->rotate(t * 50.0);
        painter->setPen(QPen(QColor(255, 255, 255, 240), 1.8));
        painter->setBrush(QColor(255, 255, 255, 140));
        QPolygonF star;
        qreal s1 = radius * 0.6;
        qreal s2 = radius * 0.16;
        star << QPointF(0, -s1) << QPointF(s2, -s2)
             << QPointF(s1, 0) << QPointF(s2, s2)
             << QPointF(0, s1) << QPointF(-s2, s2)
             << QPointF(-s1, 0) << QPointF(-s2, -s2);
        painter->drawPolygon(star);
        painter->restore();

        // Stardust Particles
        for (int i = 0; i < 3; ++i) {
            qreal a = (i * 2.09) + t * 4.0;
            qreal r = radius * 0.65;
            painter->setBrush(QColor::fromHsvF(std::fmod(hue + (i * 0.33), 1.0), 0.8, 1.0));
            painter->setPen(Qt::NoPen);
            painter->drawEllipse(QPointF(std::cos(a)*r, std::sin(a)*r), 2.2, 2.2);
        }

        painter->restore();
        return;
    }

    // =========================================================================
    // ☯️ 4. QUANTUM HELICAL DUAL BALL
    // =========================================================================
    if (type == BallType::DualColor) {
        QColor c1 = Ball::toQColor(color);
        QColor c2 = Ball::toQColor(secColor != BallColor::None ? secColor : BallColor::Yellow);

        painter->save();
        painter->rotate(t * 40.0);

        QPainterPath clip;
        clip.addEllipse(QPointF(0,0), radius, radius);
        painter->setClipPath(clip);

        painter->setBrush(c1);
        painter->setPen(Qt::NoPen);
        painter->drawRect(-radius, -radius, radius * 2, radius * 2);

        QPainterPath sPath;
        sPath.moveTo(0, -radius);
        sPath.cubicTo(radius * 0.65, -radius * 0.5, radius * 0.65, 0, 0, 0);
        sPath.cubicTo(-radius * 0.65, 0, -radius * 0.65, radius * 0.5, 0, radius);
        sPath.lineTo(radius, radius);
        sPath.lineTo(radius, -radius);
        sPath.closeSubpath();

        painter->setBrush(c2);
        painter->drawPath(sPath);

        // Electric Plasma Arc along the boundary
        painter->setPen(QPen(QColor(255, 255, 255, 240), 2.0));
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(sPath);

        qreal orbA = t * 4.5;
        painter->setBrush(Qt::white);
        painter->drawEllipse(QPointF(std::cos(orbA) * (radius * 0.5), std::sin(orbA) * (radius * 0.5)), 2.5, 2.5);
        painter->drawEllipse(QPointF(std::cos(orbA + M_PI) * (radius * 0.5), std::sin(orbA + M_PI) * (radius * 0.5)), 2.5, 2.5);

        painter->restore();

        // 3D Spherical Edge Shadow & Rim Light
        QRadialGradient sphereShadow(0, 0, radius);
        sphereShadow.setColorAt(0.0, QColor(0, 0, 0, 0));
        sphereShadow.setColorAt(0.7, QColor(0, 0, 0, 20));
        sphereShadow.setColorAt(0.9, QColor(0, 0, 0, 150));
        sphereShadow.setColorAt(1.0, QColor(0, 0, 0, 230));
        painter->setBrush(sphereShadow);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0,0), radius, radius);

        painter->restore();
        return;
    }

    // =========================================================================
    // 🧬 5. REGULAR ELEMENTAL BALLS WITH RUNES & GEODESIC SOCCER MESH
    // =========================================================================
    QColor baseCol = Ball::toQColor(color);
    if (locked) baseCol = QColor(75, 85, 99);

    // Base Shaded Sphere
    QRadialGradient grad(-radius * 0.35, -radius * 0.35, radius * 1.4);
    grad.setColorAt(0.0, baseCol.lighter(165));
    grad.setColorAt(0.35, baseCol);
    grad.setColorAt(0.85, baseCol.darker(175));
    grad.setColorAt(1.0, baseCol.darker(280));
    painter->setBrush(grad);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QPointF(0,0), radius, radius);

    // True Miniature Geodesic Soccer Mesh (5-gon + matching 6-gons)
    painter->save();
    QPainterPath clip;
    clip.addEllipse(QPointF(0,0), radius, radius);
    painter->setClipPath(clip);

    painter->setPen(QPen(QColor(15, 23, 42, 160), 0.9));
    painter->setBrush(Qt::NoBrush);

    qreal r_p = radius * 0.28;
    QPolygonF pentagon0;
    QVector<QPointF> pVerts;
    for (int i = 0; i < 5; ++i) {
        qreal a = (i * 72.0 - 90.0) * M_PI / 180.0;
        QPointF pt(r_p * std::cos(a), r_p * std::sin(a));
        pentagon0 << pt;
        pVerts << pt;
    }
    painter->setBrush(QColor(0, 0, 0, 35));
    painter->drawPolygon(pentagon0);
    painter->setBrush(Qt::NoBrush);

    for (int i = 0; i < 5; ++i) {
        QPointF v1 = pVerts[i];
        QPointF v2 = pVerts[(i + 1) % 5];
        QPointF mid = (v1 + v2) * 0.5;
        qreal midAngle = std::atan2(mid.y(), mid.x());
        
        qreal r_h_out = radius * 0.64;
        qreal r_h_far = radius * 0.90;
        
        qreal a1 = (i * 72.0 - 90.0) * M_PI / 180.0;
        qreal a2 = (((i + 1) % 5) * 72.0 - 90.0) * M_PI / 180.0;
        qreal aMid = midAngle;
        
        QPointF h3(r_h_out * std::cos(a2 + 0.16), r_h_out * std::sin(a2 + 0.16));
        QPointF h4(r_h_far * std::cos(aMid + 0.14), r_h_far * std::sin(aMid + 0.14));
        QPointF h5(r_h_far * std::cos(aMid - 0.14), r_h_far * std::sin(aMid - 0.14));
        QPointF h6(r_h_out * std::cos(a1 - 0.16), r_h_out * std::sin(a1 - 0.16));
        
        QPolygonF hexPoly;
        hexPoly << v1 << v2 << h3 << h4 << h5 << h6;
        painter->drawPolygon(hexPoly);
        
        QPolygonF outerPent;
        qreal r_rim = radius * 1.05;
        QPointF p_out1(r_rim * std::cos(a1), r_rim * std::sin(a1));
        outerPent << h6 << h5 << p_out1;
        painter->setBrush(QColor(0, 0, 0, 25));
        painter->drawPolygon(outerPent);
        painter->setBrush(Qt::NoBrush);
    }

    QRadialGradient sphereShadow(0, 0, radius);
    sphereShadow.setColorAt(0.0, QColor(0, 0, 0, 0));
    sphereShadow.setColorAt(0.7, QColor(0, 0, 0, 20));
    sphereShadow.setColorAt(0.9, QColor(0, 0, 0, 150));
    sphereShadow.setColorAt(1.0, QColor(0, 0, 0, 230));
    painter->setBrush(sphereShadow);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QPointF(0,0), radius, radius);
    painter->restore();

    // Elemental Runes
    painter->save();
    qreal runePulse = 0.75 + breath * 0.25;

    if (color == BallColor::Red) {
        painter->rotate(t * 25.0);
        painter->setPen(QPen(QColor(255, 230, 100, int(220 * runePulse)), 1.5));
        painter->setBrush(QColor(255, 100, 0, int(100 * runePulse)));
        for (int i = 0; i < 3; ++i) {
            painter->rotate(120);
            painter->drawArc(QRectF(-5, -10, 10, 10), 0, 140 * 16);
        }
        painter->setBrush(QColor(255, 255, 255, 220));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0,0), 2.8 * runePulse, 2.8 * runePulse);
    }
    else if (color == BallColor::Blue) {
        painter->setPen(QPen(QColor(180, 245, 255, int(230 * runePulse)), 1.4));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(QRectF(-3.5, -3.5, 7, 7));
        painter->drawLine(0, -3.5, 0, -8);
        painter->drawLine(0, 3.5, 0, 8);
        painter->drawLine(-3.5, 0, -8, 0);
        painter->drawLine(3.5, 0, 8, 0);
        painter->setBrush(ThemeManager::instance().getPrimaryColor());
        painter->drawEllipse(QPointF(0, -8), 1.4, 1.4);
        painter->drawEllipse(QPointF(0, 8), 1.4, 1.4);
        painter->drawEllipse(QPointF(-8, 0), 1.4, 1.4);
        painter->drawEllipse(QPointF(8, 0), 1.4, 1.4);
    }
    else if (color == BallColor::Green) {
        painter->rotate(t * -30.0);
        painter->setPen(QPen(QColor(180, 255, 180, int(220 * runePulse)), 1.4));
        painter->setBrush(QColor(0, 255, 100, int(90 * runePulse)));
        for (int i = 0; i < 3; ++i) {
            painter->rotate(120);
            painter->drawEllipse(QPointF(0, -5.0), 2.8, 2.8);
        }
        painter->setBrush(Qt::white);
        painter->drawEllipse(QPointF(0,0), 2.0, 2.0);
    }
    else if (color == BallColor::Yellow) {
        painter->setPen(QPen(QColor(255, 255, 255, int(240 * runePulse)), 1.6));
        painter->setBrush(QColor(255, 230, 50, int(150 * runePulse)));
        QPolygonF bolt;
        bolt << QPointF(1, -8) << QPointF(-3.5, 0) << QPointF(0, 0)
             << QPointF(-1.5, 8) << QPointF(4.5, -1) << QPointF(1, -1);
        painter->drawPolygon(bolt);
    }
    else if (color == BallColor::Purple) {
        painter->rotate(t * 50.0);
        painter->setPen(QPen(QColor(255, 200, 255, int(220 * runePulse)), 1.4, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(QPointF(0,0), 7.5, 7.5);
        painter->setPen(QPen(QColor(255, 255, 255, 200), 1.4));
        painter->drawArc(QRectF(-5.5, -5.5, 11, 11), 0, 180 * 16);
        painter->drawArc(QRectF(-5.5, -5.5, 11, 11), 180 * 16, 180 * 16);
        painter->setBrush(QColor(255, 255, 255, 230));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0,0), 2.2, 2.2);
    }
    painter->restore();

    // Titanium Lockdown for Locked Balls
    if (locked) {
        painter->save();
        painter->setBrush(QColor(203, 213, 225));
        painter->setPen(QPen(QColor(15, 23, 42), 1.0));
        qreal rivD = radius * 0.65;
        painter->drawEllipse(QPointF(-rivD, -rivD), 2.0, 2.0);
        painter->drawEllipse(QPointF(rivD, -rivD), 2.0, 2.0);
        painter->drawEllipse(QPointF(-rivD, rivD), 2.0, 2.0);
        painter->drawEllipse(QPointF(rivD, rivD), 2.0, 2.0);

        painter->setBrush(QColor(15, 23, 42, 220));
        painter->setPen(QPen(QColor(239, 68, 68), 1.6));
        painter->drawRoundedRect(QRectF(-6.5, -3.5, 13, 10), 2, 2);
        painter->setBrush(Qt::NoBrush);
        painter->drawArc(-4.5, -10, 9, 9, 0, 180 * 16);

        qreal scanY = -7.0 + std::fmod(t * 16.0, 14.0);
        painter->setPen(QPen(QColor(239, 68, 68, 200), 1.5));
        painter->drawLine(-5.5, scanY, 5.5, scanY);
        painter->restore();
    }

    // Specular Highlight
    QRadialGradient glint(-radius * 0.35, -radius * 0.35, radius * 0.55);
    glint.setColorAt(0.0, QColor(255, 255, 255, 220));
    glint.setColorAt(0.45, QColor(255, 255, 255, 60));
    glint.setColorAt(1.0, QColor(255, 255, 255, 0));
    painter->setBrush(glint);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QPointF(-radius * 0.35, -radius * 0.35), radius * 0.40, radius * 0.30);

    painter->restore();
}
