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

// Cohesive Label Color Palette
static const char* BG_MAIN       = "#D4C5B6"; // Warm Sand / Parchment
static const char* BG_PANEL      = "#C9BBAA"; // Slightly deeper sand for sidebar
static const char* TEXT_MAIN     = "#0F0F0F"; // Ink Black
static const char* TEXT_DIM      = "#555555"; // Faded typewriter ink
static const char* ACCENT_ORANGE = "#E65C40"; // Stamp Red-Orange

static QString inputStyle() {
    return QString(
        "QLineEdit {"
        "  background: %1; color: %2; border: 1px solid %2; border-radius: 0px;"
        "  padding: 8px 12px; font-size: 12px; font-family: monospace;"
        "}"
        "QLineEdit:focus {"
        "  border: 1px solid %3; background: #FFFDFB;"
        "}"
    ).arg(BG_MAIN, TEXT_MAIN, ACCENT_ORANGE);
}

static QString msgBoxStyle() {
    return QString(
        "QMessageBox { background: %1; color: %2; border: 1px solid %2; }"
        "QMessageBox QLabel { color: %2; font-size: 13px; font-family: monospace; }"
        "QMessageBox QPushButton {"
        "  background: %2; color: %1; border: none; border-radius: 0px;"
        "  padding: 6px 18px; font-size: 12px; font-weight: bold; letter-spacing: 2px;"
        "}"
        "QMessageBox QPushButton:hover { background: %3; }"
    ).arg(BG_MAIN, TEXT_MAIN, ACCENT_ORANGE);
}

