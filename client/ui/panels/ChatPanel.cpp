#include "ChatPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QMessageBox>

static const char* ACCENT   = "#6366f1";
static const char* ACCENT2  = "#818cf8";
static const char* TEXT_PRI = "#f1f5f9";
static const char* TEXT_SEC = "#94a3b8";
static const char* BG_DEEP  = "#0a0f1e";
static const char* BG_PANEL = "#0f172a";
static const char* BG_CARD  = "#1e293b";
static const char* BG_INPUT = "#334155";

static QString inputStyle() {
    return QString("QLineEdit{background:%1;color:%2;border:1px solid #334155;"
        "border-radius:8px;padding:8px 12px;font-size:13px;}"
        "QLineEdit:focus{border:1px solid %3;}").arg(BG_INPUT, TEXT_PRI, ACCENT2);
}

static QString msgBoxStyle() {
    return QString(
        "QMessageBox{background:%1;color:%2;}"
        "QMessageBox QLabel{color:%2;font-size:13px;}"
        "QMessageBox QPushButton{background:%3;color:white;border:none;"
        "border-radius:6px;padding:6px 18px;font-size:13px;}"
        "QMessageBox QPushButton:hover{background:%4;}")
        .arg(BG_PANEL, TEXT_PRI, ACCENT, ACCENT2);
}

ChatPanel::ChatPanel(QWidget* parent) : QWidget(parent)
{
    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    // ── room sidebar ──────────────────────────────────────────────────────
    auto* sidebar = new QWidget;
    sidebar->setFixedWidth(220);
    sidebar->setStyleSheet(QString("background:%1;border-right:1px solid #1e293b;").arg(BG_PANEL));
    auto* sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(12,16,12,12);
    sideLayout->setSpacing(8);

    auto* roomsLabel = new QLabel("ROOMS");
    roomsLabel->setStyleSheet(QString(
        "color:%1;font-size:11px;font-weight:bold;letter-spacing:1px;").arg(TEXT_SEC));

    roomList_ = new QListWidget;
    roomList_->setStyleSheet(QString(
        "QListWidget{background:transparent;border:none;color:%1;}"
        "QListWidget::item{padding:8px 10px;border-radius:6px;margin:1px 0;}"
        "QListWidget::item:selected{background:%2;}"
        "QListWidget::item:hover{background:%3;}").arg(TEXT_PRI, ACCENT, BG_CARD));

    roomIdInput_ = new QLineEdit;
    roomIdInput_->setPlaceholderText("room-id");
    roomIdInput_->setStyleSheet(inputStyle());
    roomIdInput_->setFixedHeight(36);

    auto* btnCreate = new QPushButton("+ Create Room");
    btnCreate->setFixedHeight(34);
    btnCreate->setStyleSheet(QString(
        "QPushButton{background:%1;color:white;border:none;"
        "border-radius:6px;font-size:12px;}"
        "QPushButton:hover{background:%2;}").arg(BG_CARD, ACCENT));

    sideLayout->addWidget(roomsLabel);
    sideLayout->addWidget(roomList_, 1);
    sideLayout->addWidget(roomIdInput_);
    sideLayout->addWidget(btnCreate);

    // ── chat main area ────────────────────────────────────────────────────
    auto* chatMain = new QWidget;
    chatMain->setStyleSheet(QString("background:%1;").arg(BG_DEEP));
    auto* chatLayout = new QVBoxLayout(chatMain);
    chatLayout->setContentsMargins(0,0,0,0);
    chatLayout->setSpacing(0);

    // top bar
    auto* topBar = new QWidget;
    topBar->setFixedHeight(52);
    topBar->setStyleSheet(QString("background:%1;border-bottom:1px solid #1e293b;").arg(BG_PANEL));
    auto* topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(20,0,20,0);
    currentRoomLabel_ = new QLabel("Select or create a room");
    currentRoomLabel_->setStyleSheet(QString(
        "color:%1;font-size:15px;font-weight:bold;").arg(TEXT_PRI));
    topLayout->addWidget(currentRoomLabel_);

    // per-room message views
    chatStack_ = new QStackedWidget;
    chatStack_->setStyleSheet(QString("background:%1;").arg(BG_DEEP));

    // empty placeholder shown before any room is selected
    auto* placeholder = new QLabel("Join or create a room to start chatting");
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setStyleSheet(QString("color:%1;font-size:14px;").arg(TEXT_SEC));
    chatStack_->addWidget(placeholder); // index 0 = placeholder

    // input bar
    auto* inputBar = new QWidget;
    inputBar->setFixedHeight(64);
    inputBar->setStyleSheet(QString("background:%1;border-top:1px solid #1e293b;").arg(BG_PANEL));
    auto* inputLayout = new QHBoxLayout(inputBar);
    inputLayout->setContentsMargins(16,12,16,12);
    inputLayout->setSpacing(10);

    messageInput_ = new QLineEdit;
    messageInput_->setPlaceholderText("Message...");
    messageInput_->setFixedHeight(40);
    messageInput_->setStyleSheet(inputStyle());

    auto* btnSend = new QPushButton("Send");
    btnSend->setFixedSize(80,40);
    btnSend->setStyleSheet(QString(
        "QPushButton{background:%1;color:white;border:none;"
        "border-radius:8px;font-weight:bold;}"
        "QPushButton:hover{background:%2;}").arg(ACCENT, ACCENT2));

    inputLayout->addWidget(messageInput_);
    inputLayout->addWidget(btnSend);

    chatLayout->addWidget(topBar);
    chatLayout->addWidget(chatStack_, 1);
    chatLayout->addWidget(inputBar);

    root->addWidget(sidebar);
    root->addWidget(chatMain, 1);

    // ── connections ───────────────────────────────────────────────────────
    connect(btnSend,       &QPushButton::clicked,     this, &ChatPanel::onSend);
    connect(messageInput_, &QLineEdit::returnPressed, this, &ChatPanel::onSend);
    connect(btnCreate,     &QPushButton::clicked,     this, &ChatPanel::onCreateRoom);

    // clicking a room switches to it immediately, auto-joins if needed
    connect(roomList_, &QListWidget::currentTextChanged, this, [this](const QString& room){
        if (room.isEmpty()) return;
        switchToRoom(room);

        // if not yet joined, emit join signal
        if (!joinedRooms_.contains(room)) {
            Message msg;
            msg.type            = MessageType::JOIN;
            msg.roomId          = room.toStdString();
            msg.sender.username = displayName_.toStdString();
            emit roomJoined(msg);
        }
    });
}

