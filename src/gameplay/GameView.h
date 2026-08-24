#pragma once

#include <QGraphicsView>
#include <QResizeEvent>

class GameView : public QGraphicsView {
Q_OBJECT
public:
    explicit GameView(QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent* event) override;
};