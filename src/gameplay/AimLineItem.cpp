#include "AimLineItem.h"
#include <cmath>

AimLineItem::AimLineItem(qreal sceneWidth, qreal sceneHeight, qreal ballRadius)
        : m_sceneWidth(sceneWidth), m_sceneHeight(sceneHeight), m_ballRadius(ballRadius) {
    setZValue(10); // قرارگیری روی تمام توپ‌ها برای دیده‌شدن خط نشانه
}

QRectF AimLineItem::boundingRect() const {
    return QRectF(0, 0, m_sceneWidth, m_sceneHeight);
}

void AimLineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    if (m_points.size() < 2) return;

    painter->setRenderHint(QPainter::Antialiasing);

    // خط‌چین نئونی با شفافیت ملایم
    QPen pen(QColor(241, 245, 249, 180), 2.5, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen);

    for (int i = 0; i < m_points.size() - 1; ++i) {
        painter->drawLine(m_points[i], m_points[i + 1]);
    }

    // رسم دایره کوچک در انتهای مسیر برای نمایش نقطه فرود
    if (!m_points.isEmpty()) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(56, 189, 248, 200));
        painter->drawEllipse(m_points.last(), 4.0, 4.0);
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
    const int maxBounces = 2; // حداکثر ۲ بار بازتاب از دیوارها

    for (int bounce = 0; bounce <= maxBounces; ++bounce) {
        qreal targetX = (dx > 0) ? (m_sceneWidth - m_ballRadius) : m_ballRadius;
        qreal tX = (std::abs(dx) > 1e-6) ? (targetX - current.x()) / dx : 1e9;
        qreal tY = (dy < 0) ? (m_ballRadius - current.y()) / dy : 1e9;

        if (tX > 0 && tX < tY) {
            // برخورد با دیواره جانبی و بازتاب بردار افقی
            current += QPointF(dx * tX, dy * tX);
            m_points.append(current);
            dx = -dx;
        } else if (tY > 0) {
            // برخورد با سقف زمین بازی
            current += QPointF(dx * tY, dy * tY);
            m_points.append(current);
            break;
        } else {
            break;
        }
    }
    update();
}