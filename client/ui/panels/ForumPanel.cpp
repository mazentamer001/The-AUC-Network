#include "ForumPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QTimer>
#include <QMessageBox>

// ── Color Palette matching HomePage's warm printed-label vibe ──────────────
static const char* BG_MAIN      = "#D4C5B6"; // Warm Sand/Beige
static const char* BG_PANEL     = "#CBB9A6"; // Slightly deeper sand (panels/cards)
static const char* BG_INPUT     = "#E8DDD0"; // Pale parchment (inputs)
static const char* TEXT_MAIN    = "#0F0F0F"; // Ink Black
static const char* TEXT_SEC     = "#4A4038"; // Muted brown-black (secondary text)
static const char* ACCENT_ORANGE= "#E65C40"; // Stamp Red-Orange
static const char* GOLD         = "#B8862B"; // Ink-gold for FAQ accents

// ── Helpers for crisp 1px black lines, mirroring HomePage's dividers ───────
static QFrame* createHLine() {
    QFrame* line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFixedHeight(1);
    line->setStyleSheet(QString("background-color:%1;border:none;").arg(TEXT_MAIN));
    return line;
}
static QFrame* createVLine() {
    QFrame* line = new QFrame;
    line->setFrameShape(QFrame::VLine);
    line->setFixedWidth(1);
    line->setStyleSheet(QString("background-color:%1;border:none;").arg(TEXT_MAIN));
    return line;
}

static QString inputStyle() {
    return QString("QLineEdit{background:%1;color:%2;border:1px solid %3;"
        "border-radius:0px;padding:8px 12px;font-size:13px;letter-spacing:1px;}"
        "QLineEdit:focus{border:1px solid %4;}").arg(BG_INPUT,TEXT_MAIN,TEXT_MAIN,ACCENT_ORANGE);
}
static QString textAreaStyle() {
    return QString("QTextEdit{background:%1;color:%2;border:1px solid %3;"
        "border-radius:0px;padding:8px;font-size:13px;}"
        "QTextEdit:focus{border:1px solid %4;}").arg(BG_INPUT,TEXT_MAIN,TEXT_MAIN,ACCENT_ORANGE);
}
// Primary "stamp" button — solid orange outline, fills on hover
static QString btnStampStyle() {
    return QString("QPushButton{background:transparent;color:%1;"
        "border:2px solid %1;border-radius:4px;padding:8px 18px;"
        "font-size:12px;font-weight:bold;letter-spacing:2px;}"
        "QPushButton:hover{background:%1;color:%2;}").arg(ACCENT_ORANGE,BG_MAIN);
}
// Secondary "sign in" style button — thin ink outline
static QString btnOutlineStyle() {
    return QString("QPushButton{background:transparent;color:%1;"
        "border:1px solid %1;border-radius:0px;padding:6px 14px;"
        "font-size:12px;font-weight:bold;letter-spacing:2px;}"
        "QPushButton:hover{background:%1;color:%2;}").arg(TEXT_MAIN,BG_MAIN);
}
// Flat text-only "back" style button, matching HomePage's minimal chrome
static QString btnFlatStyle() {
    return QString("QPushButton{background:transparent;color:%1;border:none;"
        "font-size:13px;letter-spacing:1px;}"
        "QPushButton:hover{color:%2;}").arg(TEXT_SEC,TEXT_MAIN);
}
static QString voteBtn() {
    return QString("QPushButton{background:transparent;color:%1;border:1px solid %1;"
        "border-radius:0px;padding:2px 8px;font-size:12px;font-weight:bold;}"
        "QPushButton:hover{background:%1;color:%2;}").arg(TEXT_MAIN,BG_MAIN);
}
static QString msgBoxStyle() {
    return QString("QMessageBox{background:%1;color:%2;}"
        "QMessageBox QLabel{color:%2;}"
        "QMessageBox QPushButton{background:transparent;color:%3;"
        "border:2px solid %3;border-radius:4px;padding:6px 18px;letter-spacing:1px;}"
        "QMessageBox QPushButton:hover{background:%3;color:%1;}")
        .arg(BG_MAIN,TEXT_MAIN,ACCENT_ORANGE);
}

