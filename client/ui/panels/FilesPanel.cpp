#include "FilesPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QTimer>
#include <QByteArray>
#include <QComboBox>

// Cohesive Label Color Palette
static const char* BG_MAIN       = "#D4C5B6"; // Warm Sand / Parchment
static const char* BG_ALT        = "#C9BBAA"; // Slightly deeper sand for hover/contrast
static const char* TEXT_MAIN     = "#0F0F0F"; // Ink Black
static const char* TEXT_DIM      = "#555555"; // Faded typewriter ink
static const char* ACCENT_ORANGE = "#E65C40"; // Stamp Red-Orange
static const char* ACCENT_RED    = "#D03A2F"; // Danger Red

static QString inputStyle() {
    return QString(
        "QLineEdit {"
        "  background: transparent; color: %1; border: 1px solid %1; border-radius: 0px;"
        "  padding: 8px 12px; font-size: 12px; font-family: monospace;"
        "}"
        "QLineEdit:focus { background: #FFFDFB; border: 1px solid %2; }"
    ).arg(TEXT_MAIN, ACCENT_ORANGE);
}

static QString textAreaStyle() {
    return QString(
        "QTextEdit {"
        "  background: transparent; color: %1; border: 1px solid %1; border-radius: 0px;"
        "  padding: 8px 12px; font-size: 12px; font-family: monospace;"
        "}"
        "QTextEdit:focus { background: #FFFDFB; border: 1px solid %2; }"
    ).arg(TEXT_MAIN, ACCENT_ORANGE);
}

static QString btnStyle(const char* bg, const char* textCol, const char* hoverBg, const char* hoverTextCol) {
    return QString(
        "QPushButton {"
        "  background: %1; color: %2; border: 1px solid %1; border-radius: 0px;"
        "  padding: 5px 12px; font-size: 11px; font-weight: bold; letter-spacing: 1px;"
        "}"
        "QPushButton:hover { background: %3; color: %4; border: 1px solid %3; }"
    ).arg(bg, textCol, hoverBg, hoverTextCol);
}

static QString msgBoxStyle() {
    return QString(
        "QMessageBox { background: %1; color: %2; border: 1px solid %2; border-radius: 0px; }"
        "QMessageBox QLabel { color: %2; font-family: monospace; font-size: 13px; }"
        "QMessageBox QPushButton {"
        "  background: %2; color: %1; border: none; border-radius: 0px;"
        "  padding: 6px 18px; font-size: 12px; font-weight: bold; letter-spacing: 2px;"
        "}"
        "QMessageBox QPushButton:hover { background: %3; color: %2; }"
    ).arg(BG_MAIN, TEXT_MAIN, ACCENT_ORANGE);
}

// Brutalist ASCII tags instead of emojis to fit the typewriter/ledger aesthetic
static QString fileIcon(const QString& name) {
    QString ext = QFileInfo(name).suffix().toLower();
    if (ext=="pdf")                                     return "[ PDF ]";
    if (ext=="png"||ext=="jpg"||ext=="jpeg"||ext=="gif")return "[ IMG ]";
    if (ext=="mp4"||ext=="avi"||ext=="mov")             return "[ VID ]";
    if (ext=="mp3"||ext=="wav")                         return "[ AUD ]";
    if (ext=="zip"||ext=="rar")                         return "[ ZIP ]";
    if (ext=="doc"||ext=="docx")                        return "[ DOC ]";
    if (ext=="ppt"||ext=="pptx")                        return "[ PPT ]";
    if (ext=="xls"||ext=="xlsx")                        return "[ XLS ]";
    if (ext=="cpp"||ext=="h"||ext=="py"||ext=="js"
       ||ext=="txt"||ext=="md")                         return "[ SRC ]";
    return "[ BIN ]";
}

static bool isTextFile(const QString& name) {
    QString ext = QFileInfo(name).suffix().toLower();
    return ext=="txt"||ext=="md"||ext=="cpp"||ext=="h"||ext=="py"
          ||ext=="js"||ext=="json"||ext=="html"||ext=="css"||ext=="xml";
}

