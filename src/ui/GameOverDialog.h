#pragma once
#include <QDialog>
#include <QString>

class GameOverDialog : public QDialog {
    Q_OBJECT
public:
    GameOverDialog(bool won, const QString& username, int score, QWidget* parent = nullptr);

signals:
    void returnToMenu();
};
