#include "ForumPanel.h"
#include "ui/theme/Theme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>
#include <QMessageBox>
#include <QDebug>

static const char* FAQ_GOLD = "#B45309";

static QString voteBtnStyle() {
    return QString(
        "QPushButton { background: transparent; color: %1; border: 1px solid %2; border-radius: 6px; padding: 2px 8px; font-size: 12px; font-weight: 600; }"
        "QPushButton:hover { background: %3; }"
    ).arg(Theme::TEXT_PRIMARY, Theme::BORDER, Theme::SURFACE_ALT);
}

static QString backLinkStyle() {
    return QString(
        "QPushButton { background: transparent; color: %1; border: none; font-size: 12px; }"
        "QPushButton:hover { color: %2; }"
    ).arg(Theme::TEXT_SECONDARY, Theme::ACCENT);
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
    setStyleSheet(QString(
        "QuestionCard { background: %1; border: 1px solid %2; border-radius: 12px; }"
        "QuestionCard:hover { border: 1px solid %3; }"
    ).arg(Theme::SURFACE, Theme::BORDER, Theme::ACCENT));
    setCursor(Qt::PointingHandCursor);

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto* voteSide = new QWidget;
    voteSide->setFixedWidth(56);
    voteSide->setStyleSheet(QString("background: %1; border-top-left-radius: 12px; border-bottom-left-radius: 12px;").arg(Theme::SURFACE_ALT));
    auto* voteLayout = new QVBoxLayout(voteSide);
    voteLayout->setContentsMargins(0, 12, 0, 12);
    voteLayout->setSpacing(4);
    voteLayout->setAlignment(Qt::AlignCenter);

    auto* btnUp = new QPushButton("+");
    btnUp->setFixedSize(28, 26);
    btnUp->setStyleSheet(voteBtnStyle());

    votesLabel_ = new QLabel(QString::number(upvotes - downvotes));
    votesLabel_->setAlignment(Qt::AlignCenter);
    votesLabel_->setStyleSheet(QString("border: none; background: transparent; color: %1; font-size: 13px; font-weight: 600;").arg(Theme::TEXT_PRIMARY));

    auto* btnDown = new QPushButton("-");
    btnDown->setFixedSize(28, 26);
    btnDown->setStyleSheet(voteBtnStyle());

    voteLayout->addWidget(btnUp, 0, Qt::AlignCenter);
    voteLayout->addWidget(votesLabel_, 0, Qt::AlignCenter);
    voteLayout->addWidget(btnDown, 0, Qt::AlignCenter);

    auto* content = new QWidget;
    content->setStyleSheet("background: transparent;");
    auto* cLayout = new QVBoxLayout(content);
    cLayout->setContentsMargins(16, 14, 16, 14);
    cLayout->setSpacing(6);

    auto* titleLbl = new QLabel(title);
    titleLbl->setStyleSheet(QString("border: none; background: transparent; color: %1; font-size: 14px; font-weight: 500;").arg(Theme::TEXT_PRIMARY));
    titleLbl->setWordWrap(true);

    auto* bodyLbl = new QLabel(text.left(110) + (text.length() > 110 ? "..." : ""));
    bodyLbl->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));
    bodyLbl->setWordWrap(true);

    auto* metaRow = new QHBoxLayout;
    auto* authorLbl = new QLabel(author);
    authorLbl->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));
    answerCountLabel_ = new QLabel(QString::number(answerCount) + " answers");
    answerCountLabel_->setStyleSheet(QString("border: none; background: transparent; color: %1; font-size: 12px; font-weight: 500;").arg(Theme::ACCENT));

    metaRow->addWidget(authorLbl);
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
void QuestionCard::updateVotes(int up, int down) { votesLabel_->setText(QString::number(up - down)); }
void QuestionCard::updateAnswerCount(int count) { answerCountLabel_->setText(QString::number(count) + " answers"); }

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
    setStyleSheet(QString("AnswerWidget { background: %1; border: %2 solid %3; border-radius: 10px; }")
        .arg(Theme::SURFACE, isFaq ? "2px" : "1px", isFaq ? FAQ_GOLD : Theme::BORDER));

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto* voteSide = new QWidget;
    voteSide->setFixedWidth(48);
    voteSide->setStyleSheet(QString("background: %1; border-top-left-radius: 10px; border-bottom-left-radius: 10px;").arg(Theme::SURFACE_ALT));
    auto* vLayout = new QVBoxLayout(voteSide);
    vLayout->setContentsMargins(0, 10, 0, 10);
    vLayout->setSpacing(2);
    vLayout->setAlignment(Qt::AlignCenter);

    auto* btnUp = new QPushButton("+");
    btnUp->setFixedSize(26, 22);
    btnUp->setStyleSheet(voteBtnStyle());

    votesLabel_ = new QLabel(QString::number(upvotes - downvotes));
    votesLabel_->setAlignment(Qt::AlignCenter);
    votesLabel_->setStyleSheet(QString("border: none; background: transparent; color: %1; font-size: 12px; font-weight: 600;").arg(Theme::TEXT_PRIMARY));

    auto* btnDown = new QPushButton("-");
    btnDown->setFixedSize(26, 22);
    btnDown->setStyleSheet(voteBtnStyle());

    vLayout->addWidget(btnUp, 0, Qt::AlignCenter);
    vLayout->addWidget(votesLabel_, 0, Qt::AlignCenter);
    vLayout->addWidget(btnDown, 0, Qt::AlignCenter);

    auto* content = new QWidget;
    content->setStyleSheet("background: transparent;");
    auto* cLayout = new QVBoxLayout(content);
    cLayout->setContentsMargins(14, 12, 14, 12);
    cLayout->setSpacing(6);

    if (isFaq) {
        auto* faqBadge = new QLabel("FAQ answer");
        faqBadge->setStyleSheet(QString("border: none; background: transparent; color: %1; font-size: 11px; font-weight: 600;").arg(FAQ_GOLD));
        cLayout->addWidget(faqBadge);
    }

    auto* authorLbl = new QLabel(author);
    authorLbl->setStyleSheet(QString("border: none; background: transparent; color: %1; font-size: 12px; font-weight: 600;").arg(Theme::ACCENT));

    auto* textLbl = new QLabel(text);
    textLbl->setWordWrap(true);
    textLbl->setStyleSheet(QString("border: none; background: transparent; color: %1; font-size: 13px;").arg(Theme::TEXT_PRIMARY));

    cLayout->addWidget(authorLbl);
    cLayout->addWidget(textLbl);

    root->addWidget(voteSide);
    root->addWidget(content, 1);

    connect(btnUp,   &QPushButton::clicked, this, [this](){ emit upvoteClicked(questionId_, answerId_); });
    connect(btnDown, &QPushButton::clicked, this, [this](){ emit downvoteClicked(questionId_, answerId_); });
}