// ─────────────────────────────────────────────────────────────────────────────
//  FileRow
// ─────────────────────────────────────────────────────────────────────────────
FileRow::FileRow(const QString& id, const QString& filename,
                 const QString& uploader, const QString& size,
                 const QString& timestamp, const QString& url,
                 const QString& content, QWidget* parent)
    : QWidget(parent), id_(id), url_(url), filename_(filename), content_(content)
{
    setFixedHeight(50); // Slightly tighter ledger look
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString(
        "FileRow { background: transparent; border-bottom: 1px solid %1; border-top: none; border-left: none; border-right: none; }"
        "FileRow:hover { background: %2; }"
    ).arg(TEXT_MAIN, BG_ALT));

    auto* l = new QHBoxLayout(this);
    l->setContentsMargins(14, 0, 14, 0);
    l->setSpacing(14);

    auto* icon = new QLabel(fileIcon(filename));
    icon->setFixedSize(50, 32);
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet(QString(
        "background: %1; color: %2; border: 1px solid %1; font-family: monospace; font-size: 11px; font-weight: bold;"
    ).arg(TEXT_MAIN, BG_MAIN));

    auto* name = new QLabel(filename.toUpper());
    name->setStyleSheet(QString("border: none; color: %1; font-size: 12px; font-weight: 800; font-family: monospace; background: transparent;").arg(TEXT_MAIN));
    name->setMinimumWidth(160);

    auto* upl = new QLabel(uploader.toUpper());
    upl->setStyleSheet(QString("border: none; color: %1; font-size: 11px; font-family: monospace; background: transparent;").arg(TEXT_DIM));
    upl->setMinimumWidth(110);

    auto* sz = new QLabel(size);
    sz->setStyleSheet(QString("border: none; color: %1; font-size: 11px; font-family: monospace; background: transparent;").arg(TEXT_DIM));
    sz->setFixedWidth(76);

    auto* dt = new QLabel(timestamp.left(10));
    dt->setStyleSheet(QString("border: none; color: %1; font-size: 11px; font-family: monospace; background: transparent;").arg(TEXT_DIM));
    dt->setFixedWidth(90);

    auto* btnView     = new QPushButton("VIEW");
    auto* btnDownload = new QPushButton("FETCH");
    auto* btnReport   = new QPushButton("FLAG");
    
    btnView->setFixedHeight(26);
    btnDownload->setFixedHeight(26);
    btnReport->setFixedHeight(26);
    
    // Style: bg, textCol, hoverBg, hoverTextCol
    btnView->setStyleSheet(btnStyle("transparent", TEXT_MAIN, TEXT_MAIN, BG_MAIN));
    btnDownload->setStyleSheet(btnStyle(TEXT_MAIN, BG_MAIN, ACCENT_ORANGE, TEXT_MAIN));
    btnReport->setStyleSheet(btnStyle("transparent", TEXT_DIM, ACCENT_RED, BG_MAIN));

    l->addWidget(icon);
    l->addWidget(name, 2);
    l->addWidget(upl,  1);
    l->addWidget(sz);
    l->addWidget(dt);
    l->addWidget(btnView);
    l->addWidget(btnDownload);
    l->addWidget(btnReport);

    connect(btnView,     &QPushButton::clicked, this, [this](){
        emit viewClicked(id_, url_, filename_, content_); });
    connect(btnDownload, &QPushButton::clicked, this, [this](){
        emit downloadClicked(id_, url_, filename_); });
    connect(btnReport,   &QPushButton::clicked, this, [this](){
        emit reportClicked(id_, filename_); });
}

