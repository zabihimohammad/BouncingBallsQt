#pragma once
#include <QGraphicsItem>
#include <QVector>
#include <QPointF>
#include <QPainter>
#include <QTimer>
#include <QTime>

class AimLineItem : public QObject, public QGraphicsItem {
Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    AimLineItem(qreal minX = 224.0, qreal maxX = 576.0, qreal sceneHeight = 600.0, qreal ballRadius = 22.0);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    void updateAim(const QPointF& startPos, qreal angleDeg);
    void clearAim();
    void setMaxBounces(int bounces) { m_maxBounces = bounces; update(); }
    int getMaxBounces() const { return m_maxBounces; }

private:
    qreal m_minX;
    qreal m_maxX;
    qreal m_sceneHeight;
    qreal m_ballRadius;
    int m_maxBounces = 2;
    QVector<QPointF> m_points;
    QTimer* m_animTimer;
};