void AnswerWidget::updateVotes(int up, int down) { votesLabel_->setText(QString::number(up - down)); }

// ─────────────────────────────────────────────────────────────────────────────
//  PostQuestionPanel
// ─────────────────────────────────────────────────────────────────────────────
PostQuestionPanel::PostQuestionPanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("PostQuestionPanel { %1 }").arg(Theme::pageBackground()));

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(50, 40, 50, 40);
    outer->setSpacing(0);
    outer->setAlignment(Qt::AlignTop);

    auto* btnBack = new QPushButton("Back to forum");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(backLinkStyle());
    outer->addWidget(btnBack, 0, Qt::AlignLeft);
    outer->addSpacing(16);

    auto* title = new QLabel("Ask a question");
    title->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::heading()));
    outer->addWidget(title);
    outer->addSpacing(24);

    auto* card = new QWidget;
    card->setObjectName("postQCard");
    card->setMaximumWidth(560);
    card->setStyleSheet(QString("#postQCard { %1 }").arg(Theme::card()));
    auto* form = new QVBoxLayout(card);
    form->setContentsMargins(32, 32, 32, 32);
    form->setSpacing(6);

    auto lbl = [](const QString& t){
        auto* l = new QLabel(t);
        l->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));
        return l;
    };

    form->addWidget(lbl("Title"));
    title_ = new QLineEdit;
    title_->setPlaceholderText("What is your question?");
    title_->setFixedHeight(38);
    title_->setStyleSheet(Theme::textInput());
    form->addWidget(title_);
    form->addSpacing(10);

    form->addWidget(lbl("Details"));
    body_ = new QTextEdit;
    body_->setPlaceholderText("Provide more details...");
    body_->setFixedHeight(140);
    body_->setStyleSheet(Theme::textArea());
    form->addWidget(body_);
    form->addSpacing(16);

    auto* btnPost = new QPushButton("Post question");
    btnPost->setFixedHeight(42);
    btnPost->setStyleSheet(Theme::primaryButton());
    form->addWidget(btnPost);

    outer->addWidget(card);

    connect(btnPost, &QPushButton::clicked, this, &PostQuestionPanel::onSubmit);
    connect(btnBack, &QPushButton::clicked, this, &PostQuestionPanel::cancelled);
}

