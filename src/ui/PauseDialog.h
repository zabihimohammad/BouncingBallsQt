#pragma once
#include <QDialog>

class PauseDialog : public QDialog {
    Q_OBJECT
public:
    PauseDialog(QWidget* parent = nullptr);

signals:
    void resumeGame();
    void exitToMenu();
};
