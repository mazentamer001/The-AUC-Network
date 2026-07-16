#include "ProfilePanel.h"
#include "ui/theme/Theme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QScrollArea>
#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QPixmap>
#include <iostream>

ProfilePanel::ProfilePanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("ProfilePanel { %1 }").arg(Theme::pageBackground()));
    netManager_ = new QNetworkAccessManager(this);

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(QString(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { background: transparent; width: 6px; }"
        "QScrollBar::handle:vertical { background: %1; border-radius: 3px; }"
    ).arg(Theme::BORDER));

    auto* container = new QWidget;
    container->setStyleSheet("background: transparent;");

    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(50, 40, 50, 40);
    layout->setSpacing(20);

    auto* pageTitle = new QLabel("Your profile");
    pageTitle->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::heading()));
    layout->addWidget(pageTitle);

    auto* headerCard = new QWidget;
    headerCard->setObjectName("profileHeaderCard");
    headerCard->setMaximumWidth(700);
    headerCard->setStyleSheet(QString("#profileHeaderCard { %1 }").arg(Theme::card()));
    auto* headerLayout = new QHBoxLayout(headerCard);
    headerLayout->setContentsMargins(24, 24, 24, 24);
    headerLayout->setSpacing(20);

    avatarLabel_ = new QLabel;
    avatarLabel_->setFixedSize(72, 72);
    avatarLabel_->setAlignment(Qt::AlignCenter);
    avatarLabel_->setStyleSheet(QString(
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 %1, stop:1 %2);"
        "border-radius: 36px; font-size: 26px; font-weight: 600; color: white; border: none;"
    ).arg(Theme::ACCENT, Theme::ACCENT2));

    auto* infoLayout = new QVBoxLayout;
    infoLayout->setSpacing(4);

    usernameDisplay_ = new QLabel;
    usernameDisplay_->setStyleSheet(QString("border: none; background: transparent; color: %1; font-size: 18px; font-weight: 600;").arg(Theme::TEXT_PRIMARY));

    roleLabel_ = new QLabel;
    roleLabel_->setStyleSheet(QString(
        "border: none; background: %1; color: %2; font-size: 11px; font-weight: 600; padding: 2px 10px; border-radius: 8px;"
    ).arg(Theme::SURFACE_ALT, Theme::ACCENT));
    roleLabel_->setFixedWidth(70);

    emailLabel_ = new QLabel;
    emailLabel_->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));

    uniIdLabel_ = new QLabel;
    uniIdLabel_->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));

    infoLayout->addWidget(usernameDisplay_);
    infoLayout->addWidget(roleLabel_);
    infoLayout->addSpacing(4);
    infoLayout->addWidget(emailLabel_);
    infoLayout->addWidget(uniIdLabel_);

    headerLayout->addWidget(avatarLabel_);
    headerLayout->addLayout(infoLayout, 1);

    layout->addWidget(headerCard);

    auto* editCard = new QWidget;
    editCard->setObjectName("editCard");
    editCard->setMaximumWidth(700);
    editCard->setStyleSheet(QString("#editCard { %1 }").arg(Theme::card()));
    auto* editLayout = new QVBoxLayout(editCard);
    editLayout->setContentsMargins(28, 24, 28, 24);
    editLayout->setSpacing(6);

    auto* editTitle = new QLabel("Edit profile");
    editTitle->setStyleSheet(QString("border: none; background: transparent; color: %1; font-size: 16px; font-weight: 500;").arg(Theme::TEXT_PRIMARY));
    editLayout->addWidget(editTitle);
    editLayout->addSpacing(10);

    auto lbl = [](const QString& t) {
        auto* l = new QLabel(t);
        l->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));
        return l;
    };

    editLayout->addWidget(lbl("Display name"));
    displayNameInput_ = new QLineEdit;
    displayNameInput_->setFixedHeight(36);
    displayNameInput_->setStyleSheet(Theme::textInput());
    editLayout->addWidget(displayNameInput_);
    editLayout->addSpacing(8);

    editLayout->addWidget(lbl("Username"));
    usernameInput_ = new QLineEdit;
    usernameInput_->setFixedHeight(36);
    usernameInput_->setStyleSheet(Theme::textInput());
    editLayout->addWidget(usernameInput_);
    editLayout->addSpacing(8);

    editLayout->addWidget(lbl("Bio"));
    bioInput_ = new QLineEdit;
    bioInput_->setPlaceholderText("Tell people about yourself...");
    bioInput_->setFixedHeight(36);
    bioInput_->setStyleSheet(Theme::textInput());
    editLayout->addWidget(bioInput_);
    editLayout->addSpacing(8);

    editLayout->addWidget(lbl("Profile picture"));
    auto* picRow = new QHBoxLayout;
    picRow->setSpacing(8);
    photoStatusLabel_ = new QLabel("No photo selected");
    photoStatusLabel_->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));
    auto* btnChoosePhoto = new QPushButton("Choose photo...");
    btnChoosePhoto->setFixedHeight(36);
    btnChoosePhoto->setStyleSheet(QString(
        "QPushButton { background: %1; color: %2; border: 1px solid %3; border-radius: 8px; padding: 0 14px; font-size: 12px; font-weight: 500; }"
        "QPushButton:hover { background: %4; }"
    ).arg(Theme::SURFACE_ALT, Theme::TEXT_PRIMARY, Theme::BORDER, Theme::SURFACE));
    picRow->addWidget(photoStatusLabel_, 1);
    picRow->addWidget(btnChoosePhoto);
    editLayout->addLayout(picRow);
    editLayout->addSpacing(8);

    editLayout->addWidget(lbl("Major"));
    majorInput_ = new QLineEdit;
    majorInput_->setPlaceholderText("e.g. Computer Science");
    majorInput_->setFixedHeight(36);
    majorInput_->setStyleSheet(Theme::textInput());
    editLayout->addWidget(majorInput_);
    editLayout->addSpacing(8);

    editLayout->addWidget(lbl("Year"));
    yearInput_ = new QComboBox;
    yearInput_->addItems({"", "Freshman", "Sophomore", "Junior", "Senior", "Grad"});
    yearInput_->setFixedHeight(36);
    yearInput_->setStyleSheet(Theme::textInput());
    editLayout->addWidget(yearInput_);
    editLayout->addSpacing(8);

    editLayout->addWidget(lbl("Interests"));
    interestsInput_ = new QLineEdit;
    interestsInput_->setPlaceholderText("e.g. Robotics, Photography, Chess");
    interestsInput_->setFixedHeight(36);
    interestsInput_->setStyleSheet(Theme::textInput());
    editLayout->addWidget(interestsInput_);
    editLayout->addSpacing(14);

    auto* btnSave = new QPushButton("Save changes");
    btnSave->setFixedHeight(40);
    btnSave->setMaximumWidth(200);
    btnSave->setStyleSheet(Theme::primaryButton());
    editLayout->addWidget(btnSave);

    layout->addWidget(editCard);

    auto* passCard = new QWidget;
    passCard->setObjectName("passCard");
    passCard->setMaximumWidth(700);
    passCard->setStyleSheet(QString("#passCard { %1 }").arg(Theme::card()));
    auto* passLayout = new QVBoxLayout(passCard);
    passLayout->setContentsMargins(28, 24, 28, 24);
    passLayout->setSpacing(6);

    auto* passTitle = new QLabel("Change password");
    passTitle->setStyleSheet(QString("border: none; background: transparent; color: %1; font-size: 16px; font-weight: 500;").arg(Theme::TEXT_PRIMARY));
    passLayout->addWidget(passTitle);
    passLayout->addSpacing(10);

    passLayout->addWidget(lbl("Current password"));
    currentPassInput_ = new QLineEdit;
    currentPassInput_->setEchoMode(QLineEdit::Password);
    currentPassInput_->setFixedHeight(36);
    currentPassInput_->setStyleSheet(Theme::textInput());
    passLayout->addWidget(currentPassInput_);
    passLayout->addSpacing(8);

    passLayout->addWidget(lbl("New password"));
    newPassInput_ = new QLineEdit;
    newPassInput_->setEchoMode(QLineEdit::Password);
    newPassInput_->setPlaceholderText("At least 8 characters");
    newPassInput_->setFixedHeight(36);
    newPassInput_->setStyleSheet(Theme::textInput());
    passLayout->addWidget(newPassInput_);
    passLayout->addSpacing(8);

    passLayout->addWidget(lbl("Confirm new password"));
    confirmPassInput_ = new QLineEdit;
    confirmPassInput_->setEchoMode(QLineEdit::Password);
    confirmPassInput_->setFixedHeight(36);
    confirmPassInput_->setStyleSheet(Theme::textInput());
    passLayout->addWidget(confirmPassInput_);
    passLayout->addSpacing(14);

    auto* btnPass = new QPushButton("Change password");
    btnPass->setFixedHeight(40);
    btnPass->setMaximumWidth(200);
    btnPass->setStyleSheet(QString(
        "QPushButton { background: %1; color: white; border: none; border-radius: 8px; padding: 10px 20px; font-size: 13px; font-weight: 500; }"
    ).arg(Theme::DANGER));
    passLayout->addWidget(btnPass);

    layout->addWidget(passCard);
    layout->addStretch();

    scroll->setWidget(container);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(scroll);

    connect(btnSave,        &QPushButton::clicked, this, &ProfilePanel::onSaveProfile);
    connect(btnPass,        &QPushButton::clicked, this, &ProfilePanel::onChangePassword);
    connect(btnChoosePhoto, &QPushButton::clicked, this, &ProfilePanel::onChoosePhoto);
}

