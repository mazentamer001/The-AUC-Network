#include "MarketplacePanel.h"
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
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
//  ListingCard
// ─────────────────────────────────────────────────────────────────────────────
ListingCard::ListingCard(const QString& id, const QString& title,
                         const QString& price, const QString& seller,
                         const QString& description, const QString& imageData,
                         QWidget* parent)
    : QWidget(parent), id_(id)
{
    setFixedSize(220, 190);
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString(
        "ListingCard { background: %1; border: 1px solid %2; border-radius: 12px; }"
        "ListingCard:hover { border: 1px solid %3; }"
    ).arg(Theme::SURFACE, Theme::BORDER, Theme::ACCENT));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(6);

    auto* imgPlaceholder = new QLabel;
    imgPlaceholder->setFixedHeight(70);
    imgPlaceholder->setAlignment(Qt::AlignCenter);
    imgPlaceholder->setStyleSheet(QString(
        "background: %1; border-radius: 8px; border: none;"
    ).arg(Theme::SURFACE_ALT));

    if (!imageData.isEmpty()) {
        QPixmap pix;
        if (pix.loadFromData(QByteArray::fromBase64(imageData.toUtf8()))) {
            imgPlaceholder->setPixmap(pix.scaled(
                imgPlaceholder->width(), imgPlaceholder->height(),
                Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
            imgPlaceholder->setScaledContents(false);
        }
    }

    auto* titleLbl = new QLabel(title);
    titleLbl->setStyleSheet(QString("border: none; background: transparent; color: %1; font-weight: 500; font-size: 13px;").arg(Theme::TEXT_PRIMARY));
    titleLbl->setWordWrap(true);
    titleLbl->setMaximumHeight(36);

    auto* priceLbl = new QLabel(price);
    priceLbl->setStyleSheet(QString("border: none; background: transparent; color: %1; font-size: 15px; font-weight: 600;").arg(Theme::ACCENT2));

    auto* sellerLbl = new QLabel("By " + seller);
    sellerLbl->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));

    layout->addWidget(imgPlaceholder);
    layout->addWidget(titleLbl);
    layout->addWidget(priceLbl);
    layout->addStretch();
    layout->addWidget(sellerLbl);
}

void ListingCard::mousePressEvent(QMouseEvent*) { emit clicked(id_); }

// ─────────────────────────────────────────────────────────────────────────────
//  PostListingPanel
// ─────────────────────────────────────────────────────────────────────────────
PostListingPanel::PostListingPanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("PostListingPanel { %1 }").arg(Theme::pageBackground()));
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(50, 40, 50, 40);
    outer->setSpacing(0);
    outer->setAlignment(Qt::AlignTop);

    auto* btnBack = new QPushButton("Back to marketplace");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(QString(
        "QPushButton { background: transparent; color: %1; border: none; font-size: 12px; }"
        "QPushButton:hover { color: %2; }"
    ).arg(Theme::TEXT_SECONDARY, Theme::ACCENT));
    outer->addWidget(btnBack, 0, Qt::AlignLeft);
    outer->addSpacing(16);

    auto* titleLbl = new QLabel("Post a listing");
    titleLbl->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::heading()));
    outer->addWidget(titleLbl);
    outer->addSpacing(24);

    auto* card = new QWidget;
    card->setObjectName("postCard");
    card->setMaximumWidth(560);
    card->setStyleSheet(QString("#postCard { %1 }").arg(Theme::card()));
    auto* form = new QVBoxLayout(card);
    form->setContentsMargins(32, 32, 32, 32);
    form->setSpacing(6);

    auto addLabel = [&](const QString& t){
        auto* l = new QLabel(t);
        l->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));
        form->addWidget(l);
    };

    addLabel("Title");
    title_ = new QLineEdit; title_->setPlaceholderText("What are you selling?");
    title_->setStyleSheet(Theme::textInput()); title_->setFixedHeight(38);
    form->addWidget(title_);
    form->addSpacing(10);

    addLabel("Price");
    price_ = new QLineEdit; price_->setPlaceholderText("e.g. 150 EGP");
    price_->setStyleSheet(Theme::textInput()); price_->setFixedHeight(38);
    form->addWidget(price_);
    form->addSpacing(10);

    addLabel("Photo");
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
    description_->setPlaceholderText("Describe your item...");
    description_->setFixedHeight(100);
    description_->setStyleSheet(Theme::textArea());
    form->addWidget(description_);
    form->addSpacing(16);

    auto* btnSubmit = new QPushButton("Post listing");
    btnSubmit->setFixedHeight(42);
    btnSubmit->setStyleSheet(Theme::primaryButton());
    form->addWidget(btnSubmit);

    outer->addWidget(card);

    connect(btnSubmit,      &QPushButton::clicked, this, &PostListingPanel::onSubmit);
    connect(btnBack,        &QPushButton::clicked, this, &PostListingPanel::cancelled);
    connect(btnChoosePhoto, &QPushButton::clicked, this, &PostListingPanel::onChoosePhoto);
}

