#include "ScoreManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>
#include <algorithm>

ScoreManager::ScoreManager(const QString& filePath) : m_filePath(filePath) {
    loadScores();
}

void ScoreManager::sortRecords() {
    std::stable_sort(m_records.begin(), m_records.end(), [](const ScoreRecord& a, const ScoreRecord& b) {
        return a.score > b.score;
    });
}

void ScoreManager::loadScores() {
    m_records.clear();
    QFile file(m_filePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isArray()) {
        return;
    }

    QJsonArray arr = doc.array();
    for (const auto& val : arr) {
        if (val.isObject()) {
            m_records.append(ScoreRecord::fromJson(val.toObject()));
        }
    }

    sortRecords();
}

void ScoreManager::writeToFile() {
    QJsonArray arr;
    for (const auto& rec : m_records) {
        arr.append(rec.toJson());
    }

    QFile file(m_filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
        file.close();
    }
}

void ScoreManager::saveScore(const QString& username, const QString& mode, int score) {
    ScoreRecord rec;
    rec.username = username.trimmed().isEmpty() ? "Player" : username.trimmed();
    rec.mode = mode;
    rec.score = score;
    rec.timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm");

    saveScore(rec);
}

void ScoreManager::saveScore(const ScoreRecord& record) {
    m_records.append(record);
    sortRecords();
    writeToFile();
}

void ScoreManager::clearScores() {
    m_records.clear();
    writeToFile();
}

QVector<ScoreRecord> ScoreManager::getScores() const {
    return m_records;
}

QVector<ScoreRecord> ScoreManager::getScoresByMode(const QString& mode) const {
    if (mode.isEmpty() || mode == "All") {
        return m_records;
    }

    QVector<ScoreRecord> filtered;
    for (const auto& rec : m_records) {
        if (rec.mode.compare(mode, Qt::CaseInsensitive) == 0) {
            filtered.append(rec);
        }
    }
    return filtered;
}

QVector<ScoreRecord> ScoreManager::getTopScores(const QString& mode, int limit) const {
    QVector<ScoreRecord> baseList = getScoresByMode(mode);
    if (baseList.size() > limit) {
        baseList.resize(limit);
    }
    return baseList;
}

int ScoreManager::getHighestScore(const QString& mode) const {
    auto top = getScoresByMode(mode);
    if (top.isEmpty()) return 0;
    return top.first().score;
}

bool ScoreManager::isHighScore(const QString& mode, int score) const {
    if (score <= 0) return false;
    return score > getHighestScore(mode);
}