void ProfilePanel::renderAvatar(const QString& data)
{
    if (data.isEmpty()) {
        avatarLabel_->setPixmap(QPixmap());
        QString u = usernameDisplay_->text();
        avatarLabel_->setText(u.isEmpty() ? "?" : QString(u[0].toUpper()));
        return;
    }

    if (data.startsWith("http://") || data.startsWith("https://")) {
        // legacy/manual external URL — fetch over the network
        QNetworkRequest req{QUrl(data)};
        QNetworkReply* reply = netManager_->get(req);
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            reply->deleteLater();
            if (reply->error() != QNetworkReply::NoError) {
                std::cerr << "Avatar load failed: " << reply->errorString().toStdString() << "\n";
                return;
            }
            QPixmap pix;
            if (pix.loadFromData(reply->readAll())) {
                avatarLabel_->setPixmap(pix.scaled(
                    avatarLabel_->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
            }
        });
        return;
    }

    // otherwise treat the whole string as raw base64 image bytes — decode locally, no network needed
    QByteArray decoded = QByteArray::fromBase64(data.toUtf8());
    QPixmap pix;
    if (pix.loadFromData(decoded)) {
        avatarLabel_->setPixmap(pix.scaled(
            avatarLabel_->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }
}

void ProfilePanel::onChoosePhoto()
{
    QString path = QFileDialog::getOpenFileName(this, "Choose profile picture", QString(),
                                                  "Images (*.png *.jpg *.jpeg *.gif)");
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Read error", "Could not read the selected image.");
        return;
    }
    QByteArray fileData = file.readAll();
    file.close();

    if (fileData.size() > 500 * 1024) {
        QMessageBox::warning(this, "Image too large", "Profile pictures must be under 500KB.");
        return;
    }

    pendingImageBase64_ = QString::fromLatin1(fileData.toBase64());

    // instant local preview
    QPixmap pix;
    if (pix.loadFromData(fileData)) {
        avatarLabel_->setPixmap(pix.scaled(
            avatarLabel_->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }

    QFileInfo info(path);
    photoStatusLabel_->setText("Selected: " + info.fileName() + " (saved on submit)");
}

void ProfilePanel::setCurrentUser(const QString& displayName, const QString& userId,
                                   const QString& username, const QString& role,
                                   const QString& token)
{
    token_           = token;
    userId_          = userId;
    currentUsername_ = username;

    usernameDisplay_->setText(username);
    roleLabel_->setText(role.isEmpty() ? "User" : role);
    displayNameInput_->setText(displayName);
    usernameInput_->setText(username);
    avatarLabel_->setText(username.isEmpty() ? "?" : QString(username[0].toUpper()));

    QTimer::singleShot(300, this, [this](){
        Message msg;
        msg.type  = MessageType::PROFILE_GET;
        msg.token = token_.toStdString();
        emit sendMessage(msg);
    });
}

void ProfilePanel::receiveMessage(const Message& msg)
{
    if (msg.type == MessageType::PROFILE_GET) {
        populateFields(msg);
        return;
    }
    if (msg.type == MessageType::PROFILE_EDIT) {
        QMessageBox::information(this, "Profile updated", "Your profile has been updated.");
        return;
    }
}

void ProfilePanel::populateFields(const Message& msg)
{
    QString username    = QString::fromStdString(msg.username);
    QString displayName = QString::fromStdString(msg.displayName);
    QString bio         = QString::fromStdString(msg.bio);
    QString picData      = QString::fromStdString(msg.profilePicUrl);
    QString role        = QString::fromStdString(msg.role);
    QString email       = QString::fromStdString(msg.email);
    QString uniId       = QString::fromStdString(msg.universityId);
    QString major       = QString::fromStdString(msg.major);
    QString year        = QString::fromStdString(msg.year);
    QString interests   = QString::fromStdString(msg.interests);

    if (!username.isEmpty())    usernameDisplay_->setText(username);
    if (!displayName.isEmpty()) displayNameInput_->setText(displayName);
    if (!username.isEmpty())    usernameInput_->setText(username);
    if (!bio.isEmpty())         bioInput_->setText(bio);
    if (!role.isEmpty())        roleLabel_->setText(role);
    if (!email.isEmpty())       emailLabel_->setText(email);
    if (!uniId.isEmpty())       uniIdLabel_->setText(uniId);
    if (!major.isEmpty())       majorInput_->setText(major);
    if (!year.isEmpty())        yearInput_->setCurrentText(year);
    if (!interests.isEmpty())   interestsInput_->setText(interests);

    currentPhotoData_ = picData;
    photoStatusLabel_->setText(picData.isEmpty() ? "No photo selected" : "Current photo set");
    renderAvatar(picData);
}

void ProfilePanel::onSaveProfile()
{
    if (displayNameInput_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Missing field", "Display name cannot be empty.");
        return;
    }

    Message msg;
    msg.type          = MessageType::PROFILE_EDIT;
    msg.token         = token_.toStdString();
    msg.displayName   = displayNameInput_->text().trimmed().toStdString();
    msg.username      = usernameInput_->text().trimmed().toStdString();
    msg.bio           = bioInput_->text().trimmed().toStdString();
    // only send a new photo if one was actually picked this session — empty
    // means "leave unchanged" per ProfileService::handleEdit's patch logic
    msg.profilePicUrl = pendingImageBase64_.toStdString();
    msg.major         = majorInput_->text().trimmed().toStdString();
    msg.year          = yearInput_->currentText().toStdString();
    msg.interests     = interestsInput_->text().trimmed().toStdString();
    emit sendMessage(msg);

    pendingImageBase64_.clear();
}

void ProfilePanel::onChangePassword()
{
    if (currentPassInput_->text().isEmpty() || newPassInput_->text().isEmpty() || confirmPassInput_->text().isEmpty()) {
        QMessageBox::warning(this, "Missing fields", "All password fields are required.");
        return;
    }
    if (newPassInput_->text() != confirmPassInput_->text()) {
        QMessageBox::warning(this, "Mismatch", "New passwords do not match.");
        return;
    }
    if (newPassInput_->text().length() < 8) {
        QMessageBox::warning(this, "Too short", "Password must be at least 8 characters.");
        return;
    }

    Message msg;
    msg.type     = MessageType::PROFILE_EDIT;
    msg.token    = token_.toStdString();
    msg.password = newPassInput_->text().toStdString();
    msg.text     = currentPassInput_->text().toStdString();
    emit sendMessage(msg);

    currentPassInput_->clear();
    newPassInput_->clear();
    confirmPassInput_->clear();
}