#include "MainShell.h"
#include "ui/panels/ChatPanel.h"
#include "ui/panels/MarketplacePanel.h"
#include "ui/panels/FilesPanel.h"
#include "ui/panels/ForumPanel.h"
#include "ui/panels/ProfilePanel.h"
#include "ui/theme/Theme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QTextEdit>
#include <QLabel>
#include <QScrollBar>

MainShell::MainShell(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("MainShell { %1 }").arg(Theme::pageBackground()));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    auto* row = new QHBoxLayout;
    row->setContentsMargins(0,0,0,0);
    row->setSpacing(0);

    // ── sidebar ──────────────────────────────────────────────────────────
    auto* iconBar = new QWidget;
    iconBar->setObjectName("navSidebar");
    iconBar->setFixedWidth(180);
    iconBar->setStyleSheet(QString(
        "#navSidebar { background: %1; border-right: 1px solid %2; }"
    ).arg(Theme::SURFACE_ALT, Theme::BORDER));

    auto* iconLayout = new QVBoxLayout(iconBar);
    iconLayout->setContentsMargins(12, 20, 12, 16);
    iconLayout->setSpacing(2);
    iconLayout->setAlignment(Qt::AlignTop);

    auto* logoLabel = new QLabel("AUC Network");
    logoLabel->setFixedHeight(36);
    logoLabel->setStyleSheet(QString(
        "border: none; background: transparent; color: %1; font-size: 14px; font-weight: 600;"
    ).arg(Theme::TEXT_PRIMARY));
    iconLayout->addWidget(logoLabel);
    iconLayout->addSpacing(12);

    btnChat_    = makeNavBtn("Chat", "Open chat");
    btnMarket_  = makeNavBtn("Marketplace", "Open marketplace");
    btnFiles_   = makeNavBtn("Files", "Open files");
    btnForum_   = makeNavBtn("Forum", "Open forum");
    btnProfile_ = makeNavBtn("Profile", "Open profile");

    btnChat_->setChecked(true);

    iconLayout->addWidget(btnChat_);
    iconLayout->addWidget(btnMarket_);
    iconLayout->addWidget(btnFiles_);
    iconLayout->addWidget(btnForum_);
    iconLayout->addWidget(btnProfile_);
    iconLayout->addStretch();

    auto* btnLogout = new QToolButton;
    btnLogout->setText("Log out");
    btnLogout->setToolTip("Sign out");
    btnLogout->setFixedHeight(38);
    btnLogout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    btnLogout->setStyleSheet(QString(
        "QToolButton { background: transparent; color: %1; border: none; border-radius: 8px; font-size: 13px; text-align: left; padding-left: 12px; }"
        "QToolButton:hover { background: %2; color: %3; }"
    ).arg(Theme::TEXT_SECONDARY, Theme::SURFACE, Theme::DANGER));
    iconLayout->addWidget(btnLogout);

    // ── content stack ────────────────────────────────────────────────────
    contentStack_ = new QStackedWidget;
    contentStack_->setStyleSheet(QString("background: %1; border: none;").arg(Theme::SURFACE));

    chatPanel_   = new ChatPanel;
    marketPanel_ = new MarketplacePanel;
    filesPanel_  = new FilesPanel;
    forumPanel_   = new ForumPanel;
    profilePanel_ = new ProfilePanel;

    contentStack_->addWidget(chatPanel_);
    contentStack_->addWidget(marketPanel_);
    contentStack_->addWidget(filesPanel_);
    contentStack_->addWidget(forumPanel_);
    contentStack_->addWidget(profilePanel_);

    row->addWidget(iconBar);
    row->addWidget(contentStack_, 1);

    // ── activity log strip ───────────────────────────────────────────────
    logView_ = new QTextEdit;
    logView_->setReadOnly(true);
    logView_->setMaximumHeight(64);
    logView_->setStyleSheet(QString(
        "QTextEdit { background: %1; color: %2; border: none; border-top: 1px solid %3; font-size: 11px; padding: 6px 12px; }"
    ).arg(Theme::SURFACE_ALT, Theme::TEXT_SECONDARY, Theme::BORDER));

    root->addLayout(row, 1);
    root->addWidget(logView_);

    // ── navigation logic ─────────────────────────────────────────────────
    auto switchTo = [this](int idx, QToolButton* active){
        contentStack_->setCurrentIndex(idx);
        for (auto* b : {btnChat_, btnMarket_, btnFiles_, btnForum_, btnProfile_})
            b->setChecked(false);
        active->setChecked(true);
    };

    connect(btnChat_,    &QToolButton::clicked, [=]{ switchTo(0, btnChat_); });
    connect(btnMarket_,  &QToolButton::clicked, [=]{ switchTo(1, btnMarket_); });
    connect(btnFiles_,   &QToolButton::clicked, [=]{ switchTo(2, btnFiles_); });
    connect(btnForum_,   &QToolButton::clicked, [=]{ switchTo(3, btnForum_); });
    connect(btnProfile_, &QToolButton::clicked, [=]{ switchTo(4, btnProfile_); });
    connect(btnLogout,   &QToolButton::clicked, this, &MainShell::logoutClicked);

    connect(chatPanel_,    &ChatPanel::messageSent,        this, &MainShell::sendMessage);
    connect(chatPanel_,    &ChatPanel::roomCreated,        this, &MainShell::sendMessage);
    connect(chatPanel_,    &ChatPanel::roomJoined,         this, &MainShell::sendMessage);
    connect(marketPanel_,  &MarketplacePanel::sendMessage, this, &MainShell::sendMessage);
    connect(filesPanel_,   &FilesPanel::sendMessage,       this, &MainShell::sendMessage);
    connect(forumPanel_,   &ForumPanel::sendMessage,       this, &MainShell::sendMessage);
    connect(profilePanel_, &ProfilePanel::sendMessage,      this, &MainShell::sendMessage);

    connect(marketPanel_, &MarketplacePanel::openRoom, this, [this](const QString& roomId){
        contentStack_->setCurrentIndex(0);
        for (auto* b : {btnChat_, btnMarket_, btnFiles_, btnForum_, btnProfile_})
            b->setChecked(false);
        btnChat_->setChecked(true);
        chatPanel_->openDirectRoom(roomId);
    });
}

