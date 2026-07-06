#pragma once
#include <QWidget>
#include <QStackedWidget>
#include "Message.h"

class QLabel;
class QLineEdit;
class QTextEdit;
class QPushButton;

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

private slots:
    void onSaveProfile();
    void onChangePassword();

private:
    void requestProfile();
    void populateFields(const Message& msg);

    // display
    QLabel*    avatarLabel_;
    QLabel*    usernameDisplay_;
    QLabel*    roleLabel_;
    QLabel*    emailLabel_;
    QLabel*    uniIdLabel_;

    // editable
    QLineEdit* displayNameInput_;
    QLineEdit* usernameInput_;
    QLineEdit* bioInput_;
    QLineEdit* profilePicInput_;

    // password change
    QLineEdit* currentPassInput_;
    QLineEdit* newPassInput_;
    QLineEdit* confirmPassInput_;

    QString token_;
    QString userId_;
    QString currentUsername_;
};