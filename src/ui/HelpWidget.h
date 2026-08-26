#pragma once
#include <QWidget>
#include <QMovie>
#include <QPointF>
#include <QRectF>
#include <QVector>

class HelpWidget : public QWidget {
    Q_OBJECT
public:
    explicit HelpWidget(QWidget* parent = nullptr);
    ~HelpWidget();

    Q_INVOKABLE void pauseAnimation();
    Q_INVOKABLE void resumeAnimation();

signals:
    void backClicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private slots:
    void switchTab(int index);

private:
    void drawBackground(QPainter& p);
    void drawMenu(QPainter& p);
    void drawContent(QPainter& p);
    void drawNeonBackButton(QPainter& p);

    int m_activeTab = 0; 
    int m_hoveredTab = -1;
    
    QRectF m_backBtnRect;
    bool m_backHovered = false;
    
    QVector<QRectF> m_tabRects;
    QMovie* m_movies[7];
};
