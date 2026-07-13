#pragma once
#include <QDialog>
#include <QMap>
#include <QString>
#include <QStringList>

class QLineEdit;
class QRadioButton;
class QListWidget;
class QLabel;

class CreateRoomDialog : public QDialog {
    Q_OBJECT
public:
    // availableUsers: userId -> displayName, used to populate the member picker for private rooms
    explicit CreateRoomDialog(const QMap<QString, QString>& availableUsers, QWidget* parent = nullptr);

    QString     roomName() const;
    bool        isPrivate() const;
    QStringList selectedMemberIds() const;

private slots:
    void onTypeToggled();
    void onAccept();

private:
    QLineEdit*    nameInput_;
    QRadioButton* publicRadio_;
    QRadioButton* privateRadio_;
    QListWidget*  memberList_;
    QLabel*       memberLabel_;

    QMap<QString, QString> availableUsers_;
};