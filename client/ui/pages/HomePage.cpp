#include "HomePage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>

// Color Palette matching the label vibe
static const char* BG_MAIN = "#D4C5B6";       // Warm Sand/Beige
static const char* TEXT_MAIN = "#0F0F0F";     // Ink Black
static const char* ACCENT_ORANGE = "#E65C40"; // Stamp Red-Orange

// Helpers for crisp 1px black lines to mimic the label's dividers
QFrame* createHLine() {
    QFrame* line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFixedHeight(1);
    line->setStyleSheet(QString("background-color: %1; border: none;").arg(TEXT_MAIN));
    return line;
}

QFrame* createVLine() {
    QFrame* line = new QFrame;
    line->setFrameShape(QFrame::VLine);
    line->setFixedWidth(1);
    line->setStyleSheet(QString("background-color: %1; border: none;").arg(TEXT_MAIN));
    return line;
}

HomePage::HomePage(QWidget* parent) : QWidget(parent)
{
    // 1. Base Background
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("HomePage { background-color: %1; }").arg(BG_MAIN));

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(50, 60, 50, 60); // Framing it like a printed label
    outer->setSpacing(0);

    // ── BRANDING (Bolded "THE NETWORK") ───────────────────────────────────
    auto* logo = new QLabel("THE NETWORK");
    logo->setAlignment(Qt::AlignCenter);
    logo->setStyleSheet(QString(
        "color: %1; font-size: 84px; font-weight: 900; " // Maximized thickness (900)
        "letter-spacing: 10px; font-family: sans-serif;"
    ).arg(TEXT_MAIN));
    outer->addWidget(logo);
    outer->addSpacing(40);

    outer->addWidget(createHLine());

    // ── SUBTITLE & VERSION (Mimicking "WOODS | NR 024") ───────────────────
    auto* titleRow = new QHBoxLayout;
    titleRow->setContentsMargins(0, 0, 0, 0);
    titleRow->setSpacing(0);

    auto* sectionName = new QLabel("CAMPUS");
    sectionName->setStyleSheet(QString(
        "color: %1; font-size: 32px; font-weight: bold; "
        "letter-spacing: 2px; padding: 20px 20px;"
    ).arg(TEXT_MAIN));

    auto* versionNum = new QLabel("VER  1.0");
    versionNum->setAlignment(Qt::AlignCenter);
    versionNum->setStyleSheet(QString(
        "color: %1; font-size: 24px; font-family: monospace; "
        "letter-spacing: 4px; padding: 20px 0px;"
    ).arg(TEXT_MAIN));

    titleRow->addWidget(sectionName, 3);
    titleRow->addWidget(createVLine());
    titleRow->addWidget(versionNum, 1);

    outer->addLayout(titleRow);
    outer->addWidget(createHLine());

    // ── TYPE & SIZE (Mimicking "EAU DE PARFUM ---- 50ml") ─────────────────
    auto* typeRow = new QHBoxLayout;
    typeRow->setContentsMargins(0, 30, 0, 30);
    
    auto* typeLabel = new QLabel("STUDENT PLATFORM");
    typeLabel->setStyleSheet(QString("color: %1; font-size: 14px; font-weight: bold; letter-spacing: 2px;").arg(TEXT_MAIN));
    
    auto* typeLine = createHLine(); // Central connector line
    
    auto* sizeLabel = new QLabel("ONLINE");
    sizeLabel->setStyleSheet(QString("color: %1; font-size: 14px; font-weight: bold; letter-spacing: 2px;").arg(TEXT_MAIN));
    
    typeRow->addWidget(typeLabel);
    typeRow->addSpacing(15);
    typeRow->addWidget(typeLine, 1); 
    typeRow->addSpacing(15);
    typeRow->addWidget(sizeLabel);
    
    outer->addLayout(typeRow);
    outer->addWidget(createHLine());

    // ── CENTRALIZED ACTION BUTTONS ────────────────────────────────────────
    auto* centralButtonLayout = new QVBoxLayout;
    centralButtonLayout->setContentsMargins(0, 50, 0, 50);
    centralButtonLayout->setAlignment(Qt::AlignCenter);
    centralButtonLayout->setSpacing(25);

    // Large, circular orange-red stamp button
    auto* btnGet = new QPushButton("GET\nSTARTED");
    btnGet->setFixedSize(150, 150);
    btnGet->setStyleSheet(QString(
        "QPushButton {"
        "  background: transparent; color: %1; "
        "  border: 2px solid %1; border-radius: 75px;" // Perfectly circular
        "  font-size: 16px; font-weight: bold; letter-spacing: 1px;"
        "}"
        "QPushButton:hover { background: %1; color: %2; }"
    ).arg(ACCENT_ORANGE, BG_MAIN));

    // Sleek rectangular outline button below it
    auto* btnSign = new QPushButton("SIGN IN");
    btnSign->setFixedSize(150, 42);
    btnSign->setStyleSheet(QString(
        "QPushButton {"
        "  background: transparent; color: %1; "
        "  border: 1px solid %1; "
        "  font-size: 13px; font-weight: bold; letter-spacing: 2px;"
        "}"
        "QPushButton:hover { background: %1; color: %2; }"
    ).arg(TEXT_MAIN, BG_MAIN));

    centralButtonLayout->addWidget(btnGet, 0, Qt::AlignCenter);
    centralButtonLayout->addWidget(btnSign, 0, Qt::AlignCenter);

    outer->addLayout(centralButtonLayout);
    
    // Pushes all elements flush against their top bounds safely
    outer->addStretch();

    // ── CONNECTIONS ───────────────────────────────────────────────────────
    connect(btnGet, &QPushButton::clicked, this, &HomePage::registerClicked);
    connect(btnSign, &QPushButton::clicked, this, &HomePage::loginClicked);
}