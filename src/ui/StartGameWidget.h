#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QComboBox>

class StartGameWidget : public QWidget {
    Q_OBJECT
public:
    StartGameWidget(QWidget* parent = nullptr);

signals:
    void launchGame(const QString& username, const QString& mode);
    void backClicked();

private:
    QLineEdit* m_nameInput;
    QComboBox* m_modeCombo;
};
