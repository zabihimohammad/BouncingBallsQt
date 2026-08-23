#pragma once
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QVector>
#include <QPointF>
#include <QColor>
#include <QBoxLayout>

struct MenuBall {
    QPointF pos;
    QPointF velocity;
    qreal radius;
    qreal mass; // جرم توپ برای محاسبه فیزیک برخورد
    QColor color;
    bool isBackground;
};

struct GasBubble {
    QPointF pos;
    qreal speed;
    qreal size;
    qreal wobblePhase;
};

class MainMenuWidget : public QWidget {
    Q_OBJECT
public:
    explicit MainMenuWidget(QWidget* parent = nullptr);

    signals:
        void startGameClicked();
    void scoreboardClicked();
    void settingsClicked();
    void exitClicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void updateAnimation();

private:
    void setupUI();
    void initBalls();
    void initBubbles();
    void resetBallsToCorners(int w, int h); // شلیک مجدد از گوشه‌ها

    QTimer* m_animTimer;
    QVector<MenuBall> m_balls;
    QVector<GasBubble> m_bubbles;
    QPointF m_mousePos;
    
    QBoxLayout* m_buttonsLayout;
    QLabel* m_titleLabel; // برای تغییر سایز دینامیک
    qreal m_time = 0.0;
};