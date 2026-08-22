#pragma once
#include "Ball.h"
#include <vector>
#include <QPointF>
#include <QSet>

class GridManager {
public:
    static const int ROWS = 14;
    static const int COLS_EVEN = 8;
    static const int COLS_ODD = 7;
    static constexpr qreal BALL_RADIUS = 22.0;
    static constexpr qreal BALL_DIAMETER = BALL_RADIUS * 2.0;

    GridManager();

    void initGrid();
    void loadLevel(int levelNumber);
    void generateRandomLevel();
    void addRowFromTop();

    bool isOccupied(int r, int c) const;
    Ball* getBall(int r, int c);
    bool setBall(int r, int c, Ball* ball);
    void removeBall(int r, int c);

    QPointF getCenterPos(int r, int c) const;
    void getGridCoords(const QPointF& pos, int& outR, int& outC) const;

    std::vector<std::pair<int, int>> getNeighbors(int r, int c) const;
    std::vector<std::pair<int, int>> findMatches(int startR, int startC, BallColor color);
    std::vector<std::pair<int, int>> findFloatingBalls();

    bool isBottomReached() const;
    bool isCleared() const;
    std::vector<BallColor> getRemainingColors() const;

private:
    std::vector<std::vector<Ball*>> m_grid;
};