void PostListingPanel::onChoosePhoto()
{
    QString path = QFileDialog::getOpenFileName(this, "Choose listing photo", QString(),
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

void PostListingPanel::onSubmit()
{
    if (title_->text().trimmed().isEmpty() || price_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Missing fields", "Title and price are required.");
        return;
    }
    Message msg;
    msg.type            = MessageType::MARKET_POST;
    msg.title           = title_->text().trimmed().toStdString();
    msg.price           = price_->text().trimmed().toStdString();
    msg.mediaUrl        = pendingImageBase64_.toStdString();
    msg.text            = description_->toPlainText().trimmed().toStdString();
    msg.sender.username = displayName_.toStdString();
    emit submitted(msg);

    title_->clear(); price_->clear(); description_->clear();
    pendingImageBase64_.clear();
    photoStatusLabel_->setText("No photo selected");
}

// ─────────────────────────────────────────────────────────────────────────────
//  ListingDetailPanel
// ─────────────────────────────────────────────────────────────────────────────
ListingDetailPanel::ListingDetailPanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("ListingDetailPanel { %1 }").arg(Theme::pageBackground()));
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(50, 40, 50, 40);
    outer->setSpacing(0);
    outer->setAlignment(Qt::AlignTop);

    auto* btnBack = new QPushButton("Back to listings");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(QString(
        "QPushButton { background: transparent; color: %1; border: none; font-size: 12px; }"
        "QPushButton:hover { color: %2; }"
    ).arg(Theme::TEXT_SECONDARY, Theme::ACCENT));
    outer->addWidget(btnBack, 0, Qt::AlignLeft);
    outer->addSpacing(24);

    auto* card = new QWidget;
    card->setObjectName("detailCard");
    card->setMaximumWidth(640);
    card->setStyleSheet(QString("#detailCard { %1 }").arg(Theme::card()));
    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(32, 32, 32, 32);
    layout->setSpacing(10);

    img_ = new QLabel;
    img_->setFixedHeight(160);
    img_->setAlignment(Qt::AlignCenter);
    img_->setStyleSheet(QString("background: %1; border-radius: 8px; border: none;").arg(Theme::SURFACE_ALT));
    layout->addWidget(img_);

    title_ = new QLabel;
    title_->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::heading()));
    title_->setWordWrap(true);

    price_ = new QLabel;
    price_->setStyleSheet(QString("border: none; background: transparent; color: %1; font-size: 20px; font-weight: 600;").arg(Theme::ACCENT2));

    seller_ = new QLabel;
    seller_->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));

    description_ = new QTextEdit;
    description_->setReadOnly(true);
    description_->setFixedHeight(120);
    description_->setStyleSheet(QString(
        "QTextEdit { background: transparent; color: %1; border: none; font-size: 13px; }"
    ).arg(Theme::TEXT_PRIMARY));

    auto* btnInquire = new QPushButton("Contact seller");
    btnInquire->setFixedHeight(44);
    btnInquire->setStyleSheet(Theme::primaryButton());

    layout->addWidget(title_);
    layout->addWidget(price_);
    layout->addWidget(seller_);
    layout->addSpacing(8);
    layout->addWidget(description_);
    layout->addWidget(btnInquire);

    outer->addWidget(card);

    connect(btnBack, &QPushButton::clicked, this, &ListingDetailPanel::backClicked);
    connect(btnInquire, &QPushButton::clicked, this, [this](){
        Message msg;
        msg.type     = MessageType::MARKET_INQUIRY;
        msg.parentId = currentId_.toStdString();
        msg.title    = currentTitle_.toStdString();
        msg.price    = currentPrice_.toStdString();
        msg.text     = "Hi, I'm interested in your listing: " + currentTitle_.toStdString();
        msg.sender.username = displayName_.toStdString();
        emit inquiryClicked(msg);
    });
}