// ─────────────────────────────────────────────────────────────────────────────
//  QuestionCard
// ─────────────────────────────────────────────────────────────────────────────
QuestionCard::QuestionCard(const QString& id, const QString& title,
                           const QString& text, const QString& author,
                           const QString& timestamp, int upvotes, int downvotes,
                           int answerCount, QWidget* parent)
    : QWidget(parent), id_(id)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("QuestionCard{background:%1;border:1px solid %2;}"
        "QuestionCard:hover{background:%3;}").arg(BG_MAIN,TEXT_MAIN,BG_PANEL));
    setCursor(Qt::PointingHandCursor);

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    // ── vote sidebar ──────────────────────────────────────────────────────
    auto* voteSide = new QWidget;
    voteSide->setFixedWidth(64);
    voteSide->setAttribute(Qt::WA_StyledBackground, true);
    voteSide->setStyleSheet(QString("background:%1;").arg(BG_PANEL));
    auto* voteLayout = new QVBoxLayout(voteSide);
    voteLayout->setContentsMargins(0,12,0,12);
    voteLayout->setSpacing(4);
    voteLayout->setAlignment(Qt::AlignCenter);

    auto* btnUp = new QPushButton("▲");
    btnUp->setFixedSize(32,28);
    btnUp->setStyleSheet(voteBtn());

    votesLabel_ = new QLabel(QString::number(upvotes - downvotes));
    votesLabel_->setAlignment(Qt::AlignCenter);
    votesLabel_->setStyleSheet(QString("color:%1;font-size:14px;font-weight:bold;"
        "background:transparent;").arg(TEXT_MAIN));

    auto* btnDown = new QPushButton("▼");
    btnDown->setFixedSize(32,28);
    btnDown->setStyleSheet(voteBtn());

    voteLayout->addStretch();
    voteLayout->addWidget(btnUp,        0, Qt::AlignCenter);
    voteLayout->addWidget(votesLabel_,  0, Qt::AlignCenter);
    voteLayout->addWidget(btnDown,      0, Qt::AlignCenter);
    voteLayout->addStretch();

    // ── content ───────────────────────────────────────────────────────────
    auto* content = new QWidget;
    content->setAttribute(Qt::WA_StyledBackground, false);
    auto* cLayout = new QVBoxLayout(content);
    cLayout->setContentsMargins(16,14,16,14);
    cLayout->setSpacing(6);

    auto* titleLbl = new QLabel(title.toUpper());
    titleLbl->setStyleSheet(QString("color:%1;font-size:15px;font-weight:900;"
        "letter-spacing:1px;background:transparent;").arg(TEXT_MAIN));
    titleLbl->setWordWrap(true);

    auto* bodyLbl = new QLabel(text.left(120) + (text.length()>120?"...":""));
    bodyLbl->setStyleSheet(QString("color:%1;font-size:13px;background:transparent;").arg(TEXT_SEC));
    bodyLbl->setWordWrap(true);

    auto* metaRow = new QHBoxLayout;
    auto* authorLbl = new QLabel(author.toUpper());
    authorLbl->setStyleSheet(QString("color:%1;font-size:11px;font-weight:bold;"
        "letter-spacing:1px;background:transparent;").arg(TEXT_SEC));
    auto* timeLbl = new QLabel(timestamp.left(10));
    timeLbl->setStyleSheet(QString("color:%1;font-size:11px;background:transparent;").arg(TEXT_SEC));
    answerCountLabel_ = new QLabel(QString::number(answerCount) + " ANSWERS");
    answerCountLabel_->setStyleSheet(QString("color:%1;font-size:11px;font-weight:bold;"
        "letter-spacing:1px;background:transparent;").arg(ACCENT_ORANGE));

    metaRow->addWidget(authorLbl);
    metaRow->addSpacing(12);
    metaRow->addWidget(timeLbl);
    metaRow->addStretch();
    metaRow->addWidget(answerCountLabel_);

    cLayout->addWidget(titleLbl);
    cLayout->addWidget(bodyLbl);
    cLayout->addLayout(metaRow);

    root->addWidget(voteSide);
    root->addWidget(createVLine());
    root->addWidget(content, 1);

    connect(btnUp,   &QPushButton::clicked, this, [this](){ emit upvoteClicked(id_); });
    connect(btnDown, &QPushButton::clicked, this, [this](){ emit downvoteClicked(id_); });
}

void QuestionCard::mousePressEvent(QMouseEvent*) { emit clicked(id_); }
void QuestionCard::updateVotes(int up, int down) {
    votesLabel_->setText(QString::number(up - down));
}
void QuestionCard::updateAnswerCount(int count) {
    answerCountLabel_->setText(QString::number(count) + " ANSWERS");
}

