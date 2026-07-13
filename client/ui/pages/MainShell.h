#pragma once
#include <QWidget>
#include <QStackedWidget>
#include "Message.h"
#include "ui/panels/UsersSidebar.h"

class QToolButton;
class QTextEdit;
class ChatPanel;
class MarketplacePanel;
class FilesPanel;
class ForumPanel;
class ProfilePanel;

class MainShell : public QWidget {
    Q_OBJECT
public:
    explicit MainShell(QWidget* parent = nullptr);

    void setCurrentUser(const QString& displayName, const QString& userId,
                        const QString& username,    const QString& token);
    void routeMessage(const Message& msg);
    void setChatUserStatus(const QString& userId, UserStatus status);

signals:
    void sendMessage(const Message& msg);
    void logoutClicked();

private:
    QToolButton* makeNavBtn(const QString& icon, const QString& tip);
    void         log(const QString& text);

    QStackedWidget*    contentStack_;
    QTextEdit*         logView_;
    ChatPanel*         chatPanel_;
    MarketplacePanel*  marketPanel_;
    FilesPanel*        filesPanel_;
    ForumPanel*        forumPanel_;
    ProfilePanel*      profilePanel_;

    QToolButton* btnChat_;
    QToolButton* btnMarket_;
    QToolButton* btnFiles_;
    QToolButton* btnForum_;
    QToolButton* btnProfile_;

    QString token_;
};