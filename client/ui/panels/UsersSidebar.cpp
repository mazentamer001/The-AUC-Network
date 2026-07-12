#include "UsersSidebar.h"
#include "ui/theme/Theme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QMouseEvent>

// ─────────────────────────────────────────────────────────────────────────────
//  UserCard
// ─────────────────────────────────────────────────────────────────────────────
UserCard::UserCard(const QString& userId, const QString& displayName,
                   const QString& username, const QString& bio,
                   QWidget* parent)
    : QWidget(parent), userId_(userId)
{
    setAttribute(Qt::WA_StyledBackground, true);
    // Updated to use Theme colors
    setStyleSheet(QString(
        "UserCard{background:%1;border-radius:8px;border:1px solid %2;}"
        "UserCard:hover{background:%3;}").arg(Theme::SURFACE, Theme::BORDER, Theme::SURFACE_ALT));
    setCursor(Qt::PointingHandCursor);
    setFixedHeight(72);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10,10,10,10);
    layout->setSpacing(10);

    auto* avatar = new QLabel(displayName.isEmpty() ? "?" : QString(displayName[0].toUpper()));
    avatar->setFixedSize(40,40);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet(QString(
        "background:%1;border-radius:20px;color:white;"
        "font-size:16px;font-weight:bold;").arg(Theme::ACCENT2));

    auto* infoLayout = new QVBoxLayout;
    infoLayout->setSpacing(2);
    infoLayout->setContentsMargins(0,0,0,0);

    auto* nameLbl = new QLabel(displayName);
    nameLbl->setStyleSheet(Theme::bodyText() + "font-weight:bold;");
    nameLbl->setMaximumWidth(140);

    auto* usernameLbl = new QLabel("@" + username);
    usernameLbl->setStyleSheet(Theme::mutedText());

    QString bioPreview = bio.left(40) + (bio.length() > 40 ? "..." : "");
    auto* bioLbl = new QLabel(bioPreview);
    bioLbl->setStyleSheet(Theme::mutedText());
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
    setStyleSheet(QString("background:%1;border-left:1px solid %2;").arg(Theme::SURFACE, Theme::BORDER));
    setFixedWidth(200);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    auto* header = new QWidget;
    header->setFixedHeight(52);
    header->setAttribute(Qt::WA_StyledBackground, true);
    header->setStyleSheet(QString("background:%1;border-bottom:1px solid %2;").arg(Theme::SURFACE, Theme::BORDER));
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(14,0,14,0);

    auto* title = new QLabel("Online");
    title->setStyleSheet(Theme::bodyText() + "font-weight:bold;");

    countLabel_ = new QLabel("0");
    countLabel_->setStyleSheet(QString(
        "color:white;background:%1;border-radius:10px;"
        "padding:1px 7px;font-size:11px;font-weight:bold;").arg(Theme::ACCENT2));

    headerLayout->addWidget(title);
    headerLayout->addStretch();
    headerLayout->addWidget(countLabel_);

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(QString(
        "QScrollArea{border:none;background:%1;}"
        "QScrollBar:vertical{background:%1;width:4px;border-radius:2px;}"
        "QScrollBar::handle:vertical{background:%2;border-radius:2px;}")
        .arg(Theme::SURFACE_ALT, Theme::BORDER));

    auto* container = new QWidget;
    container->setAttribute(Qt::WA_StyledBackground, true);
    container->setStyleSheet(QString("background:%1;").arg(Theme::SURFACE_ALT));
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