#pragma once
#include <QWidget>
#include <QStackedWidget>
#include <QMap>
#include <QSet>
#include <QStringList>
#include "Message.h"
#include "ui/panels/UsersSidebar.h"

class UsersSidebar;
class QListWidget;
class QListWidgetItem;
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
    void setUserStatus(const QString& userId, UserStatus status);
     void resetState();

signals:
    void messageSent(const Message& msg);
    void roomCreated(const Message& msg);
    void roomJoined (const Message& msg);

private slots:
    void onSend();
    void onCreateRoom();
    void onMessageUser(const QString& userId);  
    void onViewProfile(const QString& userId); 

private:
    struct RoomMeta {
        QString     type;     // "PUBLIC", "GROUP", "DIRECT"
        QStringList members;  // member userIds — meaningful for GROUP/DIRECT
    };

    void switchToRoom(const QString& roomId);
    void appendChat  (const QString& roomId, const QString& sender, const QString& text);
    QListWidgetItem* ensureRoomListItem(const QString& roomId, const QString& name);
    void applyMemberFilter(const QString& roomId);
    QString resolveDirectRoomName(const QString& roomId) const; 

    QListWidget*              roomList_;
    QStackedWidget*           chatStack_;
    QMap<QString, QTextEdit*> roomViews_;
    QSet<QString>             joinedRooms_;
    QLineEdit*                messageInput_;
    QLabel*                   currentRoomLabel_;
    UsersSidebar*             usersSidebar_;

    QMap<QString, QString>  roomNames_;   // roomId -> display name
    QMap<QString, RoomMeta> roomMeta_;    // roomId -> type/members
    QMap<QString, QString>  knownUsers_;  // userId -> displayName (feeds the create-room dialog)

    QString currentRoom_;
    QString displayName_;
    QString userId_;
};