void MainShell::setCurrentUser(const QString& displayName,
                                const QString& userId,
                                const QString& username,
                                const QString& token)
{
    token_ = token;
    chatPanel_->setCurrentUser(displayName, userId);
    chatPanel_->addOnlineUser(userId, displayName, username, ""); // add self
    marketPanel_->setCurrentUser(displayName, userId);
    marketPanel_->setToken(token);
    filesPanel_->setCurrentUser(displayName, userId, token);
    forumPanel_->setCurrentUser(displayName, userId, token);
    profilePanel_->setCurrentUser(displayName, userId, username, "", token);
    log("Signed in as " + displayName);
}

void MainShell::routeMessage(const Message& msg)
{
    switch (msg.type)
    {
    case MessageType::CHAT_PUBLIC:
    case MessageType::CHAT_PRIVATE:
    case MessageType::JOIN:
        chatPanel_->receiveMessage(msg);
        break;
    case MessageType::LEAVE:
        chatPanel_->receiveMessage(msg);
        break;

    case MessageType::PRESENCE: {
        // user came online — add to sidebar
        QString userId      = QString::fromStdString(msg.sender.userId);
        QString displayName = QString::fromStdString(msg.displayName);
        QString username    = QString::fromStdString(msg.sender.username);
        QString bio         = QString::fromStdString(msg.bio);
        if (!userId.isEmpty())
            chatPanel_->addOnlineUser(userId, displayName, username, bio);
        break;
    }

     
    case MessageType::USER_ONLINE: {
        QString uid = QString::fromStdString(msg.sender.userId);
        chatPanel_->addOnlineUser(uid,
            QString::fromStdString(msg.displayName),
            QString::fromStdString(msg.sender.username),
            QString::fromStdString(msg.bio));
        chatPanel_->setUserStatus(uid, UserStatus::ONLINE);
        break;
    }
    case MessageType::USER_AWAY: {
        QString uid = QString::fromStdString(msg.sender.userId);
        chatPanel_->setUserStatus(uid, UserStatus::AWAY);
        break;
    }
    case MessageType::USER_OFFLINE: {
        QString uid = QString::fromStdString(msg.sender.userId);
        chatPanel_->setUserStatus(uid, UserStatus::OFFLINE);
        break;
    }
 


    case MessageType::MARKET_POST:
    case MessageType::MARKET_INQUIRY:
    case MessageType::MARKET_SEARCH:
        marketPanel_->receiveMessage(msg);
        break;

    case MessageType::MATERIAL_UPLOAD:
    case MessageType::MATERIAL_LIST:
    case MessageType::MATERIAL_GET:
    case MessageType::MATERIAL_REPORT:
        filesPanel_->receiveMessage(msg);
        break;

    case MessageType::QA_QUESTION:
    case MessageType::QA_ANSWER:
    case MessageType::QA_FAQ:
    case MessageType::QA_GET_ALL:
    case MessageType::QA_GET_ONE:
    case MessageType::FORUM_UPVOTE:
    case MessageType::FORUM_DOWNVOTE:
        forumPanel_->receiveMessage(msg);
        break;

    case MessageType::PROFILE_GET:
    case MessageType::PROFILE_EDIT:
        profilePanel_->receiveMessage(msg);
        break;

    default:
        log(QString::fromStdString(Message::typeToString(msg.type)) + " — " + QString::fromStdString(msg.text));
        break;
    }
}

QToolButton* MainShell::makeNavBtn(const QString& icon, const QString& tip)
{
    auto* btn = new QToolButton;
    btn->setText(icon);
    btn->setToolTip(tip);
    btn->setFixedHeight(38);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    btn->setCheckable(true);

    btn->setStyleSheet(QString(
        "QToolButton { background: transparent; color: %1; border: none; border-radius: 8px; font-size: 13px; font-weight: 500; text-align: left; padding-left: 12px; }"
        "QToolButton:hover { background: %2; }"
        "QToolButton:checked { background: %3; color: white; }"
    ).arg(Theme::TEXT_PRIMARY, Theme::SURFACE, Theme::ACCENT));

    return btn;
}

void MainShell::log(const QString& text)
{
    logView_->append(text);
    logView_->verticalScrollBar()->setValue(logView_->verticalScrollBar()->maximum());
}

void MainShell::setChatUserStatus(const QString& userId, UserStatus status)
{
    chatPanel_->setUserStatus(userId, status);
}
void MainShell::resetChat()
{
    chatPanel_->resetState();
}