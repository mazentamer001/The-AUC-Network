#include "UsersSidebar.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QMenu>
#include <QAction>

static const char* ACCENT2  = "#818cf8";
static const char* TEXT_PRI = "#f1f5f9";
static const char* TEXT_SEC = "#94a3b8";
static const char* BG_PANEL = "#0f172a";
static const char* BG_CARD  = "#1e293b";
static const char* BG_DEEP  = "#0a0f1e";

// ─────────────────────────────────────────────────────────────────────────────
//  UserCard
// ─────────────────────────────────────────────────────────────────────────────
UserCard::UserCard(const QString& userId, const QString& displayName,
                   const QString& username, const QString& bio,
                   QWidget* parent)
    : QWidget(parent), userId_(userId)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString(
        "UserCard{background:%1;border-radius:8px;}"
        "UserCard:hover{background:#1a2540;}").arg(BG_CARD));
    setCursor(Qt::PointingHandCursor);
    setFixedHeight(72);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10,10,10,10);
    layout->setSpacing(10);

    // ── avatar with status dot ────────────────────────────────────────────
    auto* avatarContainer = new QWidget;
    avatarContainer->setFixedSize(44,44);
    avatarContainer->setStyleSheet("background:transparent;");
    avatarContainer->setAttribute(Qt::WA_StyledBackground, false);

    avatarLabel_ = new QLabel(
        displayName.isEmpty() ? "?" : QString(displayName[0].toUpper()),
        avatarContainer);
    avatarLabel_->setGeometry(0,0,40,40);
    avatarLabel_->setAlignment(Qt::AlignCenter);
    avatarLabel_->setStyleSheet(QString(
        "background:%1;border-radius:20px;color:white;"
        "font-size:16px;font-weight:bold;").arg(ACCENT2));

    // status dot — bottom right of avatar
    statusDot_ = new QLabel(avatarContainer);
    statusDot_->setGeometry(28,28,14,14);
    statusDot_->setStyleSheet(
        "background:#475569;border-radius:7px;"
        "border:2px solid #0f172a;");  // default = offline grey

    // ── info ──────────────────────────────────────────────────────────────
    auto* infoLayout = new QVBoxLayout;
    infoLayout->setSpacing(2);
    infoLayout->setContentsMargins(0,0,0,0);

    auto* nameLbl = new QLabel(displayName);
    nameLbl->setStyleSheet(QString(
        "color:%1;font-size:13px;font-weight:bold;background:transparent;").arg(TEXT_PRI));
    nameLbl->setMaximumWidth(130);

    auto* usernameLbl = new QLabel("@" + username);
    usernameLbl->setStyleSheet(QString(
        "color:%1;font-size:11px;background:transparent;").arg(TEXT_SEC));

    QString bioPreview = bio.left(36) + (bio.length() > 36 ? "..." : "");
    auto* bioLbl = new QLabel(bioPreview);
    bioLbl->setStyleSheet(QString(
        "color:%1;font-size:11px;background:transparent;").arg(TEXT_SEC));

    infoLayout->addWidget(nameLbl);
    infoLayout->addWidget(usernameLbl);
    if (!bioPreview.isEmpty())
        infoLayout->addWidget(bioLbl);

    layout->addWidget(avatarContainer);
    layout->addLayout(infoLayout, 1);
}


void UserCard::setStatus(UserStatus status)
{
    status_ = status;
    updateStatusDot();
}

