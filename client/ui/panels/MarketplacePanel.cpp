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
#include <QDebug>

// ── palette (matches HomePage's printed-label aesthetic) ───────────────────────
static const char* BG_MAIN       = "#D4C5B6"; // Warm Sand/Beige
static const char* BG_CARD       = "#CBBBA8"; // Slightly deeper sand for cards
static const char* TEXT_MAIN     = "#0F0F0F"; // Ink Black
static const char* TEXT_SEC      = "#5A4B3E"; // Muted brown-black
static const char* ACCENT_ORANGE = "#E65C40"; // Stamp Red-Orange

// ── divider helpers (kept local/static so they don't clash with HomePage.cpp) ──
static QFrame* createHLineMP() {
    QFrame* line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFixedHeight(1);
    line->setStyleSheet(QString("background-color: %1; border: none;").arg(TEXT_MAIN));
    return line;
}

static QFrame* createVLineMP() {
    QFrame* line = new QFrame;
    line->setFrameShape(QFrame::VLine);
    line->setFixedWidth(1);
    line->setStyleSheet(QString("background-color: %1; border: none;").arg(TEXT_MAIN));
    return line;
}

// ── shared style builders ───────────────────────────────────────────────────
static QString labelHeaderStyle() {
    return QString("color:%1;font-size:12px;font-weight:bold;letter-spacing:2px;").arg(TEXT_SEC);
}

static QString inputStyle() {
    return QString("QLineEdit{background:transparent;color:%1;border:none;"
        "border-bottom:1px solid %1;padding:8px 4px;font-size:14px;}"
        "QLineEdit:focus{border-bottom:2px solid %2;}").arg(TEXT_MAIN, ACCENT_ORANGE);
}

static QString textEditStyle() {
    return QString("QTextEdit{background:transparent;color:%1;border:1px solid %1;"
        "padding:8px;font-size:13px;}"
        "QTextEdit:focus{border:1px solid %2;}").arg(TEXT_MAIN, ACCENT_ORANGE);
}

// primary action: orange outline stamp, fills orange on hover (like GET STARTED)
static QString primaryBtnStyle() {
    return QString(
        "QPushButton{background:transparent;color:%1;border:2px solid %1;"
        "font-size:13px;font-weight:bold;letter-spacing:2px;padding:10px 20px;}"
        "QPushButton:hover{background:%1;color:%2;}").arg(ACCENT_ORANGE, BG_MAIN);
}

// secondary action: black outline, fills black on hover (like SIGN IN)
static QString secondaryBtnStyle() {
    return QString(
        "QPushButton{background:transparent;color:%1;border:1px solid %1;"
        "font-size:12px;font-weight:bold;letter-spacing:2px;padding:8px 16px;}"
        "QPushButton:hover{background:%1;color:%2;}").arg(TEXT_MAIN, BG_MAIN);
}

// flat text-only "back" link
static QString linkBtnStyle() {
    return QString("QPushButton{background:transparent;color:%1;border:none;"
        "font-size:12px;font-weight:bold;letter-spacing:1px;}"
        "QPushButton:hover{color:%2;}").arg(TEXT_SEC, TEXT_MAIN);
}

static QString msgBoxStyle() {
    return QString("QMessageBox{background:%1;color:%2;}"
        "QMessageBox QLabel{color:%2;font-size:13px;}"
        "QMessageBox QPushButton{background:transparent;color:%3;border:1px solid %3;"
        "padding:6px 18px;font-weight:bold;letter-spacing:1px;}"
        "QMessageBox QPushButton:hover{background:%3;color:%1;}")
        .arg(BG_MAIN, TEXT_MAIN, ACCENT_ORANGE);
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
        "QWidget{background:%1;border:1px solid %2;}"
        "QWidget:hover{background:%3;border:1px solid %4;}")
        .arg(BG_MAIN, TEXT_MAIN, BG_CARD, ACCENT_ORANGE));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16,16,16,16);
    layout->setSpacing(6);

    // image placeholder — bordered box, label-tag style
    auto* imgPlaceholder = new QLabel("🖼");
    imgPlaceholder->setAlignment(Qt::AlignCenter);
    imgPlaceholder->setFixedHeight(72);
    imgPlaceholder->setStyleSheet(QString(
        "background:transparent;border:1px solid %1;font-size:28px;color:%1;").arg(TEXT_MAIN));

    auto* titleLbl = new QLabel(title.toUpper());
    titleLbl->setStyleSheet(QString(
        "color:%1;font-weight:bold;font-size:12px;letter-spacing:1px;").arg(TEXT_MAIN));
    titleLbl->setWordWrap(true);
    titleLbl->setMaximumHeight(36);

    auto* priceLbl = new QLabel(price);
    priceLbl->setStyleSheet(QString(
        "color:%1;font-size:16px;font-weight:900;").arg(ACCENT_ORANGE));

    auto* divider = createHLineMP();

    auto* sellerLbl = new QLabel("BY " + seller.toUpper());
    sellerLbl->setStyleSheet(QString(
        "color:%1;font-size:10px;letter-spacing:1px;").arg(TEXT_SEC));

    layout->addWidget(imgPlaceholder);
    layout->addWidget(titleLbl);
    layout->addWidget(priceLbl);
    layout->addWidget(divider);
    layout->addWidget(sellerLbl);
    layout->addStretch();
}

