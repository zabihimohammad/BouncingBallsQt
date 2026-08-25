#pragma once

#include <QString>
#include <QVector>
#include <QJsonObject>
#include <QDateTime>

struct ScoreRecord {
    QString username;
    QString mode;
    int score = 0;
    QString timestamp; // فرمت استاندارد: YYYY-MM-DD HH:MM

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["username"] = username;
        obj["mode"] = mode;
        obj["score"] = score;
        obj["timestamp"] = timestamp.isEmpty() ? QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm") : timestamp;
        return obj;
    }

    static ScoreRecord fromJson(const QJsonObject& obj) {
        ScoreRecord rec;
        rec.username = obj["username"].toString("Unknown");
        rec.mode = obj["mode"].toString("Classic");
        rec.score = obj["score"].toInt(0);
        rec.timestamp = obj["timestamp"].toString(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm"));
        return rec;
    }
};

class ScoreManager {
public:
    explicit ScoreManager(const QString& filePath = "scores.json");

    void loadScores();
    void saveScore(const QString& username, const QString& mode, int score);
    void saveScore(const ScoreRecord& record);
    void clearScores();

    // متدهای تحلیلی و دسترسی به رکوردها
    QVector<ScoreRecord> getScores() const;
    QVector<ScoreRecord> getScoresByMode(const QString& mode) const;
    QVector<ScoreRecord> getTopScores(const QString& mode = "", int limit = 10) const;

    int getHighestScore(const QString& mode = "") const;
    bool isHighScore(const QString& mode, int score) const;

private:
    QString m_filePath;
    QVector<ScoreRecord> m_records;

    void writeToFile();
    void sortRecords();
};