#include "ChatPanel.h"
#include "ui/theme/Theme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QMessageBox>

ChatPanel::ChatPanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("ChatPanel { %1 }").arg(Theme::pageBackground()));

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

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

    roomIdInput_ = new QLineEdit;
    roomIdInput_->setPlaceholderText("Room name");
    roomIdInput_->setFixedHeight(36);
    roomIdInput_->setStyleSheet(Theme::textInput());

    auto* btnCreate = new QPushButton("Create room");
    btnCreate->setFixedHeight(36);
    btnCreate->setStyleSheet(Theme::primaryButton());

    sideLayout->addWidget(roomsLabel);
    sideLayout->addWidget(roomList_, 1);
    sideLayout->addWidget(roomIdInput_);
    sideLayout->addWidget(btnCreate);

    auto* chatMain = new QWidget;
    chatMain->setStyleSheet("background: transparent; border: none;");
    auto* chatLayout = new QVBoxLayout(chatMain);
    chatLayout->setContentsMargins(0, 0, 0, 0);
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

    auto* btnSummarize = new QPushButton("Summarize");
    btnSummarize->setFixedHeight(32);
    btnSummarize->setStyleSheet(QString(
        "QPushButton { background: %1; color: %2; border: 1px solid %3; border-radius: 8px; padding: 4px 14px; font-size: 12px; font-weight: 500; }"
        "QPushButton:hover { background: %4; }"
    ).arg(Theme::SURFACE, Theme::ACCENT, Theme::BORDER, Theme::SURFACE_ALT));

    topLayout->addWidget(currentRoomLabel_);
    topLayout->addStretch();
    topLayout->addWidget(btnSummarize);

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
    inputLayout->setContentsMargins(16, 12, 16, 12);
    inputLayout->setSpacing(12);

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

    root->addWidget(sidebar);
    root->addWidget(chatMain, 1);

    connect(btnSend,       &QPushButton::clicked,     this, &ChatPanel::onSend);
    connect(messageInput_, &QLineEdit::returnPressed, this, &ChatPanel::onSend);
    connect(btnCreate,     &QPushButton::clicked,     this, &ChatPanel::onCreateRoom);
    connect(btnSummarize,  &QPushButton::clicked,     this, [this](){
        if (currentRoom_.isEmpty()) {
            QMessageBox::warning(this, "No room selected", "Join or create a room first.");
            return;
        }
        Message msg;
        msg.type            = MessageType::CHAT_SUMMARIZE;
        msg.roomId          = currentRoom_.toStdString();
        msg.sender.username = displayName_.toStdString();
        emit messageSent(msg);
    });

    connect(roomList_, &QListWidget::currentTextChanged, this, [this](const QString& room){
        if (room.isEmpty()) return;
        switchToRoom(room);

        if (!joinedRooms_.contains(room)) {
            Message msg;
            msg.type            = MessageType::JOIN;
            msg.roomId          = room.toStdString();
            msg.sender.username = displayName_.toStdString();
            emit roomJoined(msg);
        }
    });
}

void ChatPanel::setCurrentUser(const QString& displayName, const QString& userId)
{
    displayName_ = displayName;
    userId_      = userId;
}

void ChatPanel::switchToRoom(const QString& room)
{
    currentRoom_ = room;
    currentRoomLabel_->setText(room);

    if (!roomViews_.contains(room)) {
        auto* view = new QTextEdit;
        view->setReadOnly(true);
        view->setStyleSheet(QString(
            "QTextEdit { background: transparent; color: %1; border: none; padding: 16px 24px; font-size: 13px; line-height: 1.5; }"
        ).arg(Theme::TEXT_PRIMARY));
        roomViews_[room] = view;
        chatStack_->addWidget(view);
    }

    chatStack_->setCurrentWidget(roomViews_[room]);
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
    msg.type            = MessageType::CHAT_PUBLIC;
    msg.roomId          = currentRoom_.toStdString();
    msg.text            = text.toStdString();
    msg.sender.username = displayName_.toStdString();
    msg.sender.userId   = userId_.toStdString();

    appendChat(currentRoom_, displayName_, text);

    emit messageSent(msg);
    messageInput_->clear();
}

