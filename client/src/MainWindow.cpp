#include "MainWindow.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QMetaObject>
#include <QDateTime>

#include "Client.h"
#include "Message.h"

MainWindow::MainWindow(Client* client, QWidget* parent)
    : QMainWindow(parent), client(client)
{
    setupUi();
    connectClient();
}

void MainWindow::setupUi()
{
    setWindowTitle("The AUC Network");
    resize(700, 500);

    QWidget* central = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(central);

    QLabel* title = new QLabel("The AUC Network Chat");
    title->setStyleSheet("font-size: 24px; font-weight: bold;");

    statusLabel = new QLabel("Connecting...");
    statusLabel->setStyleSheet("color: orange;");

    QHBoxLayout* topLayout = new QHBoxLayout();

    usernameInput = new QLineEdit();
    usernameInput->setPlaceholderText("Username");
    usernameInput->setText("Student");

    roomInput = new QLineEdit();
    roomInput->setPlaceholderText("Room");
    roomInput->setText("room_general");

    topLayout->addWidget(usernameInput);
    topLayout->addWidget(roomInput);

    chatBox = new QTextEdit();
    chatBox->setReadOnly(true);
    chatBox->setPlaceholderText("Chat messages will appear here...");

    QHBoxLayout* bottomLayout = new QHBoxLayout();

    messageInput = new QLineEdit();
    messageInput->setPlaceholderText("Type your message here...");

    sendButton = new QPushButton("Send");
    sendButton->setEnabled(false);

    bottomLayout->addWidget(messageInput);
    bottomLayout->addWidget(sendButton);

    layout->addWidget(title);
    layout->addWidget(statusLabel);
    layout->addLayout(topLayout);
    layout->addWidget(chatBox);
    layout->addLayout(bottomLayout);

    setCentralWidget(central);

    connect(sendButton, &QPushButton::clicked, this, [this]() {
        sendMessage();
    });

    connect(messageInput, &QLineEdit::returnPressed, this, [this]() {
        sendMessage();
    });
}

void MainWindow::connectClient()
{
    client->setOnConnected([this]() {
        QMetaObject::invokeMethod(this, [this]() {
            statusLabel->setText("Connected");
            statusLabel->setStyleSheet("color: green;");
            sendButton->setEnabled(true);
            chatBox->append("<b>[System]</b> Connected to server.");
        }, Qt::QueuedConnection);
    });

    client->setOnMessage([this](const Message& msg) {
        QMetaObject::invokeMethod(this, [this, msg]() {
            showMessage(msg);
        }, Qt::QueuedConnection);
    });
}

void MainWindow::sendMessage()
{
    QString text = messageInput->text().trimmed();

    if (text.isEmpty()) {
        QMessageBox::warning(this, "Empty Message", "Please type a message first.");
        return;
    }

    Message msg;
    msg.type = MessageType::CHAT_PUBLIC;
    msg.sender.userId = "u001";
    msg.sender.username = usernameInput->text().trimmed();
    msg.sender.role = "student";
    msg.roomId = roomInput->text().trimmed();
    msg.text = text;
    msg.mediaUrl = "";
    msg.timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);

    if (msg.sender.username.isEmpty())
        msg.sender.username = "Student";

    if (msg.roomId.isEmpty())
        msg.roomId = "room_general";

    client->send(msg);

    chatBox->append("<b>[Me]</b> " + text);
    messageInput->clear();
}

void MainWindow::showMessage(const Message& msg)
{
    chatBox->append("<b>[" + msg.roomId + "] " + msg.sender.username + ":</b> " + msg.text);
}
