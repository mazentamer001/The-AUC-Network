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
#include <QFrame>
#include <QMouseEvent>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QDebug>

// ── palette ───────────────────────────────────────────────────────────────────
static const char* ACCENT   = "#6366f1";
static const char* ACCENT2  = "#818cf8";
static const char* TEXT_PRI = "#f1f5f9";
static const char* TEXT_SEC = "#94a3b8";
static const char* BG_DEEP  = "#0a0f1e";
static const char* BG_PANEL = "#0f172a";
static const char* BG_CARD  = "#1e293b";
static const char* BG_INPUT = "#334155";
static const char* SUCCESS  = "#22c55e";

static QString inputStyle() {
    return QString("QLineEdit{background:%1;color:%2;border:1px solid #334155;"
        "border-radius:8px;padding:8px 12px;font-size:13px;}"
        "QLineEdit:focus{border:1px solid %3;}").arg(BG_INPUT, TEXT_PRI, ACCENT2);
}

static QString btnStyle(const char* bg, const char* hover) {
    return QString("QPushButton{background:%1;color:white;border:none;"
        "border-radius:8px;padding:8px 18px;font-size:13px;font-weight:bold;}"
        "QPushButton:hover{background:%2;}").arg(bg, hover);
}

static QString msgBoxStyle() {
    return QString("QMessageBox{background:%1;color:%2;}"
        "QMessageBox QLabel{color:%2;}"
        "QMessageBox QPushButton{background:%3;color:white;border:none;"
        "border-radius:6px;padding:6px 18px;}"
        "QMessageBox QPushButton:hover{background:%4;}")
        .arg(BG_PANEL, TEXT_PRI, ACCENT, ACCENT2);
}

// ─────────────────────────────────────────────────────────────────────────────
//  ListingCard
// ─────────────────────────────────────────────────────────────────────────────
ListingCard::ListingCard(const QString& id, const QString& title,
                         const QString& price, const QString& seller,
                         const QString& description, QWidget* parent)
    : QWidget(parent), id_(id)
{
    setFixedSize(240, 200);
    setCursor(Qt::PointingHandCursor);
    setStyleSheet(QString(
        "QWidget{background:%1;border-radius:12px;}"
        "QWidget:hover{background:%2;}").arg(BG_CARD, "#263349"));

    auto* shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(16); shadow->setOffset(0,4);
    shadow->setColor(QColor(0,0,0,100));
    setGraphicsEffect(shadow);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16,16,16,16);
    layout->setSpacing(8);

    // image placeholder
    auto* imgPlaceholder = new QLabel("🖼");
    imgPlaceholder->setAlignment(Qt::AlignCenter);
    imgPlaceholder->setFixedHeight(80);
    imgPlaceholder->setStyleSheet(QString(
        "background:%1;border-radius:8px;font-size:32px;").arg(BG_DEEP));

    auto* titleLbl = new QLabel(title);
    titleLbl->setStyleSheet(QString("color:%1;font-weight:bold;font-size:13px;").arg(TEXT_PRI));
    titleLbl->setWordWrap(true);
    titleLbl->setMaximumHeight(36);

    auto* priceLbl = new QLabel(price);
    priceLbl->setStyleSheet(QString("color:%1;font-size:15px;font-weight:bold;").arg(SUCCESS));

    auto* sellerLbl = new QLabel("by " + seller);
    sellerLbl->setStyleSheet(QString("color:%1;font-size:11px;").arg(TEXT_SEC));

    layout->addWidget(imgPlaceholder);
    layout->addWidget(titleLbl);
    layout->addWidget(priceLbl);
    layout->addWidget(sellerLbl);
    layout->addStretch();
}

void ListingCard::mousePressEvent(QMouseEvent*) { emit clicked(id_); }