void PostQuestionPanel::onSubmit()
{
    if (title_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Missing title", "Title is required.");
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
    setStyleSheet(QString("QuestionDetailPanel { %1 }").arg(Theme::pageBackground()));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(QString(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { background: transparent; width: 6px; }"
        "QScrollBar::handle:vertical { background: %1; border-radius: 3px; }"
    ).arg(Theme::BORDER));

    auto* container = new QWidget;
    container->setStyleSheet("background: transparent;");
    auto* cLayout = new QVBoxLayout(container);
    cLayout->setContentsMargins(50, 32, 50, 32);
    cLayout->setSpacing(16);

    auto* btnBack = new QPushButton("Back to forum");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(backLinkStyle());

    auto* qCard = new QWidget;
    qCard->setObjectName("qCard");
    qCard->setStyleSheet(QString("#qCard { %1 }").arg(Theme::card()));
    auto* qLayout = new QVBoxLayout(qCard);
    qLayout->setContentsMargins(24, 20, 24, 20);
    qLayout->setSpacing(10);

    titleLabel_ = new QLabel;
    titleLabel_->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::heading()));
    titleLabel_->setWordWrap(true);

    metaLabel_ = new QLabel;
    metaLabel_->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));

    bodyLabel_ = new QLabel;
    bodyLabel_->setWordWrap(true);
    bodyLabel_->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::bodyText()));

    auto* voteRow = new QHBoxLayout;
    auto* btnQUp = new QPushButton("Upvote");
    auto* btnQDown = new QPushButton("Downvote");
    votesLabel_ = new QLabel;
    votesLabel_->setStyleSheet(QString("border: none; background: transparent; color: %1; font-size: 13px; font-weight: 600;").arg(Theme::TEXT_PRIMARY));
    btnQUp->setStyleSheet(voteBtnStyle());
    btnQDown->setStyleSheet(voteBtnStyle());
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

    auto* answersHeader = new QLabel("Answers");
    answersHeader->setStyleSheet(QString("border: none; background: transparent; color: %1; font-size: 15px; font-weight: 500;").arg(Theme::TEXT_PRIMARY));

    auto* answersContainer = new QWidget;
    answersContainer->setStyleSheet("background: transparent;");
    answersLayout_ = new QVBoxLayout(answersContainer);
    answersLayout_->setContentsMargins(0, 0, 0, 0);
    answersLayout_->setSpacing(10);

    auto* answerHeader = new QLabel("Your answer");
    answerHeader->setStyleSheet(QString("border: none; background: transparent; color: %1; font-size: 13px; font-weight: 500;").arg(Theme::TEXT_PRIMARY));

    answerInput_ = new QTextEdit;
    answerInput_->setPlaceholderText("Write your answer...");
    answerInput_->setFixedHeight(100);
    answerInput_->setStyleSheet(Theme::textArea());

    auto* btnSubmitAnswer = new QPushButton("Post answer");
    btnSubmitAnswer->setFixedHeight(38);
    btnSubmitAnswer->setMaximumWidth(180);
    btnSubmitAnswer->setStyleSheet(Theme::primaryButton());

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
    connect(btnQUp,  &QPushButton::clicked, this, [this](){ emit upvoteQuestion(questionId_); });
    connect(btnQDown,&QPushButton::clicked, this, [this](){ emit downvoteQuestion(questionId_); });
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
    metaLabel_->setText("Asked by " + author + "  ·  " + timestamp.left(10));
    bodyLabel_->setText(text);
    votesLabel_->setText(QString("%1 votes").arg(up - down));
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

void QuestionDetailPanel::updateAnswerVotes(const QString& answerId, int up, int down)
{
    if (answerWidgets_.contains(answerId))
        answerWidgets_[answerId]->updateVotes(up, down);
}