// ─────────────────────────────────────────────────────────────────────────────
//  ViewPanel
// ─────────────────────────────────────────────────────────────────────────────
ViewPanel::ViewPanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("background: %1;").arg(BG_MAIN));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(32, 24, 32, 24);
    root->setSpacing(16);

    auto* btnBack = new QPushButton("← RETURN TO INDEX");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(QString(
        "QPushButton { color: %1; font-size: 12px; font-weight: bold; letter-spacing: 2px; background: transparent; border: none; text-align: left; }"
        "QPushButton:hover { color: %2; }"
    ).arg(TEXT_MAIN, ACCENT_ORANGE));
    btnBack->setFixedWidth(200);

    iconLabel_ = new QLabel;
    iconLabel_->setAlignment(Qt::AlignCenter);
    iconLabel_->setStyleSheet(QString("color: %1; font-size: 24px; font-family: monospace; font-weight: 800; background: transparent;").arg(TEXT_MAIN));

    nameLabel_ = new QLabel;
    nameLabel_->setAlignment(Qt::AlignCenter);
    nameLabel_->setStyleSheet(QString("color: %1; font-size: 18px; font-weight: 900; letter-spacing: 2px; background: transparent;").arg(TEXT_MAIN));

    urlLabel_ = new QLabel;
    urlLabel_->setAlignment(Qt::AlignCenter);
    urlLabel_->setStyleSheet(QString("color: %1; font-size: 11px; font-family: monospace; background: transparent;").arg(TEXT_DIM));

    contentView_ = new QTextEdit;
    contentView_->setReadOnly(true);
    contentView_->setStyleSheet(QString(
        "QTextEdit { background: transparent; color: %1; border: 1px solid %1; border-radius: 0px; padding: 16px; font-family: monospace; font-size: 13px; line-height: 1.5; }"
    ).arg(TEXT_MAIN));

    previewLabel_ = new QLabel("BINARY PAYLOAD — FETCH TO EXAMINE");
    previewLabel_->setAlignment(Qt::AlignCenter);
    previewLabel_->setStyleSheet(QString(
        "color: %1; font-size: 14px; font-family: monospace; font-weight: bold; background: %2; border: 1px solid %1; padding: 40px;"
    ).arg(TEXT_MAIN, BG_ALT));

    root->addWidget(btnBack, 0, Qt::AlignLeft);
    root->addSpacing(10);
    root->addWidget(iconLabel_);
    root->addWidget(nameLabel_);
    root->addWidget(urlLabel_);
    root->addSpacing(10);
    root->addWidget(contentView_, 1);
    root->addWidget(previewLabel_, 1);

    connect(btnBack, &QPushButton::clicked, this, &ViewPanel::backClicked);
}

