#pragma once
#include <QWidget>
#include <QStackedWidget>
#include "Client.h"
#include "Message.h"

class HomePage;
class RegisterPage;
class LoginPage;
class MainShell;

class MainWindow : public QWidget {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void showHome();
    void showRegister();
    void showLogin();
    void showShell();
    void onMessage(const Message& msg);
    void sendToServer(const Message& msg);

private:
    Client* client_;
    QStackedWidget* stack_;

    HomePage* homePage_;
    RegisterPage* registerPage_;
    LoginPage* loginPage_;
    MainShell* mainShell_;

    QString token_;
    QString userId_;
    QString displayName_;
    QString username_;
};