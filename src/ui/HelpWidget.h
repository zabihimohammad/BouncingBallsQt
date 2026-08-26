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

private:
    void drawBackground(QPainter& p);
    void drawMenu(QPainter& p);
    void drawContent(QPainter& p);
    void drawNeonBackButton(QPainter& p);
    
    struct MenuItem {
        QString text;
        int type; // 0=Category, 1=Video
        int videoIndex;
        bool isExpanded;
        int id;
        QRectF rect;
    };
    QVector<MenuItem> m_menuItems;
    void buildMenuTree();

    int m_activeVideoIndex = 0; 
    int m_hoveredId = -1;
    
    QRectF m_backBtnRect;
    bool m_backHovered = false;
    
    QMovie* m_movies[7];
};