void ViewPanel::load(const QString& filename, const QString& content, const QString& url)
{
    iconLabel_->setText(fileIcon(filename));
    nameLabel_->setText(filename.toUpper());
    urlLabel_->setText("ORIGIN: " + url);

    if (isTextFile(filename) && !content.isEmpty()) {
        QByteArray decoded = QByteArray::fromBase64(content.toUtf8());
        contentView_->setPlainText(QString::fromUtf8(decoded));
        contentView_->setVisible(true);
        previewLabel_->setVisible(false);
    } else {
        contentView_->setVisible(false);
        previewLabel_->setText(fileIcon(filename) + "\n\n" + filename.toUpper() + "\n\nFETCH REQUIRED TO EXAMINE THIS PAYLOAD.");
        previewLabel_->setVisible(true);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  ReportPanel
// ─────────────────────────────────────────────────────────────────────────────
ReportPanel::ReportPanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("background: %1;").arg(BG_MAIN));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto* outer = new QVBoxLayout;
    outer->setContentsMargins(40, 24, 40, 24);
    outer->setAlignment(Qt::AlignTop);

    auto* btnBack = new QPushButton("← RETURN TO INDEX");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(QString(
        "QPushButton { color: %1; font-size: 12px; font-weight: bold; letter-spacing: 2px; background: transparent; border: none; text-align: left; }"
        "QPushButton:hover { color: %2; }"
    ).arg(TEXT_MAIN, ACCENT_ORANGE));
    btnBack->setFixedWidth(200);

    auto* card = new QWidget;
    card->setMaximumWidth(620);
    card->setStyleSheet(QString("background: transparent; border: 1px solid %1; border-radius: 0px;").arg(TEXT_MAIN));
    
    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(40, 36, 40, 36);
    cardLayout->setSpacing(16);

    auto* title = new QLabel("SUBMIT INFRACTION REPORT");
    title->setStyleSheet(QString("border: none; color: %1; font-size: 18px; font-weight: 900; letter-spacing: 2px;").arg(TEXT_MAIN));

    fileLabel_ = new QLabel;
    fileLabel_->setStyleSheet(QString("border: none; color: %1; font-size: 12px; font-family: monospace; font-weight: bold;").arg(TEXT_DIM));

    auto lbl = [](const QString& t) {
        auto* l = new QLabel(t);
        l->setStyleSheet("border: none; color: #0F0F0F; font-size: 11px; font-weight: 800; letter-spacing: 2px;");
        return l;
    };

    auto* categoryLbl = lbl("VIOLATION CLASS");
    auto* category = new QComboBox;
    category->addItems({"INAPPROPRIATE CONTENT", "COPYRIGHT VIOLATION", "SPAM / MISLEADING", "MALWARE / HARMFUL", "OTHER"});
    category->setFixedHeight(42);
    category->setStyleSheet(QString(
        "QComboBox { background: transparent; color: %1; border: 1px solid %1; border-radius: 0px; padding: 8px 12px; font-size: 12px; font-family: monospace; font-weight: bold; }"
        "QComboBox::drop-down { border: none; }"
        "QComboBox QAbstractItemView { background: %2; color: %1; border: 1px solid %1; selection-background-color: %1; selection-color: %2; }"
    ).arg(TEXT_MAIN, BG_MAIN));

    reasonInput_ = new QLineEdit;
    reasonInput_->setPlaceholderText("BRIEF SUMMARY...");
    reasonInput_->setFixedHeight(42);
    reasonInput_->setStyleSheet(inputStyle());

    detailsInput_ = new QTextEdit;
    detailsInput_->setPlaceholderText("ADDITIONAL CONTEXT (OPTIONAL)...");
    detailsInput_->setFixedHeight(100);
    detailsInput_->setStyleSheet(textAreaStyle());

    auto* btnRow = new QHBoxLayout;
    auto* btnCancel = new QPushButton("ABORT");
    auto* btnSubmit = new QPushButton("FILE REPORT");
    
    btnCancel->setFixedHeight(42);
    btnSubmit->setFixedHeight(42);
    
    btnCancel->setStyleSheet(btnStyle("transparent", TEXT_MAIN, TEXT_MAIN, BG_MAIN));
    btnSubmit->setStyleSheet(btnStyle(TEXT_MAIN, BG_MAIN, ACCENT_RED, BG_MAIN));
    
    btnRow->addWidget(btnCancel);
    btnRow->addWidget(btnSubmit);

    cardLayout->addWidget(title);
    cardLayout->addWidget(fileLabel_);
    cardLayout->addSpacing(12);
    cardLayout->addWidget(categoryLbl);
    cardLayout->addWidget(category);
    cardLayout->addSpacing(6);
    cardLayout->addWidget(lbl("REASON *"));
    cardLayout->addWidget(reasonInput_);
    cardLayout->addSpacing(6);
    cardLayout->addWidget(lbl("CONTEXT LOG"));
    cardLayout->addWidget(detailsInput_);
    cardLayout->addSpacing(16);
    cardLayout->addLayout(btnRow);

    outer->addWidget(btnBack, 0, Qt::AlignLeft);
    outer->addSpacing(16);
    outer->addWidget(card);

    root->addLayout(outer);
    root->addStretch();

    connect(category, &QComboBox::currentTextChanged, this, [this, category](){ });

    connect(btnSubmit, &QPushButton::clicked, this, [this, category](){
        if (reasonInput_->text().trimmed().isEmpty()) return;
        QString fullReason = category->currentText() + " — " + reasonInput_->text().trimmed();
        if (!detailsInput_->toPlainText().trimmed().isEmpty())
            fullReason += "\n" + detailsInput_->toPlainText().trimmed();

        Message msg;
        msg.type            = MessageType::MATERIAL_REPORT;
        msg.token           = token_.toStdString();
        msg.parentId        = fileId_.toStdString();
        msg.text            = fullReason.toStdString();
        msg.sender.username = username_.toStdString();
        emit submitted(msg);

        reasonInput_->clear();
        detailsInput_->clear();
    });
    
    connect(btnCancel, &QPushButton::clicked, this, &ReportPanel::cancelled);
    connect(btnBack,   &QPushButton::clicked, this, &ReportPanel::cancelled);
}

