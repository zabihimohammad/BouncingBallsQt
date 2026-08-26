#pragma once
#include <QGraphicsObject>
#include <QPainter>
#include "../core/Ball.h"
#include "../core/GridManager.h"

class BallItem : public QGraphicsObject {
    Q_OBJECT
public:
    BallItem(BallColor color, BallType type, BallColor secColor, bool locked, qreal radius, QGraphicsItem* parent = nullptr);
    BallItem(const Ball* ball, qreal radius, QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    void updateData(BallColor color, BallType type, BallColor secColor = BallColor::None, bool locked = false);

    static void paintBall(QPainter* painter, const QPointF& center, qreal radius, BallColor color, BallType type, BallColor secColor = BallColor::None, bool locked = false);

private:
    BallColor m_color;
    BallType m_type;
    BallColor m_secColor;
    bool m_isLocked;
    qreal m_radius;
};
