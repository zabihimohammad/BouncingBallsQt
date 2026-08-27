#include "../ui/ThemeManager.h"
#include "BallItem.h"
#include <QPainterPath>
#include <QTime>
#include <QRandomGenerator>
#include <cmath>

BallItem::BallItem(BallColor color, BallType type, BallColor secColor, bool locked, qreal radius, int freezeLevel, bool isMystery, bool isKey, BallType containedSkill, bool isTimeCrystal, bool isChronoBomb, qreal chronoTimer, QGraphicsItem* parent)
        : QGraphicsObject(parent), m_color(color), m_type(type), m_secColor(secColor), m_isLocked(locked),
          m_radius(radius), m_freezeLevel(freezeLevel), m_isMystery(isMystery), m_isKey(isKey), m_containedSkill(containedSkill), m_isTimeCrystal(isTimeCrystal), m_isChronoBomb(isChronoBomb), m_chronoTimer(chronoTimer) {
}

BallItem::BallItem(const Ball* ball, qreal radius, QGraphicsItem* parent)
        : QGraphicsObject(parent), m_radius(radius) {
    if (ball) {
        m_color = ball->getPrimaryColor();
        m_type = ball->getType();
        m_secColor = ball->getSecondaryColor();
        m_isLocked = ball->isLocked();
        m_freezeLevel = ball->getFreezeLevel();
        m_isMystery = ball->isMystery();
        m_isKey = ball->isKey();
        m_containedSkill = ball->getContainedSkill();
        m_isTimeCrystal = ball->isTimeCrystal();
        m_isChronoBomb = ball->isChronoBomb();
        m_chronoTimer = ball->getChronoBombTimer();
    } else {
        m_color = BallColor::None;
        m_type = BallType::Regular;
        m_secColor = BallColor::None;
        m_isLocked = false;
        m_freezeLevel = 0;
        m_isMystery = false;
        m_isKey = false;
        m_containedSkill = BallType::Regular;
        m_isTimeCrystal = false;
        m_isChronoBomb = false;
        m_chronoTimer = 5.0;
    }
}

QRectF BallItem::boundingRect() const {
    return QRectF(-m_radius - 8, -m_radius - 8, (m_radius + 8) * 2, (m_radius + 8) * 2);
}

void BallItem::updateData(BallColor color, BallType type, BallColor secColor, bool locked, int freezeLevel, bool isMystery, bool isKey, BallType containedSkill, bool isTimeCrystal, bool isChronoBomb, qreal chronoTimer) {
    m_color = color;
    m_type = type;
    m_secColor = secColor;
    m_isLocked = locked;
    m_freezeLevel = freezeLevel;
    m_isMystery = isMystery;
    m_isKey = isKey;
    m_containedSkill = containedSkill;
    m_isTimeCrystal = isTimeCrystal;
    m_isChronoBomb = isChronoBomb;
    m_chronoTimer = chronoTimer;
    update();
}

void BallItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    paintBall(painter, QPointF(0, 0), m_radius, m_color, m_type, m_secColor, m_isLocked, m_freezeLevel, m_isMystery, m_isKey, m_containedSkill, m_isTimeCrystal, m_isChronoBomb, m_chronoTimer);
}