void ReportPanel::setFile(const QString& id, const QString& filename)
{
    fileId_ = id;
    fileLabel_->setText("TARGET: " + filename.toUpper());
}

// ─────────────────────────────────────────────────────────────────────────────
//  FilesPanel
// ─────────────────────────────────────────────────────────────────────────────
FilesPanel::FilesPanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("background: %1;").arg(BG_MAIN));

    stack_ = new QStackedWidget(this);
    stack_->setStyleSheet(QString("background: transparent; border: none;"));

    // ── browse page ───────────────────────────────────────────────────────
    auto* browsePage = new QWidget;
    browsePage->setAttribute(Qt::WA_StyledBackground, true);
    browsePage->setStyleSheet(QString("background: transparent;"));

    auto* browseLayout = new QVBoxLayout(browsePage);
    browseLayout->setContentsMargins(32, 28, 32, 28);
    browseLayout->setSpacing(16);

    // TOP BAR
    auto* topBar = new QHBoxLayout;
    auto* pageTitle = new QLabel("FILE REGISTRY");
    pageTitle->setStyleSheet(QString("color: %1; font-size: 22px; font-weight: 900; letter-spacing: 3px; background: transparent;").arg(TEXT_MAIN));

    searchBar_ = new QLineEdit;
    searchBar_->setPlaceholderText("QUERY INDEX...");
    searchBar_->setFixedHeight(42);
    searchBar_->setMaximumWidth(320);
    searchBar_->setStyleSheet(inputStyle());

    auto* btnUpload = new QPushButton("+ INGEST PAYLOAD");
    btnUpload->setFixedHeight(42);
    btnUpload->setStyleSheet(btnStyle(TEXT_MAIN, BG_MAIN, ACCENT_ORANGE, TEXT_MAIN));

    topBar->addWidget(pageTitle);
    topBar->addStretch();
    topBar->addWidget(searchBar_);
    topBar->addSpacing(16);
    topBar->addWidget(btnUpload);

    // COLUMN HEADER ROW
    auto* header = new QWidget;
    header->setAttribute(Qt::WA_StyledBackground, true);
    header->setFixedHeight(36);
    // Solid bottom border only, to separate headers from the list
    header->setStyleSheet(QString("background: transparent; border-bottom: 2px solid %1;").arg(TEXT_MAIN));
    
    auto* hLayout = new QHBoxLayout(header);
    hLayout->setContentsMargins(64, 0, 14, 0); // Aligned to account for the [ ICON ] width
    hLayout->setSpacing(14);

    auto makeHdr = [](const QString& t, int minW=0) {
        auto* l = new QLabel(t);
        l->setStyleSheet("border: none; color: #0F0F0F; font-size: 11px; font-weight: 900; letter-spacing: 2px; background: transparent;");
        if (minW) l->setMinimumWidth(minW);
        return l;
    };
    
    hLayout->addWidget(makeHdr("IDENTIFIER"), 2);
    hLayout->addWidget(makeHdr("AUTHOR"), 1);
    hLayout->addWidget(makeHdr("WEIGHT"));
    hLayout->addWidget(makeHdr("TIMESTAMP"));
    hLayout->addSpacing(230); // Space for action buttons

    // SCROLL AREA
    scrollArea_ = new QScrollArea;
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setStyleSheet(QString(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { background: %1; width: 8px; }"
        "QScrollBar::handle:vertical { background: %2; min-height: 20px; }"
    ).arg(BG_ALT, TEXT_MAIN));

    listWidget_ = new QWidget;
    listWidget_->setAttribute(Qt::WA_StyledBackground, true);
    listWidget_->setStyleSheet(QString("background: transparent;"));
    
    listLayout_ = new QVBoxLayout(listWidget_);
    listLayout_->setContentsMargins(0, 0, 0, 0);
    listLayout_->setSpacing(0); // Removing spacing so the 1px borders merge into a continuous grid
    listLayout_->setAlignment(Qt::AlignTop);

    emptyLabel_ = new QLabel("INDEX EMPTY.\nNO PAYLOADS DETECTED.");
    emptyLabel_->setAlignment(Qt::AlignCenter);
    emptyLabel_->setStyleSheet(QString(
        "color: %1; font-size: 14px; font-weight: bold; font-family: monospace; letter-spacing: 2px; padding: 60px; background: transparent;"
    ).arg(TEXT_DIM));
    
    listLayout_->addWidget(emptyLabel_);
    scrollArea_->setWidget(listWidget_);

    browseLayout->addLayout(topBar);
    browseLayout->addSpacing(8);
    browseLayout->addWidget(header);
    browseLayout->addWidget(scrollArea_, 1);

    // ── sub panels ────────────────────────────────────────────────────────
    viewPanel_   = new ViewPanel;
    reportPanel_ = new ReportPanel;

    stack_->addWidget(browsePage);   // 0
    stack_->addWidget(viewPanel_);   // 1
    stack_->addWidget(reportPanel_); // 2

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(stack_);

    // connections
    connect(btnUpload,  &QPushButton::clicked,    this, &FilesPanel::onUpload);
    connect(searchBar_, &QLineEdit::textChanged,  this, &FilesPanel::onSearch);
    connect(viewPanel_,   &ViewPanel::backClicked,   this, &FilesPanel::showBrowse);
    connect(reportPanel_, &ReportPanel::cancelled,   this, &FilesPanel::showBrowse);
    connect(reportPanel_, &ReportPanel::submitted,   this, [this](const Message& msg){
        emit sendMessage(msg);
        showBrowse();
        auto* box = new QMessageBox(this);
        box->setStyleSheet(msgBoxStyle());
        box->setWindowTitle("LOGGED");
        box->setText("INFRACTION LOGGED FOR REVIEW.");
        box->exec();
    });
}

