#include "CreateRoomDialog.h"
#include "ui/theme/Theme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

CreateRoomDialog::CreateRoomDialog(const QMap<QString, QString>& availableUsers, QWidget* parent)
    : QDialog(parent), availableUsers_(availableUsers)
{
    setWindowTitle("Create room");
    setMinimumWidth(360);
    setStyleSheet(QString("QDialog { background: %1; }").arg(Theme::SURFACE));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(14);

    auto* title = new QLabel("Create a room");
    title->setStyleSheet(QString("border:none; background:transparent; %1").arg(Theme::heading()));
    root->addWidget(title);

    nameInput_ = new QLineEdit;
    nameInput_->setPlaceholderText("Room name");
    nameInput_->setFixedHeight(38);
    nameInput_->setStyleSheet(Theme::textInput());
    root->addWidget(nameInput_);

    // ── public / private choice ─────────────────────────────────────────
    auto* typeRow = new QHBoxLayout;
    publicRadio_  = new QRadioButton("Public");
    privateRadio_ = new QRadioButton("Private");
    publicRadio_->setChecked(true);

    QString radioStyle = QString("QRadioButton { color: %1; font-size: 13px; }").arg(Theme::TEXT_PRIMARY);
    publicRadio_->setStyleSheet(radioStyle);
    privateRadio_->setStyleSheet(radioStyle);

    auto* group = new QButtonGroup(this);
    group->addButton(publicRadio_);
    group->addButton(privateRadio_);

    typeRow->addWidget(publicRadio_);
    typeRow->addWidget(privateRadio_);
    typeRow->addStretch();
    root->addLayout(typeRow);

    auto* hint = new QLabel("Public: everyone can see and join.\nPrivate: only the people you pick can see it.");
    hint->setStyleSheet(QString("border:none; background:transparent; %1").arg(Theme::mutedText()));
    root->addWidget(hint);

    // ── member picker (private only) ────────────────────────────────────
    memberLabel_ = new QLabel("Select members");
    memberLabel_->setStyleSheet(QString("border:none; background:transparent; %1").arg(Theme::bodyText()));

    memberList_ = new QListWidget;
    memberList_->setStyleSheet(QString(
        "QListWidget { background: #1e293b; border: 1px solid %1; border-radius: 8px; color: %2; font-size: 13px; }"
        "QListWidget::item { padding: 6px 8px; }"
    ).arg(Theme::BORDER, Theme::TEXT_PRIMARY));
    memberList_->setFixedHeight(160);

    for (auto it = availableUsers_.constBegin(); it != availableUsers_.constEnd(); ++it) {
        auto* item = new QListWidgetItem(it.value());
        item->setData(Qt::UserRole, it.key());
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        memberList_->addItem(item);
    }

    root->addWidget(memberLabel_);
    root->addWidget(memberList_);

    // dialog opens in "Public" mode — hide the picker until Private is chosen
    memberLabel_->hide();
    memberList_->hide();

    // ── buttons ──────────────────────────────────────────────────────────
    auto* btnRow = new QHBoxLayout;
    auto* btnCancel = new QPushButton("Cancel");
    auto* btnOk     = new QPushButton("Create");
    btnCancel->setFixedHeight(38);
    btnOk->setFixedHeight(38);
    btnCancel->setStyleSheet(QString(
        "QPushButton { background: transparent; color: %1; border: 1px solid %2; border-radius: 8px; padding: 0 16px; }"
        "QPushButton:hover { background: %2; }"
    ).arg(Theme::TEXT_SECONDARY, Theme::BORDER));
    btnOk->setStyleSheet(Theme::primaryButton());
    btnRow->addStretch();
    btnRow->addWidget(btnCancel);
    btnRow->addWidget(btnOk);
    root->addLayout(btnRow);

    connect(publicRadio_, &QRadioButton::toggled, this, &CreateRoomDialog::onTypeToggled);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(btnOk,     &QPushButton::clicked, this, &CreateRoomDialog::onAccept);

    onTypeToggled();
}

void CreateRoomDialog::onTypeToggled()
{
    bool priv = privateRadio_->isChecked();
    memberLabel_->setVisible(priv);
    memberList_->setVisible(priv);
    adjustSize();
}

void CreateRoomDialog::onAccept()
{
    if (nameInput_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Room name required", "Please enter a room name.");
        return;
    }
    if (privateRadio_->isChecked() && selectedMemberIds().isEmpty()) {
        QMessageBox::warning(this, "Pick members", "Select at least one person to invite.");
        return;
    }
    accept();
}

QString CreateRoomDialog::roomName() const { return nameInput_->text().trimmed(); }
bool CreateRoomDialog::isPrivate() const   { return privateRadio_->isChecked(); }

QStringList CreateRoomDialog::selectedMemberIds() const
{
    QStringList ids;
    for (int i = 0; i < memberList_->count(); ++i) {
        auto* item = memberList_->item(i);
        if (item->checkState() == Qt::Checked)
            ids << item->data(Qt::UserRole).toString();
    }
    return ids;
}