// ─────────────────────────────────────────────────────────────────────────────
//  AnswerWidget
// ─────────────────────────────────────────────────────────────────────────────
AnswerWidget::AnswerWidget(const QString& questionId, const QString& answerId,
                           const QString& author, const QString& text,
                           int upvotes, int downvotes, bool isFaq,
                           QWidget* parent)
    : QWidget(parent), questionId_(questionId), answerId_(answerId)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("AnswerWidget{background:%1;border:%2 solid %3;}")
        .arg(BG_MAIN, isFaq ? "2px" : "1px", isFaq ? GOLD : TEXT_MAIN));

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    // vote column
    auto* voteSide = new QWidget;
    voteSide->setFixedWidth(52);
    voteSide->setAttribute(Qt::WA_StyledBackground, true);
    voteSide->setStyleSheet(QString("background:%1;").arg(BG_PANEL));
    auto* vLayout = new QVBoxLayout(voteSide);
    vLayout->setContentsMargins(0,10,0,10);
    vLayout->setSpacing(2);
    vLayout->setAlignment(Qt::AlignCenter);

    auto* btnUp   = new QPushButton("▲");
    btnUp->setFixedSize(28,24);
    btnUp->setStyleSheet(voteBtn());

    votesLabel_ = new QLabel(QString::number(upvotes - downvotes));
    votesLabel_->setAlignment(Qt::AlignCenter);
    votesLabel_->setStyleSheet(QString("color:%1;font-size:13px;font-weight:bold;"
        "background:transparent;").arg(TEXT_MAIN));

    auto* btnDown = new QPushButton("▼");
    btnDown->setFixedSize(28,24);
    btnDown->setStyleSheet(voteBtn());

    vLayout->addStretch();
    vLayout->addWidget(btnUp,       0, Qt::AlignCenter);
    vLayout->addWidget(votesLabel_, 0, Qt::AlignCenter);
    vLayout->addWidget(btnDown,     0, Qt::AlignCenter);
    vLayout->addStretch();

    // content
    auto* content = new QWidget;
    content->setAttribute(Qt::WA_StyledBackground, false);
    auto* cLayout = new QVBoxLayout(content);
    cLayout->setContentsMargins(14,12,14,12);
    cLayout->setSpacing(6);

    if (isFaq) {
        auto* faqBadge = new QLabel("★ FAQ ANSWER");
        faqBadge->setStyleSheet(QString("color:%1;font-size:11px;font-weight:bold;"
            "letter-spacing:1px;background:transparent;").arg(GOLD));
        cLayout->addWidget(faqBadge);
    }

    auto* authorLbl = new QLabel(author.toUpper());
    authorLbl->setStyleSheet(QString("color:%1;font-size:11px;font-weight:bold;"
        "letter-spacing:1px;background:transparent;").arg(ACCENT_ORANGE));

    auto* textLbl = new QLabel(text);
    textLbl->setWordWrap(true);
    textLbl->setStyleSheet(QString("color:%1;font-size:13px;background:transparent;").arg(TEXT_MAIN));

    cLayout->addWidget(authorLbl);
    cLayout->addWidget(textLbl);

    root->addWidget(voteSide);
    root->addWidget(createVLine());
    root->addWidget(content, 1);

    connect(btnUp,   &QPushButton::clicked, this, [this](){
        emit upvoteClicked(questionId_, answerId_); });
    connect(btnDown, &QPushButton::clicked, this, [this](){
        emit downvoteClicked(questionId_, answerId_); });
}

void AnswerWidget::updateVotes(int up, int down) {
    votesLabel_->setText(QString::number(up - down));
}

// ─────────────────────────────────────────────────────────────────────────────
//  PostQuestionPanel
// ─────────────────────────────────────────────────────────────────────────────
PostQuestionPanel::PostQuestionPanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("background:%1;").arg(BG_MAIN));

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(50,40,50,40);
    outer->setSpacing(0);

    auto* btnBack = new QPushButton("←  BACK TO FORUM");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(btnFlatStyle());
    btnBack->setFixedWidth(180);

    outer->addWidget(btnBack, 0, Qt::AlignLeft);
    outer->addSpacing(20);

    auto* title = new QLabel("ASK A QUESTION");
    title->setStyleSheet(QString("color:%1;font-size:32px;font-weight:900;"
        "letter-spacing:4px;").arg(TEXT_MAIN));
    outer->addWidget(title);
    outer->addSpacing(20);
    outer->addWidget(createHLine());
    outer->addSpacing(24);

    auto lbl = [](const QString& t){
        auto* l = new QLabel(t);
        l->setStyleSheet(QString("color:%1;font-size:11px;font-weight:bold;"
            "letter-spacing:2px;").arg(TEXT_SEC));
        return l;
    };

    title_ = new QLineEdit;
    title_->setPlaceholderText("What is your question?");
    title_->setFixedHeight(42);
    title_->setStyleSheet(inputStyle());

    body_ = new QTextEdit;
    body_->setPlaceholderText("Provide more details...");
    body_->setFixedHeight(140);
    body_->setStyleSheet(textAreaStyle());

    auto* btnRow = new QHBoxLayout;
    auto* btnCancel = new QPushButton("CANCEL");
    auto* btnPost   = new QPushButton("POST QUESTION");
    btnCancel->setFixedHeight(42);
    btnPost->setFixedHeight(42);
    btnCancel->setStyleSheet(btnOutlineStyle());
    btnPost->setStyleSheet(btnStampStyle());
    btnRow->addWidget(btnCancel);
    btnRow->addWidget(btnPost);
    btnRow->addStretch();

    outer->addWidget(lbl("TITLE *"));
    outer->addSpacing(6);
    outer->addWidget(title_);
    outer->addSpacing(16);
    outer->addWidget(lbl("DETAILS"));
    outer->addSpacing(6);
    outer->addWidget(body_);
    outer->addSpacing(20);
    outer->addLayout(btnRow);
    outer->addStretch();

    connect(btnPost,   &QPushButton::clicked, this, &PostQuestionPanel::onSubmit);
    connect(btnCancel, &QPushButton::clicked, this, &PostQuestionPanel::cancelled);
    connect(btnBack,   &QPushButton::clicked, this, &PostQuestionPanel::cancelled);
}