ChatPanel::ChatPanel(QWidget* parent) : QWidget(parent)
{
    // Force the custom sand background
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("ChatPanel { background-color: %1; }").arg(BG_MAIN));

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── SIDEBAR (Room Registry) ───────────────────────────────────────────
    auto* sidebar = new QWidget;
    sidebar->setFixedWidth(240);
    // Sharp 1px solid black border divider
    sidebar->setStyleSheet(QString("background: %1; border-right: 1px solid %2;").arg(BG_PANEL, TEXT_MAIN));
    
    auto* sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(16, 20, 16, 16);
    sideLayout->setSpacing(12);

    auto* roomsLabel = new QLabel("DIRECTORY // ROOMS");
    roomsLabel->setStyleSheet(QString(
        "border: none; color: %1; font-size: 12px; font-weight: 900; letter-spacing: 2px;"
    ).arg(TEXT_MAIN));

    roomList_ = new QListWidget;
    roomList_->setStyleSheet(QString(
        "QListWidget { background: transparent; border: none; color: %1; font-family: monospace; font-size: 13px; }"
        "QListWidget::item { padding: 8px 10px; border-radius: 0px; border: 1px solid transparent; margin: 2px 0px; }"
        "QListWidget::item:selected { background: %1; color: %2; }" // Inverts to black block with sand text
        "QListWidget::item:hover:!selected { border: 1px solid %1; }" // Outline box on hover
    ).arg(TEXT_MAIN, BG_MAIN));

    roomIdInput_ = new QLineEdit;
    roomIdInput_->setPlaceholderText("ENTER ROOM ID...");
    roomIdInput_->setStyleSheet(inputStyle());
    roomIdInput_->setFixedHeight(36);

    auto* btnCreate = new QPushButton("+ INITIALIZE ROOM");
    btnCreate->setFixedHeight(40);
    btnCreate->setStyleSheet(QString(
        "QPushButton {"
        "  background: %1; color: %2; border: none; border-radius: 0px;"
        "  font-size: 11px; font-weight: bold; letter-spacing: 1px;"
        "}"
        "QPushButton:hover { background: %3; color: %2; }"
    ).arg(TEXT_MAIN, BG_MAIN, ACCENT_ORANGE));

    sideLayout->addWidget(roomsLabel);
    sideLayout->addSpacing(4);
    sideLayout->addWidget(roomList_, 1);
    sideLayout->addWidget(roomIdInput_);
    sideLayout->addWidget(btnCreate);

    // ── MAIN CHAT AREA (The Logbook) ──────────────────────────────────────
    auto* chatMain = new QWidget;
    chatMain->setStyleSheet(QString("background: transparent; border: none;"));
    auto* chatLayout = new QVBoxLayout(chatMain);
    chatLayout->setContentsMargins(0, 0, 0, 0);
    chatLayout->setSpacing(0);

    // TOP BAR
    auto* topBar = new QWidget;
    topBar->setFixedHeight(52);
    // Solid 1px line separating the header from the content
    topBar->setStyleSheet(QString("background: %1; border-bottom: 1px solid %2; border-right: none; border-left: none;").arg(BG_MAIN, TEXT_MAIN));
    
    auto* topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(24, 0, 24, 0);
    currentRoomLabel_ = new QLabel("AWAITING CONNECTION...");
    currentRoomLabel_->setStyleSheet(QString(
        "border: none; color: %1; font-size: 14px; font-weight: 800; letter-spacing: 2px; font-family: monospace;"
    ).arg(TEXT_MAIN));
    topLayout->addWidget(currentRoomLabel_);

    // PER-ROOM MESSAGE VIEWS
    chatStack_ = new QStackedWidget;
    chatStack_->setStyleSheet(QString("background: transparent; border: none;"));

    // EMPTY PLACEHOLDER
    auto* placeholder = new QLabel("NO TARGET ACQUIRED.\nJOIN OR CREATE A ROOM TO BEGIN.");
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setStyleSheet(QString(
        "border: none; color: %1; font-size: 12px; font-weight: bold; letter-spacing: 2px; font-family: monospace;"
    ).arg(TEXT_MAIN));
    chatStack_->addWidget(placeholder); // index 0 = placeholder

    // INPUT BAR
    auto* inputBar = new QWidget;
    inputBar->setFixedHeight(64);
    // Solid line above the input
    inputBar->setStyleSheet(QString("background: %1; border-top: 1px solid %2; border-right: none; border-left: none;").arg(BG_MAIN, TEXT_MAIN));
    
    auto* inputLayout = new QHBoxLayout(inputBar);
    inputLayout->setContentsMargins(16, 12, 16, 12);
    inputLayout->setSpacing(12);

    messageInput_ = new QLineEdit;
    messageInput_->setPlaceholderText("TRANSMIT MESSAGE...");
    messageInput_->setFixedHeight(40);
    messageInput_->setStyleSheet(inputStyle());

    auto* btnSend = new QPushButton("SEND");
    btnSend->setFixedSize(90, 40);
    btnSend->setStyleSheet(QString(
        "QPushButton {"
        "  background: %1; color: %2; border: none; border-radius: 0px;"
        "  font-weight: bold; letter-spacing: 2px; font-size: 12px;"
        "}"
        "QPushButton:hover { background: %3; color: %2; }"
    ).arg(TEXT_MAIN, BG_MAIN, ACCENT_ORANGE));

    inputLayout->addWidget(messageInput_);
    inputLayout->addWidget(btnSend);

    chatLayout->addWidget(topBar);
    chatLayout->addWidget(chatStack_, 1);
    chatLayout->addWidget(inputBar);

    root->addWidget(sidebar);
    root->addWidget(chatMain, 1);

    // ── CONNECTIONS ───────────────────────────────────────────────────────
    connect(btnSend,       &QPushButton::clicked,     this, &ChatPanel::onSend);
    connect(messageInput_, &QLineEdit::returnPressed, this, &ChatPanel::onSend);
    connect(btnCreate,     &QPushButton::clicked,     this, &ChatPanel::onCreateRoom);

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

// ── HELPERS ───────────────────────────────────────────────────────────────────
void ChatPanel::setCurrentUser(const QString& displayName, const QString& userId)
{
    displayName_ = displayName;
    userId_      = userId;
}

