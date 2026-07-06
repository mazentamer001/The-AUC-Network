#include "ProfilePanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QMessageBox>
#include <QTimer>

// ── palette (matches HomePage's printed-label aesthetic) ───────────────────────
static const char* BG_MAIN       = "#D4C5B6"; // Warm Sand/Beige
static const char* BG_CARD       = "#CBBBA8"; // Slightly deeper sand for cards/avatar
static const char* TEXT_MAIN     = "#0F0F0F"; // Ink Black
static const char* TEXT_SEC      = "#5A4B3E"; // Muted brown-black
static const char* ACCENT_ORANGE = "#E65C40"; // Stamp Red-Orange
static const char* DANGER        = "#B3261E"; // Muted brick-red for destructive action

static QString inputStyle() {
    return QString("QLineEdit{background:transparent;color:%1;border:none;"
        "border-bottom:1px solid %1;padding:8px 4px;font-size:14px;}"
        "QLineEdit:focus{border-bottom:2px solid %2;}").arg(TEXT_MAIN, ACCENT_ORANGE);
}

static QString btnStyle(const char* color) {
    return QString(
        "QPushButton{background:transparent;color:%1;border:2px solid %1;"
        "font-size:13px;font-weight:bold;letter-spacing:2px;padding:10px 20px;}"
        "QPushButton:hover{background:%1;color:%2;}").arg(color, BG_MAIN);
}

static QString msgBoxStyle() {
    return QString("QMessageBox{background:%1;color:%2;}"
        "QMessageBox QLabel{color:%2;font-size:13px;}"
        "QMessageBox QPushButton{background:transparent;color:%3;border:1px solid %3;"
        "padding:6px 18px;font-weight:bold;letter-spacing:1px;}"
        "QMessageBox QPushButton:hover{background:%3;color:%1;}")
        .arg(BG_MAIN, TEXT_MAIN, ACCENT_ORANGE);
}

static QFrame* divider() {
    auto* f = new QFrame;
    f->setFrameShape(QFrame::HLine);
    f->setFixedHeight(1);
    f->setStyleSheet(QString("background:%1;border:none;").arg(TEXT_MAIN));
    return f;
}

static QLabel* sectionLabel(const QString& t) {
    auto* l = new QLabel(t);
    l->setStyleSheet(QString(
        "color:%1;font-size:11px;font-weight:bold;letter-spacing:2px;background:transparent;")
        .arg(TEXT_SEC));
    return l;
}

