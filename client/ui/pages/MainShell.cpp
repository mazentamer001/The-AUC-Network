#include "MainShell.h"
#include "ui/panels/ChatPanel.h"
#include "ui/panels/MarketplacePanel.h"
#include "ui/panels/FilesPanel.h"
#include "ui/panels/ForumPanel.h"
#include "ui/panels/ProfilePanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QTextEdit>
#include <QLabel>
#include <QFrame>
#include <QScrollBar>

// Cohesive Label Color Palette
static const char* BG_MAIN       = "#D4C5B6"; // Warm Sand / Parchment
static const char* BG_PANEL      = "#C9BBAA"; // Slightly deeper sand for sidebar/logs
static const char* TEXT_MAIN     = "#0F0F0F"; // Ink Black
static const char* ACCENT_ORANGE = "#E65C40"; // Stamp Red-Orange

MainShell::MainShell(QWidget* parent) : QWidget(parent)
{
    // Force the background color for the main shell container
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("MainShell { background-color: %1; }").arg(BG_MAIN));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto* row = new QHBoxLayout;
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(0);

    // ── TEXT SIDEBAR (Structured Left Column) ─────────────────────────────
    auto* iconBar = new QWidget;
    // Widened from 68 to 160 to comfortably fit the bold text labels
    iconBar->setFixedWidth(160);
    iconBar->setStyleSheet(QString("background: %1; border-right: 1px solid %2;").arg(BG_PANEL, TEXT_MAIN));
    
    auto* iconLayout = new QVBoxLayout(iconBar);
    iconLayout->setContentsMargins(0, 16, 0, 16); // Removed side margins so buttons stretch full width
    iconLayout->setSpacing(0);
    iconLayout->setAlignment(Qt::AlignTop);

    // Replaced the hexagon with a text-based system label
    auto* logoLabel = new QLabel("SYS // NET");
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setFixedHeight(44);
    logoLabel->setStyleSheet(QString(
        "color: %1; font-size: 14px; font-weight: 900; letter-spacing: 3px; border: none;"
    ).arg(TEXT_MAIN));
    iconLayout->addWidget(logoLabel);

    auto* divider = new QFrame;
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet(QString("background-color: %1; margin: 8px 12px; border: none;").arg(TEXT_MAIN));
    divider->setFixedHeight(1);
    iconLayout->addWidget(divider);
    iconLayout->addSpacing(8);

    // Swapped emojis for text labels
    btnChat_    = makeNavBtn("CHAT", "Open Chat");
    btnMarket_  = makeNavBtn("MARKET", "Open Marketplace");
    btnFiles_   = makeNavBtn("FILES", "Open Files");
    btnForum_   = makeNavBtn("FORUM", "Open Forum");
    btnProfile_ = makeNavBtn("PROFILE", "Open Profile");
    
    btnChat_->setChecked(true);

    iconLayout->addWidget(btnChat_);
    iconLayout->addWidget(btnMarket_);
    iconLayout->addWidget(btnFiles_);
    iconLayout->addWidget(btnForum_);
    iconLayout->addWidget(btnProfile_);
    iconLayout->addStretch();

    // Text-based Logout Button
    auto* btnLogout = new QToolButton;
    btnLogout->setText("LOGOUT");
    btnLogout->setToolTip("Disconnect");
    btnLogout->setFixedHeight(44);
    btnLogout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    btnLogout->setStyleSheet(QString(
        "QToolButton {"
        "  background: transparent; color: %1; border: none;"
        "  border-radius: 0px; font-size: 12px; font-weight: bold; letter-spacing: 2px;"
        "  text-align: left; padding-left: 16px;"
        "}"
        "QToolButton:hover { background: %1; color: %2; }"
    ).arg(TEXT_MAIN, BG_MAIN));
    iconLayout->addWidget(btnLogout);

    // ── CONTENT STACK ─────────────────────────────────────────────────────
    contentStack_ = new QStackedWidget;
    contentStack_->setStyleSheet(QString("background-color: %1; border: none;").arg(BG_MAIN));

    chatPanel_    = new ChatPanel;
    marketPanel_  = new MarketplacePanel;
    filesPanel_   = new FilesPanel;
    forumPanel_   = new ForumPanel;
    profilePanel_ = new ProfilePanel;

    contentStack_->addWidget(chatPanel_);                                        
    contentStack_->addWidget(marketPanel_);                                      
    contentStack_->addWidget(filesPanel_);                                       
    contentStack_->addWidget(forumPanel_);                                       
    contentStack_->addWidget(profilePanel_);                                    

    row->addWidget(iconBar);
    row->addWidget(contentStack_, 1);

    // ── LOG STRIP (Bottom Status Bar) ─────────────────────────────────────
    logView_ = new QTextEdit;
    logView_->setReadOnly(true);
    logView_->setMaximumHeight(90);
    logView_->setStyleSheet(QString(
        "QTextEdit {"
        "  background: %1; color: %2; border: none;"
        "  border-top: 1px solid %2; font-family: monospace;"
        "  font-size: 11px; padding: 6px;"
        "}"
    ).arg(BG_PANEL, TEXT_MAIN));

    root->addLayout(row, 1);
    root->addWidget(logView_);

    // ── NAVIGATION LOGIC ──────────────────────────────────────────────────
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

    // ── SIGNALS UP TO MAINWINDOW ──────────────────────────────────────────
    connect(chatPanel_,    &ChatPanel::messageSent,        this, &MainShell::sendMessage);
    connect(chatPanel_,    &ChatPanel::roomCreated,        this, &MainShell::sendMessage);
    connect(chatPanel_,    &ChatPanel::roomJoined,         this, &MainShell::sendMessage);
    connect(marketPanel_,  &MarketplacePanel::sendMessage, this, &MainShell::sendMessage);
    connect(filesPanel_,   &FilesPanel::sendMessage,       this, &MainShell::sendMessage);
    connect(forumPanel_,   &ForumPanel::sendMessage,       this, &MainShell::sendMessage);
    connect(profilePanel_, &ProfilePanel::sendMessage,     this, &MainShell::sendMessage);

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
    marketPanel_->setCurrentUser(displayName, userId);
    marketPanel_->setToken(token);
    filesPanel_->setCurrentUser(displayName, userId, token);
    forumPanel_->setCurrentUser(displayName, userId, token);
    profilePanel_->setCurrentUser(displayName, userId, username, "", token);
    log("SYSTEM // AUTHENTICATED: " + displayName);
}