void PostQuestionPanel::onSubmit()
{
    if (title_->text().trimmed().isEmpty()) {
        auto* box = new QMessageBox(this);
        box->setStyleSheet(msgBoxStyle());
        box->setText("Title is required.");
        box->exec();
        return;
    }
    Message msg;
    msg.type            = MessageType::QA_QUESTION;
    msg.title           = title_->text().trimmed().toStdString();
    msg.text            = body_->toPlainText().trimmed().toStdString();
    msg.sender.username = username_.toStdString();
    emit submitted(msg);
    title_->clear();
    body_->clear();
}

// ─────────────────────────────────────────────────────────────────────────────
//  QuestionDetailPanel
// ─────────────────────────────────────────────────────────────────────────────
QuestionDetailPanel::QuestionDetailPanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("background:%1;").arg(BG_MAIN));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    // scroll area wraps everything
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(QString(
        "QScrollArea{border:none;background:%1;}"
        "QScrollBar:vertical{background:%2;width:5px;}"
        "QScrollBar::handle:vertical{background:%3;}")
        .arg(BG_MAIN, BG_PANEL, TEXT_MAIN));

    auto* container = new QWidget;
    container->setAttribute(Qt::WA_StyledBackground, true);
    container->setStyleSheet(QString("background:%1;").arg(BG_MAIN));
    auto* cLayout = new QVBoxLayout(container);
    cLayout->setContentsMargins(50,32,50,32);
    cLayout->setSpacing(18);

    // back button
    auto* btnBack = new QPushButton("←  BACK TO FORUM");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(btnFlatStyle());
    btnBack->setFixedWidth(180);

    // question block (bordered, like a label section)
    auto* qCard = new QWidget;
    qCard->setAttribute(Qt::WA_StyledBackground, true);
    qCard->setStyleSheet(QString("background:%1;border:1px solid %2;").arg(BG_MAIN,TEXT_MAIN));
    auto* qLayout = new QVBoxLayout(qCard);
    qLayout->setContentsMargins(24,20,24,20);
    qLayout->setSpacing(10);

    titleLabel_ = new QLabel;
    titleLabel_->setStyleSheet(QString("color:%1;font-size:22px;font-weight:900;"
        "letter-spacing:2px;background:transparent;").arg(TEXT_MAIN));
    titleLabel_->setWordWrap(true);

    metaLabel_ = new QLabel;
    metaLabel_->setStyleSheet(QString("color:%1;font-size:12px;letter-spacing:1px;"
        "background:transparent;").arg(TEXT_SEC));

    bodyLabel_ = new QLabel;
    bodyLabel_->setWordWrap(true);
    bodyLabel_->setStyleSheet(QString("color:%1;font-size:14px;background:transparent;").arg(TEXT_MAIN));

    // vote row for question
    auto* voteRow = new QHBoxLayout;
    auto* btnQUp   = new QPushButton("▲ UPVOTE");
    auto* btnQDown = new QPushButton("▼ DOWNVOTE");
    votesLabel_ = new QLabel;
    votesLabel_->setStyleSheet(QString("color:%1;font-size:13px;font-weight:bold;"
        "letter-spacing:1px;background:transparent;").arg(TEXT_MAIN));
    btnQUp->setStyleSheet(btnOutlineStyle());
    btnQDown->setStyleSheet(btnOutlineStyle());
    btnQUp->setFixedHeight(32);
    btnQDown->setFixedHeight(32);
    voteRow->addWidget(btnQUp);
    voteRow->addWidget(votesLabel_);
    voteRow->addWidget(btnQDown);
    voteRow->addStretch();

    qLayout->addWidget(titleLabel_);
    qLayout->addWidget(createHLine());
    qLayout->addWidget(metaLabel_);
    qLayout->addWidget(bodyLabel_);
    qLayout->addLayout(voteRow);

    // answers section
    auto* answersHeader = new QLabel("ANSWERS");
    answersHeader->setStyleSheet(QString("color:%1;font-size:16px;font-weight:bold;"
        "letter-spacing:2px;background:transparent;").arg(TEXT_MAIN));

    auto* answersContainer = new QWidget;
    answersContainer->setAttribute(Qt::WA_StyledBackground, false);
    answersLayout_ = new QVBoxLayout(answersContainer);
    answersLayout_->setContentsMargins(0,0,0,0);
    answersLayout_->setSpacing(10);

    // answer input
    auto* answerHeader = new QLabel("YOUR ANSWER");
    answerHeader->setStyleSheet(QString("color:%1;font-size:14px;font-weight:bold;"
        "letter-spacing:2px;background:transparent;").arg(TEXT_MAIN));

    answerInput_ = new QTextEdit;
    answerInput_->setPlaceholderText("Write your answer...");
    answerInput_->setFixedHeight(100);
    answerInput_->setStyleSheet(textAreaStyle());

    auto* btnSubmitAnswer = new QPushButton("POST ANSWER");
    btnSubmitAnswer->setFixedHeight(38);
    btnSubmitAnswer->setMaximumWidth(180);
    btnSubmitAnswer->setStyleSheet(btnStampStyle());

    cLayout->addWidget(btnBack, 0, Qt::AlignLeft);
    cLayout->addWidget(qCard);
    cLayout->addWidget(createHLine());
    cLayout->addWidget(answersHeader);
    cLayout->addWidget(answersContainer);
    cLayout->addWidget(createHLine());
    cLayout->addWidget(answerHeader);
    cLayout->addWidget(answerInput_);
    cLayout->addWidget(btnSubmitAnswer, 0, Qt::AlignLeft);

    scroll->setWidget(container);
    root->addWidget(scroll);

    connect(btnBack, &QPushButton::clicked, this, &QuestionDetailPanel::backClicked);
    connect(btnQUp,  &QPushButton::clicked, this, [this](){
        emit upvoteQuestion(questionId_); });
    connect(btnQDown,&QPushButton::clicked, this, [this](){
        emit downvoteQuestion(questionId_); });
    connect(btnSubmitAnswer, &QPushButton::clicked, this, [this](){
        QString text = answerInput_->toPlainText().trimmed();
        if (text.isEmpty()) return;
        Message msg;
        msg.type            = MessageType::QA_ANSWER;
        msg.parentId        = questionId_.toStdString();
        msg.text            = text.toStdString();
        msg.sender.username = username_.toStdString();
        emit answerSubmitted(msg);
        answerInput_->clear();
    });
}

