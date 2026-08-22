#include "GridManager.h"
#include <cmath>
#include <queue>
#include <set>
#include <QRandomGenerator>

GridManager::GridManager() {
    m_grid.resize(ROWS);
    for (int r = 0; r < ROWS; ++r) {
        int cols = (r % 2 == 0) ? COLS_EVEN : COLS_ODD;
        m_grid[r].resize(cols, nullptr);
    }
}

void GridManager::initGrid() {
    for (int r = 0; r < ROWS; ++r) {
        for (size_t c = 0; c < m_grid[r].size(); ++c) {
            delete m_grid[r][c];
            m_grid[r][c] = nullptr;
        }
    }
}

void GridManager::loadLevel(int levelNumber) {
    initGrid();
    int rowsToFill = 5;
    for (int r = 0; r < rowsToFill; ++r) {
        int cols = (r % 2 == 0) ? COLS_EVEN : COLS_ODD;
        for (int c = 0; c < cols; ++c) {
            BallColor col = static_cast<BallColor>((c + r + levelNumber) % 5);
            m_grid[r][c] = new Ball(col, r, c);
        }
    }
}

void GridManager::generateRandomLevel() {
    initGrid();
    for (int r = 0; r < 5; ++r) {
        int cols = (r % 2 == 0) ? COLS_EVEN : COLS_ODD;
        for (int c = 0; c < cols; ++c) {
            // Cluster logic: higher probability of neighbor color
            BallColor chosen = Ball::getRandomColor(5);
            if (c > 0 && m_grid[r][c - 1] && QRandomGenerator::global()->bounded(100) < 65) {
                chosen = m_grid[r][c - 1]->getColor();
            }
            m_grid[r][c] = new Ball(chosen, r, c);
        }
    }
}

void GridManager::addRowFromTop() {
    for (int r = ROWS - 1; r > 0; --r) {
        int currentCols = (r % 2 == 0) ? COLS_EVEN : COLS_ODD;
        int prevCols = ((r - 1) % 2 == 0) ? COLS_EVEN : COLS_ODD;
        for (int c = 0; c < currentCols; ++c) {
            if (c < prevCols) {
                m_grid[r][c] = m_grid[r - 1][c];
                if (m_grid[r][c]) m_grid[r][c]->setGridPos(r, c);
            } else {
                m_grid[r][c] = nullptr;
            }
        }
    }
    // New row at 0
    int cols0 = COLS_EVEN;
    for (int c = 0; c < cols0; ++c) {
        m_grid[0][c] = new Ball(Ball::getRandomColor(5), 0, c);
    }
}

bool GridManager::isOccupied(int r, int c) const {
    if (r < 0 || r >= ROWS) return false;
    int cols = (r % 2 == 0) ? COLS_EVEN : COLS_ODD;
    if (c < 0 || c >= cols) return false;
    return m_grid[r][c] != nullptr;
}

Ball* GridManager::getBall(int r, int c) {
    if (!isOccupied(r, c)) return nullptr;
    return m_grid[r][c];
}

bool GridManager::setBall(int r, int c, Ball* ball) {
    if (r < 0 || r >= ROWS) return false;
    int cols = (r % 2 == 0) ? COLS_EVEN : COLS_ODD;
    if (c < 0 || c >= cols) return false;
    m_grid[r][c] = ball;
    if (ball) ball->setGridPos(r, c);
    return true;
}

void GridManager::removeBall(int r, int c) {
    if (isOccupied(r, c)) {
        delete m_grid[r][c];
        m_grid[r][c] = nullptr;
    }
}

QPointF GridManager::getCenterPos(int r, int c) const {
    qreal x = BALL_RADIUS + c * BALL_DIAMETER;
    if (r % 2 != 0) {
        x += BALL_RADIUS; // Odd row offset
    }
    qreal y = BALL_RADIUS + r * (BALL_DIAMETER * 0.866025); // sqrt(3)/2
    return QPointF(x, y);
}

void GridManager::getGridCoords(const QPointF& pos, int& outR, int& outC) const {
    qreal rowHeight = BALL_DIAMETER * 0.866025;
    outR = std::round((pos.y() - BALL_RADIUS) / rowHeight);
    if (outR < 0) outR = 0;
    if (outR >= ROWS) outR = ROWS - 1;

    qreal xOffset = (outR % 2 != 0) ? BALL_RADIUS : 0.0;
    outC = std::round((pos.x() - BALL_RADIUS - xOffset) / BALL_DIAMETER);
    int maxCols = (outR % 2 == 0) ? COLS_EVEN : COLS_ODD;
    if (outC < 0) outC = 0;
    if (outC >= maxCols) outC = maxCols - 1;
}