void ListingCard::mousePressEvent(QMouseEvent*) { emit clicked(id_); }

// ─────────────────────────────────────────────────────────────────────────────
//  PostListingPanel
// ─────────────────────────────────────────────────────────────────────────────
PostListingPanel::PostListingPanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("PostListingPanel{background:%1;}").arg(BG_MAIN));
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(50,50,50,50);
    outer->setSpacing(0);

    // header
    auto* header = new QHBoxLayout;
    auto* btnBack = new QPushButton("← BACK");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(linkBtnStyle());
    header->addWidget(btnBack);
    header->addStretch();
    outer->addLayout(header);
    outer->addSpacing(16);

    auto* titleLbl = new QLabel("POST A LISTING");
    titleLbl->setStyleSheet(QString(
        "color:%1;font-size:32px;font-weight:900;letter-spacing:4px;").arg(TEXT_MAIN));
    outer->addWidget(titleLbl);
    outer->addSpacing(20);
    outer->addWidget(createHLineMP());
    outer->addSpacing(24);

    // form card
    auto* card = new QWidget;
    card->setMaximumWidth(600);
    auto* form = new QVBoxLayout(card);
    form->setContentsMargins(0,0,0,0);
    form->setSpacing(16);

    auto addLabel = [&](const QString& t){
        auto* l = new QLabel(t);
        l->setStyleSheet(labelHeaderStyle());
        form->addWidget(l);
    };

    addLabel("TITLE *");
    title_ = new QLineEdit; title_->setPlaceholderText("What are you selling?");
    title_->setStyleSheet(inputStyle()); title_->setFixedHeight(38);
    form->addWidget(title_);

    addLabel("PRICE *");
    price_ = new QLineEdit; price_->setPlaceholderText("e.g. 150 EGP");
    price_->setStyleSheet(inputStyle()); price_->setFixedHeight(38);
    form->addWidget(price_);

    addLabel("PHOTO URL");
    mediaUrl_ = new QLineEdit; mediaUrl_->setPlaceholderText("https://...");
    mediaUrl_->setStyleSheet(inputStyle()); mediaUrl_->setFixedHeight(38);
    form->addWidget(mediaUrl_);

    addLabel("DESCRIPTION");
    description_ = new QTextEdit;
    description_->setPlaceholderText("Describe your item...");
    description_->setFixedHeight(100);
    description_->setStyleSheet(textEditStyle());
    form->addWidget(description_);

    form->addSpacing(12);
    auto* btnSubmit = new QPushButton("POST LISTING");
    btnSubmit->setFixedHeight(46);
    btnSubmit->setStyleSheet(primaryBtnStyle());
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
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("ListingDetailPanel{background:%1;}").arg(BG_MAIN));
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(50,50,50,50);
    outer->setSpacing(0);

    auto* header = new QHBoxLayout;
    auto* btnBack = new QPushButton("← BACK TO LISTINGS");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(linkBtnStyle());
    header->addWidget(btnBack);
    header->addStretch();
    outer->addLayout(header);
    outer->addSpacing(20);
    outer->addWidget(createHLineMP());
    outer->addSpacing(24);

    // detail card
    auto* card = new QWidget;
    card->setMaximumWidth(700);
    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(16);

    // image placeholder
    auto* img = new QLabel("🖼");
    img->setAlignment(Qt::AlignCenter);
    img->setFixedHeight(180);
    img->setStyleSheet(QString(
        "background:transparent;border:1px solid %1;font-size:48px;color:%1;").arg(TEXT_MAIN));
    layout->addWidget(img);

    title_ = new QLabel;
    title_->setStyleSheet(QString(
        "color:%1;font-size:26px;font-weight:900;letter-spacing:2px;").arg(TEXT_MAIN));
    title_->setWordWrap(true);

    price_ = new QLabel;
    price_->setStyleSheet(QString(
        "color:%1;font-size:26px;font-weight:900;").arg(ACCENT_ORANGE));

    seller_ = new QLabel;
    seller_->setStyleSheet(QString(
        "color:%1;font-size:12px;letter-spacing:1px;").arg(TEXT_SEC));

    auto* divider = createHLineMP();

    description_ = new QTextEdit;
    description_->setReadOnly(true);
    description_->setFixedHeight(120);
    description_->setStyleSheet(QString(
        "QTextEdit{background:transparent;color:%1;border:none;font-size:13px;}").arg(TEXT_MAIN));

    auto* btnInquire = new QPushButton("💬  CONTACT SELLER");
    btnInquire->setFixedHeight(48);
    btnInquire->setStyleSheet(primaryBtnStyle());

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
    title_->setText(title.toUpper());
    price_->setText(price);
    seller_->setText("SOLD BY  " + seller.toUpper());
    description_->setText(description.isEmpty() ? "No description provided." : description);
    QWidget::show();
}