void QuestionDetailPanel::load(const QString& qId, const QString& title,
                                const QString& text, const QString& author,
                                const QString& timestamp, int up, int down)
{
    questionId_ = qId;
    titleLabel_->setText(title.toUpper());
    metaLabel_->setText("ASKED BY  " + author.toUpper() + "   ·   " + timestamp.left(10));
    bodyLabel_->setText(text);
    votesLabel_->setText(QString("  %1 VOTES  ").arg(up - down));
    clearAnswers();
}

void QuestionDetailPanel::addAnswer(const QString& qId, const QString& aId,
                                     const QString& author, const QString& text,
                                     int up, int down, bool isFaq)
{
    auto* w = new AnswerWidget(qId, aId, author, text, up, down, isFaq);
    answerWidgets_[aId] = w;
    answersLayout_->addWidget(w);
    connect(w, &AnswerWidget::upvoteClicked,   this, &QuestionDetailPanel::upvoteAnswer);
    connect(w, &AnswerWidget::downvoteClicked, this, &QuestionDetailPanel::downvoteAnswer);
}

void QuestionDetailPanel::clearAnswers()
{
    while (QLayoutItem* item = answersLayout_->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    answerWidgets_.clear();
}

// ─────────────────────────────────────────────────────────────────────────────
//  FaqPanel
// ─────────────────────────────────────────────────────────────────────────────
FaqPanel::FaqPanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("background:%1;").arg(BG_MAIN));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(50,32,50,32);
    root->setSpacing(16);

    auto* btnBack = new QPushButton("←  BACK TO FORUM");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(btnFlatStyle());
    btnBack->setFixedWidth(180);

    auto* title = new QLabel("★  FREQUENTLY ASKED QUESTIONS");
    title->setStyleSheet(QString("color:%1;font-size:28px;font-weight:900;"
        "letter-spacing:2px;background:transparent;").arg(TEXT_MAIN));

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(QString(
        "QScrollArea{border:none;background:%1;}"
        "QScrollBar:vertical{background:%2;width:5px;}"
        "QScrollBar::handle:vertical{background:%3;}")
        .arg(BG_MAIN, BG_PANEL, TEXT_MAIN));

    auto* container = new QWidget;
    container->setAttribute(Qt::WA_StyledBackground, true);
    container->setStyleSheet(QString("background:%1;").arg(BG_MAIN));
    layout_ = new QVBoxLayout(container);
    layout_->setContentsMargins(0,0,0,0);
    layout_->setSpacing(12);
    layout_->setAlignment(Qt::AlignTop);
    scroll->setWidget(container);

    root->addWidget(btnBack, 0, Qt::AlignLeft);
    root->addWidget(title);
    root->addWidget(createHLine());
    root->addWidget(scroll, 1);

    connect(btnBack, &QPushButton::clicked, this, &FaqPanel::backClicked);
}