std::vector<std::pair<int, int>> GridManager::getNeighbors(int r, int c) const {
    std::vector<std::pair<int, int>> neighbors;
    bool even = (r % 2 == 0);
    std::vector<std::pair<int, int>> deltas = even ?
        std::vector<std::pair<int, int>>{{0, -1}, {0, 1}, {-1, -1}, {-1, 0}, {1, -1}, {1, 0}} :
        std::vector<std::pair<int, int>>{{0, -1}, {0, 1}, {-1, 0}, {-1, 1}, {1, 0}, {1, 1}};

    for (auto d : deltas) {
        int nr = r + d.first;
        int nc = c + d.second;
        if (nr >= 0 && nr < ROWS) {
            int cols = (nr % 2 == 0) ? COLS_EVEN : COLS_ODD;
            if (nc >= 0 && nc < cols) {
                neighbors.push_back({nr, nc});
            }
        }
    }
    return neighbors;
}

std::vector<std::pair<int, int>> GridManager::findMatches(int startR, int startC, BallColor color) {
    std::vector<std::pair<int, int>> matched;
    if (!isOccupied(startR, startC)) return matched;

    std::set<std::pair<int, int>> visited;
    std::queue<std::pair<int, int>> q;

    q.push({startR, startC});
    visited.insert({startR, startC});

    while (!q.empty()) {
        auto [r, c] = q.front();
        q.pop();
        matched.push_back({r, c});

        for (auto [nr, nc] : getNeighbors(r, c)) {
            if (isOccupied(nr, nc) && visited.find({nr, nc}) == visited.end()) {
                BallColor ncCol = m_grid[nr][nc]->getColor();
                if (ncCol == color || ncCol == BallColor::RAINBOW || color == BallColor::RAINBOW) {
                    visited.insert({nr, nc});
                    q.push({nr, nc});
                }
            }
        }
    }

    if (matched.size() < 3 && color != BallColor::RAINBOW) {
        return {};
    }
    return matched;
}

std::vector<std::pair<int, int>> GridManager::findFloatingBalls() {
    std::set<std::pair<int, int>> connectedToTop;
    std::queue<std::pair<int, int>> q;

    // Start BFS from row 0
    for (int c = 0; c < COLS_EVEN; ++c) {
        if (isOccupied(0, c)) {
            q.push({0, c});
            connectedToTop.insert({0, c});
        }
    }

    while (!q.empty()) {
        auto [r, c] = q.front();
        q.pop();

        for (auto [nr, nc] : getNeighbors(r, c)) {
            if (isOccupied(nr, nc) && connectedToTop.find({nr, nc}) == connectedToTop.end()) {
                connectedToTop.insert({nr, nc});
                q.push({nr, nc});
            }
        }
    }

    std::vector<std::pair<int, int>> floating;
    for (int r = 0; r < ROWS; ++r) {
        int cols = (r % 2 == 0) ? COLS_EVEN : COLS_ODD;
        for (int c = 0; c < cols; ++c) {
            if (isOccupied(r, c) && connectedToTop.find({r, c}) == connectedToTop.end()) {
                floating.push_back({r, c});
            }
        }
    }
    return floating;
}

bool GridManager::isBottomReached() const {
    int lastRow = ROWS - 1;
    int cols = (lastRow % 2 == 0) ? COLS_EVEN : COLS_ODD;
    for (int c = 0; c < cols; ++c) {
        if (isOccupied(lastRow, c)) return true;
    }
    return false;
}

bool GridManager::isCleared() const {
    for (int r = 0; r < ROWS; ++r) {
        int cols = (r % 2 == 0) ? COLS_EVEN : COLS_ODD;
        for (int c = 0; c < cols; ++c) {
            if (m_grid[r][c]) return false;
        }
    }
    return true;
}

std::vector<BallColor> GridManager::getRemainingColors() const {
    std::set<BallColor> colors;
    for (int r = 0; r < ROWS; ++r) {
        int cols = (r % 2 == 0) ? COLS_EVEN : COLS_ODD;
        for (int c = 0; c < cols; ++c) {
            if (m_grid[r][c]) {
                colors.insert(m_grid[r][c]->getColor());
            }
        }
    }
    return std::vector<BallColor>(colors.begin(), colors.end());
}
