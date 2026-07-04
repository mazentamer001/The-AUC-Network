#include "TestWindow.h"
#include <QThread>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QScrollBar>
#include <QFrame>

TestWindow::TestWindow(QWidget* parent) : QWidget(parent)
{
    setWindowTitle("AUC Networking");
    setMinimumSize(800, 600);

    // ── networking ────────────────────────────────────────────────────────
    auto* io = new boost::asio::io_context();
    client_  = new Client(*io, "127.0.0.1", "12345");

    client_->setOnConnected([this](){
        QMetaObject::invokeMethod(this, [this]{
            log("✅ Connected to server");
        }, Qt::QueuedConnection);
    });

    client_->setOnMessage([this](const Message& msg){
        QMetaObject::invokeMethod(this, [this, msg]{
            onMessage(msg);
        }, Qt::QueuedConnection);
    });

    auto* ioThread = new QThread(this);
    connect(ioThread, &QThread::started, [io](){ io->run(); });
    ioThread->start();

    // ── pages ─────────────────────────────────────────────────────────────
    stack_ = new QStackedWidget;
    setupRegisterPage();
    setupLoginPage();
    setupChatPage();
    stack_->addWidget(registerPage_); // 0
    stack_->addWidget(loginPage_);    // 1
    stack_->addWidget(chatPage_);     // 2
    stack_->setCurrentIndex(0);

    // ── log ───────────────────────────────────────────────────────────────
    logView_ = new QTextEdit;
    logView_->setReadOnly(true);
    logView_->setMaximumHeight(120);
    logView_->setStyleSheet("font-family:monospace;font-size:11px;background:#1e1e1e;color:#d4d4d4;");

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);
    root->addWidget(stack_, 1);
    auto* logBox = new QGroupBox("Server Log");
    auto* logLayout = new QVBoxLayout(logBox);
    logLayout->addWidget(logView_);
    root->addWidget(logBox);
}

// ─────────────────────────────────────────────────────────────────────────────
//  REGISTER PAGE
// ─────────────────────────────────────────────────────────────────────────────
void TestWindow::setupRegisterPage()
{
    registerPage_ = new QWidget;
    auto* outer = new QVBoxLayout(registerPage_);
    outer->setContentsMargins(60,40,60,40);

    auto* box  = new QGroupBox("Create Account");
    auto* form = new QFormLayout(box);
    form->setSpacing(10);

    regUsername_    = new QLineEdit; form->addRow("Username *",      regUsername_);
    regDisplayName_ = new QLineEdit; form->addRow("Display Name *",  regDisplayName_);
    regEmail_       = new QLineEdit; form->addRow("Email *",         regEmail_);
    regPassword_    = new QLineEdit;
    regPassword_->setEchoMode(QLineEdit::Password);
                                     form->addRow("Password *",      regPassword_);
    regUniId_       = new QLineEdit;
    regUniId_->setPlaceholderText("900XXXXXX");
                                     form->addRow("University ID *", regUniId_);
    regBio_         = new QLineEdit; form->addRow("Bio",             regBio_);

    auto* btnReg = new QPushButton("Register");
    btnReg->setStyleSheet("background:#2563eb;color:white;padding:8px;border-radius:4px;font-size:14px;");
    auto* btnSwitch = new QPushButton("Already have an account? Login →");
    btnSwitch->setFlat(true);
    btnSwitch->setStyleSheet("color:#2563eb;");

    outer->addStretch();
    outer->addWidget(box);
    outer->addWidget(btnReg);
    outer->addWidget(btnSwitch, 0, Qt::AlignCenter);
    outer->addStretch();

    connect(btnReg,    &QPushButton::clicked, this, &TestWindow::onRegister);
    connect(btnSwitch, &QPushButton::clicked, this, &TestWindow::showLogin);
}