void ChatPanel::onCreateRoom()
{
    QString roomId = roomIdInput_->text().trimmed();
    if (roomId.isEmpty()) {
        QMessageBox::warning(this, "Room name required", "Please enter a room name.");
        return;
    }

    joinedRooms_.insert(roomId);
    switchToRoom(roomId);

    if (roomList_->findItems(roomId, Qt::MatchExactly).isEmpty())
        roomList_->addItem(roomId);
    roomList_->setCurrentRow(roomList_->row(
        roomList_->findItems(roomId, Qt::MatchExactly).first()));

    Message msg;
    msg.type            = MessageType::CHAT_CREATE;
    msg.roomId          = roomId.toStdString();
    msg.text            = roomId.toStdString();
    msg.role            = "PUBLIC";
    msg.sender.username = displayName_.toStdString();
    emit roomCreated(msg);
    roomIdInput_->clear();
}

void ChatPanel::receiveMessage(const Message& msg)
{
    QString room   = QString::fromStdString(msg.roomId);
    QString sender = QString::fromStdString(msg.sender.username);
    QString text   = QString::fromStdString(msg.text);

    if (msg.type == MessageType::CHAT_SUMMARIZE) {
        appendChat(room, "AI Summary", text);
        return;
    }

    if (msg.type == MessageType::CHAT_CREATE) {
        if (!room.isEmpty() && msg.role != "DIRECT" && roomList_->findItems(room, Qt::MatchExactly).isEmpty())
            roomList_->addItem(room);
        return;
    }

    if (msg.type == MessageType::JOIN) {
        joinedRooms_.insert(room);
        if (!room.isEmpty() && roomList_->findItems(room, Qt::MatchExactly).isEmpty())
            roomList_->addItem(room);
        appendChat(room, "System", text);
        return;
    }

    if (msg.type == MessageType::LEAVE || msg.type == MessageType::PRESENCE) {
        appendChat(room, "System", text);
        return;
    }

    if (sender == displayName_) return;

    if (!room.isEmpty() && roomList_->findItems(room, Qt::MatchExactly).isEmpty())
        roomList_->addItem(room);

    appendChat(room, sender, text);
}

void ChatPanel::appendChat(const QString& room, const QString& sender, const QString& text)
{
    if (room.isEmpty()) return;

    if (!roomViews_.contains(room)) {
        auto* view = new QTextEdit;
        view->setReadOnly(true);
        view->setStyleSheet(QString(
            "QTextEdit { background: transparent; color: %1; border: none; padding: 16px 24px; font-size: 13px; line-height: 1.5; }"
        ).arg(Theme::TEXT_PRIMARY));
        roomViews_[room] = view;
        chatStack_->addWidget(view);
    }

    bool isSystem  = (sender == "System");
    bool isAI      = (sender == "AI Summary");
    bool isMe      = (sender == displayName_);

    QString color = isAI ? Theme::ACCENT2 : isSystem ? Theme::TEXT_SECONDARY : isMe ? Theme::TEXT_PRIMARY : Theme::ACCENT;

    auto* view = roomViews_[room];
    view->append(
        QString("<div style='margin-bottom: 10px;'>"
                "<span style='color:%1; font-weight:600;'>%2</span>"
                "&nbsp;&nbsp;"
                "<span style='color:%3;'>%4</span>"
                "</div>")
        .arg(color, sender.toHtmlEscaped(), Theme::TEXT_PRIMARY, text.toHtmlEscaped()));

    view->verticalScrollBar()->setValue(view->verticalScrollBar()->maximum());

    if (room == currentRoom_)
        chatStack_->setCurrentWidget(view);
}

void ChatPanel::openDirectRoom(const QString& roomId)
{
    if (roomList_->findItems(roomId, Qt::MatchExactly).isEmpty())
        roomList_->addItem(roomId);

    joinedRooms_.insert(roomId);
    switchToRoom(roomId);
    roomList_->setCurrentRow(
        roomList_->row(roomList_->findItems(roomId, Qt::MatchExactly).first()));
}
