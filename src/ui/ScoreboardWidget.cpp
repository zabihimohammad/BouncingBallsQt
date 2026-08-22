#include "ScoreboardWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>

ScoreboardWidget::ScoreboardWidget(ScoreManager* scoreMgr, QWidget* parent)
    : QWidget(parent), m_scoreMgr(scoreMgr) {
    auto layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);

    auto title = new QLabel("Leaderboard / Scores", this);
    title->setStyleSheet("font-size: 22px; font-weight: bold; color: #ecf0f1; margin-bottom: 10px;");
    layout->addWidget(title, 0, Qt::AlignCenter);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(3);
    m_table->setHorizontalHeaderLabels({"Player", "Mode", "Score"});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setStyleSheet("QTableWidget { background-color: #2c3e50; color: white; gridline-color: #34495e; }"
                           "QHeaderView::section { background-color: #1abc9c; color: white; font-weight: bold; }");
    m_table->setMinimumSize(320, 260);
    layout->addWidget(m_table);

    auto backBtn = new QPushButton("Back to Menu", this);
    backBtn->setStyleSheet("QPushButton { background-color: #7f8c8d; color: white; font-size: 14px; "
                           "padding: 8px 20px; border-radius: 6px; } QPushButton:hover { background-color: #95a5a6; }");
    connect(backBtn, &QPushButton::clicked, this, &ScoreboardWidget::backClicked);
    layout->addWidget(backBtn, 0, Qt::AlignCenter);

    refresh();
}

void ScoreboardWidget::refresh() {
    m_scoreMgr->loadScores();
    auto scores = m_scoreMgr->getScores();
    m_table->setRowCount(scores.size());
    for (int i = 0; i < scores.size(); ++i) {
        m_table->setItem(i, 0, new QTableWidgetItem(scores[i].username));
        m_table->setItem(i, 1, new QTableWidgetItem(scores[i].mode));
        m_table->setItem(i, 2, new QTableWidgetItem(QString::number(scores[i].score)));
    }
}