void FaqPanel::addFaqAnswer(const QString& questionTitle,
                             const QString& answerText, const QString& author)
{
    auto* card = new QWidget;
    card->setAttribute(Qt::WA_StyledBackground, true);
    card->setStyleSheet(QString("background:%1;border:2px solid %2;").arg(BG_MAIN, GOLD));
    auto* l = new QVBoxLayout(card);
    l->setContentsMargins(20,16,20,16);
    l->setSpacing(8);

    auto* qLbl = new QLabel("Q:  " + questionTitle.toUpper());
    qLbl->setStyleSheet(QString("color:%1;font-size:13px;font-weight:bold;"
        "letter-spacing:1px;background:transparent;").arg(GOLD));
    qLbl->setWordWrap(true);

    auto* aLbl = new QLabel("A:  " + answerText);
    aLbl->setWordWrap(true);
    aLbl->setStyleSheet(QString("color:%1;font-size:13px;background:transparent;").arg(TEXT_MAIN));

    auto* authorLbl = new QLabel("—  " + author.toUpper());
    authorLbl->setStyleSheet(QString("color:%1;font-size:11px;letter-spacing:1px;"
        "background:transparent;").arg(TEXT_SEC));

    l->addWidget(qLbl);
    l->addWidget(aLbl);
    l->addWidget(authorLbl);
    layout_->addWidget(card);
}

