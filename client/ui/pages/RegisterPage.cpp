#include "RegisterPage.h"
#include "ui/theme/Theme.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>

RegisterPage::RegisterPage(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("RegisterPage { %1 }").arg(Theme::pageBackground()));

    auto* outer = new QVBoxLayout(this);
    outer->setAlignment(Qt::AlignCenter);
    outer->setSpacing(0);

    auto* card = new QWidget;
    card->setObjectName("registerCard");
    card->setFixedWidth(400);
    card->setStyleSheet(QString("#registerCard { %1 }").arg(Theme::card()));

    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(40, 36, 40, 36);
    cardLayout->setSpacing(6);

    auto* backBtn = new QPushButton("Back");
    backBtn->setFlat(true);
    backBtn->setFixedHeight(24);
    backBtn->setStyleSheet(QString(
        "QPushButton { background: transparent; color: %1; border: none; font-size: 12px; text-align: left; }"
        "QPushButton:hover { color: %2; }"
    ).arg(Theme::TEXT_SECONDARY, Theme::ACCENT));

    auto* title = new QLabel("Create your account");
    title->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::heading()));

    auto* subtitle = new QLabel("Join the AUC community");
    subtitle->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));

    auto addField = [&](QLineEdit*& field, const QString& label, const QString& placeholder, bool password = false) {
        auto* l = new QLabel(label);
        l->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));
        cardLayout->addWidget(l);
        field = new QLineEdit;
        field->setPlaceholderText(placeholder);
        field->setFixedHeight(36);
        field->setStyleSheet(Theme::textInput());
        if (password) field->setEchoMode(QLineEdit::Password);
        cardLayout->addWidget(field);
        cardLayout->addSpacing(8);
    };

    cardLayout->addWidget(backBtn, 0, Qt::AlignLeft);
    cardLayout->addSpacing(6);
    cardLayout->addWidget(title);
    cardLayout->addWidget(subtitle);
    cardLayout->addSpacing(16);

    addField(username_,    "Username",      "your.username");
    addField(displayName_, "Display name",  "Your full name");
    addField(email_,       "Email",         "name@aucegypt.edu");
    addField(password_,    "Password",      "At least 8 characters", true);
    addField(uniId_,       "University ID", "900XXXXXX");
    addField(bio_,         "Bio (optional)","Tell us about yourself");

    auto* btnSubmit = new QPushButton("Create account");
    btnSubmit->setFixedHeight(40);
    btnSubmit->setStyleSheet(Theme::primaryButton());

    auto* btnSwitch = new QPushButton("Already have an account? Sign in");
    btnSwitch->setFlat(true);
    btnSwitch->setFixedHeight(28);
    btnSwitch->setStyleSheet(QString(
        "QPushButton { background: transparent; color: %1; border: none; font-size: 12px; }"
        "QPushButton:hover { color: %2; }"
    ).arg(Theme::TEXT_SECONDARY, Theme::ACCENT));

    cardLayout->addSpacing(8);
    cardLayout->addWidget(btnSubmit);
    cardLayout->addSpacing(8);
    cardLayout->addWidget(btnSwitch, 0, Qt::AlignCenter);

    outer->addWidget(card, 0, Qt::AlignCenter);

    connect(btnSubmit, &QPushButton::clicked,     this, &RegisterPage::onSubmit);
    connect(btnSwitch, &QPushButton::clicked,     this, &RegisterPage::loginClicked);
    connect(backBtn,   &QPushButton::clicked,     this, &RegisterPage::backClicked);
    connect(password_, &QLineEdit::returnPressed, this, &RegisterPage::onSubmit);
}

void RegisterPage::onSubmit()
{
    if (username_->text().isEmpty() || displayName_->text().isEmpty() ||
        email_->text().isEmpty()    || password_->text().isEmpty()    ||
        uniId_->text().isEmpty()) {
        QMessageBox::warning(this, "Missing fields", "Please fill in all required fields.");
        return;
    }
    Message msg;
    msg.type         = MessageType::AUTH_REGISTER;
    msg.username     = username_->text().toStdString();
    msg.displayName  = displayName_->text().toStdString();
    msg.email        = email_->text().toStdString();
    msg.password     = password_->text().toStdString();
    msg.universityId = uniId_->text().toStdString();
    msg.bio          = bio_->text().toStdString();
    emit submitted(msg);
}
