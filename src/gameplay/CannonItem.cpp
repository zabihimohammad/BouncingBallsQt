#include "CannonItem.h"

CannonItem::CannonItem(qreal width, qreal height) : m_width(width), m_height(height) {}

QRectF CannonItem::boundingRect() const {
    return QRectF(-50, -50, 100, 100);
}

void CannonItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing);

    // Base
    painter->setBrush(QBrush(QColor(52, 73, 94)));
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(-35, -35, 70, 70);

    // Cannon Barrel
    painter->save();
    painter->rotate(-m_angle + 90);
    painter->setBrush(QBrush(QColor(127, 140, 141)));
    painter->drawRoundedRect(-12, -45, 24, 45, 6, 6);
    painter->restore();

    // Loaded ball
    painter->setBrush(QBrush(Ball::toQColor(m_currentBall)));
    painter->drawEllipse(-18, -18, 36, 36);

    // Next ball preview (left)
    painter->setBrush(QBrush(Ball::toQColor(m_nextBall)));
    painter->drawEllipse(-70, -10, 20, 20);
}

void CannonItem::setAngle(qreal angleDeg) {
    m_angle = angleDeg;
    update();
}

void CannonItem::setCurrentBall(BallColor c) {
    m_currentBall = c;
    update();
}

void CannonItem::setNextBall(BallColor c) {
    m_nextBall = c;
    update();
}

void CannonItem::swapBalls() {
    std::swap(m_currentBall, m_nextBall);
    update();
}
