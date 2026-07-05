#include "MainWindow.h"
#include "ui/pages/HomePage.h"
#include "ui/pages/RegisterPage.h"
#include "ui/pages/LoginPage.h"
#include "ui/pages/MainShell.h"
#include <QThread>
#include <QVBoxLayout>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent) : QWidget(parent)
{
    setWindowTitle("The Network");
    setMinimumSize(1100, 700);

    // ── networking — owned here, passed nowhere ───────────────────────────
    auto* io = new boost::asio::io_context();
    client_  = new Client(*io, "127.0.0.1", "12345");

    client_->setOnMessage([this](const Message& msg){
        QMetaObject::invokeMethod(this, [this, msg]{
            onMessage(msg);
        }, Qt::QueuedConnection);
    });

    auto* ioThread = new QThread(this);
    connect(ioThread, &QThread::started, [io](){ io->run(); });
    ioThread->start();

    // ── pages ─────────────────────────────────────────────────────────────
    homePage_     = new HomePage;
    registerPage_ = new RegisterPage;
    loginPage_    = new LoginPage;
    mainShell_    = new MainShell;

    stack_ = new QStackedWidget;
    stack_->addWidget(homePage_);     // 0
    stack_->addWidget(registerPage_); // 1
    stack_->addWidget(loginPage_);    // 2
    stack_->addWidget(mainShell_);    // 3

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->addWidget(stack_);

    // ── wire page signals ─────────────────────────────────────────────────
    connect(homePage_,     &HomePage::registerClicked,     this, &MainWindow::showRegister);
    connect(homePage_,     &HomePage::loginClicked,        this, &MainWindow::showLogin);
    connect(registerPage_, &RegisterPage::loginClicked,    this, &MainWindow::showLogin);
    connect(registerPage_, &RegisterPage::backClicked,     this, &MainWindow::showHome);
    connect(loginPage_,    &LoginPage::registerClicked,    this, &MainWindow::showRegister);
    connect(loginPage_,    &LoginPage::backClicked,        this, &MainWindow::showHome);
    connect(mainShell_,    &MainShell::logoutClicked,      this, &MainWindow::showHome);

    // form submissions → send to server
    connect(registerPage_, &RegisterPage::submitted, this, &MainWindow::sendToServer);
    connect(loginPage_,    &LoginPage::submitted,    this, &MainWindow::sendToServer);

    // chat/room actions from shell → add token → send to server
    connect(mainShell_, &MainShell::sendMessage, this, [this](Message msg){
        msg.token = token_.toStdString();
        sendToServer(msg);
    });
}

// ── page navigation ───────────────────────────────────────────────────────────
void MainWindow::showHome()     { stack_->setCurrentIndex(0); token_.clear(); }
void MainWindow::showRegister() { stack_->setCurrentIndex(1); }
void MainWindow::showLogin()    { stack_->setCurrentIndex(2); }
void MainWindow::showShell()    { stack_->setCurrentIndex(3); }

// ── send to server ────────────────────────────────────────────────────────────
void MainWindow::sendToServer(const Message& msg)
{
    client_->send(msg);
}

// ── handle all incoming messages ──────────────────────────────────────────────
void MainWindow::onMessage(const Message& msg)
{
    switch (msg.type)
    {
    case MessageType::AUTH_RESPONSE:
        if (!msg.token.empty()) {
            // successful login
            token_       = QString::fromStdString(msg.token);
            userId_      = QString::fromStdString(msg.sender.userId);
            displayName_ = QString::fromStdString(msg.displayName);
            mainShell_->setCurrentUser(displayName_, userId_, token_);
            showShell();
        } else if (!token_.isEmpty()) {
            // already logged in — generic server confirmation, ignore silently
        } else {
            // registration success
            QMessageBox::information(this, "Account Created",
                "Registration successful!\nYou can now sign in.");
            showLogin();
        }
        break;

    case MessageType::ERROR:
        QMessageBox::critical(this, "Error",
            QString::fromStdString(msg.text));
        break;

    default:
        // route everything else to the shell
        mainShell_->routeMessage(msg);
        break;
    }
}