// ─────────────────────────────────────────────────────────────────────────────
//  PostListingPanel
// ─────────────────────────────────────────────────────────────────────────────
PostListingPanel::PostListingPanel(QWidget* parent) : QWidget(parent)
{
    setStyleSheet(QString("background:%1;").arg(BG_DEEP));
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(40,30,40,30);
    outer->setSpacing(0);

    // header
    auto* header = new QHBoxLayout;
    auto* btnBack = new QPushButton("← Back");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(QString("color:%1;font-size:13px;").arg(TEXT_SEC));
    auto* titleLbl = new QLabel("Post a Listing");
    titleLbl->setStyleSheet(QString("color:%1;font-size:22px;font-weight:bold;").arg(TEXT_PRI));
    header->addWidget(btnBack);
    header->addStretch();
    outer->addLayout(header);
    outer->addSpacing(8);
    outer->addWidget(titleLbl);
    outer->addSpacing(24);

    // form card
    auto* card = new QWidget;
    card->setStyleSheet(QString("background:%1;border-radius:12px;").arg(BG_PANEL));
    card->setMaximumWidth(600);
    auto* form = new QVBoxLayout(card);
    form->setContentsMargins(30,28,30,28);
    form->setSpacing(14);

    auto addLabel = [&](const QString& t){ 
        auto* l = new QLabel(t);
        l->setStyleSheet(QString("color:%1;font-size:12px;font-weight:bold;").arg(TEXT_SEC));
        form->addWidget(l);
    };

    addLabel("TITLE *");
    title_ = new QLineEdit; title_->setPlaceholderText("What are you selling?");
    title_->setStyleSheet(inputStyle()); title_->setFixedHeight(42);
    form->addWidget(title_);

    addLabel("PRICE *");
    price_ = new QLineEdit; price_->setPlaceholderText("e.g. 150 EGP");
    price_->setStyleSheet(inputStyle()); price_->setFixedHeight(42);
    form->addWidget(price_);

    addLabel("PHOTO URL");
    mediaUrl_ = new QLineEdit; mediaUrl_->setPlaceholderText("https://...");
    mediaUrl_->setStyleSheet(inputStyle()); mediaUrl_->setFixedHeight(42);
    form->addWidget(mediaUrl_);

    addLabel("DESCRIPTION");
    description_ = new QTextEdit;
    description_->setPlaceholderText("Describe your item...");
    description_->setFixedHeight(100);
    description_->setStyleSheet(QString(
        "QTextEdit{background:%1;color:%2;border:1px solid #334155;"
        "border-radius:8px;padding:8px;font-size:13px;}"
        "QTextEdit:focus{border:1px solid %3;}").arg(BG_INPUT, TEXT_PRI, ACCENT2));
    form->addWidget(description_);

    form->addSpacing(8);
    auto* btnSubmit = new QPushButton("Post Listing");
    btnSubmit->setFixedHeight(46);
    btnSubmit->setStyleSheet(btnStyle(ACCENT, ACCENT2));
    form->addWidget(btnSubmit);

    outer->addWidget(card);
    outer->addStretch();

    connect(btnSubmit, &QPushButton::clicked, this, &PostListingPanel::onSubmit);
    connect(btnBack,   &QPushButton::clicked, this, &PostListingPanel::cancelled);
}

void PostListingPanel::onSubmit()
{
    if (title_->text().trimmed().isEmpty() || price_->text().trimmed().isEmpty()) {
        auto* box = new QMessageBox(this);
        box->setStyleSheet(msgBoxStyle());
        box->setText("Title and price are required.");
        box->exec();
        return;
    }
    Message msg;
    msg.type            = MessageType::MARKET_POST;
    msg.title           = title_->text().trimmed().toStdString();
    msg.price           = price_->text().trimmed().toStdString();
    msg.mediaUrl        = mediaUrl_->text().trimmed().toStdString();
    msg.text            = description_->toPlainText().trimmed().toStdString();
    msg.sender.username = displayName_.toStdString();
    emit submitted(msg);

    // clear form
    title_->clear(); price_->clear(); mediaUrl_->clear(); description_->clear();
}

