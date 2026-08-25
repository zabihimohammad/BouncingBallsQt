#pragma once

#include <QGraphicsItem>
#include <QPainter>
#include <QVector>
#include <QPointF>

class AimLineItem : public QGraphicsItem {
public:
    AimLineItem(qreal minX = 224.0, qreal maxX = 576.0, qreal sceneHeight = 600.0, qreal ballRadius = 22.0);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    void updateAim(const QPointF& startPos, qreal angleDeg);
    void clearAim();

private:
    qreal m_minX;
    qreal m_maxX;
    qreal m_sceneHeight;
    qreal m_ballRadius;
    QVector<QPointF> m_points;
};