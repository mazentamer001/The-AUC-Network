#include "OpportunitiesPanel.h"
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QScrollArea>
#include <QMouseEvent>
#include <QMessageBox>
#include "ui/theme/Theme.h"

// ─────────────────────────────────────────────────────────────────────────────
//  OpportunityCard
// ─────────────────────────────────────────────────────────────────────────────
OpportunityCard::OpportunityCard(const QString& id, const QString& title,
                                  const QString& organization, const QString& oppType,
                                  const QString& deadline, const QString& description,
                                  QWidget* parent)
    : QWidget(parent), id_(id)
{
    setFixedSize(220, 190);
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString(
        "OpportunityCard { background: %1; border: 1px solid %2; border-radius: 12px; }"
        "OpportunityCard:hover { border: 1px solid %3; }"
    ).arg(Theme::SURFACE, Theme::BORDER, Theme::ACCENT));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(6);

    auto* typeLbl = new QLabel(oppType);
    typeLbl->setStyleSheet(QString(
        "border: none; background: %1; color: white; font-size: 10px; "
        "font-weight: 600; padding: 3px 8px; border-radius: 6px;"
    ).arg(Theme::ACCENT));
    typeLbl->setFixedWidth(90);
    typeLbl->setAlignment(Qt::AlignCenter);

    auto* titleLbl = new QLabel(title);
    titleLbl->setStyleSheet(QString("border: none; background: transparent; color: %1; font-weight: 500; font-size: 13px;").arg(Theme::TEXT_PRIMARY));
    titleLbl->setWordWrap(true);
    titleLbl->setMaximumHeight(36);

    auto* orgLbl = new QLabel(organization);
    orgLbl->setStyleSheet(QString("border: none; background: transparent; color: %1; font-size: 13px; font-weight: 600;").arg(Theme::ACCENT2));
    orgLbl->setWordWrap(true);

    auto* deadlineLbl = new QLabel(deadline.isEmpty() ? "No deadline" : "Deadline: " + deadline);
    deadlineLbl->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));

    layout->addWidget(typeLbl);
    layout->addWidget(titleLbl);
    layout->addWidget(orgLbl);
    layout->addStretch();
    layout->addWidget(deadlineLbl);
}

void OpportunityCard::mousePressEvent(QMouseEvent*) { emit clicked(id_); }

// ─────────────────────────────────────────────────────────────────────────────
//  PostOpportunityPanel
// ─────────────────────────────────────────────────────────────────────────────
PostOpportunityPanel::PostOpportunityPanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("PostOpportunityPanel { %1 }").arg(Theme::pageBackground()));
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(50, 40, 50, 40);
    outer->setSpacing(0);
    outer->setAlignment(Qt::AlignTop);

    auto* btnBack = new QPushButton("Back to opportunities");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(QString(
        "QPushButton { background: transparent; color: %1; border: none; font-size: 12px; }"
        "QPushButton:hover { color: %2; }"
    ).arg(Theme::TEXT_SECONDARY, Theme::ACCENT));
    outer->addWidget(btnBack, 0, Qt::AlignLeft);
    outer->addSpacing(16);

    auto* titleLbl = new QLabel("Post an opportunity");
    titleLbl->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::heading()));
    outer->addWidget(titleLbl);
    outer->addSpacing(24);

    auto* card = new QWidget;
    card->setObjectName("postOppCard");
    card->setMaximumWidth(560);
    card->setStyleSheet(QString("#postOppCard { %1 }").arg(Theme::card()));
    auto* form = new QVBoxLayout(card);
    form->setContentsMargins(32, 32, 32, 32);
    form->setSpacing(6);

    auto addLabel = [&](const QString& t){
        auto* l = new QLabel(t);
        l->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));
        form->addWidget(l);
    };

    addLabel("Title");
    title_ = new QLineEdit; title_->setPlaceholderText("e.g. Marketing Intern");
    title_->setStyleSheet(Theme::textInput()); title_->setFixedHeight(38);
    form->addWidget(title_);
    form->addSpacing(10);

    addLabel("Organization");
    organization_ = new QLineEdit; organization_->setPlaceholderText("e.g. AUC Robotics Club");
    organization_->setStyleSheet(Theme::textInput()); organization_->setFixedHeight(38);
    form->addWidget(organization_);
    form->addSpacing(10);

    addLabel("Type");
    type_ = new QComboBox;
    type_->addItems({"Club", "Competition", "Job"});
    type_->setStyleSheet(Theme::textInput()); type_->setFixedHeight(38);
    form->addWidget(type_);
    form->addSpacing(10);

    addLabel("Deadline");
    deadline_ = new QLineEdit; deadline_->setPlaceholderText("e.g. 2026-08-01");
    deadline_->setStyleSheet(Theme::textInput()); deadline_->setFixedHeight(38);
    form->addWidget(deadline_);
    form->addSpacing(10);

    addLabel("Contact info");
    contactInfo_ = new QLineEdit; contactInfo_->setPlaceholderText("email or link to apply");
    contactInfo_->setStyleSheet(Theme::textInput()); contactInfo_->setFixedHeight(38);
    form->addWidget(contactInfo_);
    form->addSpacing(10);

    addLabel("Description");
    description_ = new QTextEdit;
    description_->setPlaceholderText("Describe the opportunity...");
    description_->setFixedHeight(100);
    description_->setStyleSheet(Theme::textArea());
    form->addWidget(description_);
    form->addSpacing(16);

    auto* btnSubmit = new QPushButton("Post opportunity");
    btnSubmit->setFixedHeight(42);
    btnSubmit->setStyleSheet(Theme::primaryButton());
    form->addWidget(btnSubmit);

    outer->addWidget(card);

    connect(btnSubmit, &QPushButton::clicked, this, &PostOpportunityPanel::onSubmit);
    connect(btnBack,   &QPushButton::clicked, this, &PostOpportunityPanel::cancelled);
}