// ─────────────────────────────────────────────────────────────────────────────
//  ListingDetailPanel
// ─────────────────────────────────────────────────────────────────────────────
ListingDetailPanel::ListingDetailPanel(QWidget* parent) : QWidget(parent)
{
    setStyleSheet(QString("background:%1;").arg(BG_DEEP));
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(40,30,40,30);
    outer->setSpacing(0);

    auto* header = new QHBoxLayout;
    auto* btnBack = new QPushButton("← Back to listings");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(QString("color:%1;font-size:13px;").arg(TEXT_SEC));
    header->addWidget(btnBack);
    header->addStretch();
    outer->addLayout(header);
    outer->addSpacing(20);

    // detail card
    auto* card = new QWidget;
    card->setStyleSheet(QString("background:%1;border-radius:12px;").arg(BG_PANEL));
    card->setMaximumWidth(700);
    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(32,28,32,28);
    layout->setSpacing(16);

    // image placeholder
    auto* img = new QLabel("🖼");
    img->setAlignment(Qt::AlignCenter);
    img->setFixedHeight(180);
    img->setStyleSheet(QString(
        "background:%1;border-radius:8px;font-size:56px;").arg(BG_DEEP));
    layout->addWidget(img);

    title_ = new QLabel;
    title_->setStyleSheet(QString("color:%1;font-size:22px;font-weight:bold;").arg(TEXT_PRI));
    title_->setWordWrap(true);

    price_ = new QLabel;
    price_->setStyleSheet(QString("color:%1;font-size:24px;font-weight:bold;").arg(SUCCESS));

    seller_ = new QLabel;
    seller_->setStyleSheet(QString("color:%1;font-size:13px;").arg(TEXT_SEC));

    auto* divider = new QFrame;
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("background:#1e293b;");

    description_ = new QTextEdit;
    description_->setReadOnly(true);
    description_->setFixedHeight(120);
    description_->setStyleSheet(QString(
        "QTextEdit{background:transparent;color:%1;border:none;font-size:13px;}").arg(TEXT_PRI));

    auto* btnInquire = new QPushButton("💬  Contact Seller");
    btnInquire->setFixedHeight(48);
    btnInquire->setStyleSheet(btnStyle(ACCENT, ACCENT2));

    layout->addWidget(title_);
    layout->addWidget(price_);
    layout->addWidget(seller_);
    layout->addWidget(divider);
    layout->addWidget(description_);
    layout->addWidget(btnInquire);

    outer->addWidget(card);
    outer->addStretch();

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
                               const QString& sellerId, const QString& description)
{
    currentId_       = id;
    currentTitle_    = title;
    currentPrice_    = price;
    currentSellerId_ = sellerId;
    title_->setText(title);
    price_->setText(price);
    seller_->setText("Sold by  " + seller);
    description_->setText(description.isEmpty() ? "No description provided." : description);
    QWidget::show();
}

// ─────────────────────────────────────────────────────────────────────────────
//  MarketplacePanel
// ─────────────────────────────────────────────────────────────────────────────
MarketplacePanel::MarketplacePanel(QWidget* parent) : QWidget(parent)
{
    setStyleSheet(QString("background:%1;").arg(BG_DEEP));
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    stack_ = new QStackedWidget;

    // ── browse page ───────────────────────────────────────────────────────
    auto* browsePage = new QWidget;
    browsePage->setStyleSheet(QString("background:%1;").arg(BG_DEEP));
    auto* browseLayout = new QVBoxLayout(browsePage);
    browseLayout->setContentsMargins(24,20,24,20);
    browseLayout->setSpacing(16);

    // top bar
    auto* topBar = new QHBoxLayout;
    auto* pageTitle = new QLabel("Marketplace");
    pageTitle->setStyleSheet(QString("color:%1;font-size:22px;font-weight:bold;").arg(TEXT_PRI));

    searchBar_ = new QLineEdit;
    searchBar_->setPlaceholderText("🔍  Search listings...");
    searchBar_->setFixedHeight(40);
    searchBar_->setMaximumWidth(340);
    searchBar_->setStyleSheet(inputStyle());

    auto* btnPost = new QPushButton("+ Post Listing");
    btnPost->setFixedHeight(40);
    btnPost->setStyleSheet(btnStyle(ACCENT, ACCENT2));

    topBar->addWidget(pageTitle);
    topBar->addStretch();
    topBar->addWidget(searchBar_);
    topBar->addSpacing(12);
    topBar->addWidget(btnPost);

    // scroll area for grid
    scrollArea_ = new QScrollArea;
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setStyleSheet(
        "QScrollArea{border:none;background:transparent;}"
        "QScrollBar:vertical{background:#1e293b;width:6px;border-radius:3px;}"
        "QScrollBar::handle:vertical{background:#334155;border-radius:3px;}");

    gridWidget_ = new QWidget;
    gridWidget_->setStyleSheet("background:transparent;");
    grid_ = new QGridLayout(gridWidget_);
    grid_->setSpacing(16);
    grid_->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    scrollArea_->setWidget(gridWidget_);

    // empty state
    auto* emptyLabel = new QLabel("No listings yet.\nBe the first to post something!");
    emptyLabel->setAlignment(Qt::AlignCenter);
    emptyLabel->setStyleSheet(QString("color:%1;font-size:15px;").arg(TEXT_SEC));
    grid_->addWidget(emptyLabel, 0, 0);

    browseLayout->addLayout(topBar);
    browseLayout->addWidget(scrollArea_, 1);

    // ── sub panels ────────────────────────────────────────────────────────
    postPanel_   = new PostListingPanel;
    detailPanel_ = new ListingDetailPanel;

    stack_->addWidget(browsePage);   // 0
    stack_->addWidget(postPanel_);   // 1
    stack_->addWidget(detailPanel_); // 2
    stack_->setCurrentIndex(0);

    root->addWidget(stack_);

    // ── connections ───────────────────────────────────────────────────────
    connect(btnPost,    &QPushButton::clicked,  this, &MarketplacePanel::showPostForm);
    connect(searchBar_, &QLineEdit::textChanged, this, &MarketplacePanel::onSearch);

    connect(postPanel_, &PostListingPanel::submitted, this, [this](const Message& msg){
        emit sendMessage(msg);
        showBrowse();
    });
    connect(postPanel_, &PostListingPanel::cancelled, this, &MarketplacePanel::showBrowse);

    connect(detailPanel_, &ListingDetailPanel::backClicked,     this, &MarketplacePanel::showBrowse);
    connect(detailPanel_, &ListingDetailPanel::inquiryClicked, this, [this](const Message& msg){
        emit sendMessage(msg);   //send and wait for server response
    });

    // request listings on load
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
    detailPanel_->show(l.id, l.title, l.price, l.seller, l.sellerId, l.description);
    stack_->setCurrentIndex(2);
}