void FaqPanel::clear()
{
    while (QLayoutItem* item = layout_->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  ForumPanel
// ─────────────────────────────────────────────────────────────────────────────
ForumPanel::ForumPanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("background:%1;").arg(BG_MAIN));

    stack_ = new QStackedWidget(this);
    stack_->setStyleSheet(QString("background:%1;").arg(BG_MAIN));

    // ── browse page ───────────────────────────────────────────────────────
    auto* browsePage = new QWidget;
    browsePage->setAttribute(Qt::WA_StyledBackground, true);
    browsePage->setStyleSheet(QString("background:%1;").arg(BG_MAIN));
    auto* browseLayout = new QVBoxLayout(browsePage);
    browseLayout->setContentsMargins(50,32,50,32);
    browseLayout->setSpacing(16);

    // branding row, echoing HomePage's masthead
    auto* pageTitle = new QLabel("FORUM");
    pageTitle->setStyleSheet(QString("color:%1;font-size:36px;font-weight:900;"
        "letter-spacing:6px;background:transparent;").arg(TEXT_MAIN));
    browseLayout->addWidget(pageTitle);
    browseLayout->addWidget(createHLine());
    browseLayout->addSpacing(4);

    // top bar
    auto* topBar = new QHBoxLayout;

    searchBar_ = new QLineEdit;
    searchBar_->setPlaceholderText("Search questions...");
    searchBar_->setFixedHeight(38);
    searchBar_->setMaximumWidth(260);
    searchBar_->setStyleSheet(inputStyle());

    auto* btnFaq = new QPushButton("★  FAQ");
    btnFaq->setFixedHeight(38);
    btnFaq->setStyleSheet(btnOutlineStyle());

    auto* btnAsk = new QPushButton("+  ASK QUESTION");
    btnAsk->setFixedHeight(38);
    btnAsk->setStyleSheet(btnStampStyle());

    topBar->addWidget(searchBar_);
    topBar->addStretch();
    topBar->addWidget(btnFaq);
    topBar->addSpacing(8);
    topBar->addWidget(btnAsk);

    // scroll area for cards
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(QString(
        "QScrollArea{border:none;background:%1;}"
        "QScrollBar:vertical{background:%2;width:5px;}"
        "QScrollBar::handle:vertical{background:%3;}")
        .arg(BG_MAIN, BG_PANEL, TEXT_MAIN));

    cardsWidget_ = new QWidget;
    cardsWidget_->setAttribute(Qt::WA_StyledBackground, true);
    cardsWidget_->setStyleSheet(QString("background:%1;").arg(BG_MAIN));
    cardsLayout_ = new QVBoxLayout(cardsWidget_);
    cardsLayout_->setContentsMargins(0,4,0,4);
    cardsLayout_->setSpacing(10);
    cardsLayout_->setAlignment(Qt::AlignTop);

    emptyLabel_ = new QLabel("NO QUESTIONS YET. BE THE FIRST TO ASK!");
    emptyLabel_->setAlignment(Qt::AlignCenter);
    emptyLabel_->setStyleSheet(QString("color:%1;font-size:14px;font-weight:bold;"
        "letter-spacing:1px;padding:60px;background:transparent;").arg(TEXT_SEC));
    cardsLayout_->addWidget(emptyLabel_);
    scroll->setWidget(cardsWidget_);

    browseLayout->addLayout(topBar);
    browseLayout->addWidget(scroll, 1);

    // ── sub panels ────────────────────────────────────────────────────────
    postPanel_   = new PostQuestionPanel;
    detailPanel_ = new QuestionDetailPanel;
    faqPanel_    = new FaqPanel;

    stack_->addWidget(browsePage);    // 0
    stack_->addWidget(postPanel_);    // 1
    stack_->addWidget(detailPanel_);  // 2
    stack_->addWidget(faqPanel_);     // 3

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->addWidget(stack_);

    // connections
    connect(btnAsk,  &QPushButton::clicked, this, &ForumPanel::showPost);
    connect(btnFaq,  &QPushButton::clicked, this, &ForumPanel::showFaq);
    connect(searchBar_, &QLineEdit::textChanged, this, &ForumPanel::onSearch);

    connect(postPanel_, &PostQuestionPanel::submitted, this, [this](const Message& msg){
        emit sendMessage(msg);
        showBrowse();
    });
    connect(postPanel_, &PostQuestionPanel::cancelled, this, &ForumPanel::showBrowse);

    connect(detailPanel_, &QuestionDetailPanel::backClicked,     this, &ForumPanel::showBrowse);
    connect(detailPanel_, &QuestionDetailPanel::answerSubmitted, this, [this](const Message& msg){
        emit sendMessage(msg);
    });
    connect(detailPanel_, &QuestionDetailPanel::upvoteQuestion,   this, [this](const QString& qId){
        sendVote(qId, "", true);
    });
    connect(detailPanel_, &QuestionDetailPanel::downvoteQuestion, this, [this](const QString& qId){
        sendVote(qId, "", false);
    });
    connect(detailPanel_, &QuestionDetailPanel::upvoteAnswer,  this,
            [this](const QString& qId, const QString& aId){ sendVote(qId, aId, true); });
    connect(detailPanel_, &QuestionDetailPanel::downvoteAnswer, this,
            [this](const QString& qId, const QString& aId){ sendVote(qId, aId, false); });

    connect(faqPanel_, &FaqPanel::backClicked, this, &ForumPanel::showBrowse);
}

void ForumPanel::setCurrentUser(const QString& displayName,
                                 const QString& userId,
                                 const QString& token)
{
    displayName_ = displayName;
    userId_      = userId;
    token_       = token;
    postPanel_->setUser(displayName);
    detailPanel_->setUser(displayName);
    QTimer::singleShot(900, this, &ForumPanel::requestQuestions);
}

void ForumPanel::requestQuestions()
{
    Message msg;
    msg.type  = MessageType::QA_GET_ALL;
    msg.token = token_.toStdString();
    msg.sender.username = displayName_.toStdString();
    emit sendMessage(msg);
}

void ForumPanel::showPost()    { stack_->setCurrentIndex(1); }
void ForumPanel::showBrowse()  { stack_->setCurrentIndex(0); }
void ForumPanel::showFaq()     { stack_->setCurrentIndex(3); }

void ForumPanel::onQuestionClicked(const QString& id)
{
    if (!questions_.contains(id)) return;
    auto& q = questions_[id];
    detailPanel_->load(q.id, q.title, q.text, q.author, q.timestamp,
                       q.upvotes, q.downvotes);
    stack_->setCurrentIndex(2);

    // request answers
    Message msg;
    msg.type     = MessageType::QA_GET_ONE;
    msg.token    = token_.toStdString();
    msg.parentId = id.toStdString();
    msg.sender.username = displayName_.toStdString();
    emit sendMessage(msg);
}

void ForumPanel::onSearch(const QString& query)
{
    QString q = query.toLower();
    for (auto* card : cards_) {
        auto& data = questions_[card->questionId()];
        bool match = q.isEmpty() ||
                     data.title.toLower().contains(q) ||
                     data.text.toLower().contains(q);
        card->setVisible(match);
    }
}

