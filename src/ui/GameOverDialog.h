#pragma once
#include <QDialog>
#include <QString>

class GameOverDialog : public QDialog {
Q_OBJECT
public:
    GameOverDialog(bool won, const QString& username, int score, const QString& mode = "Classic", QWidget* parent = nullptr);

    int getStarsEarned() const { return m_stars; }
    int getMedalTier() const { return m_medalTier; }

signals:
    void returnToMenu();
    void restartGame();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    bool m_won;
    QString m_username;
    int m_score;
    QString m_mode;
    int m_stars = 0;
    int m_medalTier = 0; // 0: Bronze, 1: Silver, 2: Gold, 3: Platinum
};