// ── helpers ───────────────────────────────────────────────────────────────────
void ChatPanel::setCurrentUser(const QString& displayName, const QString& userId)
{
    displayName_ = displayName;
    userId_      = userId;
}

// Get or create a QTextEdit for a room and switch chatStack_ to it
void ChatPanel::switchToRoom(const QString& room)
{
    currentRoom_ = room;
    currentRoomLabel_->setText("# " + room);

    if (!roomViews_.contains(room)) {
        auto* view = new QTextEdit;
        view->setReadOnly(true);
        view->setStyleSheet(QString(
            "QTextEdit{background:%1;color:%2;border:none;"
            "padding:16px;font-size:13px;}").arg(BG_DEEP, TEXT_PRI));
        roomViews_[room] = view;
        chatStack_->addWidget(view);
    }

    chatStack_->setCurrentWidget(roomViews_[room]);
}

// ── send ──────────────────────────────────────────────────────────────────────
void ChatPanel::onSend()
{
    if (currentRoom_.isEmpty()) {
        auto* box = new QMessageBox(this);
        box->setStyleSheet(msgBoxStyle());
        box->setWindowTitle("No room");
        box->setText("Join or create a room first.");
        box->exec();
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

    // show our own message immediately without waiting for echo
    appendChat(currentRoom_, displayName_, text);

    emit messageSent(msg);
    messageInput_->clear();
}

// ── create room ───────────────────────────────────────────────────────────────
void ChatPanel::onCreateRoom()
{
    QString roomId = roomIdInput_->text().trimmed();
    if (roomId.isEmpty()) {
        auto* box = new QMessageBox(this);
        box->setStyleSheet(msgBoxStyle());
        box->setWindowTitle("Missing");
        box->setText("Enter a room ID.");
        box->exec();
        return;
    }

    // mark as joined locally before server responds
    joinedRooms_.insert(roomId);
    switchToRoom(roomId);

    // add to sidebar
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

// ── receive incoming message ──────────────────────────────────────────────────
void ChatPanel::receiveMessage(const Message& msg)
{
    QString room   = QString::fromStdString(msg.roomId);
    QString sender = QString::fromStdString(msg.sender.username);
    QString text   = QString::fromStdString(msg.text);

    if (msg.type == MessageType::JOIN) {
        joinedRooms_.insert(room);
        // add to sidebar if not there
        if (!room.isEmpty() && roomList_->findItems(room, Qt::MatchExactly).isEmpty())
            roomList_->addItem(room);
        appendChat(room, "system", text);
        return;
    }

    if (msg.type == MessageType::LEAVE || msg.type == MessageType::PRESENCE) {
        appendChat(room, "system", text);
        return;
    }

    // CHAT_PUBLIC / CHAT_PRIVATE
    // don't echo back our own messages (we show them immediately in onSend)
    if (sender == displayName_) return;

    if (!room.isEmpty() && roomList_->findItems(room, Qt::MatchExactly).isEmpty())
        roomList_->addItem(room);

    appendChat(room, sender, text);
}

// ── append to the correct room's view ────────────────────────────────────────
void ChatPanel::appendChat(const QString& room, const QString& sender, const QString& text)
{
    if (room.isEmpty()) return;

    // ensure view exists
    if (!roomViews_.contains(room)) {
        auto* view = new QTextEdit;
        view->setReadOnly(true);
        view->setStyleSheet(QString(
            "QTextEdit{background:%1;color:%2;border:none;"
            "padding:16px;font-size:13px;}").arg(BG_DEEP, TEXT_PRI));
        roomViews_[room] = view;
        chatStack_->addWidget(view);
    }

    bool isSystem = (sender == "system");
    bool isMe     = (sender == displayName_);
    QString color = isSystem ? "#475569" : isMe ? "#818cf8" : "#4ade80";

    auto* view = roomViews_[room];
    view->append(
        QString("<span style='color:%1;font-weight:600;'>%2</span>"
                "<span style='color:#475569;'>&nbsp;&nbsp;</span>"
                "<span style='color:#e2e8f0;'>%3</span>")
        .arg(color, sender.toHtmlEscaped(), text.toHtmlEscaped()));
    view->verticalScrollBar()->setValue(view->verticalScrollBar()->maximum());

    // if this room is currently active, make sure stack shows it
    if (room == currentRoom_)
        chatStack_->setCurrentWidget(view);
}

void ChatPanel::openDirectRoom(const QString& roomId)
{
    if (roomList_->findItems(roomId, Qt::MatchExactly).isEmpty())
        roomList_->addItem(roomId);

    // mark as joined so clicking it doesn't send a JOIN
    joinedRooms_.insert(roomId);

    // switch to it
    switchToRoom(roomId);
    roomList_->setCurrentRow(
        roomList_->row(roomList_->findItems(roomId, Qt::MatchExactly).first()));
}