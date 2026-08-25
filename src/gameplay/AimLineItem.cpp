#include "AimLineItem.h"
#include <cmath>

AimLineItem::AimLineItem(qreal minX, qreal maxX, qreal sceneHeight, qreal ballRadius)
        : m_minX(minX), m_maxX(maxX), m_sceneHeight(sceneHeight), m_ballRadius(ballRadius) {
    setZValue(10);
}

QRectF AimLineItem::boundingRect() const {
    return QRectF(m_minX, 0, m_maxX - m_minX, m_sceneHeight);
}

void AimLineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    if (m_points.size() < 2) return;

    painter->setRenderHint(QPainter::Antialiasing);

    QPen pen(QColor(56, 189, 248, 200), 2.5, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen);

    for (int i = 0; i < m_points.size() - 1; ++i) {
        painter->drawLine(m_points[i], m_points[i + 1]);
    }

    if (!m_points.isEmpty()) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(0, 242, 254));
        painter->drawEllipse(m_points.last(), 4.5, 4.5);
    }
}

void AimLineItem::clearAim() {
    m_points.clear();
    update();
}

void AimLineItem::updateAim(const QPointF& startPos, qreal angleDeg) {
    m_points.clear();
    m_points.append(startPos);

    qreal rad = angleDeg * M_PI / 180.0;
    qreal dx = std::cos(rad);
    qreal dy = -std::sin(rad);

    QPointF current = startPos;
    const int maxBounces = 2;

    for (int bounce = 0; bounce <= maxBounces; ++bounce) {
        qreal targetX = (dx > 0) ? (m_maxX - m_ballRadius) : (m_minX + m_ballRadius);
        qreal tX = (std::abs(dx) > 1e-6) ? (targetX - current.x()) / dx : 1e9;
        qreal tY = (dy < 0) ? (m_ballRadius - current.y()) / dy : 1e9;

        if (tX > 0 && tX < tY) {
            current += QPointF(dx * tX, dy * tX);
            m_points.append(current);
            dx = -dx;
        } else if (tY > 0) {
            current += QPointF(dx * tY, dy * tY);
            m_points.append(current);
            break;
        } else {
            break;
        }
    }
    update();
}