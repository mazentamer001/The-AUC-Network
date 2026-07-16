#pragma once
#include <QWidget>
#include <QMap>
#include <QSet>
#include "Message.h"

class QVBoxLayout;
class QLabel;
class QScrollArea;

enum class UserStatus { OFFLINE, ONLINE, AWAY };

// ── single user card ──────────────────────────────────────────────────────────
class UserCard : public QWidget {
    Q_OBJECT
public:
UserCard(const QString& userId, const QString& displayName,
const QString& username, const QString& bio, const QString& photoData,
QWidget* parent = nullptr);
QString userId() const { return userId_; }
void setStatus(UserStatus status);
void setPhoto(const QString& photoData);   // update the avatar after the card already exists

signals:
void viewProfileClicked(const QString& userId);
void messageClicked(const QString& userId);

private:
void mousePressEvent(QMouseEvent*) override;
void updateStatusDot();
void renderAvatar(const QString& photoData);

    QString     userId_;
    QString     displayName_;
    UserStatus  status_ = UserStatus::OFFLINE;
    QLabel*     statusDot_;
    QLabel*     avatarLabel_;
};

// ── users sidebar ─────────────────────────────────────────────────────────────
class UsersSidebar : public QWidget {
    Q_OBJECT
public:
explicit UsersSidebar(QWidget* parent = nullptr);

void addUser      (const QString& userId, const QString& displayName,
const QString& username, const QString& bio, const QString& photoData = QString());
void removeUser   (const QString& userId);
void setUserStatus(const QString& userId, UserStatus status);
void setUserPhoto (const QString& userId, const QString& photoData);
void setCurrentUser(const QString& userId) { currentUserId_ = userId; }

void showAllUsers();
void filterToUsers(const QSet<QString>& userIds);
void clearAll();  

signals:
void profileRequested(const QString& userId);
void messageRequested(const QString& userId);

private:
void updateCount();

    QVBoxLayout*             cardsLayout_;
    QLabel*                  countLabel_;
    QMap<QString, UserCard*> cards_;
    QString                  currentUserId_;

bool             filterActive_ = false;
    QSet<QString>    currentFilter_;
};