// ─────────────────────────────────────────────────────────────────────────────
//  ForumPanel
// ─────────────────────────────────────────────────────────────────────────────
ForumPanel::ForumPanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("ForumPanel { %1 }").arg(Theme::pageBackground()));

    stack_ = new QStackedWidget(this);
    stack_->setStyleSheet("background: transparent;");

    auto* browsePage = new QWidget;
    browsePage->setStyleSheet("background: transparent;");
    auto* browseLayout = new QVBoxLayout(browsePage);
    browseLayout->setContentsMargins(50, 32, 50, 32);
    browseLayout->setSpacing(16);

    auto* pageTitle = new QLabel("Forum");
    pageTitle->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::heading()));
    browseLayout->addWidget(pageTitle);

    auto* topBar = new QHBoxLayout;
    searchBar_ = new QLineEdit;
    searchBar_->setPlaceholderText("Search questions...");
    searchBar_->setFixedHeight(38);
    searchBar_->setMaximumWidth(260);
    searchBar_->setStyleSheet(Theme::textInput());

    auto* btnFaq = new QPushButton("FAQ");
    btnFaq->setFixedHeight(38);
    btnFaq->setStyleSheet(QString(
        "QPushButton { background: %1; color: %2; border: 1px solid %3; border-radius: 8px; padding: 8px 16px; font-size: 13px; font-weight: 500; }"
        "QPushButton:hover { background: %4; }"
    ).arg(Theme::SURFACE, Theme::TEXT_PRIMARY, Theme::BORDER, Theme::SURFACE_ALT));

    auto* btnAsk = new QPushButton("Ask question");
    btnAsk->setFixedHeight(38);
    btnAsk->setStyleSheet(Theme::primaryButton());

    topBar->addWidget(searchBar_);
    topBar->addStretch();
    topBar->addWidget(btnFaq);
    topBar->addSpacing(8);
    topBar->addWidget(btnAsk);

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(QString(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { background: transparent; width: 6px; }"
        "QScrollBar::handle:vertical { background: %1; border-radius: 3px; }"
    ).arg(Theme::BORDER));

    cardsWidget_ = new QWidget;
    cardsWidget_->setStyleSheet("background: transparent;");
    cardsLayout_ = new QVBoxLayout(cardsWidget_);
    cardsLayout_->setContentsMargins(0, 4, 0, 4);
    cardsLayout_->setSpacing(10);
    cardsLayout_->setAlignment(Qt::AlignTop);

    emptyLabel_ = new QLabel("No questions yet. Be the first to ask!");
    emptyLabel_->setAlignment(Qt::AlignCenter);
    emptyLabel_->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));
    cardsLayout_->addWidget(emptyLabel_);
    scroll->setWidget(cardsWidget_);

    browseLayout->addLayout(topBar);
    browseLayout->addWidget(scroll, 1);

    postPanel_   = new PostQuestionPanel;
    detailPanel_ = new QuestionDetailPanel;
    faqPanel_    = new FaqPanel;

    stack_->addWidget(browsePage);
    stack_->addWidget(postPanel_);
    stack_->addWidget(detailPanel_);
    stack_->addWidget(faqPanel_);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(stack_);

    connect(btnAsk,  &QPushButton::clicked, this, &ForumPanel::showPost);
    connect(btnFaq,  &QPushButton::clicked, this, &ForumPanel::showFaq);
    connect(searchBar_, &QLineEdit::textChanged, this, &ForumPanel::onSearch);
    connect(faqPanel_, &FaqPanel::backClicked,     this, &ForumPanel::showBrowse);
    connect(faqPanel_, &FaqPanel::questionClicked, this, &ForumPanel::onQuestionClicked);
    connect(faqPanel_, &FaqPanel::upvoteClicked,   this, [this](const QString& qId){ sendVote(qId, "", true); });
    connect(faqPanel_, &FaqPanel::downvoteClicked, this, [this](const QString& qId){ sendVote(qId, "", false); });


    connect(postPanel_, &PostQuestionPanel::submitted, this, [this](const Message& msg){
        emit sendMessage(msg);
        showBrowse();
    });
    connect(postPanel_, &PostQuestionPanel::cancelled, this, &ForumPanel::showBrowse);

    connect(detailPanel_, &QuestionDetailPanel::backClicked, this, &ForumPanel::showBrowse);
    connect(detailPanel_, &QuestionDetailPanel::answerSubmitted, this, [this](const Message& msg){
        emit sendMessage(msg);
    });
    connect(detailPanel_, &QuestionDetailPanel::upvoteQuestion, this, [this](const QString& qId){
        sendVote(qId, "", true);
    });
    connect(detailPanel_, &QuestionDetailPanel::downvoteQuestion, this, [this](const QString& qId){
        sendVote(qId, "", false);
    });
    connect(detailPanel_, &QuestionDetailPanel::upvoteAnswer, this,
            [this](const QString& qId, const QString& aId){ sendVote(qId, aId, true); });
    connect(detailPanel_, &QuestionDetailPanel::downvoteAnswer, this,
            [this](const QString& qId, const QString& aId){ sendVote(qId, aId, false); });

    connect(faqPanel_, &FaqPanel::backClicked, this, &ForumPanel::showBrowse);
}