void ListingDetailPanel::show(const QString& id, const QString& title,
                               const QString& price, const QString& seller,
                               const QString& sellerId, const QString& description,
                               const QString& imageData)
{
    currentId_       = id;
    currentTitle_    = title;
    currentPrice_    = price;
    currentSellerId_ = sellerId;
    title_->setText(title);
    price_->setText(price);
    seller_->setText("Sold by " + seller);
    description_->setText(description.isEmpty() ? "No description provided." : description);

    img_->setPixmap(QPixmap());
    if (!imageData.isEmpty()) {
        QPixmap pix;
        if (pix.loadFromData(QByteArray::fromBase64(imageData.toUtf8()))) {
            img_->setPixmap(pix.scaled(img_->width(), img_->height(),
                Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        }
    }

    QWidget::show();
}

// ─────────────────────────────────────────────────────────────────────────────
//  MarketplacePanel
// ─────────────────────────────────────────────────────────────────────────────
MarketplacePanel::MarketplacePanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("MarketplacePanel { %1 }").arg(Theme::pageBackground()));
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    stack_ = new QStackedWidget;

    auto* browsePage = new QWidget;
    browsePage->setStyleSheet("background: transparent;");
    auto* browseLayout = new QVBoxLayout(browsePage);
    browseLayout->setContentsMargins(40, 32, 40, 32);
    browseLayout->setSpacing(16);

    auto* pageTitle = new QLabel("Marketplace");
    pageTitle->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::heading()));
    browseLayout->addWidget(pageTitle);

    auto* topBar = new QHBoxLayout;
    searchBar_ = new QLineEdit;
    searchBar_->setPlaceholderText("Search listings...");
    searchBar_->setFixedHeight(38);
    searchBar_->setMaximumWidth(300);
    searchBar_->setStyleSheet(Theme::textInput());

    auto* btnPost = new QPushButton("Post listing");
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

    auto* emptyLabel = new QLabel("No listings yet. Be the first to post something!");
    emptyLabel->setAlignment(Qt::AlignCenter);
    emptyLabel->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));
    grid_->addWidget(emptyLabel, 0, 0);

    browseLayout->addWidget(scrollArea_, 1);

    postPanel_   = new PostListingPanel;
    detailPanel_ = new ListingDetailPanel;

    stack_->addWidget(browsePage);
    stack_->addWidget(postPanel_);
    stack_->addWidget(detailPanel_);
    stack_->setCurrentIndex(0);

    root->addWidget(stack_);

    connect(btnPost,    &QPushButton::clicked,  this, &MarketplacePanel::showPostForm);
    connect(searchBar_, &QLineEdit::textChanged, this, &MarketplacePanel::onSearch);

    connect(postPanel_, &PostListingPanel::submitted, this, [this](const Message& msg){
        emit sendMessage(msg);
        showBrowse();
    });
    connect(postPanel_, &PostListingPanel::cancelled, this, &MarketplacePanel::showBrowse);

    connect(detailPanel_, &ListingDetailPanel::backClicked, this, &MarketplacePanel::showBrowse);
    connect(detailPanel_, &ListingDetailPanel::inquiryClicked, this, [this](const Message& msg){
        emit sendMessage(msg);
    });

    QTimer::singleShot(500, this, [this](){
        if (!token_.isEmpty()) {
            Message msg;
            msg.type  = MessageType::MARKET_SEARCH;
            msg.token = token_.toStdString();
            msg.text  = "";
            emit sendMessage(msg);
        }
    });
}

