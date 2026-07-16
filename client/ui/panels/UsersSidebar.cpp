#include "UsersSidebar.h"
#include "ui/theme/Theme.h" // Includes your Theme namespace
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QMenu>
#include <QAction>
#include <QPixmap>

// ─────────────────────────────────────────────────────────────────────────────
//  UserCard
// ─────────────────────────────────────────────────────────────────────────────
UserCard::UserCard(const QString& userId, const QString& displayName,
                   const QString& username, const QString& bio, const QString& photoData,
                   QWidget* parent)
    : QWidget(parent), userId_(userId), displayName_(displayName)
{
    setAttribute(Qt::WA_StyledBackground, true);
    // Use SURFACE for the card and SURFACE_ALT on hover, matching the light theme
    setStyleSheet(QString(
        "UserCard { background: %1; border-radius: 8px; border: 1px solid %2; }"
        "UserCard:hover { background: %3; }")
        .arg(Theme::SURFACE, Theme::BORDER, Theme::SURFACE_ALT));
    setCursor(Qt::PointingHandCursor);
    setFixedHeight(72);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10,10,10,10);
    layout->setSpacing(10);

    // ── avatar with status dot ────────────────────────────────────────────
    auto* avatarContainer = new QWidget;
    avatarContainer->setFixedSize(44,44);
    avatarContainer->setStyleSheet("background:transparent;border:none;");
    avatarContainer->setAttribute(Qt::WA_StyledBackground, false);

    avatarLabel_ = new QLabel(avatarContainer);
    avatarLabel_->setGeometry(0,0,40,40);
    avatarLabel_->setAlignment(Qt::AlignCenter);
    renderAvatar(photoData);

    // status dot — bottom right of avatar
    statusDot_ = new QLabel(avatarContainer);
    statusDot_->setGeometry(28,28,14,14);
    // Offline state default, border matches the card SURFACE
    statusDot_->setStyleSheet(QString(
        "background:#D1D5DB;border-radius:7px;"
        "border:2px solid %1;").arg(Theme::SURFACE));

    // ── info ──────────────────────────────────────────────────────────────
    auto* infoLayout = new QVBoxLayout;
    infoLayout->setSpacing(2);
    infoLayout->setContentsMargins(0,0,0,0);

    auto* nameLbl = new QLabel(displayName);
    nameLbl->setStyleSheet(QString(
        "color:%1;font-size:13px;font-weight:bold;background:transparent;border:none;")
        .arg(Theme::TEXT_PRIMARY));
    nameLbl->setMaximumWidth(130);

    auto* usernameLbl = new QLabel("@" + username);
    usernameLbl->setStyleSheet(QString(
        "color:%1;font-size:11px;background:transparent;border:none;")
        .arg(Theme::TEXT_SECONDARY));

    QString bioPreview = bio.left(36) + (bio.length() > 36 ? "..." : "");
    auto* bioLbl = new QLabel(bioPreview);
    bioLbl->setStyleSheet(QString(
        "color:%1;font-size:11px;background:transparent;border:none;")
        .arg(Theme::TEXT_SECONDARY));

    infoLayout->addWidget(nameLbl);
    infoLayout->addWidget(usernameLbl);
    if (!bioPreview.isEmpty())
        infoLayout->addWidget(bioLbl);

    layout->addWidget(avatarContainer);
    layout->addLayout(infoLayout, 1);
}

void UserCard::renderAvatar(const QString& photoData)
{
    // try to decode an actual profile picture first; fall back to the
    // gradient-initial placeholder if there's no photo or it fails to decode
    if (!photoData.isEmpty()) {
        QPixmap pix;
        // support both raw base64 image bytes (our embedded scheme) and,
        // just in case, a plain http(s) URL left over from manual entry —
        // we can't fetch a URL synchronously here, so only base64 renders
        bool looksLikeUrl = photoData.startsWith("http://") || photoData.startsWith("https://");
        if (!looksLikeUrl && pix.loadFromData(QByteArray::fromBase64(photoData.toUtf8()))) {
            avatarLabel_->setPixmap(pix.scaled(40, 40, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
            avatarLabel_->setText("");
            avatarLabel_->setStyleSheet(
                "background:transparent;border-radius:20px;border:none;");
            return;
        }
    }

    // fallback: gradient circle with the first letter of the display name
    avatarLabel_->setPixmap(QPixmap());
    avatarLabel_->setText(displayName_.isEmpty() ? "?" : QString(displayName_[0].toUpper()));
    avatarLabel_->setStyleSheet(QString(
        "background:%1;border-radius:20px;color:white;"
        "font-size:16px;font-weight:bold;border:none;").arg(Theme::ACCENT));
}

void UserCard::setPhoto(const QString& photoData)
{
    renderAvatar(photoData);
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
        statusDot_->setStyleSheet(QString(
            "background:#22c55e;border-radius:7px;border:2px solid %1;")
            .arg(Theme::SURFACE));
        auto* glow = new QGraphicsDropShadowEffect;
        glow->setBlurRadius(8);
        glow->setOffset(0,0);
        glow->setColor(QColor("#22c55e"));
        statusDot_->setGraphicsEffect(glow);
        break;
    }
    case UserStatus::AWAY: {
        statusDot_->setStyleSheet(QString(
            "background:#f59e0b;border-radius:7px;border:2px solid %1;")
            .arg(Theme::SURFACE));
        auto* glow = new QGraphicsDropShadowEffect;
        glow->setBlurRadius(8);
        glow->setOffset(0,0);
        glow->setColor(QColor("#f59e0b"));
        statusDot_->setGraphicsEffect(glow);
        break;
    }
    case UserStatus::OFFLINE:
    default:
        statusDot_->setStyleSheet(QString(
            "background:#D1D5DB;border-radius:7px;border:2px solid %1;")
            .arg(Theme::SURFACE));
        break;
    }
}