// ─────────────────────────────────────────────────────────────────────────────
//  MarketplacePanel
// ─────────────────────────────────────────────────────────────────────────────
MarketplacePanel::MarketplacePanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("MarketplacePanel{background:%1;}").arg(BG_MAIN));
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    stack_ = new QStackedWidget;

    // ── browse page ───────────────────────────────────────────────────────
    auto* browsePage = new QWidget;
    browsePage->setAttribute(Qt::WA_StyledBackground, true);
    browsePage->setStyleSheet(QString("background:%1;").arg(BG_MAIN));
    auto* browseLayout = new QVBoxLayout(browsePage);
    browseLayout->setContentsMargins(50,40,50,40);
    browseLayout->setSpacing(0);

    // branding, mirroring HomePage's logo treatment
    auto* pageTitle = new QLabel("MARKETPLACE");
    pageTitle->setStyleSheet(QString(
        "color:%1;font-size:40px;font-weight:900;letter-spacing:6px;").arg(TEXT_MAIN));
    browseLayout->addWidget(pageTitle);
    browseLayout->addSpacing(16);
    browseLayout->addWidget(createHLineMP());
    browseLayout->addSpacing(20);

    // top bar: search + post button
    auto* topBar = new QHBoxLayout;
    searchBar_ = new QLineEdit;
    searchBar_->setPlaceholderText("SEARCH LISTINGS...");
    searchBar_->setFixedHeight(38);
    searchBar_->setMaximumWidth(340);
    searchBar_->setStyleSheet(inputStyle());

    auto* btnPost = new QPushButton("+ POST LISTING");
    btnPost->setFixedHeight(40);
    btnPost->setStyleSheet(secondaryBtnStyle());

    topBar->addWidget(searchBar_);
    topBar->addStretch();
    topBar->addWidget(btnPost);
    browseLayout->addLayout(topBar);
    browseLayout->addSpacing(20);
    browseLayout->addWidget(createHLineMP());
    browseLayout->addSpacing(20);

    // scroll area for grid
    scrollArea_ = new QScrollArea;
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setStyleSheet(QString(
        "QScrollArea{border:none;background:transparent;}"
        "QScrollBar:vertical{background:%1;width:6px;}"
        "QScrollBar::handle:vertical{background:%2;}").arg(BG_CARD, TEXT_SEC));

    gridWidget_ = new QWidget;
    gridWidget_->setStyleSheet("background:transparent;");
    grid_ = new QGridLayout(gridWidget_);
    grid_->setSpacing(16);
    grid_->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    scrollArea_->setWidget(gridWidget_);

    // empty state
    auto* emptyLabel = new QLabel("NO LISTINGS YET.\nBE THE FIRST TO POST SOMETHING!");
    emptyLabel->setAlignment(Qt::AlignCenter);
    emptyLabel->setStyleSheet(QString(
        "color:%1;font-size:14px;font-weight:bold;letter-spacing:1px;").arg(TEXT_SEC));
    grid_->addWidget(emptyLabel, 0, 0);

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

    if (msg.type == MessageType::MARKET_DELETE)
    {
        QString id = QString::fromStdString(msg.parentId);
        listings_.remove(id);
        auto existing = gridWidget_->findChildren<ListingCard*>();
        for (auto* c : existing)
            if (c->listingId() == id) { c->deleteLater(); break; }
    }
}   