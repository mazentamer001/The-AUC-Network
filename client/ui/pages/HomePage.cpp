#include "HomePage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QLinearGradient>

static const char* ACCENT  = "#6366f1";
static const char* ACCENT2 = "#818cf8";
static const char* TEXT_PRI = "#f1f5f9";
static const char* TEXT_SEC = "#94a3b8";
static const char* BG_CARD  = "#1e293b";

HomePage::HomePage(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0,0,0,0);
    outer->setSpacing(0);

    // ── nav bar ───────────────────────────────────────────────────────────
    auto* nav = new QWidget;
    nav->setFixedHeight(60);
    nav->setStyleSheet("background:rgba(15,23,42,0.85);border-bottom:1px solid #1e293b;");
    auto* navLayout = new QHBoxLayout(nav);
    navLayout->setContentsMargins(32,0,32,0);
    auto* logo = new QLabel("⬡  The Network");
    logo->setStyleSheet(QString("color:%1;font-size:18px;font-weight:bold;").arg(ACCENT2));
    navLayout->addWidget(logo);
    navLayout->addStretch();

    // ── hero ──────────────────────────────────────────────────────────────
    auto* hero = new QWidget;
    hero->setStyleSheet("background:transparent;");
    auto* heroLayout = new QVBoxLayout(hero);
    heroLayout->setAlignment(Qt::AlignCenter);
    heroLayout->setSpacing(0);

    auto* title = new QLabel("The Network");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(QString(
        "color:%1;font-size:72px;font-weight:900;"
        "letter-spacing:-3px;background:transparent;").arg(TEXT_PRI));

    auto* subtitle = new QLabel("Your university. Connected.");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet(QString(
        "color:%1;font-size:20px;font-weight:300;"
        "letter-spacing:3px;background:transparent;").arg(TEXT_SEC));

    auto* line = new QWidget;
    line->setFixedSize(80,3);
    line->setStyleSheet(QString("background:%1;border-radius:2px;").arg(ACCENT));

    // buttons
    auto* btnRow = new QHBoxLayout;
    btnRow->setSpacing(16);
    btnRow->setAlignment(Qt::AlignCenter);

    auto* btnGet = new QPushButton("  Get Started");
    btnGet->setFixedSize(200,52);
    btnGet->setStyleSheet(QString(
        "QPushButton{background:%1;color:white;border:none;"
        "border-radius:26px;font-size:15px;font-weight:bold;}"
        "QPushButton:hover{background:%2;}").arg(ACCENT, ACCENT2));

    auto* btnSign = new QPushButton("  Sign In");
    btnSign->setFixedSize(200,52);
    btnSign->setStyleSheet(QString(
        "QPushButton{background:transparent;color:%1;"
        "border:2px solid %1;border-radius:26px;font-size:15px;font-weight:bold;}"
        "QPushButton:hover{background:%1;color:white;}").arg(ACCENT2));

    btnRow->addWidget(btnGet);
    btnRow->addWidget(btnSign);

    // feature pills
    auto* pillRow = new QHBoxLayout;
    pillRow->setSpacing(12);
    pillRow->setAlignment(Qt::AlignCenter);
    for (auto& f : {"💬  Chat","🛒  Marketplace","📁  Files","❓  Forum"}) {
        auto* pill = new QLabel(f);
        pill->setStyleSheet(QString(
            "background:%1;color:%2;border-radius:16px;"
            "padding:6px 18px;font-size:13px;").arg(BG_CARD, TEXT_SEC));
        pillRow->addWidget(pill);
    }

    heroLayout->addStretch(2);
    heroLayout->addWidget(title);
    heroLayout->addSpacing(10);
    heroLayout->addWidget(subtitle);
    heroLayout->addSpacing(20);
    heroLayout->addWidget(line, 0, Qt::AlignCenter);
    heroLayout->addSpacing(44);
    heroLayout->addLayout(btnRow);
    heroLayout->addSpacing(36);
    heroLayout->addLayout(pillRow);
    heroLayout->addStretch(3);

    outer->addWidget(nav);
    outer->addWidget(hero, 1);

    connect(btnGet,  &QPushButton::clicked, this, &HomePage::registerClicked);
    connect(btnSign, &QPushButton::clicked, this, &HomePage::loginClicked);
}

void HomePage::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    QLinearGradient grad(0,0,width(),height());
    grad.setColorAt(0.0, QColor("#0a0f1e"));
    grad.setColorAt(0.5, QColor("#0f172a"));
    grad.setColorAt(1.0, QColor("#1a0a2e"));
    p.fillRect(rect(), grad);
}