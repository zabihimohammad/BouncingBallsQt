#pragma once
#include <QGraphicsView>

class GameView : public QGraphicsView {
    Q_OBJECT
public:
    GameView(QWidget* parent = nullptr);
};
