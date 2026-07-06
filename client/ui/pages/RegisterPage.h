#pragma once
#include <QWidget>
#include "Message.h"

class QLineEdit;

class RegisterPage : public QWidget {
    Q_OBJECT
public:
    explicit RegisterPage(QWidget* parent = nullptr);
signals:
    void submitted(const Message& msg);
    void loginClicked();
    void backClicked();
private slots:
    void onSubmit();
private:
    QLineEdit* username_;
    QLineEdit* displayName_;
    QLineEdit* email_;
    QLineEdit* password_;
    QLineEdit* uniId_;
    QLineEdit* bio_;
};