void ForumPanel::setCurrentUser(const QString& displayName, const QString& userId, const QString& token)
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
void ForumPanel::showBrowse()
{
    stack_->setCurrentIndex(0);
    clearCards();
    requestQuestions();
}


void ForumPanel::onQuestionClicked(const QString& id)
{
    if (!questions_.contains(id)) return;
    auto& q = questions_[id];
    detailPanel_->load(q.id, q.title, q.text, q.author, q.timestamp, q.upvotes, q.downvotes);
    stack_->setCurrentIndex(2);

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
        bool match = q.isEmpty() || data.title.toLower().contains(q) || data.text.toLower().contains(q);
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
    msg.sender.userId   = userId_.toStdString();
    emit sendMessage(msg);
}

void ForumPanel::addQuestionCard(const QString& id, const QString& title,
                                  const QString& text, const QString& author,
                                  const QString& timestamp, int up, int down, int answers)
{
    if (cards_.contains(id)) return;

    emptyLabel_->setVisible(false);
    auto* card = new QuestionCard(id, title, text, author, timestamp, up, down, answers);
    cards_[id] = card;
    cardsLayout_->addWidget(card);

    connect(card, &QuestionCard::clicked, this, &ForumPanel::onQuestionClicked);
    connect(card, &QuestionCard::upvoteClicked, this, [this](const QString& qId){ sendVote(qId, "", true); });
    connect(card, &QuestionCard::downvoteClicked, this, [this](const QString& qId){ sendVote(qId, "", false); });
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
        QString text       = QString::fromStdString(msg.text);
        QString author    = QString::fromStdString(msg.sender.username);
        QString timestamp = QString::fromStdString(msg.timestamp);

        if (text == "vote_update" && questions_.contains(id)) {
            QString role = QString::fromStdString(msg.role);
            QStringList parts = role.split(":");
            if (parts.size() == 2) {
                questions_[id].upvotes   = parts[0].toInt();
                questions_[id].downvotes = parts[1].toInt();
                if (cards_.contains(id))
                    cards_[id]->updateVotes(questions_[id].upvotes, questions_[id].downvotes);
            }
            return;
        }

        if (id.isEmpty() || title.isEmpty()) return;

        int up = 0, down = 0;
        QString role = QString::fromStdString(msg.role);
        QStringList roleParts = role.split(":");
        if (roleParts.size() == 2) { up = roleParts[0].toInt(); down = roleParts[1].toInt(); }

        if (questions_.contains(id)) {
            // already known — refresh votes in case this fetch has newer data
            questions_[id].upvotes   = up;
            questions_[id].downvotes = down;
            if (cards_.contains(id))
                cards_[id]->updateVotes(up, down);
            return;
        }

        int answerCount = questions_.contains(id) ? questions_[id].answerCount : 0;
        questions_[id] = {id, title, text, author, timestamp, up, down, answerCount};
        addQuestionCard(id, title, text, author, timestamp, up, down, answerCount);
        return;
    }

    if (msg.type == MessageType::QA_ANSWER)
    {
        QString qId    = QString::fromStdString(msg.parentId);
        QString aId    = QString::fromStdString(msg.filename);
        QString author = QString::fromStdString(msg.sender.username);
        QString text   = QString::fromStdString(msg.text);

        if (text == "vote_update") {
            QString role = QString::fromStdString(msg.role);
            QStringList parts = role.split(":");
            if (parts.size() == 2)
                detailPanel_->updateAnswerVotes(aId, parts[0].toInt(), parts[1].toInt());
            return;
        }

        if (qId.isEmpty() || aId.isEmpty() || text.isEmpty()) return;

        QString roleStr = QString::fromStdString(msg.role);
        QStringList roleParts = roleStr.split(":");
        bool isFaq = (!roleParts.isEmpty() && roleParts[0] == "FAQ");
        int up = 0, down = 0;
        if (roleParts.size() == 3) { up = roleParts[1].toInt(); down = roleParts[2].toInt(); }

        if (detailPanel_->currentQuestionId() == qId)
            detailPanel_->addAnswer(qId, aId, author, text, up, down, isFaq);

        if (questions_.contains(qId) && !knownAnswerIds_.contains(aId)) {
            knownAnswerIds_.insert(aId);
            questions_[qId].answerCount++;
            if (cards_.contains(qId))
                cards_[qId]->updateAnswerCount(questions_[qId].answerCount);
        }
        return;
    }

    // ── NEW: handle top-5-upvoted-questions response for the FAQ panel ────────
    if (msg.type == MessageType::QA_FAQ)
    {
        if (!msg.title.empty())   // a real top-question entry, not a generic status message
        {
            QString id        = QString::fromStdString(msg.parentId);
            QString title     = QString::fromStdString(msg.title);
            QString text      = QString::fromStdString(msg.text);
            QString author    = QString::fromStdString(msg.sender.username);
            QString timestamp = QString::fromStdString(msg.timestamp);

            int up = 0, down = 0;
            QString role = QString::fromStdString(msg.role);
            QStringList parts = role.split(":");
            if (parts.size() == 2) { up = parts[0].toInt(); down = parts[1].toInt(); }

            int answerCount = questions_.contains(id) ? questions_[id].answerCount : 0;
            faqPanel_->addFaqQuestion(id, title, text, author, timestamp, up, down, answerCount);
        }
        return;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  FaqPanel
// ─────────────────────────────────────────────────────────────────────────────
FaqPanel::FaqPanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("FaqPanel { %1 }").arg(Theme::pageBackground()));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(50, 32, 50, 32);
    root->setSpacing(16);

    auto* btnBack = new QPushButton("Back to forum");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(backLinkStyle());

    auto* title = new QLabel("Top questions");
    title->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::heading()));

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(QString(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { background: transparent; width: 6px; }"
        "QScrollBar::handle:vertical { background: %1; border-radius: 3px; }"
    ).arg(Theme::BORDER));

    auto* container = new QWidget;
    container->setStyleSheet("background: transparent;");
    layout_ = new QVBoxLayout(container);
    layout_->setContentsMargins(0, 0, 0, 0);
    layout_->setSpacing(10);
    layout_->setAlignment(Qt::AlignTop);

    emptyLabel_ = new QLabel("No FAQ yet.");
    emptyLabel_->setAlignment(Qt::AlignCenter);
    emptyLabel_->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));
    layout_->addWidget(emptyLabel_);

    scroll->setWidget(container);

    root->addWidget(btnBack, 0, Qt::AlignLeft);
    root->addWidget(title);
    root->addWidget(scroll, 1);

    connect(btnBack, &QPushButton::clicked, this, &FaqPanel::backClicked);
}

