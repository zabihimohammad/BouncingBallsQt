#pragma once
#include <QGraphicsItem>
#include <QPainter>
#include <QVector>
#include <QPointF>

class AimLineItem : public QGraphicsItem {
public:
    AimLineItem(qreal sceneWidth, qreal sceneHeight);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    void updateAim(const QPointF& startPos, qreal angleDeg);

private:
    qreal m_sceneWidth;
    qreal m_sceneHeight;
    QVector<QPointF> m_points;
};