void MarketplacePanel::onSearch()
{
    QString query = searchBar_->text().trimmed().toLower();
    // filter locally
    int col = 0, row = 0;
    clearGrid();
    for (auto& l : listings_) {
        if (query.isEmpty() || l.title.toLower().contains(query) ||
            l.description.toLower().contains(query)) {
            addCard(l.id, l.title, l.price, l.seller, l.sellerId, l.description);
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
                                const QString& sellerId, const QString& description)
{
    int count = grid_->count();
    int col   = count % 4;
    int row   = count / 4;
    auto* card = new ListingCard(id, title, price, seller, description);
    connect(card, &ListingCard::clicked, this, &MarketplacePanel::onCardClicked);
    grid_->addWidget(card, row, col);
}

void MarketplacePanel::receiveMessage(const Message& msg)
{
    if (msg.type == MessageType::MARKET_POST)
    {
        qDebug() << "Market msg type:" << QString::fromStdString(Message::typeToString(msg.type));
        qDebug() << "id:" << QString::fromStdString(msg.parentId);
        qDebug() << "title:" << QString::fromStdString(msg.title);
        qDebug() << "price:" << QString::fromStdString(msg.price);
        // could be a search result or a new listing confirmation
        QString id     = QString::fromStdString(msg.parentId);
        QString title  = QString::fromStdString(msg.title);
        QString price  = QString::fromStdString(msg.price);
        QString seller = QString::fromStdString(msg.sender.username);
        QString selId  = QString::fromStdString(msg.sender.userId);
        QString desc   = QString::fromStdString(msg.text);

        if (id.isEmpty() || title.isEmpty()) return; // just a text response

        // store locally
        listings_[id] = {id, title, price, seller, selId, desc};

        // add to grid if not already there
        auto existing = gridWidget_->findChildren<ListingCard*>();
        for (auto* c : existing)
            if (c->listingId() == id) return; // already shown

        // remove empty state label if present
        if (grid_->count() == 1) {
            auto* item = grid_->itemAt(0);
            if (item && qobject_cast<QLabel*>(item->widget())) {
                clearGrid();
            }
        }

        addCard(id, title, price, seller, selId, desc);
    }

    if (msg.type == MessageType::MARKET_INQUIRY) {

         QString roomId = QString::fromStdString(msg.roomId);
        // buyer: confirm sent
        auto* box = new QMessageBox(this);
        box->setStyleSheet(msgBoxStyle());
        box->setWindowTitle("Inquiry Sent");
        box->setText(QString::fromStdString(msg.text));
        box->exec();

        if (!roomId.isEmpty())
            emit openRoom(roomId);
    }
}