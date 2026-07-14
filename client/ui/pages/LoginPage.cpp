#include "LoginPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QFrame>

// Cohesive Label Color Palette
static const char* BG_MAIN       = "#D4C5B6"; // Warm Sand / Parchment
static const char* TEXT_MAIN     = "#0F0F0F"; // Ink Black
static const char* ACCENT_ORANGE = "#E65C40"; // Stamp Red-Orange
static const char* BG_INPUT      = "#C9BBAA"; // Slightly deeper sand tone for depth

static QString inputStyle() {
    return QString(
        "QLineEdit {"
        "  background: %1; color: %2; border: 1px solid %2; border-radius: 0px;" // Sharp corners
        "  padding: 12px 14px; font-size: 13px; font-family: sans-serif;"
        "}"
        "QLineEdit:focus {"
        "  border: 1px solid %3; background: #FFFDFB;" // Highlights with orange on focus
        "}"
    ).arg(BG_INPUT, TEXT_MAIN, ACCENT_ORANGE);
}

// 1px divider helper to match the structured label look
QFrame* createFormDivider() {
    QFrame* line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFixedHeight(1);
    line->setStyleSheet(QString("background-color: %1; border: none;").arg(TEXT_MAIN));
    return line;
}

LoginPage::LoginPage(QWidget* parent) : QWidget(parent)
{
    // 1. Force the custom sand background onto this view
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("LoginPage { background-color: %1; }").arg(BG_MAIN));

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(50, 40, 50, 60);
    outer->setSpacing(0);

    // ── TOP NAVIGATION (Back Button) ──────────────────────────────────────
    auto* btnBack = new QPushButton("← RETURN TO HOME");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(QString(
        "QPushButton {"
        "  color: %1; font-size: 12px; font-weight: bold; letter-spacing: 2px;"
        "  background: transparent; border: none; text-align: left; padding: 10px 0px;"
        "}"
        "QPushButton:hover { color: %2; }"
    ).arg(TEXT_MAIN, ACCENT_ORANGE));
    outer->addWidget(btnBack, 0, Qt::AlignLeft);
    
    outer->addStretch(1);

    // ── THE ENTRY BLOCK (Apothecary Frame) ────────────────────────────────
    auto* card = new QWidget;
    card->setFixedWidth(440);
    // Flat 1px outline border with no roundness—emulating a printed section block
    card->setStyleSheet(QString("background: transparent; border: 1px solid %1;").arg(TEXT_MAIN));

    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(40, 45, 40, 45);
    cardLayout->setSpacing(16);

    // Typography adjustments
    auto* title = new QLabel("SECURE ACCESS");
    title->setStyleSheet(QString(
        "border: none; color: %1; font-size: 28px; font-weight: 800; "
        "letter-spacing: 4px; font-family: sans-serif;"
    ).arg(TEXT_MAIN));
    
    auto* sub = new QLabel("THE NETWORK // VERIFICATION REQUIRED");
    sub->setStyleSheet(QString(
        "border: none; color: %1; font-size: 11px; font-weight: bold; "
        "letter-spacing: 2px; font-family: monospace;"
    ).arg(TEXT_MAIN));

    username_ = new QLineEdit;
    username_->setPlaceholderText("IDENTITY / USERNAME");
    username_->setStyleSheet(inputStyle());
    username_->setFixedHeight(46);

    password_ = new QLineEdit;
    password_->setPlaceholderText("PASSWORD");
    password_->setEchoMode(QLineEdit::Password);
    password_->setStyleSheet(inputStyle());
    password_->setFixedHeight(46);

    // Solid Ink-Black Action Button that flashes Stamp-Orange when hovered
    auto* btnLogin = new QPushButton("VALIDATE & ENTER");
    btnLogin->setFixedHeight(50);
    btnLogin->setStyleSheet(QString(
        "QPushButton {"
        "  background: %1; color: %2; border: none; border-radius: 0px;"
        "  font-size: 14px; font-weight: bold; letter-spacing: 3px;"
        "}"
        "QPushButton:hover {"
        "  background: %3; color: %2;"
        "}"
    ).arg(TEXT_MAIN, BG_MAIN, ACCENT_ORANGE));

    cardLayout->addWidget(title);
    cardLayout->addWidget(sub);
    cardLayout->addSpacing(5);
    cardLayout->addWidget(createFormDivider());
    cardLayout->addSpacing(10);
    cardLayout->addWidget(username_);
    cardLayout->addWidget(password_);
    cardLayout->addSpacing(10);
    cardLayout->addWidget(btnLogin);

    outer->addWidget(card, 0, Qt::AlignCenter);
    
    outer->addStretch(1);

    // ── FOOTER SWITCH ACTION ──────────────────────────────────────────────
    auto* btnSwitch = new QPushButton("NO ACCOUNT? REGISTER OUTLINE HERE →");
    btnSwitch->setFlat(true);
    btnSwitch->setStyleSheet(QString(
        "QPushButton {"
        "  color: %1; font-size: 12px; font-weight: bold; letter-spacing: 2px;"
        "  background: transparent; border: none;"
        "}"
        "QPushButton:hover { color: %2; text-decoration: underline; }"
    ).arg(TEXT_MAIN, ACCENT_ORANGE));
    outer->addWidget(btnSwitch, 0, Qt::AlignCenter);

    // ── SLOTS & CONNECTIONS ───────────────────────────────────────────────
    connect(btnLogin,  &QPushButton::clicked,     this, &LoginPage::onSubmit);
    connect(password_, &QLineEdit::returnPressed, this, &LoginPage::onSubmit);
    connect(btnSwitch, &QPushButton::clicked,     this, &LoginPage::registerClicked);
    connect(btnBack,   &QPushButton::clicked,     this, &LoginPage::backClicked);
}

void LoginPage::onSubmit()
{
    if (username_->text().isEmpty() || password_->text().isEmpty()) {
        QMessageBox::warning(this, "Missing Fields", "Please populate identity parameters completely.");
        return;
    }
    Message msg;
    msg.type     = MessageType::AUTH_LOGIN;
    msg.username = username_->text().toStdString();
    msg.password = password_->text().toStdString();
    emit submitted(msg);
}