// ─────────────────────────────────────────────────────────────────────────────
//  LOGIN PAGE
// ─────────────────────────────────────────────────────────────────────────────
void TestWindow::setupLoginPage()
{
    loginPage_ = new QWidget;
    auto* outer = new QVBoxLayout(loginPage_);
    outer->setContentsMargins(60,40,60,40);

    auto* box  = new QGroupBox("Login");
    auto* form = new QFormLayout(box);
    form->setSpacing(10);

    loginUsername_ = new QLineEdit;
    loginUsername_->setPlaceholderText("Username or email");
                                    form->addRow("Username / Email", loginUsername_);
    loginPassword_ = new QLineEdit;
    loginPassword_->setEchoMode(QLineEdit::Password);
                                    form->addRow("Password",         loginPassword_);

    auto* btnLogin  = new QPushButton("Login");
    btnLogin->setStyleSheet("background:#16a34a;color:white;padding:8px;border-radius:4px;font-size:14px;");
    auto* btnSwitch = new QPushButton("No account? Register →");
    btnSwitch->setFlat(true);
    btnSwitch->setStyleSheet("color:#16a34a;");

    outer->addStretch();
    outer->addWidget(box);
    outer->addWidget(btnLogin);
    outer->addWidget(btnSwitch, 0, Qt::AlignCenter);
    outer->addStretch();

    connect(btnLogin,  &QPushButton::clicked, this, &TestWindow::onLogin);
    connect(btnSwitch, &QPushButton::clicked, this, &TestWindow::showRegister);
}

// ─────────────────────────────────────────────────────────────────────────────
//  CHAT PAGE
// ─────────────────────────────────────────────────────────────────────────────
void TestWindow::setupChatPage()
{
    chatPage_ = new QWidget;
    auto* root = new QHBoxLayout(chatPage_);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    // ── left sidebar: rooms ───────────────────────────────────────────────
    auto* sidebar = new QWidget;
    sidebar->setFixedWidth(200);
    sidebar->setStyleSheet("background:#1e293b;");
    auto* sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(8,8,8,8);
    sideLayout->setSpacing(6);

    auto* roomsLabel = new QLabel("Rooms");
    roomsLabel->setStyleSheet("color:#94a3b8;font-weight:bold;font-size:12px;");

    roomList_ = new QListWidget;
    roomList_->setStyleSheet(
        "QListWidget{background:#1e293b;color:#e2e8f0;border:none;}"
        "QListWidget::item:selected{background:#2563eb;border-radius:4px;}"
        "QListWidget::item:hover{background:#334155;border-radius:4px;}");

    // create/join room controls
    roomIdInput_ = new QLineEdit;
    roomIdInput_->setPlaceholderText("room-id");
    roomIdInput_->setStyleSheet("background:#334155;color:white;border:none;padding:4px;border-radius:4px;");

    auto* btnCreate = new QPushButton("+ Create");
    auto* btnJoin   = new QPushButton("→ Join");
    btnCreate->setStyleSheet("background:#2563eb;color:white;border:none;padding:4px;border-radius:4px;");
    btnJoin->setStyleSheet("background:#475569;color:white;border:none;padding:4px;border-radius:4px;");

    auto* roomBtns = new QHBoxLayout;
    roomBtns->addWidget(btnCreate);
    roomBtns->addWidget(btnJoin);

    sideLayout->addWidget(roomsLabel);
    sideLayout->addWidget(roomList_, 1);
    sideLayout->addWidget(roomIdInput_);
    sideLayout->addLayout(roomBtns);

    // ── right: chat area ──────────────────────────────────────────────────
    auto* chatArea = new QWidget;
    auto* chatLayout = new QVBoxLayout(chatArea);
    chatLayout->setContentsMargins(0,0,0,0);
    chatLayout->setSpacing(0);

    // top bar
    auto* topBar = new QWidget;
    topBar->setStyleSheet("background:#0f172a;");
    topBar->setFixedHeight(44);
    auto* topLayout = new QHBoxLayout(topBar);
    currentRoomLabel_ = new QLabel("Select or join a room");
    currentRoomLabel_->setStyleSheet("color:white;font-weight:bold;font-size:14px;");
    topLayout->addWidget(currentRoomLabel_);

    // messages
    chatView_ = new QTextEdit;
    chatView_->setReadOnly(true);
    chatView_->setStyleSheet(
        "background:#0f172a;color:#e2e8f0;border:none;padding:12px;font-size:13px;");

    // input bar
    auto* inputBar = new QWidget;
    inputBar->setStyleSheet("background:#1e293b;");
    inputBar->setFixedHeight(52);
    auto* inputLayout = new QHBoxLayout(inputBar);
    inputLayout->setContentsMargins(8,8,8,8);

    messageInput_ = new QLineEdit;
    messageInput_->setPlaceholderText("Type a message...");
    messageInput_->setStyleSheet(
        "background:#334155;color:white;border:none;padding:8px;border-radius:6px;font-size:13px;");

    auto* btnSend = new QPushButton("Send");
    btnSend->setStyleSheet(
        "background:#2563eb;color:white;border:none;padding:8px 16px;border-radius:6px;font-weight:bold;");
    btnSend->setFixedWidth(80);

    inputLayout->addWidget(messageInput_);
    inputLayout->addWidget(btnSend);

    chatLayout->addWidget(topBar);
    chatLayout->addWidget(chatView_, 1);
    chatLayout->addWidget(inputBar);

    root->addWidget(sidebar);
    root->addWidget(chatArea, 1);

    // connections
    connect(btnSend,      &QPushButton::clicked,  this, &TestWindow::onSendMessage);
    connect(messageInput_, &QLineEdit::returnPressed, this, &TestWindow::onSendMessage);
    connect(btnCreate,    &QPushButton::clicked,  this, &TestWindow::onCreateRoom);
    connect(btnJoin,      &QPushButton::clicked,  this, &TestWindow::onJoinRoom);
    connect(roomList_,    &QListWidget::currentTextChanged, this, [this](const QString& room){
        currentRoom_ = room;
        currentRoomLabel_->setText("# " + room);
    });
}

