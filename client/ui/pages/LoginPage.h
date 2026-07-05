#pragma once
#include <QWidget>
#include "Message.h"

class QLineEdit;

class LoginPage : public QWidget {
    Q_OBJECT
public:
    explicit LoginPage(QWidget* parent = nullptr);
signals:
    void submitted(const Message& msg);
    void registerClicked();
    void backClicked();
protected:
    void paintEvent(QPaintEvent*) override;
private slots:
    void onSubmit();
private:
    QLineEdit* username_;
    QLineEdit* password_;
};