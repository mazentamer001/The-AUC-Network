#include "RegisterPage.h"
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
QFrame* createRegDivider() {
    QFrame* line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFixedHeight(1);
    line->setStyleSheet(QString("background-color: %1; border: none;").arg(TEXT_MAIN));
    return line;
}

RegisterPage::RegisterPage(QWidget* parent) : QWidget(parent)
{
    // Force the custom sand background
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("RegisterPage { background-color: %1; }").arg(BG_MAIN));

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(50, 40, 50, 40); // Tighter bottom margin to fit all fields comfortably
    outer->setSpacing(0);

    // ── TOP NAVIGATION (Back Button) ──────────────────────────────────────
    auto* btnBack = new QPushButton("← RETURN");
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
    card->setFixedWidth(460);
    // Flat 1px outline border with no roundness
    card->setStyleSheet(QString("background: transparent; border: 1px solid %1;").arg(TEXT_MAIN));

    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(40, 36, 40, 36);
    cardLayout->setSpacing(14);

    auto* title = new QLabel("NEW REGISTRATION");
    title->setStyleSheet(QString(
        "border: none; color: %1; font-size: 24px; font-weight: 800; "
        "letter-spacing: 3px; font-family: sans-serif;"
    ).arg(TEXT_MAIN));
    
    auto* sub = new QLabel("INDEX CREATION // SYSTEM ENTRY");
    sub->setStyleSheet(QString(
        "border: none; color: %1; font-size: 11px; font-weight: bold; "
        "letter-spacing: 2px; font-family: monospace;"
    ).arg(TEXT_MAIN));

    cardLayout->addWidget(title);
    cardLayout->addWidget(sub);
    cardLayout->addSpacing(5);
    cardLayout->addWidget(createRegDivider());
    cardLayout->addSpacing(5);

    // Helper to generate styled fields
    auto addField = [&](QLineEdit*& f, const QString& ph, bool pw = false) {
        f = new QLineEdit; 
        f->setPlaceholderText(ph);
        f->setStyleSheet(inputStyle()); 
        f->setFixedHeight(44);
        if(pw) f->setEchoMode(QLineEdit::Password);
        cardLayout->addWidget(f);
    };

    // Fields initialized with uppercase placeholders to match the brutalist aesthetic
    addField(username_,    "USERNAME");
    addField(displayName_, "DISPLAY NAME");
    addField(email_,       "EMAIL ADDRESS");
    addField(password_,    "PASSWORD", true);
    addField(uniId_,       "UNIVERSITY ID (900XXXXXX)");
    addField(bio_,         "BIO (OPTIONAL)");

    cardLayout->addSpacing(10);
    
    // Solid Ink-Black Action Button
    auto* btnSubmit = new QPushButton("INITIALIZE ACCOUNT");
    btnSubmit->setFixedHeight(50);
    btnSubmit->setStyleSheet(QString(
        "QPushButton {"
        "  background: %1; color: %2; border: none; border-radius: 0px;"
        "  font-size: 14px; font-weight: bold; letter-spacing: 3px;"
        "}"
        "QPushButton:hover {"
        "  background: %3; color: %2;"
        "}"
    ).arg(TEXT_MAIN, BG_MAIN, ACCENT_ORANGE));
    cardLayout->addWidget(btnSubmit);

    outer->addWidget(card, 0, Qt::AlignCenter);
    
    outer->addStretch(1);

    // ── FOOTER SWITCH ACTION ──────────────────────────────────────────────
    auto* btnSwitch = new QPushButton("EXISTING INDEX? VALIDATE HERE →");
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
    connect(btnSubmit, &QPushButton::clicked,     this, &RegisterPage::onSubmit);
    connect(btnSwitch, &QPushButton::clicked,     this, &RegisterPage::loginClicked);
    connect(btnBack,   &QPushButton::clicked,     this, &RegisterPage::backClicked);
    connect(password_, &QLineEdit::returnPressed, this, &RegisterPage::onSubmit);
}

void RegisterPage::onSubmit()
{
    if (username_->text().isEmpty() || displayName_->text().isEmpty() ||
        email_->text().isEmpty()    || password_->text().isEmpty()    ||
        uniId_->text().isEmpty()) {
        QMessageBox::warning(this, "Missing Fields", "All parameters except Bio are required for initialization.");
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