// ─────────────────────────────────────────────────────────────────────────────
ProfilePanel::ProfilePanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("background:%1;").arg(BG_MAIN));

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(QString(
        "QScrollArea{border:none;background:%1;}"
        "QScrollBar:vertical{background:%2;width:5px;}"
        "QScrollBar::handle:vertical{background:%3;}")
        .arg(BG_MAIN, BG_CARD, TEXT_SEC));

    auto* container = new QWidget;
    container->setAttribute(Qt::WA_StyledBackground, true);
    container->setStyleSheet(QString("background:%1;").arg(BG_MAIN));

    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(50,50,50,50);
    layout->setSpacing(28);

    // ── page title, mirroring HomePage's branding treatment ────────────────
    auto* pageTitle = new QLabel("MY PROFILE");
    pageTitle->setStyleSheet(QString(
        "color:%1;font-size:36px;font-weight:900;letter-spacing:5px;background:transparent;")
        .arg(TEXT_MAIN));
    layout->addWidget(pageTitle);
    layout->addSpacing(4);
    layout->addWidget(divider());

    // ── avatar + basic info ──────────────────────────────────────────────
    auto* headerCard = new QWidget;
    headerCard->setMaximumWidth(700);
    auto* headerLayout = new QHBoxLayout(headerCard);
    headerLayout->setContentsMargins(0,20,0,20);
    headerLayout->setSpacing(24);

    // avatar box — square, bordered, label-tag style (no circle)
    avatarLabel_ = new QLabel("👤");
    avatarLabel_->setFixedSize(80,80);
    avatarLabel_->setAlignment(Qt::AlignCenter);
    avatarLabel_->setStyleSheet(QString(
        "background:transparent;border:1px solid %1;font-size:32px;color:%1;").arg(TEXT_MAIN));

    auto* infoLayout = new QVBoxLayout;
    infoLayout->setSpacing(4);

    usernameDisplay_ = new QLabel;
    usernameDisplay_->setStyleSheet(QString(
        "color:%1;font-size:22px;font-weight:900;letter-spacing:1px;background:transparent;")
        .arg(TEXT_MAIN));

    roleLabel_ = new QLabel;
    roleLabel_->setStyleSheet(QString(
        "color:%1;font-size:11px;font-weight:bold;background:transparent;"
        "border:1px solid %1;padding:2px 8px;letter-spacing:1px;").arg(ACCENT_ORANGE));
    roleLabel_->setFixedHeight(20);
    roleLabel_->setFixedWidth(roleLabel_->sizeHint().width());

    emailLabel_ = new QLabel;
    emailLabel_->setStyleSheet(QString(
        "color:%1;font-size:13px;background:transparent;").arg(TEXT_SEC));

    uniIdLabel_ = new QLabel;
    uniIdLabel_->setStyleSheet(QString(
        "color:%1;font-size:13px;background:transparent;").arg(TEXT_SEC));

    infoLayout->addWidget(usernameDisplay_);
    infoLayout->addWidget(roleLabel_);
    infoLayout->addSpacing(4);
    infoLayout->addWidget(emailLabel_);
    infoLayout->addWidget(uniIdLabel_);

    headerLayout->addWidget(avatarLabel_);
    headerLayout->addLayout(infoLayout, 1);

    layout->addWidget(headerCard);
    layout->addWidget(divider());

    // ── edit profile section ─────────────────────────────────────────────
    auto* editCard = new QWidget;
    editCard->setMaximumWidth(700);
    auto* editLayout = new QVBoxLayout(editCard);
    editLayout->setContentsMargins(0,20,0,20);
    editLayout->setSpacing(14);

    auto* editTitle = new QLabel("EDIT PROFILE");
    editTitle->setStyleSheet(QString(
        "color:%1;font-size:18px;font-weight:900;letter-spacing:2px;background:transparent;")
        .arg(TEXT_MAIN));
    editLayout->addWidget(editTitle);
    editLayout->addSpacing(6);

    editLayout->addWidget(sectionLabel("DISPLAY NAME"));
    displayNameInput_ = new QLineEdit;
    displayNameInput_->setPlaceholderText("Display name");
    displayNameInput_->setFixedHeight(38);
    displayNameInput_->setStyleSheet(inputStyle());
    editLayout->addWidget(displayNameInput_);

    editLayout->addWidget(sectionLabel("USERNAME"));
    usernameInput_ = new QLineEdit;
    usernameInput_->setPlaceholderText("Username");
    usernameInput_->setFixedHeight(38);
    usernameInput_->setStyleSheet(inputStyle());
    editLayout->addWidget(usernameInput_);

    editLayout->addWidget(sectionLabel("BIO"));
    bioInput_ = new QLineEdit;
    bioInput_->setPlaceholderText("Tell people about yourself...");
    bioInput_->setFixedHeight(38);
    bioInput_->setStyleSheet(inputStyle());
    editLayout->addWidget(bioInput_);

    editLayout->addWidget(sectionLabel("PROFILE PICTURE URL"));
    profilePicInput_ = new QLineEdit;
    profilePicInput_->setPlaceholderText("https://...");
    profilePicInput_->setFixedHeight(38);
    profilePicInput_->setStyleSheet(inputStyle());
    editLayout->addWidget(profilePicInput_);

    editLayout->addSpacing(10);
    auto* btnSave = new QPushButton("SAVE CHANGES");
    btnSave->setFixedHeight(44);
    btnSave->setMaximumWidth(220);
    btnSave->setStyleSheet(btnStyle(ACCENT_ORANGE));
    editLayout->addWidget(btnSave);

    layout->addWidget(editCard);
    layout->addWidget(divider());

    // ── change password section ──────────────────────────────────────────
    auto* passCard = new QWidget;
    passCard->setMaximumWidth(700);
    auto* passLayout = new QVBoxLayout(passCard);
    passLayout->setContentsMargins(0,20,0,20);
    passLayout->setSpacing(14);

    auto* passTitle = new QLabel("CHANGE PASSWORD");
    passTitle->setStyleSheet(QString(
        "color:%1;font-size:18px;font-weight:900;letter-spacing:2px;background:transparent;")
        .arg(TEXT_MAIN));
    passLayout->addWidget(passTitle);
    passLayout->addSpacing(6);

    passLayout->addWidget(sectionLabel("CURRENT PASSWORD"));
    currentPassInput_ = new QLineEdit;
    currentPassInput_->setEchoMode(QLineEdit::Password);
    currentPassInput_->setPlaceholderText("Current password");
    currentPassInput_->setFixedHeight(38);
    currentPassInput_->setStyleSheet(inputStyle());
    passLayout->addWidget(currentPassInput_);

    passLayout->addWidget(sectionLabel("NEW PASSWORD"));
    newPassInput_ = new QLineEdit;
    newPassInput_->setEchoMode(QLineEdit::Password);
    newPassInput_->setPlaceholderText("New password (min 8 characters)");
    newPassInput_->setFixedHeight(38);
    newPassInput_->setStyleSheet(inputStyle());
    passLayout->addWidget(newPassInput_);

    passLayout->addWidget(sectionLabel("CONFIRM NEW PASSWORD"));
    confirmPassInput_ = new QLineEdit;
    confirmPassInput_->setEchoMode(QLineEdit::Password);
    confirmPassInput_->setPlaceholderText("Confirm new password");
    confirmPassInput_->setFixedHeight(38);
    confirmPassInput_->setStyleSheet(inputStyle());
    passLayout->addWidget(confirmPassInput_);

    passLayout->addSpacing(10);
    auto* btnPass = new QPushButton("CHANGE PASSWORD");
    btnPass->setFixedHeight(44);
    btnPass->setMaximumWidth(220);
    btnPass->setStyleSheet(btnStyle(DANGER));
    passLayout->addWidget(btnPass);

    layout->addWidget(passCard);

    // ── assemble ──────────────────────────────────────────────────────────
    layout->addStretch();

    scroll->setWidget(container);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->addWidget(scroll);

    connect(btnSave, &QPushButton::clicked, this, &ProfilePanel::onSaveProfile);
    connect(btnPass, &QPushButton::clicked, this, &ProfilePanel::onChangePassword);
}

