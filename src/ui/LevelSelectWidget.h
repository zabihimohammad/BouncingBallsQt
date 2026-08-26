#pragma once

#include <QWidget>
#include <QVector>
#include <QRectF>
#include <QString>
#include <QColor>

struct LevelCard {
    int levelNumber;
    QString title;
    QString subtitle;
    QString difficulty;
    QColor themeColor;
    QRectF rect;
};

class LevelSelectWidget : public QWidget {
Q_OBJECT
public:
    explicit LevelSelectWidget(QWidget* parent = nullptr);

    Q_INVOKABLE void pauseAnimation() {}
    Q_INVOKABLE void resumeAnimation() { update(); }

signals:
    void levelSelected(int levelNumber);
    void backClicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void initCards();
    QVector<LevelCard> m_cards;
    QRectF m_backBtnRect;
    int m_hoveredIndex = -1;
    bool m_backHovered = false;
    QPointF m_mousePos;
};