void ChatPanel::switchToRoom(const QString& room)
{
    currentRoom_ = room;
    currentRoomLabel_->setText("ACTIVE // " + room.toUpper());

    if (!roomViews_.contains(room)) {
        auto* view = new QTextEdit;
        view->setReadOnly(true);
        // Clean typewriter styling for the log
        view->setStyleSheet(QString(
            "QTextEdit {"
            "  background: transparent; color: %1; border: none;"
            "  padding: 16px 24px; font-size: 13px; font-family: monospace; line-height: 1.5;"
            "}"
        ).arg(TEXT_MAIN));
        roomViews_[room] = view;
        chatStack_->addWidget(view);
    }

    chatStack_->setCurrentWidget(roomViews_[room]);
}

// ── SEND ──────────────────────────────────────────────────────────────────────
void ChatPanel::onSend()
{
    if (currentRoom_.isEmpty()) {
        auto* box = new QMessageBox(this);
        box->setStyleSheet(msgBoxStyle());
        box->setWindowTitle("ERR // NO ROUTE");
        box->setText("ESTABLISH OR JOIN A ROOM FIRST.");
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

    appendChat(currentRoom_, displayName_, text);

    emit messageSent(msg);
    messageInput_->clear();
}

// ── CREATE ROOM ───────────────────────────────────────────────────────────────
void ChatPanel::onCreateRoom()
{
    QString roomId = roomIdInput_->text().trimmed();
    if (roomId.isEmpty()) {
        auto* box = new QMessageBox(this);
        box->setStyleSheet(msgBoxStyle());
        box->setWindowTitle("ERR // NULL ID");
        box->setText("ROOM IDENTIFIER REQUIRED.");
        box->exec();
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

// ── RECEIVE INCOMING MESSAGE ──────────────────────────────────────────────────
void ChatPanel::receiveMessage(const Message& msg)
{
    QString room   = QString::fromStdString(msg.roomId);
    QString sender = QString::fromStdString(msg.sender.username);
    QString text   = QString::fromStdString(msg.text);

    if (msg.type == MessageType::JOIN) {
        joinedRooms_.insert(room);
        if (!room.isEmpty() && roomList_->findItems(room, Qt::MatchExactly).isEmpty())
            roomList_->addItem(room);
        appendChat(room, "SYSTEM", text);
        return;
    }

    if (msg.type == MessageType::LEAVE || msg.type == MessageType::PRESENCE) {
        appendChat(room, "SYSTEM", text);
        return;
    }

    if (sender == displayName_) return;

    if (!room.isEmpty() && roomList_->findItems(room, Qt::MatchExactly).isEmpty())
        roomList_->addItem(room);

    appendChat(room, sender, text);
}

// ── APPEND TO THE CORRECT ROOM'S VIEW ────────────────────────────────────────
void ChatPanel::appendChat(const QString& room, const QString& sender, const QString& text)
{
    if (room.isEmpty()) return;

    if (!roomViews_.contains(room)) {
        auto* view = new QTextEdit;
        view->setReadOnly(true);
        view->setStyleSheet(QString(
            "QTextEdit {"
            "  background: transparent; color: %1; border: none;"
            "  padding: 16px 24px; font-size: 13px; font-family: monospace; line-height: 1.5;"
            "}"
        ).arg(TEXT_MAIN));
        roomViews_[room] = view;
        chatStack_->addWidget(view);
    }

    bool isSystem = (sender.toUpper() == "SYSTEM");
    bool isMe     = (sender == displayName_);
    
    // System messages are dim. My messages are black. Other messages are the Stamp Orange.
    QString color = isSystem ? TEXT_DIM : isMe ? TEXT_MAIN : ACCENT_ORANGE;

    auto* view = roomViews_[room];
    
    // Structural, typewriter layout for the chat feed
    view->append(
        QString("<div style='margin-bottom: 6px;'>"
                "<span style='color:%1; font-weight:800; letter-spacing: 1px;'>[%2]</span>"
                "&nbsp;&nbsp;"
                "<span style='color:%3;'>%4</span>"
                "</div>")
        .arg(color, sender.toUpper().toHtmlEscaped(), TEXT_MAIN, text.toHtmlEscaped()));
        
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