#include "HomePage.h"
#include "ui/theme/Theme.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

HomePage::HomePage(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("HomePage { %1 }").arg(Theme::pageBackground()));

    auto* outer = new QVBoxLayout(this);
    outer->setAlignment(Qt::AlignCenter);
    outer->setSpacing(0);

    auto* card = new QWidget;
    card->setObjectName("homeCard");
    card->setFixedWidth(380);
    card->setStyleSheet(QString("#homeCard { %1 }").arg(Theme::card()));

    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(40, 48, 40, 48);
    cardLayout->setSpacing(8);
    cardLayout->setAlignment(Qt::AlignCenter);

    auto* title = new QLabel("AUC Network");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::heading()));

    auto* subtitle = new QLabel("Connect with your campus community");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setWordWrap(true);
    subtitle->setFixedWidth(260);
    subtitle->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));

    auto* btnGet = new QPushButton("Get started");
    btnGet->setFixedHeight(40);
    btnGet->setStyleSheet(Theme::primaryButton());

    auto* btnSign = new QPushButton("Sign in");
    btnSign->setFlat(true);
    btnSign->setFixedHeight(32);
    btnSign->setStyleSheet(QString(
        "QPushButton { background: transparent; color: %1; border: none; font-size: 13px; }"
        "QPushButton:hover { color: %2; }"
    ).arg(Theme::NAV_TEXT, Theme::ACCENT2));

    cardLayout->addWidget(title);
    cardLayout->addSpacing(4);
    cardLayout->addWidget(subtitle);
    cardLayout->addSpacing(28);
    cardLayout->addWidget(btnGet);
    cardLayout->addSpacing(8);
    cardLayout->addWidget(btnSign);

    outer->addWidget(card, 0, Qt::AlignCenter);

    connect(btnGet, &QPushButton::clicked, this, &HomePage::registerClicked);
    connect(btnSign, &QPushButton::clicked, this, &HomePage::loginClicked);
}