// ─────────────────────────────────────────────────────────────────────────────
//  SLOTS
// ─────────────────────────────────────────────────────────────────────────────
void TestWindow::showLogin()    { stack_->setCurrentIndex(1); }
void TestWindow::showRegister() { stack_->setCurrentIndex(0); }

void TestWindow::onRegister()
{
    if (regUsername_->text().isEmpty() || regPassword_->text().isEmpty() ||
        regEmail_->text().isEmpty()    || regUniId_->text().isEmpty()    ||
        regDisplayName_->text().isEmpty()) {
        QMessageBox::warning(this, "Missing fields", "All fields marked * are required.");
        return;
    }
    Message msg;
    msg.type         = MessageType::AUTH_REGISTER;
    msg.username     = regUsername_->text().toStdString();
    msg.displayName  = regDisplayName_->text().toStdString();
    msg.email        = regEmail_->text().toStdString();
    msg.password     = regPassword_->text().toStdString();
    msg.universityId = regUniId_->text().toStdString();
    msg.bio          = regBio_->text().toStdString();
    client_->send(msg);
    log("→ REGISTER: " + regUsername_->text());
}

void TestWindow::onLogin()
{
    if (loginUsername_->text().isEmpty() || loginPassword_->text().isEmpty()) {
        QMessageBox::warning(this, "Missing fields", "Please enter username and password.");
        return;
    }
    Message msg;
    msg.type     = MessageType::AUTH_LOGIN;
    msg.username = loginUsername_->text().toStdString();
    msg.password = loginPassword_->text().toStdString();
    client_->send(msg);
    log("→ LOGIN: " + loginUsername_->text());
}

void TestWindow::onSendMessage()
{
    if (currentRoom_.isEmpty()) {
        QMessageBox::warning(this, "No room", "Join or create a room first.");
        return;
    }
    QString text = messageInput_->text().trimmed();
    if (text.isEmpty()) return;

    Message msg;
    msg.type            = MessageType::CHAT_PUBLIC;
    msg.token           = token_.toStdString();
    msg.roomId          = currentRoom_.toStdString();
    msg.text            = text.toStdString();
    msg.sender.username = displayName_.toStdString();
    msg.sender.userId   = userId_.toStdString();
    client_->send(msg);

    messageInput_->clear();
}

