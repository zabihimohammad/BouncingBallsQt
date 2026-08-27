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

GridManager::~GridManager() {
    clearGrid();
}

void GridManager::clearGrid() {
    for (int r = 0; r < ROWS; ++r) {
        for (size_t c = 0; c < m_grid[r].size(); ++c) {
            if (m_grid[r][c]) {
                delete m_grid[r][c];
                m_grid[r][c] = nullptr;
            }
        }
    }
}

void GridManager::initGrid() {
    clearGrid();
}

BallColor GridManager::getStandardColor(int index) {
    switch (std::abs(index) % 5) {
        case 0: return BallColor::Red;
        case 1: return BallColor::Green;
        case 2: return BallColor::Blue;
        case 3: return BallColor::Yellow;
        case 4: return BallColor::Purple;
        default: return BallColor::Red;
    }
}

void GridManager::loadLevel(int levelNumber) {
    clearGrid();

    if (levelNumber == 1) {
        for (int r = 0; r < 4; ++r) {
            int cols = (r % 2 == 0) ? COLS_EVEN : COLS_ODD;
            for (int c = 0; c < cols; ++c) {
                BallColor col = (c % 3 == 0) ? BallColor::Red : ((c % 3 == 1) ? BallColor::Blue : BallColor::Green);
                m_grid[r][c] = new Ball(col, BallType::Regular, BallColor::None, r, c);
            }
        }
    } else if (levelNumber == 2) {
        for (int r = 0; r < 3; ++r) {
            int cols = (r % 2 == 0) ? COLS_EVEN : COLS_ODD;
            for (int c = 0; c < cols; ++c) {
                BallColor col = (r == 0) ? BallColor::Yellow : ((c % 2 == 0) ? BallColor::Purple : BallColor::Blue);
                m_grid[r][c] = new Ball(col, BallType::Regular, BallColor::None, r, c);
            }
        }
        for (int r = 3; r < 5; ++r) {
            int cols = (r % 2 == 0) ? COLS_EVEN : COLS_ODD;
            for (int c = 0; c < cols; ++c) {
                auto* b = new Ball(Ball::getRandomColor(4), BallType::Regular, BallColor::None, r, c);
                b->setFreezeLevel(r == 4 ? 2 : 1);
                m_grid[r][c] = b;
            }
        }
    } else if (levelNumber == 3) {
        for (int r = 0; r < 5; ++r) {
            int cols = (r % 2 == 0) ? COLS_EVEN : COLS_ODD;
            for (int c = 0; c < cols; ++c) {
                BallColor col = getStandardColor(r + c);
                auto* b = new Ball(col, BallType::Regular, BallColor::None, r, c);
                if (r <= 1 && (c == 0 || c == cols - 1)) {
                    b->setLocked(true);
                }
                m_grid[r][c] = b;
            }
        }
        if (m_grid[3][3]) {
            m_grid[3][3]->setKey(true);
            m_grid[3][3]->setPrimaryColor(BallColor::Yellow);
        }
    } else if (levelNumber == 4) {
        for (int r = 0; r < 5; ++r) {
            int cols = (r % 2 == 0) ? COLS_EVEN : COLS_ODD;
            for (int c = 0; c < cols; ++c) {
                if ((r == 1 && c == 3) || (r == 3 && (c == 2 || c == 5))) {
                    m_grid[r][c] = new Ball(BallColor::Black, BallType::Regular, BallColor::None, r, c);
                } else {
                    auto* b = new Ball(Ball::getRandomColor(5), BallType::Regular, BallColor::None, r, c);
                    if (r >= 3) b->setMystery(true);
                    m_grid[r][c] = b;
                }
            }
        }
    } else {
        for (int r = 0; r < 6; ++r) {
            int cols = (r % 2 == 0) ? COLS_EVEN : COLS_ODD;
            for (int c = 0; c < cols; ++c) {
                if (r == 2 && (c == 2 || c == cols - 2)) {
                    m_grid[r][c] = new Ball(BallColor::Black, BallType::Regular, BallColor::None, r, c);
                } else {
                    auto* b = new Ball(getStandardColor(r * 2 + c), BallType::Regular, BallColor::None, r, c);
                    if (r == 0 && (c == 2 || c == 5)) b->setLocked(true);
                    if (r == 4) b->setFreezeLevel(2);
                    if (r == 5) b->setMystery(true);
                    m_grid[r][c] = b;
                }
            }
        }
        if (m_grid[3][3]) {
            m_grid[3][3]->setKey(true);
            m_grid[3][3]->setPrimaryColor(BallColor::Yellow);
        }
    }
}

