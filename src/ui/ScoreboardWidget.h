#pragma once
#include <QWidget>
#include <QTableWidget>
#include "../core/ScoreManager.h"

class ScoreboardWidget : public QWidget {
    Q_OBJECT
public:
    ScoreboardWidget(ScoreManager* scoreMgr, QWidget* parent = nullptr);
    void refresh();

signals:
    void backClicked();

private:
    ScoreManager* m_scoreMgr;
    QTableWidget* m_table;
};
