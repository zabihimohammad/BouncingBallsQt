#pragma once
#include <QString>
#include <QVector>
#include <QPair>

struct ScoreRecord {
    QString username;
    QString mode;
    int score;
};

class ScoreManager {
public:
    ScoreManager(const QString& filePath = "scores.json");

    void loadScores();
    void saveScore(const QString& username, const QString& mode, int score);
    QVector<ScoreRecord> getScores() const;

private:
    QString m_filePath;
    QVector<ScoreRecord> m_records;
};