void ForumPanel::sendVote(const QString& qId, const QString& aId, bool upvote)
{
    Message msg;
    msg.type            = upvote ? MessageType::FORUM_UPVOTE : MessageType::FORUM_DOWNVOTE;
    msg.token           = token_.toStdString();
    msg.parentId        = qId.toStdString();
    msg.filename        = aId.toStdString();
    msg.role            = upvote ? "UP" : "DOWN";
    msg.sender.username = displayName_.toStdString();
    msg.sender.userId   = userId_.toStdString();  // needed for duplicate check
    emit sendMessage(msg);
}

void ForumPanel::addQuestionCard(const QString& id, const QString& title,
                                  const QString& text, const QString& author,
                                  const QString& timestamp, int up, int down,
                                  int answers)
{
    if (cards_.contains(id)) return;

    emptyLabel_->setVisible(false);
    auto* card = new QuestionCard(id, title, text, author, timestamp,
                                  up, down, answers);
    cards_[id] = card;
    cardsLayout_->addWidget(card);

    connect(card, &QuestionCard::clicked,       this, &ForumPanel::onQuestionClicked);
    connect(card, &QuestionCard::upvoteClicked,   this, [this](const QString& qId){
        sendVote(qId, "", true); });
    connect(card, &QuestionCard::downvoteClicked, this, [this](const QString& qId){
        sendVote(qId, "", false); });
}

void ForumPanel::clearCards()
{
    for (auto* c : cards_) c->deleteLater();
    cards_.clear();
    cardsLayout_->addWidget(emptyLabel_);
    emptyLabel_->setVisible(true);
}

void ForumPanel::receiveMessage(const Message& msg)
{
    if (msg.type == MessageType::QA_QUESTION)
    {
        QString id        = QString::fromStdString(msg.parentId);
        QString title     = QString::fromStdString(msg.title);
        QString text      = QString::fromStdString(msg.text);
        QString author    = QString::fromStdString(msg.sender.username);
        QString timestamp = QString::fromStdString(msg.timestamp);
         qDebug() << "QA_QUESTION received:"
             << "id=" << QString::fromStdString(msg.parentId)
             << "title=" << QString::fromStdString(msg.title)
             << "text=" << QString::fromStdString(msg.text).left(30);

        // vote update — role = "upvotes:downvotes"
        if (text == "vote_update" && questions_.contains(id)) {
            QString role = QString::fromStdString(msg.role);
            QStringList parts = role.split(":");
            if (parts.size() == 2) {
                questions_[id].upvotes   = parts[0].toInt();
                questions_[id].downvotes = parts[1].toInt();
                if (cards_.contains(id))
                    cards_[id]->updateVotes(questions_[id].upvotes,
                                            questions_[id].downvotes);
            }
            return;
        }

        if (id.isEmpty() || title.isEmpty()) return;
        if (questions_.contains(id)) return;

        questions_[id] = {id, title, text, author, timestamp, 0, 0, 0};
        addQuestionCard(id, title, text, author, timestamp, 0, 0, 0);
        return;
    }

    if (msg.type == MessageType::QA_ANSWER)
    {
        QString qId    = QString::fromStdString(msg.parentId);
        QString aId    = QString::fromStdString(msg.filename);
        QString author = QString::fromStdString(msg.sender.username);
        QString text   = QString::fromStdString(msg.text);
        bool    isFaq  = (msg.role == "FAQ");

        // vote update
        if (text == "vote_update") {
            QString role = QString::fromStdString(msg.role);
            QStringList parts = role.split(":");
            if (parts.size() == 2)
                detailPanel_->updateAnswerVotes(aId, parts[0].toInt(), parts[1].toInt());
            return;
        }

        if (qId.isEmpty() || aId.isEmpty() || text.isEmpty()) return;

        // add answer to detail panel if viewing this question
        if (detailPanel_->currentQuestionId() == qId)
            detailPanel_->addAnswer(qId, aId, author, text, 0, 0, isFaq);

        // if FAQ add to faq panel
        if (isFaq && questions_.contains(qId))
            faqPanel_->addFaqAnswer(questions_[qId].title, text, author);

        // only increment count for new answers
        if (questions_.contains(qId) && !knownAnswerIds_.contains(aId)) {
            knownAnswerIds_.insert(aId);
            questions_[qId].answerCount++;
            if (cards_.contains(qId))
                cards_[qId]->updateAnswerCount(questions_[qId].answerCount);
        }
        return;
    }
}

void QuestionDetailPanel::updateAnswerVotes(const QString& answerId, int up, int down)
{
    if (answerWidgets_.contains(answerId))
        answerWidgets_[answerId]->updateVotes(up, down);
}