void BallItem::paintBall(QPainter* painter, const QPointF& center, qreal radius, BallColor color, BallType type, BallColor secColor, bool locked, int freezeLevel, bool isMystery, bool isKey, BallType containedSkill, bool isTimeCrystal, bool isChronoBomb, qreal chronoTimer) {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->translate(center);

    qint64 ms = QTime::currentTime().msecsSinceStartOfDay();
    qreal t = (ms % 600000) / 1000.0;
    qreal breath = (std::sin(t * 4.0) + 1.0) * 0.5;

    // ۱. گوی پرتو فوتونی (Photon Beam)
    if (type == BallType::PhotonBeam) {
        QRadialGradient photonGlow(0, 0, radius * 1.55);
        photonGlow.setColorAt(0.0, QColor(255, 255, 255, int(220 + breath * 35)));
        photonGlow.setColorAt(0.4, QColor(0, 242, 254, 180));
        photonGlow.setColorAt(0.8, QColor(165, 94, 234, 100));
        photonGlow.setColorAt(1.0, Qt::transparent);
        painter->setBrush(photonGlow);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0, 0), radius * 1.55, radius * 1.55);

        QRadialGradient photonCore(0, 0, radius);
        photonCore.setColorAt(0.0, Qt::white);
        photonCore.setColorAt(0.3, QColor(180, 255, 255));
        photonCore.setColorAt(0.8, QColor(0, 242, 254));
        photonCore.setColorAt(1.0, QColor(10, 25, 50));
        painter->setBrush(photonCore);
        painter->setPen(QPen(Qt::white, 2.0));
        painter->drawEllipse(QPointF(0, 0), radius, radius);

        painter->save();
        painter->rotate(t * 180.0);
        painter->setPen(QPen(QColor(255, 255, 255, 220), 1.8, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(QPointF(0, 0), radius * 0.7, radius * 0.7);
        painter->restore();

        painter->setFont(QFont("Segoe UI", 12, QFont::Black));
        painter->setPen(QColor(15, 23, 42));
        painter->drawText(QRectF(-radius, -radius, radius * 2, radius * 2), Qt::AlignCenter, "⚡");

        painter->restore();
        return;
    }

    // ۲. گوی لیزر
    if (type == BallType::Laser) {
        QRadialGradient outerGlow(0, 0, radius * 1.45);
        outerGlow.setColorAt(0.0, QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), int(180 + breath * 75)));
        outerGlow.setColorAt(0.5, QColor(ThemeManager::instance().getSecondaryColor().red(), ThemeManager::instance().getSecondaryColor().green(), ThemeManager::instance().getSecondaryColor().blue(), int(80 + breath * 40)));
        outerGlow.setColorAt(1.0, QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), 0));
        painter->setBrush(outerGlow);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0,0), radius * 1.45, radius * 1.45);

        QRadialGradient laserCore(0, 0, radius);
        laserCore.setColorAt(0.0, QColor(255, 255, 255));
        laserCore.setColorAt(0.3, QColor(180, 255, 255));
        laserCore.setColorAt(0.7, ThemeManager::instance().getPrimaryColor());
        laserCore.setColorAt(1.0, ThemeManager::instance().getSecondaryColor());
        painter->setBrush(laserCore);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0,0), radius, radius);

        painter->setPen(QPen(QColor(15, 23, 42), 1.5));
        painter->setBrush(QColor(226, 232, 240));
        painter->drawRoundedRect(QRectF(-radius - 2, -6, 5, 12), 2, 2);
        painter->drawRoundedRect(QRectF(radius - 3, -6, 5, 12), 2, 2);

        painter->save();
        painter->setBrush(Qt::NoBrush);
        painter->rotate(t * 120.0);
        painter->setPen(QPen(QColor(255, 255, 255, 200), 1.5, Qt::DashLine));
        painter->drawEllipse(QPointF(0,0), radius * 0.75, radius * 0.35);
        painter->rotate(t * -240.0);
        painter->setPen(QPen(QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), 220), 1.5));
        painter->drawEllipse(QPointF(0,0), radius * 0.35, radius * 0.75);
        painter->restore();

        qreal vib = std::sin(t * 40.0) * 1.2;
        painter->setPen(QPen(QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), 200), 8.0, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(QPointF(-radius + 2, vib), QPointF(radius - 2, vib));
        painter->setPen(QPen(QColor(255, 255, 255, 255), 3.0, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(QPointF(-radius + 2, vib), QPointF(radius - 2, vib));

        painter->restore();
        return;
    }

    // ۳. گوی بمب
    if (type == BallType::Bomb) {
        QRadialGradient bombHull(0, 0, radius);
        bombHull.setColorAt(0.0, QColor(60, 20, 20));
        bombHull.setColorAt(0.6, QColor(30, 20, 25));
        bombHull.setColorAt(1.0, QColor(10, 10, 15));
        painter->setBrush(bombHull);
        painter->setPen(QPen(QColor(245, 158, 11), 1.2));
        painter->drawEllipse(QPointF(0,0), radius, radius);

        painter->setPen(QPen(QColor(255, 140, 0, int(180 + breath * 75)), 1.5));
        painter->drawLine(QPointF(-radius * 0.6, -radius * 0.3), QPointF(0, 0));
        painter->drawLine(QPointF(radius * 0.6, -radius * 0.4), QPointF(0, 0));
        painter->drawLine(QPointF(-radius * 0.3, radius * 0.6), QPointF(0, 0));
        painter->drawLine(QPointF(radius * 0.5, radius * 0.5), QPointF(0, 0));

        painter->setPen(QPen(QColor(245, 158, 11), 3.0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(QPointF(0,0), radius * 0.8, radius * 0.8);

        qreal coreR = radius * (0.38 + breath * 0.15);
        QRadialGradient fireCore(0, 0, coreR);
        fireCore.setColorAt(0.0, QColor(255, 255, 255));
        fireCore.setColorAt(0.25, QColor(255, 220, 50));
        fireCore.setColorAt(0.6, QColor(239, 68, 68));
        fireCore.setColorAt(1.0, QColor(185, 28, 28));
        painter->setBrush(fireCore);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0,0), coreR, coreR);

        painter->restore();
        return;
    }

    // ۴. گوی رنگین‌کمان
    if (type == BallType::Rainbow) {
        qreal hue = std::fmod(t * 0.15, 1.0);
        QColor prismColor = QColor::fromHsvF(hue, 0.85, 0.95);

        QRadialGradient rainbowHalo(0, 0, radius * 1.35);
        rainbowHalo.setColorAt(0.0, QColor::fromHsvF(std::fmod(hue + 0.5, 1.0), 0.9, 1.0, (150.0 + breath * 80.0) / 255.0));
        rainbowHalo.setColorAt(0.7, prismColor);
        rainbowHalo.setColorAt(1.0, Qt::transparent);
        painter->setBrush(rainbowHalo);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0,0), radius * 1.35, radius * 1.35);

        QRadialGradient crystalBody(0, 0, radius);
        crystalBody.setColorAt(0.0, QColor(255, 255, 255));
        crystalBody.setColorAt(0.4, prismColor.lighter(130));
        crystalBody.setColorAt(0.85, prismColor);
        crystalBody.setColorAt(1.0, prismColor.darker(200));
        painter->setBrush(crystalBody);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0,0), radius, radius);

        painter->restore();
        return;
    }

    // ۵. گوی دو رنگ
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

        painter->setPen(QPen(QColor(255, 255, 255, 240), 2.0));
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(sPath);

        painter->restore();

        QRadialGradient sphereShadow(0, 0, radius);
        sphereShadow.setColorAt(0.7, QColor(0, 0, 0, 0));
        sphereShadow.setColorAt(1.0, QColor(0, 0, 0, 200));
        painter->setBrush(sphereShadow);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0,0), radius, radius);
    }
        // ۶. گوی‌های رنگی پایه
    else {
        QColor baseCol = isMystery ? QColor(71, 85, 105) : (color == BallColor::Black ? QColor(30, 41, 59) : Ball::toQColor(color));
        if (locked) baseCol = QColor(75, 85, 99);

        QRadialGradient grad(-radius * 0.35, -radius * 0.35, radius * 1.4);
        grad.setColorAt(0.0, baseCol.lighter(165));
        grad.setColorAt(0.35, baseCol);
        grad.setColorAt(0.85, baseCol.darker(175));
        grad.setColorAt(1.0, baseCol.darker(280));
        painter->setBrush(grad);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0,0), radius, radius);

        // مش ژئودزیک
        if (!isMystery && color != BallColor::Black && freezeLevel == 0) {
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
            }

            QRadialGradient sphereShadow(0, 0, radius);
            sphereShadow.setColorAt(0.0, QColor(0, 0, 0, 0));
            sphereShadow.setColorAt(0.7, QColor(0, 0, 0, 20));
            sphereShadow.setColorAt(1.0, QColor(0, 0, 0, 230));
            painter->setBrush(sphereShadow);
            painter->setPen(Qt::NoPen);
            painter->drawEllipse(QPointF(0,0), radius, radius);
            painter->restore();
        }
    }

    // ۷. وضعیت‌های تعاملی و ویژه
    if (freezeLevel > 0) {
        painter->save();
        QColor iceColor = (freezeLevel == 2) ? QColor(147, 197, 253, 190) : QColor(191, 219, 254, 130);
        painter->setBrush(iceColor);
        painter->setPen(QPen(QColor(255, 255, 255, 220), 2.0));
        painter->drawRoundedRect(QRectF(-radius + 2, -radius + 2, (radius - 2) * 2, (radius - 2) * 2), 6, 6);

        painter->setPen(QPen(Qt::white, 1.5));
        painter->drawLine(-radius * 0.6, -radius * 0.4, 0, 0);
        painter->drawLine(0, 0, radius * 0.5, radius * 0.6);
        painter->restore();
    }

    if (isMystery) {
        painter->setPen(Qt::white);
        painter->setFont(QFont("Consolas", 14, QFont::Black));
        painter->drawText(QRectF(-radius, -radius, radius * 2, radius * 2), Qt::AlignCenter, "?");
    }

    if (isKey) {
        painter->save();
        painter->rotate(t * 30.0);
        painter->setPen(QPen(QColor(255, 255, 255, 240), 2));
        painter->setBrush(QColor(255, 215, 0, 220));
        painter->drawText(QRectF(-radius, -radius, radius * 2, radius * 2), Qt::AlignCenter, "★");
        painter->restore();
    }

    if (locked) {
        painter->save();
        painter->setBrush(QColor(15, 23, 42, 220));
        painter->setPen(QPen(QColor(239, 68, 68), 1.6));
        painter->drawRoundedRect(QRectF(-6.5, -3.5, 13, 10), 2, 2);
        painter->setBrush(Qt::NoBrush);
        painter->drawArc(-4.5, -10, 9, 9, 0, 180 * 16);
        painter->restore();
    }

    if (containedSkill != BallType::Regular) {
        QString icon = (containedSkill == BallType::Bomb) ? "💣" : ((containedSkill == BallType::Laser) ? "⚡" : "🌈");
        painter->setFont(QFont("Segoe UI Emoji", 10, QFont::Bold));
        painter->setPen(Qt::white);
        painter->drawText(QRectF(-radius, -radius + 1, radius * 2, radius * 2), Qt::AlignCenter, icon);
    }

    // کریستال انجماد زمان (Chrono-Freeze)
    if (isTimeCrystal) {
        painter->save();
        qreal crystalPulse = std::sin(t * 6.0) * 0.3 + 0.7;
        painter->setPen(QPen(QColor(0, 242, 254, int(220 * crystalPulse)), 1.8));
        painter->setBrush(QColor(0, 242, 254, int(60 * crystalPulse)));
        painter->drawEllipse(QPointF(0,0), radius * 0.88, radius * 0.88);

        painter->setFont(QFont("Segoe UI Emoji", 10, QFont::Bold));
        painter->setPen(Qt::white);
        painter->drawText(QRectF(-radius, -radius + 1, radius * 2, radius * 2), Qt::AlignCenter, "⏳");
        painter->restore();
    }

    // ۸. رندر بمب ساعتی با شمارشگر زنده (Chrono Bomb)
    if (isChronoBomb) {
        painter->save();
        qreal alertPulse = (chronoTimer <= 2.0) ? (std::sin(t * 16.0) * 0.5 + 0.5) : (std::sin(t * 8.0) * 0.3 + 0.7);
        QColor alertColor = (chronoTimer <= 2.0) ? QColor(255, 51, 102) : QColor(245, 158, 11);

        painter->setPen(QPen(QColor(alertColor.red(), alertColor.green(), alertColor.blue(), int(230 * alertPulse)), 2.0, Qt::DashLine));
        painter->setBrush(QColor(15, 23, 42, 210));
        painter->drawEllipse(QPointF(0, 0), radius * 0.9, radius * 0.9);

        painter->setFont(QFont("Consolas", 8, QFont::Bold));
        painter->setPen(alertColor);
        QString timeText = QString("%1s").arg(chronoTimer, 0, 'f', 1);
        painter->drawText(QRectF(-radius, -radius + 4, radius * 2, radius * 2), Qt::AlignCenter, timeText);

        painter->setFont(QFont("Segoe UI Emoji", 7));
        painter->drawText(QRectF(-radius, -radius - 5, radius * 2, radius * 2), Qt::AlignCenter, "💣");
        painter->restore();
    }

    QRadialGradient glint(-radius * 0.35, -radius * 0.35, radius * 0.55);
    glint.setColorAt(0.0, QColor(255, 255, 255, 220));
    glint.setColorAt(0.45, QColor(255, 255, 255, 60));
    glint.setColorAt(1.0, QColor(255, 255, 255, 0));
    painter->setBrush(glint);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QPointF(-radius * 0.35, -radius * 0.35), radius * 0.40, radius * 0.30);

    painter->restore();
}