#include "OpportunitiesPanel.h"
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QScrollArea>
#include <QMouseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QPixmap>
#include "ui/theme/Theme.h"

// ─────────────────────────────────────────────────────────────────────────────
//  OpportunityCard
// ─────────────────────────────────────────────────────────────────────────────
OpportunityCard::OpportunityCard(const QString& id, const QString& title,
                                  const QString& category, const QString& poster,
                                  const QString& description, const QString& imageData,
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

    auto* badge = new QLabel(category);
    badge->setFixedHeight(22);
    badge->setStyleSheet(QString(
        "background: %1; border-radius: 6px; border: none; color: white; font-size: 11px; font-weight: 600; padding: 2px 8px;"
    ).arg(Theme::ACCENT));
    badge->setAlignment(Qt::AlignCenter);
    badge->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    auto* titleLbl = new QLabel(title);
    titleLbl->setStyleSheet(QString("border: none; background: transparent; color: %1; font-weight: 500; font-size: 13px;").arg(Theme::TEXT_PRIMARY));
    titleLbl->setWordWrap(true);
    titleLbl->setMaximumHeight(36);

    auto* posterLbl = new QLabel("Posted by " + poster);
    posterLbl->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));

    QLabel* imgLbl = nullptr;
    if (!imageData.isEmpty()) {
        QPixmap pix;
        if (pix.loadFromData(QByteArray::fromBase64(imageData.toUtf8()))) {
            imgLbl = new QLabel;
            imgLbl->setFixedHeight(50);
            imgLbl->setAlignment(Qt::AlignCenter);
            imgLbl->setPixmap(pix.scaled(200, 50, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        }
    }

    layout->addWidget(badge, 0, Qt::AlignLeft);
    if (imgLbl) layout->addWidget(imgLbl);
    layout->addWidget(titleLbl);
    layout->addStretch();
    layout->addWidget(posterLbl);
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
    title_ = new QLineEdit; title_->setPlaceholderText("e.g. Frontend Intern, Research Assistant");
    title_->setStyleSheet(Theme::textInput()); title_->setFixedHeight(38);
    form->addWidget(title_);
    form->addSpacing(10);

    addLabel("Type");
    category_ = new QComboBox;
    category_->addItems({"Job", "Internship", "Volunteer", "Research", "Other"});
    category_->setStyleSheet(Theme::textInput()); category_->setFixedHeight(38);
    form->addWidget(category_);
    form->addSpacing(10);

    addLabel("Location");
    location_ = new QLineEdit; location_->setPlaceholderText("e.g. Remote, New Cairo Campus");
    location_->setStyleSheet(Theme::textInput()); location_->setFixedHeight(38);
    form->addWidget(location_);
    form->addSpacing(10);

    addLabel("Flyer / photo");
    auto* photoRow = new QHBoxLayout;
    photoRow->setSpacing(8);
    photoStatusLabel_ = new QLabel("No photo selected");
    photoStatusLabel_->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));
    auto* btnChoosePhoto = new QPushButton("Choose photo...");
    btnChoosePhoto->setFixedHeight(38);
    btnChoosePhoto->setStyleSheet(QString(
        "QPushButton { background: %1; color: %2; border: 1px solid %3; border-radius: 8px; padding: 0 14px; font-size: 12px; font-weight: 500; }"
        "QPushButton:hover { background: %4; }"
    ).arg(Theme::SURFACE_ALT, Theme::TEXT_PRIMARY, Theme::BORDER, Theme::SURFACE));
    photoRow->addWidget(photoStatusLabel_, 1);
    photoRow->addWidget(btnChoosePhoto);
    form->addLayout(photoRow);
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

    connect(btnSubmit,      &QPushButton::clicked, this, &PostOpportunityPanel::onSubmit);
    connect(btnBack,        &QPushButton::clicked, this, &PostOpportunityPanel::cancelled);
    connect(btnChoosePhoto, &QPushButton::clicked, this, &PostOpportunityPanel::onChoosePhoto);
}

void PostOpportunityPanel::onChoosePhoto()
{
    QString path = QFileDialog::getOpenFileName(this, "Choose flyer / photo", QString(),
                                                  "Images (*.png *.jpg *.jpeg *.gif)");
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Read error", "Could not read the selected image.");
        return;
    }
    QByteArray fileData = file.readAll();
    file.close();

    if (fileData.size() > 500 * 1024) {
        QMessageBox::warning(this, "Image too large", "Photos must be under 500KB.");
        return;
    }

    pendingImageBase64_ = QString::fromLatin1(fileData.toBase64());

    QFileInfo info(path);
    photoStatusLabel_->setText("Selected: " + info.fileName());
}

