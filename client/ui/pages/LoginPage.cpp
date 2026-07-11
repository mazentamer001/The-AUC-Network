#include "LoginPage.h"
#include "ui/theme/Theme.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>

LoginPage::LoginPage(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("LoginPage { %1 }").arg(Theme::pageBackground()));

    auto* outer = new QVBoxLayout(this);
    outer->setAlignment(Qt::AlignCenter);
    outer->setSpacing(0);

    auto* card = new QWidget;
    card->setObjectName("loginCard");
    card->setFixedWidth(380);
    card->setStyleSheet(QString("#loginCard { %1 }").arg(Theme::card()));

    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(40, 40, 40, 40);
    cardLayout->setSpacing(6);

    auto* backBtn = new QPushButton("Back");
    backBtn->setFlat(true);
    backBtn->setFixedHeight(24);
    backBtn->setStyleSheet(QString(
        "QPushButton { background: transparent; color: %1; border: none; font-size: 12px; text-align: left; }"
        "QPushButton:hover { color: %2; }"
    ).arg(Theme::TEXT_SECONDARY, Theme::ACCENT));

    auto* title = new QLabel("Welcome back");
    title->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::heading()));

    auto* subtitle = new QLabel("Sign in to continue");
    subtitle->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));

    auto* userLabel = new QLabel("Username");
    userLabel->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));

    username_ = new QLineEdit;
    username_->setPlaceholderText("your.username");
    username_->setFixedHeight(38);
    username_->setStyleSheet(Theme::textInput());

    auto* passLabel = new QLabel("Password");
    passLabel->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));

    password_ = new QLineEdit;
    password_->setPlaceholderText("Enter your password");
    password_->setEchoMode(QLineEdit::Password);
    password_->setFixedHeight(38);
    password_->setStyleSheet(Theme::textInput());

    auto* btnLogin = new QPushButton("Sign in");
    btnLogin->setFixedHeight(40);
    btnLogin->setStyleSheet(Theme::primaryButton());

    auto* btnSwitch = new QPushButton("No account? Register");
    btnSwitch->setFlat(true);
    btnSwitch->setFixedHeight(28);
    btnSwitch->setStyleSheet(QString(
        "QPushButton { background: transparent; color: %1; border: none; font-size: 12px; }"
        "QPushButton:hover { color: %2; }"
    ).arg(Theme::TEXT_SECONDARY, Theme::ACCENT));

    cardLayout->addWidget(backBtn, 0, Qt::AlignLeft);
    cardLayout->addSpacing(8);
    cardLayout->addWidget(title);
    cardLayout->addWidget(subtitle);
    cardLayout->addSpacing(20);
    cardLayout->addWidget(userLabel);
    cardLayout->addWidget(username_);
    cardLayout->addSpacing(12);
    cardLayout->addWidget(passLabel);
    cardLayout->addWidget(password_);
    cardLayout->addSpacing(20);
    cardLayout->addWidget(btnLogin);
    cardLayout->addSpacing(8);
    cardLayout->addWidget(btnSwitch, 0, Qt::AlignCenter);

    outer->addWidget(card, 0, Qt::AlignCenter);

    connect(btnLogin,  &QPushButton::clicked,     this, &LoginPage::onSubmit);
    connect(password_, &QLineEdit::returnPressed, this, &LoginPage::onSubmit);
    connect(btnSwitch, &QPushButton::clicked,     this, &LoginPage::registerClicked);
    connect(backBtn,   &QPushButton::clicked,     this, &LoginPage::backClicked);
}

void LoginPage::onSubmit()
{
    if (username_->text().isEmpty() || password_->text().isEmpty()) {
        QMessageBox::warning(this, "Missing fields", "Please enter your username and password.");
        return;
    }
    Message msg;
    msg.type     = MessageType::AUTH_LOGIN;
    msg.username = username_->text().toStdString();
    msg.password = password_->text().toStdString();
    emit submitted(msg);
}