void GridManager::generateRandomLevel(int rows, int colorCount, int clusterChance, bool seedHazards) {
    clearGrid();
    auto rng = QRandomGenerator::global();
    bool hasLockedBall = false;

    for (int r = 0; r < rows; ++r) {
        int cols = (r % 2 == 0) ? COLS_EVEN : COLS_ODD;
        for (int c = 0; c < cols; ++c) {
            BallColor chosen = Ball::getRandomColor(colorCount);
            if (c > 0 && m_grid[r][c - 1] && rng->bounded(100) < clusterChance) {
                chosen = m_grid[r][c - 1]->getPrimaryColor();
            }

            auto* newBall = new Ball(chosen, BallType::Regular, BallColor::None, r, c);

            if (seedHazards && r >= 2) {
                int hChance = rng->bounded(100);
                if (hChance < 8) {
                    newBall->setPrimaryColor(BallColor::Black);
                } else if (hChance < 18) {
                    newBall->setFreezeLevel(rng->bounded(2) + 1);
                } else if (hChance < 26) {
                    newBall->setLocked(true);
                    hasLockedBall = true;
                }
            }

            m_grid[r][c] = newBall;
        }
    }

    if (hasLockedBall) {
        int targetRow = rows - 1;
        int targetCols = (targetRow % 2 == 0) ? COLS_EVEN : COLS_ODD;
        int centerCol = targetCols / 2;

        if (m_grid[targetRow][centerCol]) {
            auto* keyBall = m_grid[targetRow][centerCol];
            keyBall->setLocked(false);
            keyBall->setFreezeLevel(0);
            keyBall->setKey(true);
            keyBall->setPrimaryColor(BallColor::Yellow);
        }
    }
}

void GridManager::addRowFromTop(int wave, int colorCount, int clusterChance) {
    int lastRow = ROWS - 1;
    for (size_t c = 0; c < m_grid[lastRow].size(); ++c) {
        if (m_grid[lastRow][c]) {
            delete m_grid[lastRow][c];
            m_grid[lastRow][c] = nullptr;
        }
    }

    for (int r = ROWS - 1; r > 0; --r) {
        int targetCols = (r % 2 == 0) ? COLS_EVEN : COLS_ODD;
        int srcCols = ((r - 1) % 2 == 0) ? COLS_EVEN : COLS_ODD;

        for (int c = 0; c < targetCols; ++c) {
            if (c < srcCols) {
                m_grid[r][c] = m_grid[r - 1][c];
                if (m_grid[r][c]) {
                    m_grid[r][c]->setGridPos(r, c);
                }
            } else {
                m_grid[r][c] = nullptr;
            }
        }
    }

    auto rng = QRandomGenerator::global();
    bool hasLockedBallInRow = false;

    for (int c = 0; c < COLS_EVEN; ++c) {
        int chance = rng->bounded(100);
        Ball* newBall = nullptr;

        if (wave >= 3 && chance < (3 + wave)) {
            newBall = new Ball(BallColor::Black, BallType::Regular, BallColor::None, 0, c);
        } else if (wave >= 2 && chance < (8 + wave * 2)) {
            newBall = new Ball(Ball::getRandomColor(colorCount), BallType::Regular, BallColor::None, 0, c, true);
            hasLockedBallInRow = true;
        } else if (wave >= 2 && chance < (18 + wave * 3)) {
            newBall = new Ball(Ball::getRandomColor(colorCount), BallType::Regular, BallColor::None, 0, c);
            newBall->setFreezeLevel(rng->bounded(2) + 1);
        } else if (chance < 32) {
            newBall = new Ball(Ball::getRandomColor(colorCount), BallType::Regular, BallColor::None, 0, c);
            int skillR = rng->bounded(3);
            if (skillR == 0) newBall->setContainedSkill(BallType::Bomb);
            else if (skillR == 1) newBall->setContainedSkill(BallType::Laser);
            else newBall->setContainedSkill(BallType::Rainbow);
        } else {
            BallColor col = Ball::getRandomColor(colorCount);
            if (c > 0 && m_grid[0][c - 1] && rng->bounded(100) < clusterChance) {
                col = m_grid[0][c - 1]->getPrimaryColor();
            }
            newBall = new Ball(col, BallType::Regular, BallColor::None, 0, c);
        }

        m_grid[0][c] = newBall;
    }

    if (hasLockedBallInRow) {
        int keyCol = rng->bounded(COLS_EVEN);
        if (m_grid[0][keyCol] && !m_grid[0][keyCol]->isBlack()) {
            m_grid[0][keyCol]->setLocked(false);
            m_grid[0][keyCol]->setFreezeLevel(0);
            m_grid[0][keyCol]->setKey(true);
            m_grid[0][keyCol]->setPrimaryColor(BallColor::Yellow);
        }
    }
}

