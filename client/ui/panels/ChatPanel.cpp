#include "ChatPanel.h"
#include "ui/theme/Theme.h"
#include "CreateRoomDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QMessageBox>
#include <QUuid>
#include "ui/panels/UsersSidebar.h"

ChatPanel::ChatPanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("ChatPanel { %1 }").arg(Theme::pageBackground()));

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    // ── sidebar ──────────────────────────────────────────────────────────
    auto* sidebar = new QWidget;
    sidebar->setObjectName("chatSidebar");
    sidebar->setFixedWidth(240);
    sidebar->setStyleSheet(QString(
        "#chatSidebar { background: %1; border-right: 1px solid %2; }"
    ).arg(Theme::SURFACE_ALT, Theme::BORDER));

    auto* sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(16, 20, 16, 16);
    sideLayout->setSpacing(10);

    auto* roomsLabel = new QLabel("Rooms");
    roomsLabel->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::heading()));

    roomList_ = new QListWidget;
    roomList_->setStyleSheet(QString(
        "QListWidget { background: transparent; border: none; color: %1; font-size: 13px; }"
        "QListWidget::item { padding: 8px 10px; border-radius: 8px; margin: 1px 0px; }"
        "QListWidget::item:selected { background: %2; color: white; }"
        "QListWidget::item:hover:!selected { background: %3; }"
    ).arg(Theme::TEXT_PRIMARY, Theme::ACCENT, Theme::BORDER));

    auto* btnCreate = new QPushButton("Create room");
    btnCreate->setFixedHeight(36);
    btnCreate->setStyleSheet(Theme::primaryButton());

    sideLayout->addWidget(roomsLabel);
    sideLayout->addWidget(roomList_, 1);
    sideLayout->addWidget(btnCreate);

    // ── main chat area ───────────────────────────────────────────────────
    auto* chatMain = new QWidget;
    chatMain->setStyleSheet("background: transparent; border: none;");
    auto* chatLayout = new QVBoxLayout(chatMain);
    chatLayout->setContentsMargins(0,0,0,0);
    chatLayout->setSpacing(0);

    auto* topBar = new QWidget;
    topBar->setObjectName("chatTopBar");
    topBar->setFixedHeight(52);
    topBar->setStyleSheet(QString(
        "#chatTopBar { background: %1; border-bottom: 1px solid %2; }"
    ).arg(Theme::SURFACE, Theme::BORDER));

    auto* topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(24, 0, 24, 0);
    currentRoomLabel_ = new QLabel("Select a room");
    currentRoomLabel_->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::bodyText()));
    topLayout->addWidget(currentRoomLabel_);

    topLayout->addStretch();
    btnSummarize_ = new QPushButton("Summarize");
    btnSummarize_->setFixedHeight(32);
    btnSummarize_->setStyleSheet(Theme::secondaryButton());
    topLayout->addWidget(btnSummarize_);

    chatStack_ = new QStackedWidget;
    chatStack_->setStyleSheet("background: transparent; border: none;");

    auto* placeholder = new QLabel("No room selected.\nJoin or create a room to start chatting.");
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));
    chatStack_->addWidget(placeholder);

    auto* inputBar = new QWidget;
    inputBar->setObjectName("chatInputBar");
    inputBar->setFixedHeight(64);
    inputBar->setStyleSheet(QString(
        "#chatInputBar { background: %1; border-top: 1px solid %2; }"
    ).arg(Theme::SURFACE, Theme::BORDER));

    auto* inputLayout = new QHBoxLayout(inputBar);
    inputLayout->setContentsMargins(16,12,16,12);
    inputLayout->setSpacing(10);

    messageInput_ = new QLineEdit;
    messageInput_->setPlaceholderText("Type a message...");
    messageInput_->setFixedHeight(40);
    messageInput_->setStyleSheet(Theme::textInput());

    auto* btnSend = new QPushButton("Send");
    btnSend->setFixedSize(90, 40);
    btnSend->setStyleSheet(Theme::primaryButton());

    inputLayout->addWidget(messageInput_);
    inputLayout->addWidget(btnSend);

    chatLayout->addWidget(topBar);
    chatLayout->addWidget(chatStack_, 1);
    chatLayout->addWidget(inputBar);

    usersSidebar_ = new UsersSidebar;
    connect(usersSidebar_, &UsersSidebar::messageRequested, this, &ChatPanel::onMessageUser);
    connect(usersSidebar_, &UsersSidebar::profileRequested,  this, &ChatPanel::onViewProfile);
    root->addWidget(sidebar);
    root->addWidget(chatMain, 1);
    root->addWidget(usersSidebar_);

    // ── connections ──────────────────────────────────────────────────────
    connect(btnSend,       &QPushButton::clicked,     this, &ChatPanel::onSend);
    connect(messageInput_, &QLineEdit::returnPressed, this, &ChatPanel::onSend);
    connect(btnCreate,     &QPushButton::clicked,     this, &ChatPanel::onCreateRoom);

    connect(btnSummarize_, &QPushButton::clicked,     this, &ChatPanel::onSummarize);

    // clicking a room switches to it immediately, auto-joins if needed
    connect(roomList_, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem* current, QListWidgetItem*){
        if (!current) return;
        QString roomId = current->data(Qt::UserRole).toString();
        if (roomId.isEmpty()) return;

        if (!joinedRooms_.contains(roomId)) {
            joinedRooms_.insert(roomId);   // optimistic, same pattern as onCreateRoom
            Message msg;
            msg.type            = MessageType::JOIN;
            msg.roomId          = roomId.toStdString();
            msg.sender.username = displayName_.toStdString();
            msg.sender.userId   = userId_.toStdString();
            emit roomJoined(msg);          // JOIN goes out first
        }

        switchToRoom(roomId);              // THEN request history
    });
}