void FilesPanel::setCurrentUser(const QString& displayName,
                                 const QString& userId,
                                 const QString& token)
{
    displayName_ = displayName;
    userId_      = userId;
    token_       = token;
    reportPanel_->setToken(token);
    reportPanel_->setUser(displayName);
    QTimer::singleShot(800, this, [this](){ requestFileList(); });
}

void FilesPanel::requestFileList()
{
    Message msg;
    msg.type            = MessageType::MATERIAL_LIST;
    msg.token           = token_.toStdString();
    msg.sender.username = displayName_.toStdString();
    emit sendMessage(msg);
}

void FilesPanel::showBrowse() { stack_->setCurrentIndex(0); }

void FilesPanel::onUpload(){

    QString path = QFileDialog::getOpenFileName(this, "Select File to Upload");
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        auto* box = new QMessageBox(this);
        box->setStyleSheet(msgBoxStyle());
        box->setText("ERR // READ FAILURE.");
        box->exec();
        return;
    }

    QByteArray fileData = file.readAll();
    if (fileData.size() > 5 * 1024 * 1024) {
        auto* box = new QMessageBox(this);
        box->setStyleSheet(msgBoxStyle());
        box->setWindowTitle("ERR // OVERFLOW");
        box->setText("PAYLOAD EXCEEDS 5MB LIMIT.");
        box->exec();
        return;
    }

    QFileInfo info(path);
    QString b64 = fileData.toBase64();

    pendingContent_[info.fileName()] = b64;

    Message msg;
    msg.type            = MessageType::MATERIAL_UPLOAD;
    msg.token           = token_.toStdString();
    msg.filename        = info.fileName().toStdString();
    msg.text            = b64.toStdString();
    msg.sender.username = displayName_.toStdString();
    emit sendMessage(msg);
}