bool GridManager::isValidCoord(int r, int c) const {
    if (r < 0 || r >= ROWS) return false;
    int cols = (r % 2 == 0) ? COLS_EVEN : COLS_ODD;
    return (c >= 0 && c < cols);
}

bool GridManager::isOccupied(int r, int c) const {
    if (!isValidCoord(r, c)) return false;
    return m_grid[r][c] != nullptr;
}

Ball* GridManager::getBall(int r, int c) {
    if (!isOccupied(r, c)) return nullptr;
    return m_grid[r][c];
}

const Ball* GridManager::getBall(int r, int c) const {
    if (!isOccupied(r, c)) return nullptr;
    return m_grid[r][c];
}

bool GridManager::setBall(int r, int c, Ball* ball) {
    if (!isValidCoord(r, c)) return false;
    m_grid[r][c] = ball;
    if (ball) {
        ball->setGridPos(r, c);
    }
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
        x += BALL_RADIUS;
    }
    qreal y = BALL_RADIUS + r * (BALL_DIAMETER * 0.866025);
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

    for (const auto& d : deltas) {
        int nr = r + d.first;
        int nc = c + d.second;
        if (isValidCoord(nr, nc)) {
            neighbors.push_back({nr, nc});
        }
    }
    return neighbors;
}

std::vector<std::pair<int, int>> GridManager::findMatches(int startR, int startC, BallColor color, BallType type, BallColor secondaryColor) {
    if (!isOccupied(startR, startC)) return {};

    Ball* startBall = m_grid[startR][startC];
    BallType matchType = (color != BallColor::None) ? type : startBall->getType();
    BallColor c1 = (color != BallColor::None) ? color : startBall->getPrimaryColor();
    BallColor c2 = (color != BallColor::None) ? secondaryColor : startBall->getSecondaryColor();

    auto findConnectedForColor = [&](BallColor targetColor) -> std::vector<std::pair<int, int>> {
        if (targetColor == BallColor::None) return {};

        std::vector<std::pair<int, int>> matched;
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
                    Ball* nb = m_grid[nr][nc];
                    if (!nb->isLocked() && !nb->isBlack() && !nb->isFrozen()) {
                        bool matches = (nb->getPrimaryColor() == targetColor) ||
                                       (nb->getType() == BallType::DualColor && nb->getSecondaryColor() == targetColor) ||
                                       (nb->getType() == BallType::Rainbow);
                        if (matches) {
                            visited.insert({nr, nc});
                            q.push({nr, nc});
                        }
                    }
                }
            }
        }
        return matched;
    };

    if (matchType == BallType::Rainbow) {
        std::set<std::pair<int, int>> allMatched;
        allMatched.insert({startR, startC});
        for (auto [nr, nc] : getNeighbors(startR, startC)) {
            if (isOccupied(nr, nc)) {
                Ball* nb = m_grid[nr][nc];
                if (!nb->isLocked() && !nb->isBlack() && !nb->isFrozen()) {
                    auto sub = findConnectedForColor(nb->getPrimaryColor());
                    if (sub.size() >= 2) {
                        for (auto p : sub) allMatched.insert(p);
                    }
                }
            }
        }
        if (allMatched.size() >= 3) {
            return std::vector<std::pair<int, int>>(allMatched.begin(), allMatched.end());
        }
        return {};
    }

    if (matchType == BallType::DualColor) {
        std::set<std::pair<int, int>> allMatched;
        auto m1 = findConnectedForColor(c1);
        if (m1.size() >= 3) {
            for (auto p : m1) allMatched.insert(p);
        }
        if (c2 != BallColor::None && c2 != c1) {
            auto m2 = findConnectedForColor(c2);
            if (m2.size() >= 3) {
                for (auto p : m2) allMatched.insert(p);
            }
        }
        return std::vector<std::pair<int, int>>(allMatched.begin(), allMatched.end());
    }

    auto m = findConnectedForColor(c1);
    if (m.size() >= 3) {
        return m;
    }
    return {};
}