void ChatPanel::setCurrentUser(const QString& displayName, const QString& userId)
{
    displayName_ = displayName;
    userId_      = userId;
}

QListWidgetItem* ChatPanel::ensureRoomListItem(const QString& roomId, const QString& name)
{
    roomNames_[roomId] = name;

    for (int i = 0; i < roomList_->count(); ++i) {
        auto* item = roomList_->item(i);
        if (item->data(Qt::UserRole).toString() == roomId) {
            item->setText(name);
            return item;
        }
    }
    auto* item = new QListWidgetItem(name);
    item->setData(Qt::UserRole, roomId);
    roomList_->addItem(item);
    return item;
}

void ChatPanel::applyMemberFilter(const QString& roomId)
{
    auto it = roomMeta_.find(roomId);
    if (it == roomMeta_.end() || it->type == "PUBLIC" || it->type.isEmpty()) {
        usersSidebar_->showAllUsers();
    } else {
        usersSidebar_->filterToUsers(QSet<QString>(it->members.begin(), it->members.end()));
    }
}

void ChatPanel::switchToRoom(const QString& roomId)
{
    currentRoom_ = roomId;
    currentRoomLabel_->setText(roomNames_.value(roomId, roomId));

    if (!roomViews_.contains(roomId)) {
        auto* view = new QTextEdit;
        view->setReadOnly(true);
        view->setStyleSheet(QString(
            "QTextEdit { background: transparent; color: %1; border: none; padding: 16px 24px; font-size: 13px; line-height: 1.5; }"
        ).arg(Theme::TEXT_PRIMARY));
        roomViews_[roomId] = view;
        chatStack_->addWidget(view);
    }

    chatStack_->setCurrentWidget(roomViews_[roomId]);
    applyMemberFilter(roomId);

    if (!historyRequested_.contains(roomId)) {
        historyRequested_.insert(roomId);
        Message msg;
        msg.type             = MessageType::CHAT_HISTORY;
        msg.roomId           = roomId.toStdString();
        msg.sender.username  = displayName_.toStdString();
        msg.sender.userId    = userId_.toStdString();
        emit messageSent(msg);
    }
}

