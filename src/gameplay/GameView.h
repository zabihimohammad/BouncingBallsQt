#pragma once

#include <QGraphicsView>
#include <QResizeEvent>

class GameView : public QGraphicsView {
Q_OBJECT
public:
    explicit GameView(QWidget* parent = nullptr);

public slots:
    void triggerShake(int intensity);

private slots:
    void updateShake();

private:
    class QTimer* m_shakeTimer = nullptr;
    qreal m_currentShake = 0.0;
    
protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void resizeEvent(QResizeEvent* event) override;
};