#pragma once
#include <QDialog>

class PauseDialog : public QDialog {
    Q_OBJECT
public:
    explicit PauseDialog(QWidget* parent = nullptr);
    ~PauseDialog() = default;

signals:
    void resumeGame();
    void openSettings();
    void exitToMenu();

protected:
    void paintEvent(QPaintEvent* event) override;
};