void FilesPanel::onSearch(const QString& query)
{
    QString q = query.toLower();
    clearList();
    bool any = false;
    for (auto& f : files_) {
        if (q.isEmpty() || f.filename.toLower().contains(q) ||
            f.uploader.toLower().contains(q)) {
            addFileRow(f.id, f.filename, f.uploader, f.size, f.timestamp, f.url, f.content);
            any = true;
        }
    }
    emptyLabel_->setVisible(!any);
}

void FilesPanel::receiveMessage(const Message& msg)
{
    if (msg.type == MessageType::MATERIAL_UPLOAD) {
        QString id       = QString::fromStdString(msg.parentId);
        QString filename = QString::fromStdString(msg.filename);
        QString url      = QString::fromStdString(msg.mediaUrl);
        QString uploader = QString::fromStdString(msg.sender.username);
        QString size     = QString::fromStdString(msg.text);
        QString ts       = QString::fromStdString(msg.timestamp);

        if (id.isEmpty() || filename.isEmpty()) return;
        if (files_.contains(id)) return;

        QString cachedContent = pendingContent_.value(filename, "");
        files_[id] = {id, filename, uploader, size, ts, url, cachedContent};
        if (!cachedContent.isEmpty()) pendingContent_.remove(filename);
        emptyLabel_->setVisible(false);
        addFileRow(id, filename, uploader, size, ts, url);
    }
}

void FilesPanel::addFileRow(const QString& id, const QString& filename,
                             const QString& uploader, const QString& size,
                             const QString& timestamp, const QString& url,
                             const QString& content)
{
    auto* row = new FileRow(id, filename, uploader, size, timestamp, url, content);
    connect(row, &FileRow::viewClicked,     this, &FilesPanel::showView);
    connect(row, &FileRow::downloadClicked, this, &FilesPanel::downloadFile);
    connect(row, &FileRow::reportClicked,   this, [this](const QString& fid, const QString& fname){
        showReport(fid, fname);
    });
    listLayout_->addWidget(row);
}

void FilesPanel::clearList()
{
    while (QLayoutItem* item = listLayout_->takeAt(0)) {
        if (item->widget() && item->widget() != emptyLabel_)
            item->widget()->deleteLater();
        delete item;
    }
    listLayout_->addWidget(emptyLabel_);
}

void FilesPanel::showView(const QString& id, const QString& url,
                           const QString& filename, const QString& content)
{
    QString c = content;
    if (c.isEmpty() && files_.contains(id))
        c = files_[id].content;

    viewPanel_->load(filename, c, url);
    stack_->setCurrentIndex(1);
}

void FilesPanel::showReport(const QString& id, const QString& filename)
{
    reportPanel_->setFile(id, filename);
    stack_->setCurrentIndex(2);
}

void FilesPanel::downloadFile(const QString& id, const QString& url,
                               const QString& filename)
{
    QString content = files_.contains(id) ? files_[id].content : "";

    if (content.isEmpty()) {
        auto* box = new QMessageBox(this);
        box->setStyleSheet(msgBoxStyle());
        box->setWindowTitle("ERR // NOT FOUND");
        box->setText("PAYLOAD NOT AVAILABLE IN LOCAL CACHE.\nWAITING ON HTTP IMPLEMENTATION.");
        box->exec();
        return;
    }

    QString savePath = QFileDialog::getSaveFileName(this, "SAVE TARGET AS...", filename);
    if (savePath.isEmpty()) return;

    QByteArray data = QByteArray::fromBase64(content.toUtf8());
    QFile file(savePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data);
        file.close();
        auto* box = new QMessageBox(this);
        box->setStyleSheet(msgBoxStyle());
        box->setWindowTitle("SUCCESS // FETCHED");
        box->setText("PAYLOAD EXPORTED TO:\n" + savePath);
        box->exec();
    }
}