void PostOpportunityPanel::onSubmit()
{
    if (title_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Missing fields", "Title is required.");
        return;
    }
    Message msg;
    msg.type            = MessageType::OPP_POST;
    msg.title           = title_->text().trimmed().toStdString();
    msg.category         = category_->currentText().toStdString();
    msg.location          = location_->text().trimmed().toStdString();
    msg.mediaUrl        = pendingImageBase64_.toStdString();
    msg.text            = description_->toPlainText().trimmed().toStdString();
    msg.sender.username = displayName_.toStdString();
    emit submitted(msg);

    title_->clear(); location_->clear(); description_->clear();
    category_->setCurrentIndex(0);
    pendingImageBase64_.clear();
    photoStatusLabel_->setText("No photo selected");
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

    img_ = new QLabel;
    img_->setFixedHeight(160);
    img_->setAlignment(Qt::AlignCenter);
    img_->setStyleSheet(QString("background: %1; border-radius: 8px; border: none;").arg(Theme::SURFACE_ALT));
    img_->setVisible(false);
    layout->addWidget(img_);

    title_ = new QLabel;
    title_->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::heading()));
    title_->setWordWrap(true);

    category_ = new QLabel;
    category_->setStyleSheet(QString("border: none; background: transparent; color: %1; font-size: 14px; font-weight: 600;").arg(Theme::ACCENT2));

    poster_ = new QLabel;
    poster_->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));

    description_ = new QTextEdit;
    description_->setReadOnly(true);
    description_->setFixedHeight(120);
    description_->setStyleSheet(QString(
        "QTextEdit { background: transparent; color: %1; border: none; font-size: 13px; }"
    ).arg(Theme::TEXT_PRIMARY));

    auto* btnInquire = new QPushButton("Apply / Contact poster");
    btnInquire->setFixedHeight(44);
    btnInquire->setStyleSheet(Theme::primaryButton());

    layout->addWidget(title_);
    layout->addWidget(category_);
    layout->addWidget(poster_);
    layout->addSpacing(8);
    layout->addWidget(description_);
    layout->addWidget(btnInquire);

    outer->addWidget(card);

    connect(btnBack, &QPushButton::clicked, this, &OpportunityDetailPanel::backClicked);
    connect(btnInquire, &QPushButton::clicked, this, [this](){
        Message msg;
        msg.type     = MessageType::OPP_INQUIRY;
        msg.parentId = currentId_.toStdString();
        msg.title    = currentTitle_.toStdString();
        msg.text     = "Hi, I'm interested in your opportunity: " + currentTitle_.toStdString();
        msg.sender.username = displayName_.toStdString();
        emit inquiryClicked(msg);
    });
}

void OpportunityDetailPanel::show(const QString& id, const QString& title, const QString& category,
                                   const QString& location, const QString& poster, const QString& posterId,
                                   const QString& description, const QString& imageData)
{
    currentId_       = id;
    currentTitle_    = title;
    currentPosterId_ = posterId;
    title_->setText(title);
    category_->setText(category + (location.isEmpty() ? "" : " · " + location));
    poster_->setText("Posted by " + poster);
    description_->setText(description.isEmpty() ? "No description provided." : description);

    img_->setPixmap(QPixmap());
    img_->setVisible(false);
    if (!imageData.isEmpty()) {
        QPixmap pix;
        if (pix.loadFromData(QByteArray::fromBase64(imageData.toUtf8()))) {
            img_->setPixmap(pix.scaled(img_->width(), img_->height(),
                Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
            img_->setVisible(true);
        }
    }

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
    connect(detailPanel_, &OpportunityDetailPanel::inquiryClicked, this, [this](const Message& msg){
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
    detailPanel_->show(o.id, o.title, o.category, o.location, o.poster, o.posterId, o.description, o.imageData);
    stack_->setCurrentIndex(2);
}

void OpportunitiesPanel::onSearch()
{
    QString query = searchBar_->text().trimmed().toLower();
    clearGrid();
    for (auto& o : opportunities_) {
        if (query.isEmpty() || o.title.toLower().contains(query) ||
            o.description.toLower().contains(query) || o.category.toLower().contains(query)) {
            addCard(o.id, o.title, o.category, o.location, o.poster, o.posterId, o.description, o.imageData);
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

void OpportunitiesPanel::addCard(const QString& id, const QString& title, const QString& category,
                                  const QString& location, const QString& poster, const QString& posterId,
                                  const QString& description, const QString& imageData)
{
    int count = grid_->count();
    int col   = count % 4;
    int row   = count / 4;
    auto* card = new OpportunityCard(id, title, category, poster, description, imageData);
    connect(card, &OpportunityCard::clicked, this, &OpportunitiesPanel::onCardClicked);
    grid_->addWidget(card, row, col);
}

void OpportunitiesPanel::receiveMessage(const Message& msg)
{
    if (msg.type == MessageType::OPP_POST)
    {
        QString id       = QString::fromStdString(msg.parentId);
        QString title    = QString::fromStdString(msg.title);
        QString category = QString::fromStdString(msg.category);
        QString location = QString::fromStdString(msg.location);
        QString poster   = QString::fromStdString(msg.sender.username);
        QString posterId = QString::fromStdString(msg.sender.userId);
        QString desc     = QString::fromStdString(msg.text);
        QString imgData  = QString::fromStdString(msg.mediaUrl);

        if (id.isEmpty() || title.isEmpty()) return;

        opportunities_[id] = {id, title, category, location, poster, posterId, desc, imgData};

        auto existing = gridWidget_->findChildren<OpportunityCard*>();
        for (auto* c : existing)
            if (c->opportunityId() == id) return;

        if (grid_->count() == 1) {
            auto* item = grid_->itemAt(0);
            if (item && qobject_cast<QLabel*>(item->widget())) {
                clearGrid();
            }
        }

        addCard(id, title, category, location, poster, posterId, desc, imgData);
    }

    if (msg.type == MessageType::OPP_INQUIRY) {
        QString roomId = QString::fromStdString(msg.roomId);
        QMessageBox::information(this, "Application sent", QString::fromStdString(msg.text));
        if (!roomId.isEmpty())
            emit openRoom(roomId);
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