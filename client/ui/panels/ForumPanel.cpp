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
#include <QGraphicsDropShadowEffect>

static const char* ACCENT   = "#6366f1";
static const char* ACCENT2  = "#818cf8";
static const char* TEXT_PRI = "#f1f5f9";
static const char* TEXT_SEC = "#94a3b8";
static const char* BG_DEEP  = "#0a0f1e";
static const char* BG_PANEL = "#0f172a";
static const char* BG_CARD  = "#1e293b";
static const char* BG_INPUT = "#334155";
static const char* SUCCESS  = "#22c55e";
static const char* DANGER   = "#ef4444";
static const char* GOLD     = "#f59e0b";

static QString inputStyle() {
    return QString("QLineEdit{background:%1;color:%2;border:1px solid #1e293b;"
        "border-radius:8px;padding:8px 12px;font-size:13px;}"
        "QLineEdit:focus{border:1px solid %3;}").arg(BG_INPUT,TEXT_PRI,ACCENT2);
}
static QString textAreaStyle() {
    return QString("QTextEdit{background:%1;color:%2;border:1px solid #1e293b;"
        "border-radius:8px;padding:8px;font-size:13px;}"
        "QTextEdit:focus{border:1px solid %3;}").arg(BG_INPUT,TEXT_PRI,ACCENT2);
}
static QString btnStyle(const char* bg, const char* hover) {
    return QString("QPushButton{background:%1;color:white;border:none;"
        "border-radius:6px;padding:6px 14px;font-size:12px;font-weight:bold;}"
        "QPushButton:hover{background:%2;}").arg(bg,hover);
}
static QString voteBtn(const char* col) {
    return QString("QPushButton{background:transparent;color:%1;border:1px solid %1;"
        "border-radius:4px;padding:2px 8px;font-size:12px;font-weight:bold;}"
        "QPushButton:hover{background:%1;color:white;}").arg(col);
}
static QString msgBoxStyle() {
    return QString("QMessageBox{background:%1;color:%2;}"
        "QMessageBox QLabel{color:%2;}"
        "QMessageBox QPushButton{background:%3;color:white;border:none;"
        "border-radius:6px;padding:6px 18px;}"
        "QMessageBox QPushButton:hover{background:%4;}")
        .arg(BG_PANEL,TEXT_PRI,ACCENT,ACCENT2);
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
    setStyleSheet(QString("QuestionCard{background:%1;border-radius:10px;"
        "border:1px solid #1e293b;}QuestionCard:hover{background:#1a2540;}").arg(BG_CARD));
    setCursor(Qt::PointingHandCursor);

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    // ── vote sidebar ──────────────────────────────────────────────────────
    auto* voteSide = new QWidget;
    voteSide->setFixedWidth(64);
    voteSide->setAttribute(Qt::WA_StyledBackground, true);
    voteSide->setStyleSheet(QString(
        "background:%1;border-radius:10px 0 0 10px;").arg(BG_PANEL));
    auto* voteLayout = new QVBoxLayout(voteSide);
    voteLayout->setContentsMargins(0,12,0,12);
    voteLayout->setSpacing(4);
    voteLayout->setAlignment(Qt::AlignCenter);

    auto* btnUp = new QPushButton("▲");
    btnUp->setFixedSize(32,28);
    btnUp->setStyleSheet(voteBtn(SUCCESS));

    votesLabel_ = new QLabel(QString::number(upvotes - downvotes));
    votesLabel_->setAlignment(Qt::AlignCenter);
    votesLabel_->setStyleSheet(QString("color:%1;font-size:14px;font-weight:bold;"
        "background:transparent;").arg(TEXT_PRI));

    auto* btnDown = new QPushButton("▼");
    btnDown->setFixedSize(32,28);
    btnDown->setStyleSheet(voteBtn(DANGER));

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

    auto* titleLbl = new QLabel(title);
    titleLbl->setStyleSheet(QString("color:%1;font-size:15px;font-weight:bold;"
        "background:transparent;").arg(TEXT_PRI));
    titleLbl->setWordWrap(true);

    auto* bodyLbl = new QLabel(text.left(120) + (text.length()>120?"...":""));
    bodyLbl->setStyleSheet(QString("color:%1;font-size:13px;background:transparent;").arg(TEXT_SEC));
    bodyLbl->setWordWrap(true);

    auto* metaRow = new QHBoxLayout;
    auto* authorLbl = new QLabel("👤 " + author);
    authorLbl->setStyleSheet(QString("color:%1;font-size:11px;background:transparent;").arg(TEXT_SEC));
    auto* timeLbl = new QLabel(timestamp.left(10));
    timeLbl->setStyleSheet(QString("color:%1;font-size:11px;background:transparent;").arg(TEXT_SEC));
    answerCountLabel_ = new QLabel("💬 " + QString::number(answerCount) + " answers");
    //?
    answerCountLabel_->setStyleSheet(QString("color:%1;font-size:11px;background:transparent;").arg(ACCENT2));

    metaRow->addWidget(authorLbl);
    metaRow->addSpacing(12);
    metaRow->addWidget(timeLbl);
    metaRow->addStretch();
    metaRow->addWidget(answerCountLabel_);

    cLayout->addWidget(titleLbl);
    cLayout->addWidget(bodyLbl);
    cLayout->addLayout(metaRow);

    root->addWidget(voteSide);
    root->addWidget(content, 1);

    connect(btnUp,   &QPushButton::clicked, this, [this](){ emit upvoteClicked(id_); });
    connect(btnDown, &QPushButton::clicked, this, [this](){ emit downvoteClicked(id_); });
}

