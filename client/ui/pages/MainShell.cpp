#include "MainShell.h"
#include "ui/panels/ChatPanel.h"
#include "ui/panels/MarketplacePanel.h"
#include "ui/panels/FilesPanel.h"
#include "ui/panels/ForumPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QTextEdit>
#include <QLabel>
#include <QFrame>
#include <QScrollBar>

static const char* ACCENT   = "#6366f1";
static const char* ACCENT2  = "#818cf8";
static const char* TEXT_SEC = "#94a3b8";
static const char* BG_PANEL = "#0f172a";

MainShell::MainShell(QWidget* parent) : QWidget(parent)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    auto* row = new QHBoxLayout;
    row->setContentsMargins(0,0,0,0);
    row->setSpacing(0);

    // ── icon sidebar ──────────────────────────────────────────────────────
    auto* iconBar = new QWidget;
    iconBar->setFixedWidth(68);
    iconBar->setStyleSheet(QString("background:%1;border-right:1px solid #1e293b;").arg(BG_PANEL));
    auto* iconLayout = new QVBoxLayout(iconBar);
    iconLayout->setContentsMargins(8,12,8,12);
    iconLayout->setSpacing(6);
    iconLayout->setAlignment(Qt::AlignTop);

    auto* logoLabel = new QLabel("⬡");
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setFixedHeight(44);
    logoLabel->setStyleSheet(QString("color:%1;font-size:28px;").arg(ACCENT2));
    iconLayout->addWidget(logoLabel);

    auto* divider = new QFrame;
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("background:#1e293b;margin:4px 0;");
    divider->setFixedHeight(1);
    iconLayout->addWidget(divider);
    iconLayout->addSpacing(4);

    btnChat_    = makeNavBtn("💬","Chat");
    btnMarket_  = makeNavBtn("🛒","Marketplace");
    btnFiles_   = makeNavBtn("📁","Files");
    btnForum_   = makeNavBtn("❓","Forum");
    btnProfile_ = makeNavBtn("👤","Profile");
    btnChat_->setChecked(true);

    iconLayout->addWidget(btnChat_);
    iconLayout->addWidget(btnMarket_);
    iconLayout->addWidget(btnFiles_);
    iconLayout->addWidget(btnForum_);
    iconLayout->addWidget(btnProfile_);
    iconLayout->addStretch();

    auto* btnLogout = new QToolButton;
    btnLogout->setText("⏻");
    btnLogout->setToolTip("Logout");
    btnLogout->setFixedSize(52,52);
    btnLogout->setStyleSheet(QString(
        "QToolButton{background:transparent;color:%1;border:none;"
        "border-radius:12px;font-size:20px;}"
        "QToolButton:hover{background:#7f1d1d;color:white;}").arg(TEXT_SEC));
    iconLayout->addWidget(btnLogout);

    // ── content stack ─────────────────────────────────────────────────────
    contentStack_ = new QStackedWidget;

    chatPanel_   = new ChatPanel;
    marketPanel_ = new MarketplacePanel;
    filesPanel_  = new FilesPanel;
    forumPanel_  = new ForumPanel;

    auto makePlaceholder = [](const QString& icon, const QString& label) -> QWidget* {
        auto* w = new QWidget;
        w->setStyleSheet("background:#0a0f1e;");
        auto* l = new QVBoxLayout(w);
        l->setAlignment(Qt::AlignCenter);
        auto* ic = new QLabel(icon);
        ic->setAlignment(Qt::AlignCenter);
        ic->setStyleSheet("font-size:56px;background:transparent;");
        auto* lb = new QLabel(label);
        lb->setAlignment(Qt::AlignCenter);
        lb->setStyleSheet("color:#94a3b8;font-size:18px;background:transparent;");
        l->addWidget(ic); l->addSpacing(12); l->addWidget(lb);
        return w;
    };

    contentStack_->addWidget(chatPanel_);                                        // 0
    contentStack_->addWidget(marketPanel_);                                      // 1
    contentStack_->addWidget(filesPanel_);                                       // 2
    contentStack_->addWidget(forumPanel_);                                       // 3
    contentStack_->addWidget(makePlaceholder("👤","Profile — coming soon"));    // 4

    row->addWidget(iconBar);
    row->addWidget(contentStack_, 1);

    // ── log strip ─────────────────────────────────────────────────────────
    logView_ = new QTextEdit;
    logView_->setReadOnly(true);
    logView_->setMaximumHeight(90);
    logView_->setStyleSheet(QString(
        "QTextEdit{background:%1;color:#64748b;border:none;"
        "border-top:1px solid #1e293b;font-family:monospace;"
        "font-size:11px;padding:6px;}").arg(BG_PANEL));

    root->addLayout(row, 1);
    root->addWidget(logView_);

    // ── nav ───────────────────────────────────────────────────────────────
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

    // ── signals up to MainWindow ──────────────────────────────────────────
    connect(chatPanel_,   &ChatPanel::messageSent,        this, &MainShell::sendMessage);
    connect(chatPanel_,   &ChatPanel::roomCreated,        this, &MainShell::sendMessage);
    connect(chatPanel_,   &ChatPanel::roomJoined,         this, &MainShell::sendMessage);
    connect(marketPanel_, &MarketplacePanel::sendMessage, this, &MainShell::sendMessage);
    connect(filesPanel_,  &FilesPanel::sendMessage,       this, &MainShell::sendMessage);
    connect(forumPanel_,  &ForumPanel::sendMessage,       this, &MainShell::sendMessage);

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
                                const QString& token)
{
    token_ = token;
    chatPanel_->setCurrentUser(displayName, userId);
    marketPanel_->setCurrentUser(displayName, userId);
    marketPanel_->setToken(token);
    filesPanel_->setCurrentUser(displayName, userId, token);
    forumPanel_->setCurrentUser(displayName, userId, token);
    log("✅ Logged in as " + displayName);
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

    default:
        log("← " + QString::fromStdString(Message::typeToString(msg.type))
            + " | " + QString::fromStdString(msg.text));
        break;
    }
}

QToolButton* MainShell::makeNavBtn(const QString& icon, const QString& tip)
{
    auto* btn = new QToolButton;
    btn->setText(icon);
    btn->setToolTip(tip);
    btn->setFixedSize(52,52);
    btn->setCheckable(true);
    btn->setStyleSheet(QString(
        "QToolButton{background:transparent;color:%1;border:none;"
        "border-radius:12px;font-size:22px;}"
        "QToolButton:hover{background:#1e293b;color:white;}"
        "QToolButton:checked{background:%2;color:white;}").arg(TEXT_SEC, ACCENT));
    return btn;
}

void MainShell::log(const QString& text)
{
    logView_->append(text);
    logView_->verticalScrollBar()->setValue(logView_->verticalScrollBar()->maximum());
}