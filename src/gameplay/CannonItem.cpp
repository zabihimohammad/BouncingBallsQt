#include "CannonItem.h"
#include <QRadialGradient>

CannonItem::CannonItem(qreal width, qreal height) : m_width(width), m_height(height) {}

QRectF CannonItem::boundingRect() const {
    return QRectF(-60, -60, 120, 120);
}

void CannonItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing);

    // Glowing Base
    QRadialGradient baseGlow(0, 0, 45);
    baseGlow.setColorAt(0.0, QColor(30, 41, 59));
    baseGlow.setColorAt(0.8, QColor(15, 23, 42));
    baseGlow.setColorAt(1.0, QColor(56, 189, 248, 200));

    painter->setBrush(baseGlow);
    painter->setPen(QPen(QColor(56, 189, 248), 2));
    painter->drawEllipse(-40, -40, 80, 80);

    // Modern Cannon Barrel
    painter->save();
    painter->rotate(-m_angle + 90);
    painter->setBrush(QBrush(QColor(51, 65, 85)));
    painter->setPen(QPen(QColor(148, 163, 184), 1.5));
    painter->drawRoundedRect(-14, -50, 28, 50, 8, 8);
    painter->restore();

    // 3D Spherical Loaded Ball
    QRadialGradient ballGrad(-5, -5, 20);
    QColor ballCol = Ball::toQColor(m_currentBall);
    ballGrad.setColorAt(0.0, QColor(255, 255, 255, 220));
    ballGrad.setColorAt(0.4, ballCol);
    ballGrad.setColorAt(1.0, ballCol.darker(180));

    painter->setPen(Qt::NoPen);
    painter->setBrush(ballGrad);
    painter->drawEllipse(-18, -18, 36, 36);

    // Next Ball Preview Ring
    painter->setPen(QPen(QColor(148, 163, 184, 150), 1, Qt::DashLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(-85, -15, 28, 28);

    QRadialGradient nextGrad(-71, -1, 14);
    QColor nextCol = Ball::toQColor(m_nextBall);
    nextGrad.setColorAt(0.0, QColor(255, 255, 255, 200));
    nextGrad.setColorAt(0.4, nextCol);
    nextGrad.setColorAt(1.0, nextCol.darker(180));

    painter->setPen(Qt::NoPen);
    painter->setBrush(nextGrad);
    painter->drawEllipse(-83, -13, 24, 24);
}

void CannonItem::setAngle(qreal angleDeg) { m_angle = angleDeg; update(); }
void CannonItem::setCurrentBall(BallColor c) { m_currentBall = c; update(); }
void CannonItem::setNextBall(BallColor c) { m_nextBall = c; update(); }
void CannonItem::swapBalls() { std::swap(m_currentBall, m_nextBall); update(); }