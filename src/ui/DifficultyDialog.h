#pragma once
#include <QDialog>
#include <QString>
#include <QRectF>
#include <QVector>

struct DiffOption {
    int id;
    QString title;
    QString subtitle;
    QString details;
    QString multiplier;
    QColor color;
    QRectF rect;
};

class DifficultyDialog : public QDialog {
Q_OBJECT
public:
    explicit DifficultyDialog(QWidget* parent = nullptr);
    int getSelectedDifficulty() const { return m_selectedDifficulty; }

signals:
    void difficultySelected(int diff);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    int m_selectedDifficulty = 1;
    int m_hoveredIndex = -1;
    QPointF m_mousePos;
    QVector<DiffOption> m_options;
    QRectF m_cancelBtnRect;
    bool m_cancelHovered = false;
};