void UserCard::mousePressEvent(QMouseEvent* event)
{
    QMenu menu(this);
    // Styled menu matching light theme 
    menu.setStyleSheet(QString(
        "QMenu { background: %1; color: %2; border: 1px solid %3; border-radius: 6px; padding: 4px; }"
        "QMenu::item { padding: 6px 16px; border-radius: 4px; }"
        "QMenu::item:selected { background: %4; color: white; }"
    ).arg(Theme::SURFACE, Theme::TEXT_PRIMARY, Theme::BORDER, Theme::ACCENT));

    QAction* viewProfileAction = menu.addAction("View Profile");
    QAction* messageAction     = menu.addAction("Message");

    QAction* chosen = menu.exec(event->globalPosition().toPoint());

    if (chosen == viewProfileAction)
        emit viewProfileClicked(userId_);
    else if (chosen == messageAction)
        emit messageClicked(userId_);
}

// ─────────────────────────────────────────────────────────────────────────────
//  UsersSidebar
// ─────────────────────────────────────────────────────────────────────────────
UsersSidebar::UsersSidebar(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("background:%1;border-left:1px solid %2;")
        .arg(Theme::SURFACE_ALT, Theme::BORDER));
    setFixedWidth(200);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    // header
    auto* header = new QWidget;
    header->setFixedHeight(52);
    header->setAttribute(Qt::WA_StyledBackground, true);
    header->setStyleSheet(QString(
        "background:%1;border-bottom:1px solid %2;border-left:none;")
        .arg(Theme::SURFACE_ALT, Theme::BORDER));
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(14,0,14,0);

    auto* title = new QLabel("Users");
    title->setStyleSheet(QString(
        "color:%1;font-size:13px;font-weight:bold;background:transparent;border:none;")
        .arg(Theme::TEXT_PRIMARY));

    countLabel_ = new QLabel("0");
    countLabel_->setStyleSheet(QString(
        "color:white;background:%1;border-radius:10px;border:none;"
        "padding:1px 7px;font-size:11px;font-weight:bold;").arg(Theme::ACCENT));

    headerLayout->addWidget(title);
    headerLayout->addStretch();
    headerLayout->addWidget(countLabel_);

    // scroll area
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(QString(
        "QScrollArea{border:none;background:%1;}"
        "QScrollBar:vertical{background:%1;width:6px;border-radius:3px; margin: 0px 2px 0px 0px;}"
        "QScrollBar::handle:vertical{background:%2;border-radius:3px;}")
        .arg(Theme::SURFACE_ALT, Theme::BORDER));

    auto* container = new QWidget;
    container->setAttribute(Qt::WA_StyledBackground, true);
    container->setStyleSheet(QString("background:%1;border:none;").arg(Theme::SURFACE_ALT));
    cardsLayout_ = new QVBoxLayout(container);
    cardsLayout_->setContentsMargins(8,8,8,8);
    cardsLayout_->setSpacing(4);
    cardsLayout_->setAlignment(Qt::AlignTop);
    scroll->setWidget(container);

    root->addWidget(header);
    root->addWidget(scroll, 1);
}

void UsersSidebar::addUser(const QString& userId, const QString& displayName,
                            const QString& username, const QString& bio, const QString& photoData)
{
    if (cards_.contains(userId)) return;

    auto* card = new UserCard(userId, displayName, username, bio, photoData);
    cards_[userId] = card;
    cardsLayout_->addWidget(card);

    if (filterActive_)
        card->setVisible(currentFilter_.contains(userId));

    updateCount();

    connect(card, &UserCard::viewProfileClicked, this, &UsersSidebar::profileRequested);
    connect(card, &UserCard::messageClicked,      this, &UsersSidebar::messageRequested);
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

void UsersSidebar::setUserPhoto(const QString& userId, const QString& photoData)
{
    if (cards_.contains(userId))
        cards_[userId]->setPhoto(photoData);
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

void UsersSidebar::clearAll()
{
    for (auto* card : cards_)
        card->deleteLater();
    cards_.clear();
    filterActive_ = false;
    currentFilter_.clear();
    updateCount();
}