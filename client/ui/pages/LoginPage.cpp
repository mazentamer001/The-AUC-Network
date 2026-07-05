#include "LoginPage.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QPainter>
#include <QLinearGradient>
#include <QGraphicsDropShadowEffect>

static const char* ACCENT   = "#6366f1";
static const char* ACCENT2  = "#818cf8";
static const char* TEXT_PRI = "#f1f5f9";
static const char* TEXT_SEC = "#94a3b8";
static const char* BG_CARD  = "#0f172a";
static const char* BG_INPUT = "#334155";
static const char* SUCCESS  = "#22c55e";

static QString inputStyle() {
    return QString("QLineEdit{background:%1;color:%2;border:1px solid #334155;"
        "border-radius:8px;padding:10px 14px;font-size:13px;}"
        "QLineEdit:focus{border:1px solid %3;}").arg(BG_INPUT, TEXT_PRI, ACCENT2);
}

LoginPage::LoginPage(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);

    auto* card = new QWidget;
    card->setFixedWidth(420);
    card->setStyleSheet(QString("background:%1;border-radius:16px;").arg(BG_CARD));
    auto* shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(40); shadow->setOffset(0,8); shadow->setColor(QColor(0,0,0,120));
    card->setGraphicsEffect(shadow);

    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(40,36,40,36);
    cardLayout->setSpacing(14);

    auto* title = new QLabel("Welcome Back");
    title->setStyleSheet(QString("color:%1;font-size:26px;font-weight:bold;").arg(TEXT_PRI));
    auto* sub = new QLabel("Sign in to your account");
    sub->setStyleSheet(QString("color:%1;font-size:13px;").arg(TEXT_SEC));

    username_ = new QLineEdit;
    username_->setPlaceholderText("Username or email");
    username_->setStyleSheet(inputStyle());
    username_->setFixedHeight(44);

    password_ = new QLineEdit;
    password_->setPlaceholderText("Password");
    password_->setEchoMode(QLineEdit::Password);
    password_->setStyleSheet(inputStyle());
    password_->setFixedHeight(44);

    auto* btnLogin = new QPushButton("Sign In");
    btnLogin->setFixedHeight(48);
    btnLogin->setStyleSheet(QString(
        "QPushButton{background:%1;color:white;border:none;border-radius:8px;"
        "font-size:14px;font-weight:bold;}"
        "QPushButton:hover{background:%2;}").arg(SUCCESS,"#16a34a"));

    auto* btnSwitch = new QPushButton("Don't have an account? Register →");
    btnSwitch->setFlat(true);
    btnSwitch->setStyleSheet(QString("color:%1;font-size:13px;").arg(TEXT_SEC));

    cardLayout->addWidget(title);
    cardLayout->addWidget(sub);
    cardLayout->addSpacing(8);
    cardLayout->addWidget(username_);
    cardLayout->addWidget(password_);
    cardLayout->addSpacing(4);
    cardLayout->addWidget(btnLogin);
    cardLayout->addWidget(btnSwitch, 0, Qt::AlignCenter);

    auto* btnBack = new QPushButton("← Back");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(QString("color:%1;font-size:13px;").arg(TEXT_SEC));

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(40,20,40,40);
    outer->addWidget(btnBack, 0, Qt::AlignLeft);
    outer->addStretch();
    outer->addWidget(card, 0, Qt::AlignCenter);
    outer->addStretch();

    connect(btnLogin,  &QPushButton::clicked,      this, &LoginPage::onSubmit);
    connect(password_, &QLineEdit::returnPressed,  this, &LoginPage::onSubmit);
    connect(btnSwitch, &QPushButton::clicked,      this, &LoginPage::registerClicked);
    connect(btnBack,   &QPushButton::clicked,      this, &LoginPage::backClicked);
}

void LoginPage::onSubmit()
{
    if (username_->text().isEmpty() || password_->text().isEmpty()) {
        QMessageBox::warning(this,"Missing fields","Please enter username and password.");
        return;
    }
    Message msg;
    msg.type     = MessageType::AUTH_LOGIN;
    msg.username = username_->text().toStdString();
    msg.password = password_->text().toStdString();
    emit submitted(msg);
}

void LoginPage::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    QLinearGradient grad(0,0,width(),height());
    grad.setColorAt(0.0, QColor("#0a0f1e"));
    grad.setColorAt(1.0, QColor("#1a0a2e"));
    p.fillRect(rect(), grad);
}