void QuestionCard::mousePressEvent(QMouseEvent*) { emit clicked(id_); }
void QuestionCard::updateVotes(int up, int down) {
    votesLabel_->setText(QString::number(up - down));
}
void QuestionCard::updateAnswerCount(int count) {
    answerCountLabel_->setText("💬 " + QString::number(count) + " answers");
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
    setStyleSheet(QString("AnswerWidget{background:%1;border-radius:8px;"
        "border:1px solid %2;}").arg(BG_PANEL, isFaq ? GOLD : "#1e293b"));

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    // vote column
    auto* voteSide = new QWidget;
    voteSide->setFixedWidth(52);
    voteSide->setAttribute(Qt::WA_StyledBackground, true);
    voteSide->setStyleSheet(QString("background:%1;border-radius:8px 0 0 8px;").arg(BG_CARD));
    auto* vLayout = new QVBoxLayout(voteSide);
    vLayout->setContentsMargins(0,10,0,10);
    vLayout->setSpacing(2);
    vLayout->setAlignment(Qt::AlignCenter);

    auto* btnUp   = new QPushButton("▲");
    btnUp->setFixedSize(28,24);
    btnUp->setStyleSheet(voteBtn(SUCCESS));

    votesLabel_ = new QLabel(QString::number(upvotes - downvotes));
    votesLabel_->setAlignment(Qt::AlignCenter);
    votesLabel_->setStyleSheet(QString("color:%1;font-size:13px;font-weight:bold;"
        "background:transparent;").arg(TEXT_PRI));

    auto* btnDown = new QPushButton("▼");
    btnDown->setFixedSize(28,24);
    btnDown->setStyleSheet(voteBtn(DANGER));

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
        auto* faqBadge = new QLabel("⭐ FAQ Answer");
        faqBadge->setStyleSheet(QString("color:%1;font-size:11px;font-weight:bold;"
            "background:transparent;").arg(GOLD));
        cLayout->addWidget(faqBadge);
    }

    auto* authorLbl = new QLabel("👤 " + author);
    authorLbl->setStyleSheet(QString("color:%1;font-size:11px;font-weight:bold;"
        "background:transparent;").arg(ACCENT2));

    auto* textLbl = new QLabel(text);
    textLbl->setWordWrap(true);
    textLbl->setStyleSheet(QString("color:%1;font-size:13px;background:transparent;").arg(TEXT_PRI));

    cLayout->addWidget(authorLbl);
    cLayout->addWidget(textLbl);

    root->addWidget(voteSide);
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
    setStyleSheet(QString("background:%1;").arg(BG_DEEP));

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(40,24,40,24);
    outer->setSpacing(0);

    auto* btnBack = new QPushButton("← Back to Forum");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(QString("color:%1;font-size:13px;background:transparent;").arg(TEXT_SEC));
    btnBack->setFixedWidth(160);

    auto* card = new QWidget;
    card->setMaximumWidth(680);
    card->setAttribute(Qt::WA_StyledBackground, true);
    card->setStyleSheet(QString("background:%1;border-radius:12px;").arg(BG_PANEL));
    auto* cLayout = new QVBoxLayout(card);
    cLayout->setContentsMargins(32,28,32,28);
    cLayout->setSpacing(14);

    auto* title = new QLabel("Ask a Question");
    title->setStyleSheet(QString("color:%1;font-size:20px;font-weight:bold;").arg(TEXT_PRI));

    auto lbl = [](const QString& t){
        auto* l = new QLabel(t);
        l->setStyleSheet("color:#64748b;font-size:11px;font-weight:bold;letter-spacing:1px;");
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
    auto* btnCancel = new QPushButton("Cancel");
    auto* btnPost   = new QPushButton("Post Question");
    btnCancel->setFixedHeight(42);
    btnPost->setFixedHeight(42);
    btnCancel->setStyleSheet(btnStyle("#374151","#4b5563"));
    btnPost->setStyleSheet(btnStyle(ACCENT,ACCENT2));
    btnRow->addWidget(btnCancel);
    btnRow->addWidget(btnPost);

    cLayout->addWidget(title);
    cLayout->addSpacing(8);
    cLayout->addWidget(lbl("TITLE *"));
    cLayout->addWidget(title_);
    cLayout->addWidget(lbl("DETAILS"));
    cLayout->addWidget(body_);
    cLayout->addSpacing(6);
    cLayout->addLayout(btnRow);

    outer->addWidget(btnBack, 0, Qt::AlignLeft);
    outer->addSpacing(12);
    outer->addWidget(card);
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
    setStyleSheet(QString("background:%1;").arg(BG_DEEP));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    // scroll area wraps everything
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(QString(
        "QScrollArea{border:none;background:%1;}"
        "QScrollBar:vertical{background:%2;width:5px;border-radius:3px;}"
        "QScrollBar::handle:vertical{background:#334155;border-radius:3px;}")
        .arg(BG_DEEP, BG_PANEL));

    auto* container = new QWidget;
    container->setAttribute(Qt::WA_StyledBackground, true);
    container->setStyleSheet(QString("background:%1;").arg(BG_DEEP));
    auto* cLayout = new QVBoxLayout(container);
    cLayout->setContentsMargins(32,20,32,20);
    cLayout->setSpacing(16);

    // back button
    auto* btnBack = new QPushButton("← Back to Forum");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(QString("color:%1;font-size:13px;background:transparent;").arg(TEXT_SEC));
    btnBack->setFixedWidth(160);

    // question card
    auto* qCard = new QWidget;
    qCard->setAttribute(Qt::WA_StyledBackground, true);
    qCard->setStyleSheet(QString("background:%1;border-radius:10px;border:1px solid #1e293b;").arg(BG_PANEL));
    auto* qLayout = new QVBoxLayout(qCard);
    qLayout->setContentsMargins(24,20,24,20);
    qLayout->setSpacing(10);

    titleLabel_ = new QLabel;
    titleLabel_->setStyleSheet(QString("color:%1;font-size:20px;font-weight:bold;background:transparent;").arg(TEXT_PRI));
    titleLabel_->setWordWrap(true);

    metaLabel_ = new QLabel;
    metaLabel_->setStyleSheet(QString("color:%1;font-size:12px;background:transparent;").arg(TEXT_SEC));

    bodyLabel_ = new QLabel;
    bodyLabel_->setWordWrap(true);
    bodyLabel_->setStyleSheet(QString("color:%1;font-size:14px;background:transparent;").arg(TEXT_PRI));

    // vote row for question
    auto* voteRow = new QHBoxLayout;
    auto* btnQUp   = new QPushButton("▲ Upvote");
    auto* btnQDown = new QPushButton("▼ Downvote");
    votesLabel_ = new QLabel;
    votesLabel_->setStyleSheet(QString("color:%1;font-size:13px;font-weight:bold;background:transparent;").arg(TEXT_PRI));
    btnQUp->setStyleSheet(btnStyle(SUCCESS,"#16a34a"));
    btnQDown->setStyleSheet(btnStyle(DANGER,"#dc2626"));
    btnQUp->setFixedHeight(32);
    btnQDown->setFixedHeight(32);
    voteRow->addWidget(btnQUp);
    voteRow->addWidget(votesLabel_);
    voteRow->addWidget(btnQDown);
    voteRow->addStretch();

    qLayout->addWidget(titleLabel_);
    qLayout->addWidget(metaLabel_);
    qLayout->addWidget(bodyLabel_);
    qLayout->addLayout(voteRow);

    // answers section
    auto* answersHeader = new QLabel("Answers");
    answersHeader->setStyleSheet(QString("color:%1;font-size:16px;font-weight:bold;background:transparent;").arg(TEXT_PRI));

    auto* answersContainer = new QWidget;
    answersContainer->setAttribute(Qt::WA_StyledBackground, false);
    answersLayout_ = new QVBoxLayout(answersContainer);
    answersLayout_->setContentsMargins(0,0,0,0);
    answersLayout_->setSpacing(8);

    // answer input
    auto* answerHeader = new QLabel("Your Answer");
    answerHeader->setStyleSheet(QString("color:%1;font-size:14px;font-weight:bold;background:transparent;").arg(TEXT_PRI));

    answerInput_ = new QTextEdit;
    answerInput_->setPlaceholderText("Write your answer...");
    answerInput_->setFixedHeight(100);
    answerInput_->setStyleSheet(textAreaStyle());

    auto* btnSubmitAnswer = new QPushButton("Post Answer");
    btnSubmitAnswer->setFixedHeight(38);
    btnSubmitAnswer->setMaximumWidth(160);
    btnSubmitAnswer->setStyleSheet(btnStyle(ACCENT,ACCENT2));

    cLayout->addWidget(btnBack, 0, Qt::AlignLeft);
    cLayout->addWidget(qCard);
    cLayout->addWidget(answersHeader);
    cLayout->addWidget(answersContainer);
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
    titleLabel_->setText(title);
    metaLabel_->setText("Asked by  " + author + "  ·  " + timestamp.left(10));
    bodyLabel_->setText(text);
    votesLabel_->setText(QString("  %1 votes  ").arg(up - down));
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
    setStyleSheet(QString("background:%1;").arg(BG_DEEP));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(32,20,32,20);
    root->setSpacing(14);

    auto* btnBack = new QPushButton("← Back to Forum");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(QString("color:%1;font-size:13px;background:transparent;").arg(TEXT_SEC));
    btnBack->setFixedWidth(160);

    auto* title = new QLabel("⭐  Frequently Asked Questions");
    title->setStyleSheet(QString("color:%1;font-size:20px;font-weight:bold;background:transparent;").arg(TEXT_PRI));

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(QString(
        "QScrollArea{border:none;background:%1;}"
        "QScrollBar:vertical{background:%2;width:5px;border-radius:3px;}"
        "QScrollBar::handle:vertical{background:#334155;border-radius:3px;}")
        .arg(BG_DEEP, BG_PANEL));

    auto* container = new QWidget;
    container->setAttribute(Qt::WA_StyledBackground, true);
    container->setStyleSheet(QString("background:%1;").arg(BG_DEEP));
    layout_ = new QVBoxLayout(container);
    layout_->setContentsMargins(0,0,0,0);
    layout_->setSpacing(10);
    layout_->setAlignment(Qt::AlignTop);
    scroll->setWidget(container);

    root->addWidget(btnBack, 0, Qt::AlignLeft);
    root->addWidget(title);
    root->addWidget(scroll, 1);

    connect(btnBack, &QPushButton::clicked, this, &FaqPanel::backClicked);
}

