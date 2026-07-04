#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QTextEdit;
class QLineEdit;
class QPushButton;
class QLabel;
class Client;
struct Message;

class MainWindow : public QMainWindow
{
public:
    explicit MainWindow(Client* client, QWidget* parent = nullptr);

private:
    Client* client;
    QTextEdit* chatBox;
    QLineEdit* usernameInput;
    QLineEdit* roomInput;
    QLineEdit* messageInput;
    QPushButton* sendButton;
    QLabel* statusLabel;

    void setupUi();
    void connectClient();
    void sendMessage();
    void showMessage(const Message& msg);
};

#endif