void PostOpportunityPanel::onSubmit()
{
    if (title_->text().trimmed().isEmpty() || organization_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Missing fields", "Title and organization are required.");
        return;
    }

    QString typeStr = type_->currentText().toUpper(); // "CLUB", "COMPETITION", "JOB"

    Message msg;
    msg.type            = MessageType::OPP_POST;
    msg.title           = title_->text().trimmed().toStdString();
    msg.organization    = organization_->text().trimmed().toStdString();
    msg.oppType         = typeStr.toStdString();
    msg.deadline        = deadline_->text().trimmed().toStdString();
    msg.contactInfo     = contactInfo_->text().trimmed().toStdString();
    msg.text            = description_->toPlainText().trimmed().toStdString();
    msg.sender.username = displayName_.toStdString();
    emit submitted(msg);

    title_->clear(); organization_->clear(); deadline_->clear();
    contactInfo_->clear(); description_->clear();
    type_->setCurrentIndex(0);
}

// ─────────────────────────────────────────────────────────────────────────────
//  OpportunityDetailPanel
// ─────────────────────────────────────────────────────────────────────────────
OpportunityDetailPanel::OpportunityDetailPanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("OpportunityDetailPanel { %1 }").arg(Theme::pageBackground()));
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(50, 40, 50, 40);
    outer->setSpacing(0);
    outer->setAlignment(Qt::AlignTop);

    auto* btnBack = new QPushButton("Back to opportunities");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(QString(
        "QPushButton { background: transparent; color: %1; border: none; font-size: 12px; }"
        "QPushButton:hover { color: %2; }"
    ).arg(Theme::TEXT_SECONDARY, Theme::ACCENT));
    outer->addWidget(btnBack, 0, Qt::AlignLeft);
    outer->addSpacing(24);

    auto* card = new QWidget;
    card->setObjectName("oppDetailCard");
    card->setMaximumWidth(640);
    card->setStyleSheet(QString("#oppDetailCard { %1 }").arg(Theme::card()));
    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(32, 32, 32, 32);
    layout->setSpacing(10);

    title_ = new QLabel;
    title_->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::heading()));
    title_->setWordWrap(true);

    organization_ = new QLabel;
    organization_->setStyleSheet(QString("border: none; background: transparent; color: %1; font-size: 16px; font-weight: 600;").arg(Theme::ACCENT2));

    deadline_ = new QLabel;
    deadline_->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));

    description_ = new QTextEdit;
    description_->setReadOnly(true);
    description_->setFixedHeight(120);
    description_->setStyleSheet(QString(
        "QTextEdit { background: transparent; color: %1; border: none; font-size: 13px; }"
    ).arg(Theme::TEXT_PRIMARY));

    auto* btnApply = new QPushButton("Apply / Contact");
    btnApply->setFixedHeight(44);
    btnApply->setStyleSheet(Theme::primaryButton());

    layout->addWidget(title_);
    layout->addWidget(organization_);
    layout->addWidget(deadline_);
    layout->addSpacing(8);
    layout->addWidget(description_);
    layout->addWidget(btnApply);

    outer->addWidget(card);

    connect(btnBack, &QPushButton::clicked, this, &OpportunityDetailPanel::backClicked);
    connect(btnApply, &QPushButton::clicked, this, [this](){
        Message msg;
        msg.type     = MessageType::OPP_APPLY;
        msg.parentId = currentId_.toStdString();
        msg.title    = currentTitle_.toStdString();
        msg.text     = "Hi, I'm interested in this opportunity: " + currentTitle_.toStdString();
        msg.sender.username = displayName_.toStdString();
        emit applyClicked(msg);
    });
}

