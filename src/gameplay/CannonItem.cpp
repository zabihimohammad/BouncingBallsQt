#include "CannonItem.h"
#include <QRadialGradient>
#include <cmath>

CannonItem::CannonItem(qreal width, qreal height)
        : m_sceneWidth(width), m_sceneHeight(height) {
    setZValue(20); // لوله توپ بالاتر از گوی‌ها قرار می‌گیرد
}

QRectF CannonItem::boundingRect() const {
    return QRectF(-100, -70, 200, 140);
}

void CannonItem::setAngle(qreal angleDeg) {
    m_angle = angleDeg;
    update();
}

void CannonItem::setCurrentBall(BallColor color, BallType type) {
    m_currentColor = color;
    m_currentType = type;
    update();
}

void CannonItem::setNextBall(BallColor color, BallType type) {
    m_nextColor = color;
    m_nextType = type;
    update();
}

void CannonItem::swapBalls() {
    std::swap(m_currentColor, m_nextColor);
    std::swap(m_currentType, m_nextType);
    update();
}

void CannonItem::drawBall(QPainter* painter, const QPointF& center, qreal radius, BallColor color, BallType type) {
    painter->save();

    QColor baseCol = Ball::toQColor(color);
    if (type == BallType::Rainbow) baseCol = QColor(255, 230, 0);
    else if (type == BallType::Bomb) baseCol = QColor(231, 76, 60);
    else if (type == BallType::Laser) baseCol = QColor(0, 210, 211);

    // گرادیان کروی سه‌بعدی
    QRadialGradient grad(center.x() - radius * 0.3, center.y() - radius * 0.3, radius * 1.3);
    grad.setColorAt(0.0, QColor(255, 255, 255, 230));
    grad.setColorAt(0.35, baseCol);
    grad.setColorAt(1.0, baseCol.darker(200));

    painter->setPen(Qt::NoPen);
    painter->setBrush(grad);
    painter->drawEllipse(center, radius, radius);

    // علامت‌گذاری بصری برای توپ‌های خاص
    if (type == BallType::Bomb) {
        painter->setPen(QPen(Qt::black, 2));
        painter->setBrush(Qt::black);
        painter->drawEllipse(center, radius * 0.35, radius * 0.35);
    } else if (type == BallType::Rainbow) {
        painter->setPen(QPen(Qt::white, 2, Qt::DotLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(center, radius * 0.6, radius * 0.6);
    }

    painter->restore();
}

void CannonItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing);

    // ۱. پایه شلیک (Base Glow)
    QRadialGradient baseGlow(0, 0, 48);
    baseGlow.setColorAt(0.0, QColor(30, 41, 59));
    baseGlow.setColorAt(0.7, QColor(15, 23, 42));
    baseGlow.setColorAt(1.0, QColor(56, 189, 248, 180));

    painter->setBrush(baseGlow);
    painter->setPen(QPen(QColor(56, 189, 248), 2));
    painter->drawEllipse(QPointF(0, 0), 40, 40);

    // ۲. لوله گردان توپ (Rotating Barrel)
    painter->save();
    painter->rotate(-m_angle + 90.0);
    painter->setBrush(QBrush(QColor(51, 65, 85)));
    painter->setPen(QPen(QColor(148, 163, 184), 1.5));
    painter->drawRoundedRect(-13, -52, 26, 52, 6, 6);
    painter->restore();

    // ۳. گلوله آماده شلیک در مرکز
    drawBall(painter, QPointF(0, 0), 18.0, m_currentColor, m_currentType);

    // ۴. حلقه و گلوله بعدی در خشاب (Next Ball Preview)
    painter->setPen(QPen(QColor(148, 163, 184, 120), 1.5, Qt::DashLine));
    painter->setBrush(QColor(15, 23, 42, 160));
    painter->drawEllipse(QPointF(-65, 0), 18, 18);

    drawBall(painter, QPointF(-65, 0), 13.0, m_nextColor, m_nextType);
}