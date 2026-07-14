#pragma once
#include <QWidget>
#include <QStackedWidget>
#include <QMap>
#include <QSet>
#include "Message.h"

class QListWidget;
class QTextEdit;
class QLineEdit;
class QLabel;

class ChatPanel : public QWidget {
    Q_OBJECT
public:
    explicit ChatPanel(QWidget* parent = nullptr);
    void openDirectRoom(const QString& roomId);
    void receiveMessage(const Message& msg);
    void setCurrentUser(const QString& displayName, const QString& userId);

signals:
    void messageSent(const Message& msg);
    void roomCreated(const Message& msg);
    void roomJoined (const Message& msg);

private slots:
    void onSend();
    void onCreateRoom();

private:
    void switchToRoom(const QString& room);
    void appendChat(const QString& room, const QString& sender, const QString& text);

    QListWidget*              roomList_;
    QStackedWidget*           chatStack_;
    QMap<QString, QTextEdit*> roomViews_;
    QSet<QString>             joinedRooms_;   // rooms we've confirmed joined
    QLineEdit*                messageInput_;
    QLineEdit*                roomIdInput_;
    QLabel*                   currentRoomLabel_;

    QString currentRoom_;
    QString displayName_;
    QString userId_;
};