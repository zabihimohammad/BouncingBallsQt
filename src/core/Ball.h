#pragma once

#include <QColor>
#include <QPointF>

enum class BallColor {
    None,
    Red,
    Green,
    Blue,
    Yellow,
    Purple,
    Black
};

enum class BallType {
    Regular,
    DualColor,
    Rainbow,
    Bomb,
    Laser
};

class Ball {
public:
    Ball(BallColor primaryColor = BallColor::None,
         BallType type = BallType::Regular,
         BallColor secondaryColor = BallColor::None,
         int row = -1,
         int col = -1,
         bool isLocked = false);

    BallColor getPrimaryColor() const { return m_primaryColor; }
    void setPrimaryColor(BallColor color) { m_primaryColor = color; }

    BallColor getSecondaryColor() const { return m_secondaryColor; }
    void setSecondaryColor(BallColor color) { m_secondaryColor = color; }

    BallType getType() const { return m_type; }
    void setType(BallType type) { m_type = type; }

    int getRow() const { return m_row; }
    int getCol() const { return m_col; }
    void setGridPos(int r, int c) { m_row = r; m_col = c; }

    QPointF getPosition() const { return m_position; }
    void setPosition(const QPointF &pos) { m_position = pos; }

    bool isLocked() const { return m_isLocked; }
    void setLocked(bool locked) { m_isLocked = locked; }
    void unlock() { m_isLocked = false; }

    bool isKey() const { return m_isKey; }
    void setKey(bool key) { m_isKey = key; }

    int getFreezeLevel() const { return m_freezeLevel; }
    void setFreezeLevel(int level) { m_freezeLevel = level; }
    bool isFrozen() const { return m_freezeLevel > 0; }
    bool damageIce() {
        if (m_freezeLevel > 0) {
            m_freezeLevel--;
            return true;
        }
        return false;
    }

    bool isMystery() const { return m_isMystery; }
    void setMystery(bool mystery) { m_isMystery = mystery; }
    void revealMystery() { m_isMystery = false; }

    bool isEmpty() const { return m_primaryColor == BallColor::None; }
    bool isBlack() const { return m_primaryColor == BallColor::Black; }

    bool matches(const Ball* other) const;
    bool matches(BallColor color, BallType type = BallType::Regular, BallColor secColor = BallColor::None) const;

    QColor getDisplayColor() const;
    static QColor toQColor(BallColor color);
    static BallColor getRandomColor(int count = 5);

private:
    BallColor m_primaryColor;
    BallColor m_secondaryColor;
    BallType m_type;
    int m_row;
    int m_col;
    bool m_isLocked;
    bool m_isKey = false;
    int m_freezeLevel = 0;
    bool m_isMystery = false;
    QPointF m_position;
};