void FaqPanel::addFaqQuestion(const QString& id, const QString& title, const QString& text,
                               const QString& author, const QString& timestamp,
                               int upvotes, int downvotes, int answerCount)
{
    emptyLabel_->setVisible(false);

    auto* card = new QuestionCard(id, title, text, author, timestamp, upvotes, downvotes, answerCount);
    layout_->addWidget(card);

    connect(card, &QuestionCard::clicked,        this, &FaqPanel::questionClicked);
    connect(card, &QuestionCard::upvoteClicked,   this, &FaqPanel::upvoteClicked);
    connect(card, &QuestionCard::downvoteClicked, this, &FaqPanel::downvoteClicked);
}

void FaqPanel::clear()
{
    while (QLayoutItem* item = layout_->takeAt(0)) {
        if (item->widget() && item->widget() != emptyLabel_) item->widget()->deleteLater();
        delete item;
    }
    layout_->addWidget(emptyLabel_);
    emptyLabel_->setVisible(true);
}

void ForumPanel::showFaq()
{
    faqPanel_->clear();
    stack_->setCurrentIndex(3);

    Message msg;
    msg.type = MessageType::QA_GET_FAQ;
    msg.token = token_.toStdString();
    msg.sender.username = displayName_.toStdString();
    emit sendMessage(msg);
}