void TestWindow::onCreateRoom()
{
    QString roomId = roomIdInput_->text().trimmed();
    if (roomId.isEmpty()) { QMessageBox::warning(this,"Missing","Enter a room ID."); return; }

    Message msg;
    msg.type            = MessageType::CHAT_CREATE;
    msg.token           = token_.toStdString();
    msg.roomId          = roomId.toStdString();
    msg.text            = roomId.toStdString(); // room name = room id for now
    msg.role            = "PUBLIC";
    msg.sender.username = displayName_.toStdString();
    client_->send(msg);
    log("→ CREATE ROOM: " + roomId);
}

void TestWindow::onJoinRoom()
{
    QString roomId = roomIdInput_->text().trimmed();
    if (roomId.isEmpty()) { QMessageBox::warning(this,"Missing","Enter a room ID."); return; }

    Message msg;
    msg.type            = MessageType::JOIN;
    msg.token           = token_.toStdString();
    msg.roomId          = roomId.toStdString();
    msg.sender.username = displayName_.toStdString();
    client_->send(msg);
    log("→ JOIN ROOM: " + roomId);
}

// ─────────────────────────────────────────────────────────────────────────────
//  HANDLE INCOMING MESSAGES
// ─────────────────────────────────────────────────────────────────────────────
void TestWindow::onMessage(const Message& msg)
{
    switch (msg.type)
    {
    case MessageType::AUTH_RESPONSE:
        if (!msg.token.empty()) {
            token_       = QString::fromStdString(msg.token);
            userId_      = QString::fromStdString(msg.sender.userId);
            displayName_ = QString::fromStdString(msg.displayName);
            log("✅ LOGIN OK | name: " + displayName_ + " | userId: " + userId_);
            stack_->setCurrentIndex(2); // go to chat
        } else {
            log("✅ " + QString::fromStdString(msg.text));
            QMessageBox::information(this, "Success",
                QString::fromStdString(msg.text) + "\n\nYou can now login.");
            showLogin();
        }
        break;

    case MessageType::CHAT_PUBLIC:
    case MessageType::CHAT_PRIVATE: {
        QString room   = QString::fromStdString(msg.roomId);
        QString sender = QString::fromStdString(msg.sender.username);
        QString text   = QString::fromStdString(msg.text);

        // add room to sidebar if not already there
        auto items = roomList_->findItems(room, Qt::MatchExactly);
        if (items.isEmpty())
            roomList_->addItem(room);

        appendChat(room, sender, text);
        break;
    }

    case MessageType::JOIN:
    case MessageType::LEAVE: {
        QString room = QString::fromStdString(msg.roomId);
        QString text = QString::fromStdString(msg.text);

        // add room to sidebar on successful join
        auto items = roomList_->findItems(room, Qt::MatchExactly);
        if (items.isEmpty()) {
            roomList_->addItem(room);
            roomList_->setCurrentRow(roomList_->count() - 1);
            currentRoom_ = room;
            currentRoomLabel_->setText("# " + room);
        }
        appendChat(room, "system", text);
        log("ℹ️ " + text);
        break;
    }

    case MessageType::ERROR:
        log("❌ " + QString::fromStdString(msg.text));
        QMessageBox::critical(this, "Error", QString::fromStdString(msg.text));
        break;

    default:
        log("? " + QString::fromStdString(Message::typeToString(msg.type))
            + " | " + QString::fromStdString(msg.text));
        break;
    }
}

void TestWindow::appendChat(const QString& room, const QString& sender, const QString& text)
{
    // Only show messages for the currently selected room
    if (room != currentRoom_) return;

    QString color = (sender == "system") ? "#64748b" :
                    (sender == displayName_) ? "#60a5fa" : "#4ade80";

    chatView_->append(
        QString("<span style='color:%1;font-weight:bold;'>%2</span>"
                "<span style='color:#94a3b8;'> &nbsp;</span>"
                "<span style='color:#e2e8f0;'>%3</span>")
        .arg(color, sender.toHtmlEscaped(), text.toHtmlEscaped()));

    chatView_->verticalScrollBar()->setValue(
        chatView_->verticalScrollBar()->maximum());
}

void TestWindow::log(const QString& msg)
{
    logView_->append(msg);
    logView_->verticalScrollBar()->setValue(logView_->verticalScrollBar()->maximum());
}