std::vector<std::pair<int, int>> GridManager::findFloatingBalls() {
    std::set<std::pair<int, int>> connectedToTop;
    std::queue<std::pair<int, int>> q;

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

std::vector<std::pair<int, int>> GridManager::explodeBomb(int centerR, int centerC) {
    std::vector<std::pair<int, int>> destroyed;
    if (isValidCoord(centerR, centerC) && isOccupied(centerR, centerC)) {
        destroyed.push_back({centerR, centerC});
    }

    for (const auto& nb : getNeighbors(centerR, centerC)) {
        if (isOccupied(nb.first, nb.second)) {
            destroyed.push_back(nb);
        }
    }
    return destroyed;
}

std::vector<std::pair<int, int>> GridManager::damageNeighbors(int r, int c, bool keyPopped) {
    std::vector<std::pair<int, int>> affected;
    for (const auto& nb : getNeighbors(r, c)) {
        if (isOccupied(nb.first, nb.second)) {
            Ball* b = m_grid[nb.first][nb.second];
            if (b) {
                if (b->damageIce()) affected.push_back(nb);
                if (b->isMystery()) {
                    b->revealMystery();
                    affected.push_back(nb);
                }
            }
        }
    }

    if (keyPopped) {
        unlockAllChainedBalls();
    }
    return affected;
}

void GridManager::unlockAllChainedBalls() {
    for (int r = 0; r < ROWS; ++r) {
        int cols = (r % 2 == 0) ? COLS_EVEN : COLS_ODD;
        for (int c = 0; c < cols; ++c) {
            if (isOccupied(r, c) && m_grid[r][c]->isLocked()) {
                m_grid[r][c]->unlock();
            }
        }
    }
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
            if (m_grid[r][c] != nullptr) return false;
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
                Ball* b = m_grid[r][c];
                BallColor col = b->getPrimaryColor();
                if (col != BallColor::Black && col != BallColor::None) {
                    colors.insert(col);
                }
                if (b->getType() == BallType::DualColor && b->getSecondaryColor() != BallColor::None) {
                    colors.insert(b->getSecondaryColor());
                }
            }
        }
    }
    return std::vector<BallColor>(colors.begin(), colors.end());
}

std::map<BallColor, int> GridManager::getColorDistribution() const {
    std::map<BallColor, int> counts;
    for (int r = 0; r < ROWS; ++r) {
        int cols = (r % 2 == 0) ? COLS_EVEN : COLS_ODD;
        for (int c = 0; c < cols; ++c) {
            if (m_grid[r][c]) {
                Ball* b = m_grid[r][c];
                BallColor col = b->getPrimaryColor();
                if (col != BallColor::Black && col != BallColor::None) {
                    counts[col]++;
                }
                if (b->getType() == BallType::DualColor && b->getSecondaryColor() != BallColor::None) {
                    counts[b->getSecondaryColor()]++;
                }
            }
        }
    }
    return counts;
}