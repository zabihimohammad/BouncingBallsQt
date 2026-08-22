#pragma once
#include <QColor>
#include <QGraphicsItem>
#include <QPainter>

enum class BallColor {
    RED,
    BLUE,
    GREEN,
    YELLOW,
    PURPLE,
    BLACK,       // Unpoppable ground ball
    DUAL,        // Bonus: Dual-color ball
    LOCKED,      // Bonus: Locked ball
    RAINBOW,     // Special: Wildcard
    BOMB,        // Special: Explosive
    LASER        // Special: Laser line clear
};

enum class TimeEffect {
    NONE,
    PAUSE,
    SLOW_MOTION
};

class Ball {
public:
    Ball(BallColor color = BallColor::RED, int row = -1, int col = -1);

    BallColor getColor() const { return m_color; }
    void setColor(BallColor c) { m_color = c; }

    int getRow() const { return m_row; }
    int getCol() const { return m_col; }
    void setGridPos(int r, int c) { m_row = r; m_col = c; }

    bool isLocked() const { return m_locked; }
    void unlock() { m_locked = false; }

    TimeEffect getTimeEffect() const { return m_effect; }
    void setTimeEffect(TimeEffect eff) { m_effect = eff; }

    static QColor toQColor(BallColor c);
    static BallColor getRandomColor(int colorCount = 5);

private:
    BallColor m_color;
    int m_row;
    int m_col;
    bool m_locked;
    TimeEffect m_effect;
};
