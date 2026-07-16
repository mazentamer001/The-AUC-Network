#pragma once
#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStackedWidget>
#include "Message.h"

class QLabel;
class QLineEdit;
class QTextEdit;
class QPushButton;
class QComboBox;

// ── profile view ──────────────────────────────────────────────────────────────
class ProfilePanel : public QWidget {
    Q_OBJECT
public:
explicit ProfilePanel(QWidget* parent = nullptr);

void setCurrentUser(const QString& displayName, const QString& userId,
const QString& username,    const QString& role,
const QString& token);
void receiveMessage(const Message& msg);

signals:
void sendMessage(const Message& msg);
void photoChanged(const QString& userId, const QString& photoData); 

private slots:
void onSaveProfile();
void onChangePassword();
void onChoosePhoto();

private:
void requestProfile();
void populateFields(const Message& msg);
void renderAvatar(const QString& data);   // data is either an http(s) URL or raw base64 image bytes

    // display
    QLabel* avatarLabel_;
    QLabel* usernameDisplay_;
    QLabel* roleLabel_;
    QLabel* emailLabel_;
    QLabel* uniIdLabel_;
    QLabel* photoStatusLabel_;

    // editable
    QLineEdit* displayNameInput_;
    QLineEdit* usernameInput_;
    QLineEdit* bioInput_;
    QLineEdit* majorInput_;
    QComboBox* yearInput_;
    QLineEdit* interestsInput_;

    // password change
    QLineEdit* currentPassInput_;
    QLineEdit* newPassInput_;
    QLineEdit* confirmPassInput_;

    QString token_;
    QString userId_;
    QString currentUsername_;
    QString pendingImageBase64_;   // set when a new photo is picked; empty = no change on save
    QString currentPhotoData_;     // whatever we last loaded/set, for reference

    QNetworkAccessManager* netManager_ = nullptr;
};