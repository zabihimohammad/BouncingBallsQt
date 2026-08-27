#pragma once
#include <QGraphicsObject>
#include <QPainter>
#include "../core/Ball.h"
#include "../core/GridManager.h"

class BallItem : public QGraphicsObject {
Q_OBJECT
public:
    BallItem(BallColor color = BallColor::None,
             BallType type = BallType::Regular,
             BallColor secColor = BallColor::None,
             bool locked = false,
             qreal radius = GridManager::BALL_RADIUS,
             int freezeLevel = 0,
             bool isMystery = false,
             bool isKey = false,
             BallType containedSkill = BallType::Regular,
             bool isTimeCrystal = false,
             bool isChronoBomb = false,
             qreal chronoTimer = 5.0,
             QGraphicsItem* parent = nullptr);

    BallItem(const Ball* ball, qreal radius = GridManager::BALL_RADIUS, QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    void updateData(BallColor color,
                    BallType type = BallType::Regular,
                    BallColor secColor = BallColor::None,
                    bool locked = false,
                    int freezeLevel = 0,
                    bool isMystery = false,
                    bool isKey = false,
                    BallType containedSkill = BallType::Regular,
                    bool isTimeCrystal = false,
                    bool isChronoBomb = false,
                    qreal chronoTimer = 5.0);

    static void paintBall(QPainter* painter,
                          const QPointF& center,
                          qreal radius,
                          BallColor color,
                          BallType type,
                          BallColor secColor = BallColor::None,
                          bool locked = false,
                          int freezeLevel = 0,
                          bool isMystery = false,
                          bool isKey = false,
                          BallType containedSkill = BallType::Regular,
                          bool isTimeCrystal = false,
                          bool isChronoBomb = false,
                          qreal chronoTimer = 5.0);

private:
    BallColor m_color;
    BallType m_type;
    BallColor m_secColor;
    bool m_isLocked;
    qreal m_radius;
    int m_freezeLevel;
    bool m_isMystery;
    bool m_isKey;
    BallType m_containedSkill;
    bool m_isTimeCrystal;
    bool m_isChronoBomb;
    qreal m_chronoTimer;
};