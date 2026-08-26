#pragma once

#include "Ball.h"
#include <vector>
#include <utility>
#include <QPointF>

class GridManager {
public:
    static const int ROWS = 14;
    static const int COLS_EVEN = 12; // ارتقا از ۸ به ۱۲ ستون
    static const int COLS_ODD = 11;  // ارتقا از ۷ به ۱۱ ستون
    static constexpr qreal BALL_RADIUS = 22.0;
    static constexpr qreal BALL_DIAMETER = BALL_RADIUS * 2.0; // 44.0 px

    GridManager();
    ~GridManager();

    GridManager(const GridManager&) = delete;
    GridManager& operator=(const GridManager&) = delete;

    void initGrid();
    void clearGrid();
    void loadLevel(int levelNumber);
    void generateRandomLevel();
    void addRowFromTop();

    bool isOccupied(int r, int c) const;
    Ball* getBall(int r, int c);
    const Ball* getBall(int r, int c) const;
    bool setBall(int r, int c, Ball* ball);
    void removeBall(int r, int c);

    QPointF getCenterPos(int r, int c) const;
    void getGridCoords(const QPointF& pos, int& outR, int& outC) const;
    std::vector<std::pair<int, int>> getNeighbors(int r, int c) const;

    std::vector<std::pair<int, int>> findMatches(int startR, int startC,
                                                 BallColor color = BallColor::None,
                                                 BallType type = BallType::Regular,
                                                 BallColor secondaryColor = BallColor::None);
    std::vector<std::pair<int, int>> findFloatingBalls();
    std::vector<std::pair<int, int>> explodeBomb(int centerR, int centerC);
    std::vector<std::pair<int, int>> damageNeighbors(int r, int c, bool keyPopped = false);
    void unlockAllChainedBalls();

    bool isBottomReached() const;
    bool isCleared() const;
    std::vector<BallColor> getRemainingColors() const;

private:
    std::vector<std::vector<Ball*>> m_grid;
    bool isValidCoord(int r, int c) const;
    static BallColor getStandardColor(int index);
};