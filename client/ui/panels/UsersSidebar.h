#pragma once
#include <QWidget>
#include <QMap>
#include "Message.h"

class QVBoxLayout;
class QLabel;
class QScrollArea;

// ── single user card ──────────────────────────────────────────────────────────
class UserCard : public QWidget {
    Q_OBJECT
public:
    UserCard(const QString& userId, const QString& displayName,
             const QString& username, const QString& bio,
             QWidget* parent = nullptr);
    QString userId() const { return userId_; }
signals:
    void clicked(const QString& userId);
private:
    void mousePressEvent(QMouseEvent*) override;
    QString userId_;
};

// ── users sidebar ─────────────────────────────────────────────────────────────
class UsersSidebar : public QWidget {
    Q_OBJECT
public:
    explicit UsersSidebar(QWidget* parent = nullptr);

    void addUser   (const QString& userId, const QString& displayName,
                    const QString& username, const QString& bio);
    void removeUser(const QString& userId);
    void setCurrentUser(const QString& userId) { currentUserId_ = userId; }

signals:
    void userClicked(const QString& userId);

private:
    QVBoxLayout*          cardsLayout_;
    QLabel*               countLabel_;
    QMap<QString, UserCard*> cards_;
    QString               currentUserId_;
};