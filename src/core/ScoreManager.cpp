#include "ScoreManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <algorithm>

ScoreManager::ScoreManager(const QString& filePath) : m_filePath(filePath) {
    loadScores();
}

void ScoreManager::loadScores() {
    m_records.clear();
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) return;

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) return;

    QJsonArray arr = doc.array();
    for (auto val : arr) {
        QJsonObject obj = val.toObject();
        ScoreRecord rec;
        rec.username = obj["username"].toString();
        rec.mode = obj["mode"].toString();
        rec.score = obj["score"].toInt();
        m_records.append(rec);
    }
}

void ScoreManager::saveScore(const QString& username, const QString& mode, int score) {
    m_records.append({username, mode, score});
    std::sort(m_records.begin(), m_records.end(), [](const ScoreRecord& a, const ScoreRecord& b) {
        return a.score > b.score;
    });

    QJsonArray arr;
    for (const auto& rec : m_records) {
        QJsonObject obj;
        obj["username"] = rec.username;
        obj["mode"] = rec.mode;
        obj["score"] = rec.score;
        arr.append(obj);
    }

    QFile file(m_filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(arr).toJson());
        file.close();
    }
}

QVector<ScoreRecord> ScoreManager::getScores() const {
    return m_records;
}
