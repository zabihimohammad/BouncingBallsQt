#include "AimLineItem.h"
#include <cmath>

AimLineItem::AimLineItem(qreal sceneWidth, qreal sceneHeight)
    : m_sceneWidth(sceneWidth), m_sceneHeight(sceneHeight) {}

QRectF AimLineItem::boundingRect() const {
    return QRectF(0, 0, m_sceneWidth, m_sceneHeight);
}

void AimLineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    if (m_points.size() < 2) return;

    painter->setRenderHint(QPainter::Antialiasing);
    QPen pen(QColor(255, 255, 255, 180), 3, Qt::DashLine);
    painter->setPen(pen);

    for (int i = 0; i < m_points.size() - 1; ++i) {
        painter->drawLine(m_points[i], m_points[i + 1]);
    }
}

void AimLineItem::updateAim(const QPointF& startPos, qreal angleDeg) {
    m_points.clear();
    m_points.append(startPos);

    qreal rad = angleDeg * M_PI / 180.0;
    qreal dx = std::cos(rad);
    qreal dy = -std::sin(rad);

    QPointF current = startPos;
    for (int bounce = 0; bounce < 2; ++bounce) {
        // Raycast to walls
        qreal targetX = (dx > 0) ? (m_sceneWidth - 22) : 22;
        qreal tX = (dx != 0) ? (targetX - current.x()) / dx : 1e9;
        qreal tY = (dy < 0) ? (22 - current.y()) / dy : 1e9;

        if (tX < tY && tX > 0) {
            current += QPointF(dx * tX, dy * tX);
            m_points.append(current);
            dx = -dx; // Wall reflection
        } else if (tY > 0) {
            current += QPointF(dx * tY, dy * tY);
            m_points.append(current);
            break;
        }
    }
    update();
}