// ─────────────────────────────────────────────────────────────────────────────
void ProfilePanel::setCurrentUser(const QString& displayName,
                                   const QString& userId,
                                   const QString& username,
                                   const QString& role,
                                   const QString& token)
{
    token_           = token;
    userId_          = userId;
    currentUsername_ = username;

    // populate header immediately from login data
    usernameDisplay_->setText(username.toUpper());
    roleLabel_->setText(role.toUpper());
    displayNameInput_->setText(displayName);
    usernameInput_->setText(username);

    // request full profile from server
    QTimer::singleShot(300, this, [this](){
        Message msg;
        msg.type  = MessageType::PROFILE_GET;
        msg.token = token_.toStdString();
        emit sendMessage(msg);
    });
}

// ─────────────────────────────────────────────────────────────────────────────
void ProfilePanel::receiveMessage(const Message& msg)
{
    if (msg.type == MessageType::PROFILE_GET) {
        populateFields(msg);
        return;
    }

    if (msg.type == MessageType::PROFILE_EDIT) {
        auto* box = new QMessageBox(this);
        box->setStyleSheet(msgBoxStyle());
        box->setWindowTitle("Profile Updated");
        box->setText("Your profile has been updated successfully.");
        box->exec();
        return;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ProfilePanel::populateFields(const Message& msg)
{
    QString username    = QString::fromStdString(msg.username);
    QString displayName = QString::fromStdString(msg.displayName);
    QString bio         = QString::fromStdString(msg.bio);
    QString picUrl      = QString::fromStdString(msg.profilePicUrl);
    QString role        = QString::fromStdString(msg.role);
    QString email       = QString::fromStdString(msg.email);
    QString uniId       = QString::fromStdString(msg.universityId);

    if (!username.isEmpty())    usernameDisplay_->setText(username.toUpper());
    if (!displayName.isEmpty()) displayNameInput_->setText(displayName);
    if (!username.isEmpty())    usernameInput_->setText(username);
    if (!bio.isEmpty())         bioInput_->setText(bio);
    if (!picUrl.isEmpty())      profilePicInput_->setText(picUrl);
    if (!role.isEmpty())        roleLabel_->setText(role.toUpper());
    if (!email.isEmpty())       emailLabel_->setText("✉  " + email);
    if (!uniId.isEmpty())       uniIdLabel_->setText("🎓  " + uniId);

    // show profile pic initial if no URL
    if (picUrl.isEmpty())
        avatarLabel_->setText(username.isEmpty() ? "👤" :
                              QString(username[0].toUpper()));
}

// ─────────────────────────────────────────────────────────────────────────────
void ProfilePanel::onSaveProfile()
{
    if (displayNameInput_->text().trimmed().isEmpty()) {
        auto* box = new QMessageBox(this);
        box->setStyleSheet(msgBoxStyle());
        box->setText("Display name cannot be empty.");
        box->exec();
        return;
    }

    Message msg;
    msg.type        = MessageType::PROFILE_EDIT;
    msg.token       = token_.toStdString();
    msg.displayName = displayNameInput_->text().trimmed().toStdString();
    msg.username    = usernameInput_->text().trimmed().toStdString();
    msg.bio         = bioInput_->text().trimmed().toStdString();
    msg.profilePicUrl = profilePicInput_->text().trimmed().toStdString();
    emit sendMessage(msg);
}

// ─────────────────────────────────────────────────────────────────────────────
void ProfilePanel::onChangePassword()
{
    if (currentPassInput_->text().isEmpty() ||
        newPassInput_->text().isEmpty()     ||
        confirmPassInput_->text().isEmpty()) {
        auto* box = new QMessageBox(this);
        box->setStyleSheet(msgBoxStyle());
        box->setText("All password fields are required.");
        box->exec();
        return;
    }

    if (newPassInput_->text() != confirmPassInput_->text()) {
        auto* box = new QMessageBox(this);
        box->setStyleSheet(msgBoxStyle());
        box->setText("New passwords do not match.");
        box->exec();
        return;
    }

    if (newPassInput_->text().length() < 8) {
        auto* box = new QMessageBox(this);
        box->setStyleSheet(msgBoxStyle());
        box->setText("Password must be at least 8 characters.");
        box->exec();
        return;
    }

    Message msg;
    msg.type     = MessageType::PROFILE_EDIT;
    msg.token    = token_.toStdString();
    msg.password = newPassInput_->text().toStdString();
    msg.text     = currentPassInput_->text().toStdString(); // current pass for verification
    emit sendMessage(msg);

    currentPassInput_->clear();
    newPassInput_->clear();
    confirmPassInput_->clear();
}