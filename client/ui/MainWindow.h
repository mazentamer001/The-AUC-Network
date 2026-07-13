#pragma once
#include <QWidget>
#include <QStackedWidget>
#include <QThread>
#include <QCloseEvent>
#include <boost/asio.hpp>
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

protected:
    void closeEvent(QCloseEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void showHome();
    void showRegister();
    void showLogin();
    void showShell();
    void onMessage(const Message& msg);
    void sendToServer(const Message& msg);
    void onIdleTimeout();
    void sendStatusUpdate(const QString& status);

private:
    boost::asio::io_context* io_ = nullptr;
    QThread*        ioThread_ = nullptr;
    Client*        client_;
    QStackedWidget* stack_;

    HomePage* homePage_;
    RegisterPage* registerPage_;
    LoginPage* loginPage_;
    MainShell* mainShell_;

    QString token_;
    QString userId_;
    QString displayName_;
    QString username_;

    QTimer* idleTimer_ = nullptr;
    bool    isAway_    = false;

};
