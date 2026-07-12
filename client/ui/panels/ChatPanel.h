#pragma once
#include <QWidget>
#include <QStackedWidget>
#include <QMap>
#include <QSet>
#include "Message.h"

class UsersSidebar;
class QListWidget;
class QTextEdit;
class QLineEdit;
class QLabel;

class ChatPanel : public QWidget {
    Q_OBJECT
public:
    explicit ChatPanel(QWidget* parent = nullptr);

    void receiveMessage  (const Message& msg);
    void setCurrentUser  (const QString& displayName, const QString& userId);
    void openDirectRoom  (const QString& roomId);
    void addOnlineUser   (const QString& userId, const QString& displayName, const QString& username, const QString& bio);
    void removeOnlineUser(const QString& userId);
    void addKnownRoom(const QString& roomId);

signals:
    void messageSent(const Message& msg);
    void roomCreated(const Message& msg);
    void roomJoined (const Message& msg);

private slots:
    void onSend();
    void onCreateRoom();

private:
    void switchToRoom(const QString& room);
    void appendChat  (const QString& room, const QString& sender, const QString& text);

    QListWidget*              roomList_;
    QStackedWidget*           chatStack_;
    QMap<QString, QTextEdit*> roomViews_;
    QSet<QString>             joinedRooms_;
    QLineEdit*                messageInput_;
    QLineEdit*                roomIdInput_;
    QLabel*                   currentRoomLabel_;
    UsersSidebar*             usersSidebar_;

    QString currentRoom_;
    QString displayName_;
    QString userId_;
};