void UserCard::updateStatusDot()
{
    // remove old glow effect
    statusDot_->setGraphicsEffect(nullptr);

    switch (status_) {
    case UserStatus::ONLINE: {
        statusDot_->setStyleSheet(
            "background:#22c55e;border-radius:7px;border:2px solid #0f172a;");
        auto* glow = new QGraphicsDropShadowEffect;
        glow->setBlurRadius(8);
        glow->setOffset(0,0);
        glow->setColor(QColor("#22c55e"));
        statusDot_->setGraphicsEffect(glow);
        break;
    }
    case UserStatus::AWAY: {
        statusDot_->setStyleSheet(
            "background:#f59e0b;border-radius:7px;border:2px solid #0f172a;");
        auto* glow = new QGraphicsDropShadowEffect;
        glow->setBlurRadius(8);
        glow->setOffset(0,0);
        glow->setColor(QColor("#f59e0b"));
        statusDot_->setGraphicsEffect(glow);
        break;
    }
    case UserStatus::OFFLINE:
    default:
        statusDot_->setStyleSheet(
            "background:#475569;border-radius:7px;border:2px solid #0f172a;");
        break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  UsersSidebar
// ─────────────────────────────────────────────────────────────────────────────
UsersSidebar::UsersSidebar(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("background:%1;border-left:1px solid #1e293b;").arg(BG_PANEL));
    setFixedWidth(200);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    // header
    auto* header = new QWidget;
    header->setFixedHeight(52);
    header->setAttribute(Qt::WA_StyledBackground, true);
    header->setStyleSheet(QString(
        "background:%1;border-bottom:1px solid #1e293b;").arg(BG_PANEL));
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(14,0,14,0);

    auto* title = new QLabel("Users");
    title->setStyleSheet(QString(
        "color:%1;font-size:13px;font-weight:bold;background:transparent;").arg(TEXT_PRI));

    countLabel_ = new QLabel("0");
    countLabel_->setStyleSheet(QString(
        "color:white;background:%1;border-radius:10px;"
        "padding:1px 7px;font-size:11px;font-weight:bold;").arg(ACCENT2));

    headerLayout->addWidget(title);
    headerLayout->addStretch();
    headerLayout->addWidget(countLabel_);

    // scroll area
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(QString(
        "QScrollArea{border:none;background:%1;}"
        "QScrollBar:vertical{background:%2;width:4px;border-radius:2px;}"
        "QScrollBar::handle:vertical{background:#334155;border-radius:2px;}")
        .arg(BG_PANEL, BG_DEEP));

    auto* container = new QWidget;
    container->setAttribute(Qt::WA_StyledBackground, true);
    container->setStyleSheet(QString("background:%1;").arg(BG_PANEL));
    cardsLayout_ = new QVBoxLayout(container);
    cardsLayout_->setContentsMargins(8,8,8,8);
    cardsLayout_->setSpacing(4);
    cardsLayout_->setAlignment(Qt::AlignTop);
    scroll->setWidget(container);

    root->addWidget(header);
    root->addWidget(scroll, 1);
}


void UsersSidebar::removeUser(const QString& userId)
{
    if (!cards_.contains(userId)) return;
    cards_[userId]->deleteLater();
    cards_.remove(userId);
    updateCount();
}

void UsersSidebar::setUserStatus(const QString& userId, UserStatus status)
{
    if (cards_.contains(userId))
        cards_[userId]->setStatus(status);
}

void UsersSidebar::updateCount()
{
    int n = 0;
    for (auto* card : cards_)
        if (card->isVisible()) n++;
    countLabel_->setText(QString::number(n));
}

void UsersSidebar::filterToUsers(const QSet<QString>& userIds)
{
    filterActive_  = true;
    currentFilter_ = userIds;
    for (auto it = cards_.constBegin(); it != cards_.constEnd(); ++it)
        it.value()->setVisible(userIds.contains(it.key()));
    updateCount();
}

void UsersSidebar::showAllUsers()
{
    filterActive_ = false;
    currentFilter_.clear();
    for (auto* card : cards_)
        card->setVisible(true);
    updateCount();
}

void UserCard::mousePressEvent(QMouseEvent* event)
{
    QMenu menu(this);
    menu.setStyleSheet(QString(
        "QMenu { background: %1; color: %2; border: 1px solid #334155; border-radius: 6px; padding: 4px; }"
        "QMenu::item { padding: 6px 16px; border-radius: 4px; }"
        "QMenu::item:selected { background: %3; color: white; }"
    ).arg(BG_CARD, TEXT_PRI, ACCENT2));

    QAction* viewProfileAction = menu.addAction("View Profile");
    QAction* messageAction     = menu.addAction("Message");

    QAction* chosen = menu.exec(event->globalPosition().toPoint());

    if (chosen == viewProfileAction)
        emit viewProfileClicked(userId_);
    else if (chosen == messageAction)
        emit messageClicked(userId_);
}

void UsersSidebar::addUser(const QString& userId, const QString& displayName,
                            const QString& username, const QString& bio)
{
    if (cards_.contains(userId)) return;

    auto* card = new UserCard(userId, displayName, username, bio);
    cards_[userId] = card;
    cardsLayout_->addWidget(card);

    if (filterActive_)
        card->setVisible(currentFilter_.contains(userId));

    updateCount();

    connect(card, &UserCard::viewProfileClicked, this, &UsersSidebar::profileRequested);
    connect(card, &UserCard::messageClicked,      this, &UsersSidebar::messageRequested);
}   

void UsersSidebar::clearAll()
{
    for (auto* card : cards_)
        card->deleteLater();
    cards_.clear();
    filterActive_ = false;
    currentFilter_.clear();
    updateCount();
}