void ChatPanel::onSend()
{
    if (currentRoom_.isEmpty()) {
        QMessageBox::warning(this, "No room selected", "Join or create a room first.");
        return;
    }
    QString text = messageInput_->text().trimmed();
    if (text.isEmpty()) return;

    Message msg;
    msg.text            = text.toStdString();
    msg.sender.username = displayName_.toStdString();
    msg.sender.userId   = userId_.toStdString();

    if (currentRoom_.startsWith("direct:")) {
        // direct room — extract the other participant's id from the deterministic roomId
        QStringList parts = currentRoom_.split(':');
        QString otherId;
        if (parts.size() == 3)
            otherId = (parts[1] == userId_) ? parts[2] : parts[1];

        msg.type        = MessageType::CHAT_PRIVATE;
        msg.recipientId = otherId.toStdString();
        // note: no roomId set here — handlePrivate derives/creates the room itself
        // and echoes the message back to us with roomId filled in, so we don't
        // need to guess it locally for the outgoing message
    } else {
        msg.type   = MessageType::CHAT_PUBLIC;
        msg.roomId = currentRoom_.toStdString();
    }

    appendChat(currentRoom_, displayName_, text);

    emit messageSent(msg);
    messageInput_->clear();
}

void ChatPanel::onCreateRoom()
{
    QMap<QString, QString> candidates = knownUsers_;
    candidates.remove(userId_);

    CreateRoomDialog dlg(candidates, this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    QString     roomName = dlg.roomName();
    bool        priv     = dlg.isPrivate();
    QStringList members  = dlg.selectedMemberIds();

    QString roomId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    joinedRooms_.insert(roomId);

    RoomMeta meta;
    meta.type    = priv ? "GROUP" : "PUBLIC";
    meta.members = members;
    meta.members << userId_;
    roomMeta_[roomId] = meta;

    Message msg;
    msg.type              = MessageType::CHAT_CREATE;
    msg.roomId            = roomId.toStdString();
    msg.text               = roomName.toStdString();
    msg.role               = priv ? "GROUP" : "PUBLIC";
    msg.mediaUrl           = members.join(",").toStdString();
    msg.sender.username    = displayName_.toStdString();
    msg.sender.userId      = userId_.toStdString();
    emit roomCreated(msg);            // CREATE goes out first

    auto* item = ensureRoomListItem(roomId, roomName);
    switchToRoom(roomId);             // THEN request history
    roomList_->setCurrentItem(item);
}

void ChatPanel::receiveMessage(const Message& msg)
{
    QString roomId   = QString::fromStdString(msg.roomId);
    QString sender   = QString::fromStdString(msg.sender.username);
    QString text     = QString::fromStdString(msg.text);
    QString role     = QString::fromStdString(msg.role);
    QString mediaUrl = QString::fromStdString(msg.mediaUrl);

   if (msg.type == MessageType::AI_SUMMARIZE_RESPONSE) {
        awaitingSummary_ = false;
        btnSummarize_->setEnabled(true);
        btnSummarize_->setText("Summarize");
        appendChat(roomId, "AI Summary", text);
        return;
    }

    if (msg.type == MessageType::PROFILE_VIEW) {
        showProfileDialog(msg);
        return;
    }

    if (msg.type == MessageType::ERROR && awaitingSummary_) {
        awaitingSummary_ = false;
        btnSummarize_->setEnabled(true);
        btnSummarize_->setText("Summarize");
        appendChat(currentRoom_, "AI Summary", "⚠ " + text);
        return;
    }

    if (msg.type == MessageType::ERROR) {
        // not a summarize failure — let it surface as a real error, same as before
        QMessageBox::critical(this, "Error", text);
        return;
    }

    if (msg.type == MessageType::CHAT_CREATE) {
        if (roomId.isEmpty() || role == "DIRECT") return;

        RoomMeta meta;
        meta.type = role.isEmpty() ? "PUBLIC" : role;
        if (!mediaUrl.isEmpty())
            meta.members = mediaUrl.split(',', Qt::SkipEmptyParts);
        roomMeta_[roomId] = meta;

        ensureRoomListItem(roomId, text.isEmpty() ? roomId : text);

        if (roomId == currentRoom_)
            applyMemberFilter(roomId);
        return;
    }

    if (msg.type == MessageType::JOIN) {
        joinedRooms_.insert(roomId);

        RoomMeta meta = roomMeta_.value(roomId);
        if (!role.isEmpty()) meta.type = role;
        if (meta.type.isEmpty()) meta.type = "PUBLIC";
        if (!mediaUrl.isEmpty())
            meta.members = mediaUrl.split(',', Qt::SkipEmptyParts);
        roomMeta_[roomId] = meta;

        QString displayName = (meta.type == "DIRECT")
            ? resolveDirectRoomName(roomId)
            : (text.isEmpty() ? roomId : text);

        ensureRoomListItem(roomId, displayName);
        appendChat(roomId, "System", "joined " + displayName);

        if (roomId == currentRoom_)
            applyMemberFilter(roomId);
        return;
    }

    if (msg.type == MessageType::LEAVE || msg.type == MessageType::PRESENCE) {
        appendChat(roomId, "System", text);
        return;
    }
    if (msg.type == MessageType::CHAT_HISTORY) {
        ensureRoomListItem(roomId, roomNames_.value(roomId, roomId));
        appendChat(roomId, sender, text);
        return;
    }

    // CHAT_PUBLIC / CHAT_PRIVATE — don't echo our own messages back
    if (sender == displayName_) return;

    ensureRoomListItem(roomId, roomNames_.value(roomId, roomId));
    appendChat(roomId, sender, text);
}

void ChatPanel::onSummarize()
{
    if (currentRoom_.isEmpty()) {
        QMessageBox::warning(this, "No room selected", "Open a room to summarize it.");
        return;
    }
    const QStringList& log = roomLog_.value(currentRoom_);
    if (log.isEmpty()) {
        QMessageBox::information(this, "Nothing yet", "No messages in this room yet.");
        return;
    }

    btnSummarize_->setEnabled(false);
    btnSummarize_->setText("Summarizing…");
    awaitingSummary_ = true;

    Message msg;
    msg.type            = MessageType::AI_SUMMARIZE_REQUEST;
    msg.roomId          = currentRoom_.toStdString();
    msg.text             = log.join("\n").toStdString();
    msg.sender.username = displayName_.toStdString();
    msg.sender.userId   = userId_.toStdString();
    emit messageSent(msg);
}


void ChatPanel::appendChat(const QString& roomId, const QString& sender, const QString& text)
{
    if (roomId.isEmpty()) return;

    if (!roomViews_.contains(roomId)) {
        auto* view = new QTextEdit;
        view->setReadOnly(true);
        view->setStyleSheet(QString(
            "QTextEdit { background: transparent; color: %1; border: none; padding: 16px 24px; font-size: 13px; line-height: 1.5; }"
        ).arg(Theme::TEXT_PRIMARY));
        roomViews_[roomId] = view;
        chatStack_->addWidget(view);
    }

    bool isSystem = (sender == "System");
    bool isMe     = (sender == displayName_);
    bool isAI     = (sender == "AI Summary");
    QString color = isSystem ? Theme::TEXT_SECONDARY
                   : isAI    ? Theme::ACCENT2
                   : isMe    ? Theme::TEXT_PRIMARY : Theme::ACCENT;
                   
    if (!isSystem && sender != "AI Summary")
        roomLog_[roomId] << (sender + ": " + text);

    auto* view = roomViews_[roomId];
    view->append(
        QString("<div style='margin-bottom: 10px;'>"
                "<span style='color:%1; font-weight:600;'>%2</span>"
                "&nbsp;&nbsp;"
                "<span style='color:%3;'>%4</span>"
                "</div>")
        .arg(color, sender.toHtmlEscaped(), Theme::TEXT_PRIMARY, text.toHtmlEscaped()));

    view->verticalScrollBar()->setValue(view->verticalScrollBar()->maximum());

    if (roomId == currentRoom_)
        chatStack_->setCurrentWidget(view);
}

void ChatPanel::addOnlineUser(const QString& userId, const QString& displayName, const QString& username, const QString& bio, const QString& photoData)
{
    knownUsers_[userId] = displayName;
    usersSidebar_->addUser(userId, displayName, username, bio, photoData);
}

void ChatPanel::removeOnlineUser(const QString& userId)
{
    knownUsers_.remove(userId);
    usersSidebar_->removeUser(userId);
}

void ChatPanel::openDirectRoom(const QString& roomId)
{
    QString displayName = resolveDirectRoomName(roomId);
    ensureRoomListItem(roomId, displayName);

    RoomMeta meta = roomMeta_.value(roomId);
    meta.type = "DIRECT";
    if (meta.members.isEmpty() && roomId.startsWith("direct:")) {
        QStringList parts = roomId.split(':');
        if (parts.size() == 3)
            meta.members << parts[1] << parts[2];
    }
    roomMeta_[roomId] = meta;

    joinedRooms_.insert(roomId);
    switchToRoom(roomId);

    for (int i = 0; i < roomList_->count(); ++i) {
        if (roomList_->item(i)->data(Qt::UserRole).toString() == roomId) {
            roomList_->setCurrentItem(roomList_->item(i));
            break;
        }
    }
}

void ChatPanel::addKnownRoom(const QString& roomId)
{
    ensureRoomListItem(roomId, roomNames_.value(roomId, roomId));
    joinedRooms_.insert(roomId);
}

void ChatPanel::setUserPhoto(const QString& userId, const QString& photoData)
{
    usersSidebar_->setUserPhoto(userId, photoData);
}
void ChatPanel::setUserStatus(const QString& userId, UserStatus status)
{
    usersSidebar_->setUserStatus(userId, status);
}

void ChatPanel::onMessageUser(const QString& userId)
{
    if (userId.isEmpty() || userId == userId_) return;

    // mirror the server's deterministic direct-room id: direct:sortedUid1:sortedUid2
    QString uid1 = userId_, uid2 = userId;
    if (uid1 > uid2) { QString tmp = uid1; uid1 = uid2; uid2 = tmp; }
    QString roomId = "direct:" + uid1 + ":" + uid2;

    openDirectRoom(roomId);
}

void ChatPanel::onViewProfile(const QString& userId)
{
    if (userId.isEmpty() || userId == userId_) return;

    Message msg;
    msg.type            = MessageType::PROFILE_VIEW;
    msg.recipientId      = userId.toStdString();
    msg.sender.userId    = userId_.toStdString();
    msg.sender.username  = displayName_.toStdString();
    emit messageSent(msg);
}

void ChatPanel::showProfileDialog(const Message& msg)
{
    QString name      = QString::fromStdString(msg.displayName);
    QString username  = QString::fromStdString(msg.username);
    QString bio       = QString::fromStdString(msg.bio);
    QString major     = QString::fromStdString(msg.major);
    QString year      = QString::fromStdString(msg.year);
    QString interests = QString::fromStdString(msg.interests);

    QString body;
    body += "Name: " + (name.isEmpty() ? username : name) + "\n";
    body += "Username: @" + username + "\n";
    if (!major.isEmpty())     body += "Major: " + major + "\n";
    if (!year.isEmpty())      body += "Year: " + year + "\n";
    if (!interests.isEmpty()) body += "Interests: " + interests + "\n";
    if (!bio.isEmpty())       body += "\n" + bio;

    QMessageBox::information(this, "Profile", body);
}

QString ChatPanel::resolveDirectRoomName(const QString& roomId) const
{
    if (!roomId.startsWith("direct:")) return roomId;
    QStringList parts = roomId.split(':');
    if (parts.size() != 3) return roomId;
    QString otherId = (parts[1] == userId_) ? parts[2] : parts[1];
    return knownUsers_.value(otherId, otherId); // falls back to raw id if name unknown
}

void ChatPanel::resetState()
{
    roomList_->clear();
    chatStack_->setCurrentIndex(0); // back to the "no room selected" placeholder

    // roomViews_ holds QTextEdit* owned by chatStack_ — remove + delete them
    for (auto* view : roomViews_) {
        chatStack_->removeWidget(view);
        view->deleteLater();
    }
    roomViews_.clear();

    joinedRooms_.clear();
    roomNames_.clear();
    roomMeta_.clear();
    knownUsers_.clear();
    roomLog_.clear();
    historyRequested_.clear();
    currentRoom_.clear();
    currentRoomLabel_->setText("Select a room");

    usersSidebar_->showAllUsers();
    usersSidebar_->clearAll();

}

