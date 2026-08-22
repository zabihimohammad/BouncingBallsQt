#pragma once
#include <QGraphicsItem>
#include <QPainter>
#include "../core/Ball.h"

class CannonItem : public QGraphicsItem {
public:
    CannonItem(qreal width, qreal height);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    void setAngle(qreal angleDeg);
    qreal getAngle() const { return m_angle; }

    void setCurrentBall(BallColor c);
    BallColor getCurrentBall() const { return m_currentBall; }
    void setNextBall(BallColor c);
    BallColor getNextBall() const { return m_nextBall; }
    void swapBalls();

private:
    qreal m_width;
    qreal m_height;
    qreal m_angle = 90.0;
    BallColor m_currentBall = BallColor::RED;
    BallColor m_nextBall = BallColor::BLUE;
};
