#include "CannonItem.h"
#include "BallItem.h"
#include "../ui/ThemeManager.h"
#include <QRadialGradient>
#include <cmath>

CannonItem::CannonItem(qreal width, qreal height)
        : m_sceneWidth(width), m_sceneHeight(height) {
    setZValue(20);
}

QRectF CannonItem::boundingRect() const {
    return QRectF(-100, -70, 200, 140);
}

void CannonItem::setAngle(qreal angleDeg) {
    m_angle = angleDeg;
    update();
}

void CannonItem::setCurrentBall(BallColor color, BallType type, BallColor secColor) {
    m_currentColor = color;
    m_currentType = type;
    m_currentSecColor = secColor;
    update();
}

void CannonItem::setNextBall(BallColor color, BallType type, BallColor secColor) {
    m_nextColor = color;
    m_nextType = type;
    m_nextSecColor = secColor;
    update();
}

void CannonItem::swapBalls() {
    std::swap(m_currentColor, m_nextColor);
    std::swap(m_currentType, m_nextType);
    std::swap(m_currentSecColor, m_nextSecColor);
    update();
}

void CannonItem::drawBall(QPainter* painter, const QPointF& center, qreal radius, BallColor color, BallType type, BallColor secColor) {
    BallItem::paintBall(painter, center, radius, color, type, secColor, false);
}

bool CannonItem::isNextBallClicked(const QPointF& localPos) const {
    qreal dist = std::hypot(localPos.x() - (-65.0), localPos.y() - 0.0);
    return dist <= 28.0;
}

void CannonItem::triggerFireRecoil() {
    m_recoilOffset = 14.0;
    m_muzzleFlashAlpha = 1.0;
    update();
}

void CannonItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing);

    m_recoilOffset *= 0.82;
    if (m_recoilOffset < 0.2) m_recoilOffset = 0.0;
    m_muzzleFlashAlpha *= 0.78;
    if (m_muzzleFlashAlpha < 0.03) m_muzzleFlashAlpha = 0.0;

    QRadialGradient baseGlow(0, 0, 48);
    if (m_isOverdrive) {
        baseGlow.setColorAt(0.0, QColor(60, 10, 0));
        baseGlow.setColorAt(0.75, QColor(255, 60, 0, 160));
        baseGlow.setColorAt(1.0, QColor(255, 204, 0, 180));
        painter->setPen(QPen(QColor(255, 204, 0), 2.5));
    } else {
        baseGlow.setColorAt(0.0, QColor(10, 15, 30));
        baseGlow.setColorAt(0.75, QColor(15, 23, 42));
        baseGlow.setColorAt(1.0, QColor(0, 242, 254, 180));
        painter->setPen(QPen(QColor(0, 242, 254), 2));
    }
    painter->setBrush(baseGlow);
    painter->drawEllipse(QPointF(0, 0), 42, 42);

    painter->setPen(QPen(QColor(148, 163, 184, 160), 2.5));
    painter->save();
    for (int i = 0; i < 8; ++i) {
        painter->drawLine(QPointF(0, -38), QPointF(0, -44));
        painter->rotate(45.0);
    }
    painter->restore();

    painter->save();
    painter->rotate(-m_angle + 90.0);
    painter->translate(0, m_recoilOffset);

    painter->setBrush(QBrush(QColor(30, 41, 59)));
    painter->setPen(QPen(QColor(148, 163, 184), 2));
    painter->drawRoundedRect(-18, -48, 10, 48, 3, 3);
    painter->drawRoundedRect(8, -48, 10, 48, 3, 3);

    QLinearGradient plasmaTrack(0, 0, 0, -50);
    if (m_isOverdrive) {
        plasmaTrack.setColorAt(0.0, QColor(255, 60, 0, 120));
        plasmaTrack.setColorAt(1.0, QColor(255, 204, 0, 250));
    } else {
        plasmaTrack.setColorAt(0.0, QColor(0, 242, 254, 100));
        plasmaTrack.setColorAt(1.0, QColor(0, 242, 254, 250));
    }
    painter->setBrush(plasmaTrack);
    painter->setPen(Qt::NoPen);
    painter->drawRect(-6, -45, 12, 35);

    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(QColor(255, 255, 255, 150), 2));
    painter->drawLine(-15, -20, 15, -20);
    painter->drawLine(-15, -35, 15, -35);

    if (m_muzzleFlashAlpha > 0.01) {
        QRadialGradient flashGrad(0, -60, 26);
        if (m_isOverdrive) {
            flashGrad.setColorAt(0.0, QColor(255, 255, 255, int(m_muzzleFlashAlpha * 255)));
            flashGrad.setColorAt(0.5, QColor(255, 204, 0, int(m_muzzleFlashAlpha * 200)));
            flashGrad.setColorAt(1.0, Qt::transparent);
        } else {
            flashGrad.setColorAt(0.0, QColor(255, 255, 255, int(m_muzzleFlashAlpha * 255)));
            flashGrad.setColorAt(0.5, QColor(0, 242, 254, int(m_muzzleFlashAlpha * 200)));
            flashGrad.setColorAt(1.0, Qt::transparent);
        }
        painter->setBrush(flashGrad);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0, -60), 26, 26);
    }
    painter->restore();

    drawBall(painter, QPointF(0, 0), 16.0, m_currentColor, m_currentType, m_currentSecColor);

    painter->setPen(QPen(QColor(0, 242, 254, 140), 1.5, Qt::DashLine));
    painter->setBrush(QColor(10, 16, 28, 180));
    painter->drawEllipse(QPointF(-60, 0), 17, 17);

    painter->setPen(QColor(148, 163, 184));
    painter->setFont(QFont("Segoe UI", 6, QFont::Bold));
    painter->drawText(QRectF(-80, 18, 40, 14), Qt::AlignCenter, "NEXT");

    drawBall(painter, QPointF(-60, 0), 12.5, m_nextColor, m_nextType, m_nextSecColor);
}