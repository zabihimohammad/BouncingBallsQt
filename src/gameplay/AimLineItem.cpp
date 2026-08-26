#include "../ui/ThemeManager.h"
#include "AimLineItem.h"
#include <cmath>

AimLineItem::AimLineItem(qreal minX, qreal maxX, qreal sceneHeight, qreal ballRadius)
        : m_minX(minX), m_maxX(maxX), m_sceneHeight(sceneHeight), m_ballRadius(ballRadius) {
    setZValue(10);
    m_animTimer = new QTimer(this);
    connect(m_animTimer, &QTimer::timeout, [this](){ update(); });
    m_animTimer->start(33); // ~30fps for pulse
}

QRectF AimLineItem::boundingRect() const {
    return QRectF(m_minX, 0, m_maxX - m_minX, m_sceneHeight);
}

void AimLineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    if (m_points.size() < 2) return;

    painter->setRenderHint(QPainter::Antialiasing);

    // Pulse calculation (0.0 to 1.0)
    qreal timePhase = (QTime::currentTime().msecsSinceStartOfDay() % 1000) / 1000.0;
    qreal pulse = (std::sin(timePhase * M_PI * 2.0) + 1.0) * 0.5; // 0 to 1

    // 1. Draw outer glow (thick, transparent)
    QColor glowColor = ThemeManager::instance().getPrimaryColor(); glowColor.setAlpha(80 + int(pulse * 60));
    painter->setPen(QPen(glowColor, 8.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    for (int i = 0; i < m_points.size() - 1; ++i) {
        painter->drawLine(m_points[i], m_points[i + 1]);
    }

    // 2. Draw inner core (thin, bright)
    QColor coreColor(200, 250, 255, 200 + int(pulse * 55));
    painter->setPen(QPen(coreColor, 2.5, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin)); // dashed inner for tech look
    for (int i = 0; i < m_points.size() - 1; ++i) {
        painter->drawLine(m_points[i], m_points[i + 1]);
    }

    // 3. Draw Target Reticle (Crosshair) at the end
    QPointF target = m_points.last();
    
    painter->save();
    painter->translate(target);
    painter->rotate(timePhase * 360.0); // slowly rotating crosshair

    painter->setPen(QPen(QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), 200), 2.0));
    painter->drawArc(QRectF(-12, -12, 24, 24), 0, 90 * 16);
    painter->drawArc(QRectF(-12, -12, 24, 24), 180 * 16, 90 * 16);
    
    painter->setPen(QPen(QColor(255, 255, 255, 255), 1.5));
    painter->drawLine(0, -6, 0, 6);
    painter->drawLine(-6, 0, 6, 0);

    painter->setBrush(QColor(ThemeManager::instance().getPrimaryColor().red(), ThemeManager::instance().getPrimaryColor().green(), ThemeManager::instance().getPrimaryColor().blue(), 150));
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QPointF(0,0), 3, 3);
    
    painter->restore();
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