void FaqPanel::addFaqAnswer(const QString& questionTitle,
                             const QString& answerText, const QString& author)
{
    auto* card = new QWidget;
    card->setAttribute(Qt::WA_StyledBackground, true);
    card->setStyleSheet(QString("background:%1;border-radius:8px;"
        "border:2px solid %2;").arg(BG_PANEL, GOLD));
    auto* l = new QVBoxLayout(card);
    l->setContentsMargins(20,16,20,16);
    l->setSpacing(8);

    auto* qLbl = new QLabel("Q: " + questionTitle);
    qLbl->setStyleSheet(QString("color:%1;font-size:13px;font-weight:bold;"
        "background:transparent;").arg(GOLD));
    qLbl->setWordWrap(true);

    auto* aLbl = new QLabel("A: " + answerText);
    aLbl->setWordWrap(true);
    aLbl->setStyleSheet(QString("color:%1;font-size:13px;background:transparent;").arg(TEXT_PRI));

    auto* authorLbl = new QLabel("— " + author);
    authorLbl->setStyleSheet(QString("color:%1;font-size:11px;background:transparent;").arg(TEXT_SEC));

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
    setStyleSheet(QString("background:%1;").arg(BG_DEEP));

    stack_ = new QStackedWidget(this);
    stack_->setStyleSheet(QString("background:%1;").arg(BG_DEEP));

    // ── browse page ───────────────────────────────────────────────────────
    auto* browsePage = new QWidget;
    browsePage->setAttribute(Qt::WA_StyledBackground, true);
    browsePage->setStyleSheet(QString("background:%1;").arg(BG_DEEP));
    auto* browseLayout = new QVBoxLayout(browsePage);
    browseLayout->setContentsMargins(24,20,24,20);
    browseLayout->setSpacing(14);

    // top bar
    auto* topBar = new QHBoxLayout;
    auto* pageTitle = new QLabel("Forum");
    pageTitle->setStyleSheet(QString("color:%1;font-size:22px;font-weight:bold;background:transparent;").arg(TEXT_PRI));

    searchBar_ = new QLineEdit;
    searchBar_->setPlaceholderText("🔍  Search questions...");
    searchBar_->setFixedHeight(38);
    searchBar_->setMaximumWidth(260);
    searchBar_->setStyleSheet(inputStyle());

    auto* btnFaq = new QPushButton("⭐  FAQ");
    btnFaq->setFixedHeight(38);
    btnFaq->setStyleSheet(btnStyle(GOLD,"#d97706"));

    auto* btnAsk = new QPushButton("+ Ask Question");
    btnAsk->setFixedHeight(38);
    btnAsk->setStyleSheet(btnStyle(ACCENT,ACCENT2));

    topBar->addWidget(pageTitle);
    topBar->addStretch();
    topBar->addWidget(searchBar_);
    topBar->addSpacing(8);
    topBar->addWidget(btnFaq);
    topBar->addSpacing(8);
    topBar->addWidget(btnAsk);

    // scroll area for cards
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(QString(
        "QScrollArea{border:none;background:%1;}"
        "QScrollBar:vertical{background:%2;width:5px;border-radius:3px;}"
        "QScrollBar::handle:vertical{background:#334155;border-radius:3px;}")
        .arg(BG_DEEP, BG_PANEL));

    cardsWidget_ = new QWidget;
    cardsWidget_->setAttribute(Qt::WA_StyledBackground, true);
    cardsWidget_->setStyleSheet(QString("background:%1;").arg(BG_DEEP));
    cardsLayout_ = new QVBoxLayout(cardsWidget_);
    cardsLayout_->setContentsMargins(0,4,0,4);
    cardsLayout_->setSpacing(8);
    cardsLayout_->setAlignment(Qt::AlignTop);

    emptyLabel_ = new QLabel("No questions yet. Be the first to ask!");
    emptyLabel_->setAlignment(Qt::AlignCenter);
    emptyLabel_->setStyleSheet(QString("color:%1;font-size:15px;padding:60px;background:transparent;").arg(TEXT_SEC));
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

        // vote update for answer
        if (text == "vote_update") return; // handled locally for now

        // vote update for answer
        if (text == "vote_update") {
            // update answer vote display in detail panel — handled by AnswerWidget
            return;
        }

        if (qId.isEmpty() || aId.isEmpty() || text.isEmpty()) return;

        // if we're currently viewing this question, add the answer
        if (detailPanel_->currentQuestionId() == qId)
            detailPanel_->addAnswer(qId, aId, author, text, 0, 0, isFaq);

        // if FAQ, add to faq panel
        if (isFaq && questions_.contains(qId))
            faqPanel_->addFaqAnswer(questions_[qId].title, text, author);

        // increment answer count and update card
        if (questions_.contains(qId)) {
            questions_[qId].answerCount++;
            if (cards_.contains(qId))
                cards_[qId]->updateAnswerCount(questions_[qId].answerCount);
        }
        return;
    }
}