void MarketplacePanel::setCurrentUser(const QString& displayName, const QString& userId)
{
    displayName_ = displayName;
    userId_      = userId;
    postPanel_->setUser(displayName);
    detailPanel_->setUser(displayName);
}

void MarketplacePanel::showPostForm() { stack_->setCurrentIndex(1); }
void MarketplacePanel::showBrowse()   { stack_->setCurrentIndex(0); }

void MarketplacePanel::onCardClicked(const QString& id)
{
    if (!listings_.contains(id)) return;
    auto& l = listings_[id];
    detailPanel_->show(l.id, l.title, l.price, l.seller, l.sellerId, l.description, l.imageData);
    stack_->setCurrentIndex(2);
}

void MarketplacePanel::onSearch()
{
    QString query = searchBar_->text().trimmed().toLower();
    clearGrid();
    for (auto& l : listings_) {
        if (query.isEmpty() || l.title.toLower().contains(query) ||
            l.description.toLower().contains(query)) {
            addCard(l.id, l.title, l.price, l.seller, l.sellerId, l.description, l.imageData);
        }
    }
}

void MarketplacePanel::clearGrid()
{
    while (QLayoutItem* item = grid_->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
}

void MarketplacePanel::addCard(const QString& id, const QString& title,
                                const QString& price, const QString& seller,
                                const QString& sellerId, const QString& description,
                                const QString& imageData)
{
    int count = grid_->count();
    int col   = count % 4;
    int row   = count / 4;
    auto* card = new ListingCard(id, title, price, seller, description, imageData);
    connect(card, &ListingCard::clicked, this, &MarketplacePanel::onCardClicked);
    grid_->addWidget(card, row, col);
}

void MarketplacePanel::receiveMessage(const Message& msg)
{
    if (msg.type == MessageType::MARKET_POST)
    {
        QString id     = QString::fromStdString(msg.parentId);
        QString title  = QString::fromStdString(msg.title);
        QString price  = QString::fromStdString(msg.price);
        QString seller = QString::fromStdString(msg.sender.username);
        QString selId  = QString::fromStdString(msg.sender.userId);
        QString desc   = QString::fromStdString(msg.text);
        QString imgData = QString::fromStdString(msg.mediaUrl);

        if (id.isEmpty() || title.isEmpty()) return;

        listings_[id] = {id, title, price, seller, selId, desc, imgData};

        auto existing = gridWidget_->findChildren<ListingCard*>();
        for (auto* c : existing)
            if (c->listingId() == id) return;

        if (grid_->count() == 1) {
            auto* item = grid_->itemAt(0);
            if (item && qobject_cast<QLabel*>(item->widget())) {
                clearGrid();
            }
        }

        addCard(id, title, price, seller, selId, desc, imgData);
    }

    if (msg.type == MessageType::MARKET_INQUIRY) {
        QString roomId = QString::fromStdString(msg.roomId);
        QMessageBox::information(this, "Inquiry sent", QString::fromStdString(msg.text));
        if (!roomId.isEmpty())
            emit openRoom(roomId);
    }

    if (msg.type == MessageType::MARKET_DELETE)
    {
        QString id = QString::fromStdString(msg.parentId);
        listings_.remove(id);
        auto existing = gridWidget_->findChildren<ListingCard*>();
        for (auto* c : existing)
            if (c->listingId() == id) { c->deleteLater(); break; }
    }
}