void OpportunityDetailPanel::show(const QString& id, const QString& title,
                                   const QString& organization, const QString& oppType,
                                   const QString& deadline, const QString& posterId,
                                   const QString& description)
{
    currentId_       = id;
    currentTitle_    = title;
    currentPosterId_ = posterId;
    title_->setText(title);
    organization_->setText(organization + " · " + oppType);
    deadline_->setText(deadline.isEmpty() ? "No deadline listed" : "Deadline: " + deadline);
    description_->setText(description.isEmpty() ? "No description provided." : description);
    QWidget::show();
}

// ─────────────────────────────────────────────────────────────────────────────
//  OpportunitiesPanel
// ─────────────────────────────────────────────────────────────────────────────
OpportunitiesPanel::OpportunitiesPanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("OpportunitiesPanel { %1 }").arg(Theme::pageBackground()));
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    stack_ = new QStackedWidget;

    auto* browsePage = new QWidget;
    browsePage->setStyleSheet("background: transparent;");
    auto* browseLayout = new QVBoxLayout(browsePage);
    browseLayout->setContentsMargins(40, 32, 40, 32);
    browseLayout->setSpacing(16);

    auto* pageTitle = new QLabel("Opportunities");
    pageTitle->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::heading()));
    browseLayout->addWidget(pageTitle);

    auto* topBar = new QHBoxLayout;
    searchBar_ = new QLineEdit;
    searchBar_->setPlaceholderText("Search opportunities...");
    searchBar_->setFixedHeight(38);
    searchBar_->setMaximumWidth(300);
    searchBar_->setStyleSheet(Theme::textInput());

    auto* btnPost = new QPushButton("Post opportunity");
    btnPost->setFixedHeight(38);
    btnPost->setStyleSheet(Theme::primaryButton());

    topBar->addWidget(searchBar_);
    topBar->addStretch();
    topBar->addWidget(btnPost);
    browseLayout->addLayout(topBar);

    scrollArea_ = new QScrollArea;
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setStyleSheet(QString(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { background: transparent; width: 6px; }"
        "QScrollBar::handle:vertical { background: %1; border-radius: 3px; }"
    ).arg(Theme::BORDER));

    gridWidget_ = new QWidget;
    gridWidget_->setStyleSheet("background: transparent;");
    grid_ = new QGridLayout(gridWidget_);
    grid_->setSpacing(16);
    grid_->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    scrollArea_->setWidget(gridWidget_);

    auto* emptyLabel = new QLabel("No opportunities yet. Be the first to post one!");
    emptyLabel->setAlignment(Qt::AlignCenter);
    emptyLabel->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));
    grid_->addWidget(emptyLabel, 0, 0);

    browseLayout->addWidget(scrollArea_, 1);

    postPanel_   = new PostOpportunityPanel;
    detailPanel_ = new OpportunityDetailPanel;

    stack_->addWidget(browsePage);
    stack_->addWidget(postPanel_);
    stack_->addWidget(detailPanel_);
    stack_->setCurrentIndex(0);

    root->addWidget(stack_);

    connect(btnPost,    &QPushButton::clicked,  this, &OpportunitiesPanel::showPostForm);
    connect(searchBar_, &QLineEdit::textChanged, this, &OpportunitiesPanel::onSearch);

    connect(postPanel_, &PostOpportunityPanel::submitted, this, [this](const Message& msg){
        emit sendMessage(msg);
        showBrowse();
    });
    connect(postPanel_, &PostOpportunityPanel::cancelled, this, &OpportunitiesPanel::showBrowse);

    connect(detailPanel_, &OpportunityDetailPanel::backClicked, this, &OpportunitiesPanel::showBrowse);
    connect(detailPanel_, &OpportunityDetailPanel::applyClicked, this, [this](const Message& msg){
        emit sendMessage(msg);
    });

    QTimer::singleShot(500, this, [this](){
        if (!token_.isEmpty()) {
            Message msg;
            msg.type  = MessageType::OPP_SEARCH;
            msg.token = token_.toStdString();
            msg.text  = "";
            emit sendMessage(msg);
        }
    });
}

