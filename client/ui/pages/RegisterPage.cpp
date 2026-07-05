#include "RegisterPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
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

static QString inputStyle() {
    return QString("QLineEdit{background:%1;color:%2;border:1px solid #334155;"
        "border-radius:8px;padding:10px 14px;font-size:13px;}"
        "QLineEdit:focus{border:1px solid %3;}").arg(BG_INPUT, TEXT_PRI, ACCENT2);
}

RegisterPage::RegisterPage(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);

    // ── card ──────────────────────────────────────────────────────────────
    auto* card = new QWidget;
    card->setFixedWidth(460);
    card->setStyleSheet(QString("background:%1;border-radius:16px;").arg(BG_CARD));
    auto* shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(40); shadow->setOffset(0,8); shadow->setColor(QColor(0,0,0,120));
    card->setGraphicsEffect(shadow);

    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(40,36,40,36);
    cardLayout->setSpacing(14);

    auto* title = new QLabel("Create Account");
    title->setStyleSheet(QString("color:%1;font-size:26px;font-weight:bold;").arg(TEXT_PRI));
    cardLayout->addWidget(title);
    cardLayout->addSpacing(4);

    auto addField = [&](QLineEdit*& f, const QString& ph, bool pw=false){
        f = new QLineEdit; f->setPlaceholderText(ph);
        f->setStyleSheet(inputStyle()); f->setFixedHeight(44);
        if(pw) f->setEchoMode(QLineEdit::Password);
        cardLayout->addWidget(f);
    };
    addField(username_,    "Username");
    addField(displayName_, "Display Name");
    addField(email_,       "Email");
    addField(password_,    "Password", true);
    addField(uniId_,       "University ID  (900XXXXXX)");
    addField(bio_,         "Bio  (optional)");

    cardLayout->addSpacing(4);
    auto* btnSubmit = new QPushButton("Create Account");
    btnSubmit->setFixedHeight(48);
    btnSubmit->setStyleSheet(QString(
        "QPushButton{background:%1;color:white;border:none;border-radius:8px;"
        "font-size:14px;font-weight:bold;}"
        "QPushButton:hover{background:%2;}").arg(ACCENT, ACCENT2));
    cardLayout->addWidget(btnSubmit);

    auto* btnSwitch = new QPushButton("Already have an account? Sign in →");
    btnSwitch->setFlat(true);
    btnSwitch->setStyleSheet(QString("color:%1;font-size:13px;").arg(TEXT_SEC));
    cardLayout->addWidget(btnSwitch, 0, Qt::AlignCenter);

    // ── back button ───────────────────────────────────────────────────────
    auto* btnBack = new QPushButton("← Back");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(QString("color:%1;font-size:13px;").arg(TEXT_SEC));

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(40,20,40,40);
    outer->addWidget(btnBack, 0, Qt::AlignLeft);
    outer->addStretch();
    outer->addWidget(card, 0, Qt::AlignCenter);
    outer->addStretch();

    connect(btnSubmit, &QPushButton::clicked, this, &RegisterPage::onSubmit);
    connect(btnSwitch, &QPushButton::clicked, this, &RegisterPage::loginClicked);
    connect(btnBack,   &QPushButton::clicked, this, &RegisterPage::backClicked);
    connect(password_, &QLineEdit::returnPressed, this, &RegisterPage::onSubmit);
}

void RegisterPage::onSubmit()
{
    if (username_->text().isEmpty() || displayName_->text().isEmpty() ||
        email_->text().isEmpty()    || password_->text().isEmpty()    ||
        uniId_->text().isEmpty()) {
        QMessageBox::warning(this,"Missing fields","All fields except Bio are required.");
        return;
    }
    Message msg;
    msg.type         = MessageType::AUTH_REGISTER;
    msg.username     = username_->text().toStdString();
    msg.displayName  = displayName_->text().toStdString();
    msg.email        = email_->text().toStdString();
    msg.password     = password_->text().toStdString();
    msg.universityId = uniId_->text().toStdString();
    msg.bio          = bio_->text().toStdString();
    emit submitted(msg);
}

void RegisterPage::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    QLinearGradient grad(0,0,width(),height());
    grad.setColorAt(0.0, QColor("#0a0f1e"));
    grad.setColorAt(1.0, QColor("#1a0a2e"));
    p.fillRect(rect(), grad);
}