void MainShell::routeMessage(const Message& msg)
{
    switch (msg.type)
    {
    case MessageType::CHAT_PUBLIC:
    case MessageType::CHAT_PRIVATE:
    case MessageType::JOIN:
    case MessageType::LEAVE:
    case MessageType::PRESENCE:
        chatPanel_->receiveMessage(msg);
        break;

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
        log("RECV ← " + QString::fromStdString(Message::typeToString(msg.type))
            + " | " + QString::fromStdString(msg.text));
        break;
    }
}

QToolButton* MainShell::makeNavBtn(const QString& label, const QString& tip)
{
    auto* btn = new QToolButton;
    btn->setText(label);
    btn->setToolTip(tip);
    
    // Adjusted from a fixed square to a horizontal expanding bar
    btn->setFixedHeight(44);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    btn->setCheckable(true);
    
    // Realigned text to the left with padding to mimic a directory list
    btn->setStyleSheet(QString(
        "QToolButton {"
        "  background: transparent; color: %1; border: none; border-right: 0px;"
        "  border-radius: 0px; font-size: 12px; font-weight: bold; letter-spacing: 2px;"
        "  text-align: left; padding-left: 16px;"
        "}"
        "QToolButton:hover {"
        "  background: %1; color: %3;" 
        "}"
        "QToolButton:checked {"
        "  background: %2; color: %3;"
        "}"
    ).arg(TEXT_MAIN, ACCENT_ORANGE, BG_MAIN));
    
    return btn;
}

void MainShell::log(const QString& text)
{
    logView_->append(text);
    logView_->verticalScrollBar()->setValue(logView_->verticalScrollBar()->maximum());
}