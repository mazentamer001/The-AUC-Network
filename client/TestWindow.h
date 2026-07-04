#pragma once
#include <QWidget>
#include <QStackedWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QSplitter>
#include <boost/asio.hpp>
#include "Client.h"

class TestWindow : public QWidget {
    Q_OBJECT
public:
    explicit TestWindow(QWidget* parent = nullptr);
private slots:
    void onRegister();
    void onLogin();
    void onSendMessage();
    void onCreateRoom();
    void onJoinRoom();
    void showLogin();
    void showRegister();
private:
    void setupRegisterPage();
    void setupLoginPage();
    void setupChatPage();
    void log(const QString& msg);
    void onMessage(const Message& msg);
    void appendChat(const QString& room, const QString& sender, const QString& text);

    Client*         client_;
    QStackedWidget* stack_;

    // register page
    QWidget*   registerPage_;
    QLineEdit* regUsername_;
    QLineEdit* regDisplayName_;
    QLineEdit* regEmail_;
    QLineEdit* regPassword_;
    QLineEdit* regUniId_;
    QLineEdit* regBio_;

    // login page
    QWidget*   loginPage_;
    QLineEdit* loginUsername_;
    QLineEdit* loginPassword_;

    // chat page
    QWidget*     chatPage_;
    QListWidget* roomList_;
    QTextEdit*   chatView_;
    QLineEdit*   messageInput_;
    QLineEdit*   roomIdInput_;
    QLabel*      currentRoomLabel_;
    QString      currentRoom_;

    // shared log
    QTextEdit* logView_;

    QString token_;
    QString userId_;
    QString displayName_;
};