void OpportunitiesPanel::setCurrentUser(const QString& displayName, const QString& userId)
{
    displayName_ = displayName;
    userId_      = userId;
    postPanel_->setUser(displayName);
    detailPanel_->setUser(displayName);
}

void OpportunitiesPanel::showPostForm() { stack_->setCurrentIndex(1); }
void OpportunitiesPanel::showBrowse()   { stack_->setCurrentIndex(0); }

void OpportunitiesPanel::onCardClicked(const QString& id)
{
    if (!opportunities_.contains(id)) return;
    auto& o = opportunities_[id];
    detailPanel_->show(o.id, o.title, o.organization, o.oppType, o.deadline, o.posterId, o.description);
    stack_->setCurrentIndex(2);
}

void OpportunitiesPanel::onSearch()
{
    QString query = searchBar_->text().trimmed().toLower();
    clearGrid();
    for (auto& o : opportunities_) {
        if (query.isEmpty() || o.title.toLower().contains(query) ||
            o.description.toLower().contains(query) ||
            o.organization.toLower().contains(query)) {
            addCard(o.id, o.title, o.organization, o.oppType, o.deadline, o.posterId, o.description);
        }
    }
}

void OpportunitiesPanel::clearGrid()
{
    while (QLayoutItem* item = grid_->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
}

void OpportunitiesPanel::addCard(const QString& id, const QString& title, const QString& organization,
                                  const QString& oppType, const QString& deadline,
                                  const QString& posterId, const QString& description)
{
    int count = grid_->count();
    int col   = count % 4;
    int row   = count / 4;
    auto* card = new OpportunityCard(id, title, organization, oppType, deadline, description);
    connect(card, &OpportunityCard::clicked, this, &OpportunitiesPanel::onCardClicked);
    grid_->addWidget(card, row, col);
}

void OpportunitiesPanel::receiveMessage(const Message& msg)
{
    if (msg.type == MessageType::OPP_POST)
    {
        QString id     = QString::fromStdString(msg.parentId);
        QString title  = QString::fromStdString(msg.title);
        QString org    = QString::fromStdString(msg.organization);
        QString type   = QString::fromStdString(msg.oppType);
        QString deadl  = QString::fromStdString(msg.deadline);
        QString posterId = QString::fromStdString(msg.sender.userId);
        QString desc   = QString::fromStdString(msg.text);

        if (id.isEmpty() || title.isEmpty()) return;

        opportunities_[id] = {id, title, org, type, deadl, posterId, desc};

        auto existing = gridWidget_->findChildren<OpportunityCard*>();
        for (auto* c : existing)
            if (c->opportunityId() == id) return;

        if (grid_->count() == 1) {
            auto* item = grid_->itemAt(0);
            if (item && qobject_cast<QLabel*>(item->widget())) {
                clearGrid();
            }
        }

        addCard(id, title, org, type, deadl, posterId, desc);
    }

    if (msg.type == MessageType::OPP_APPLY) {
        QMessageBox::information(this, "Application sent", QString::fromStdString(msg.text));
    }

    if (msg.type == MessageType::OPP_DELETE)
    {
        QString id = QString::fromStdString(msg.parentId);
        opportunities_.remove(id);
        auto existing = gridWidget_->findChildren<OpportunityCard*>();
        for (auto* c : existing)
            if (c->opportunityId() == id) { c->deleteLater(); break; }
    }
}
