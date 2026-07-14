#include "UsersSidebar.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QMouseEvent>

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

    // avatar circle with first letter
    auto* avatar = new QLabel(displayName.isEmpty() ? "?" : QString(displayName[0].toUpper()));
    avatar->setFixedSize(40,40);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet(QString(
        "background:%1;border-radius:20px;color:white;"
        "font-size:16px;font-weight:bold;").arg(ACCENT2));

    // info
    auto* infoLayout = new QVBoxLayout;
    infoLayout->setSpacing(2);
    infoLayout->setContentsMargins(0,0,0,0);

    auto* nameLbl = new QLabel(displayName);
    nameLbl->setStyleSheet(QString(
        "color:%1;font-size:13px;font-weight:bold;background:transparent;").arg(TEXT_PRI));
    nameLbl->setMaximumWidth(140);

    auto* usernameLbl = new QLabel("@" + username);
    usernameLbl->setStyleSheet(QString(
        "color:%1;font-size:11px;background:transparent;").arg(TEXT_SEC));

    // bio preview — first 40 chars
    QString bioPreview = bio.left(40) + (bio.length() > 40 ? "..." : "");
    auto* bioLbl = new QLabel(bioPreview);
    bioLbl->setStyleSheet(QString(
        "color:%1;font-size:11px;background:transparent;").arg(TEXT_SEC));
    bioLbl->setWordWrap(false);

    infoLayout->addWidget(nameLbl);
    infoLayout->addWidget(usernameLbl);
    if (!bioPreview.isEmpty())
        infoLayout->addWidget(bioLbl);

    layout->addWidget(avatar);
    layout->addLayout(infoLayout, 1);
}

void UserCard::mousePressEvent(QMouseEvent*) { emit clicked(userId_); }

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

    auto* title = new QLabel("Online");
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

void UsersSidebar::addUser(const QString& userId, const QString& displayName,
                            const QString& username, const QString& bio)
{
    if (cards_.contains(userId)) return;

    auto* card = new UserCard(userId, displayName, username, bio);
    cards_[userId] = card;
    cardsLayout_->addWidget(card);
    countLabel_->setText(QString::number(cards_.size()));

    connect(card, &UserCard::clicked, this, &UsersSidebar::userClicked);
}

void UsersSidebar::removeUser(const QString& userId)
{
    if (!cards_.contains(userId)) return;
    cards_[userId]->deleteLater();
    cards_.remove(userId);
    countLabel_->setText(QString::number(cards_.size()));
}