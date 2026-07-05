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
#include <QGraphicsDropShadowEffect>

static const char* ACCENT   = "#6366f1";
static const char* ACCENT2  = "#818cf8";
static const char* TEXT_PRI = "#f1f5f9";
static const char* TEXT_SEC = "#94a3b8";
static const char* BG_DEEP  = "#0a0f1e";
static const char* BG_PANEL = "#0f172a";
static const char* BG_CARD  = "#1e293b";
static const char* BG_INPUT = "#334155";
static const char* DANGER   = "#ef4444";

static QString inputStyle() {
    return QString("QLineEdit{"
        "background:%1;color:%2;border:1px solid #1e293b;"
        "border-radius:8px;padding:8px 12px;font-size:13px;}"
        "QLineEdit:focus{border:1px solid %3;}").arg(BG_INPUT,TEXT_PRI,ACCENT2);
}

static QString textAreaStyle() {
    return QString("QTextEdit{"
        "background:%1;color:%2;border:1px solid #1e293b;"
        "border-radius:8px;padding:8px;font-size:13px;}"
        "QTextEdit:focus{border:1px solid %3;}").arg(BG_INPUT,TEXT_PRI,ACCENT2);
}

static QString btnStyle(const char* bg, const char* hover) {
    return QString("QPushButton{background:%1;color:white;border:none;"
        "border-radius:6px;padding:5px 12px;font-size:12px;font-weight:bold;}"
        "QPushButton:hover{background:%2;}").arg(bg,hover);
}

static QString msgBoxStyle() {
    return QString("QMessageBox{background:%1;color:%2;}"
        "QMessageBox QLabel{color:%2;}"
        "QMessageBox QPushButton{background:%3;color:white;border:none;"
        "border-radius:6px;padding:6px 18px;}"
        "QMessageBox QPushButton:hover{background:%4;}")
        .arg(BG_PANEL,TEXT_PRI,ACCENT,ACCENT2);
}

static QString fileIcon(const QString& name) {
    QString ext = QFileInfo(name).suffix().toLower();
    if (ext=="pdf")                                     return "📄";
    if (ext=="png"||ext=="jpg"||ext=="jpeg"||ext=="gif")return "🖼";
    if (ext=="mp4"||ext=="avi"||ext=="mov")             return "🎬";
    if (ext=="mp3"||ext=="wav")                         return "🎵";
    if (ext=="zip"||ext=="rar")                         return "🗜";
    if (ext=="doc"||ext=="docx")                        return "📝";
    if (ext=="ppt"||ext=="pptx")                        return "📊";
    if (ext=="xls"||ext=="xlsx")                        return "📈";
    if (ext=="cpp"||ext=="h"||ext=="py"||ext=="js"
       ||ext=="txt"||ext=="md")                         return "💻";
    return "📁";
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
    setFixedHeight(56);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString(
        "FileRow{background:%1;border-radius:8px;border:1px solid #1e293b;}"
        "FileRow:hover{background:#1a2540;}").arg(BG_CARD));

    auto* l = new QHBoxLayout(this);
    l->setContentsMargins(14,0,14,0);
    l->setSpacing(14);

    auto* icon = new QLabel(fileIcon(filename));
    icon->setFixedSize(32,32);
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet(QString(
        "background:%1;border-radius:6px;font-size:16px;").arg(BG_DEEP));

    auto* name = new QLabel(filename);
    name->setStyleSheet(QString("color:%1;font-size:13px;font-weight:600;"
        "background:transparent;").arg(TEXT_PRI));
    name->setMinimumWidth(160);

    auto* upl = new QLabel(uploader);
    upl->setStyleSheet(QString("color:%1;font-size:12px;background:transparent;").arg(TEXT_SEC));
    upl->setMinimumWidth(110);

    auto* sz = new QLabel(size);
    sz->setStyleSheet(QString("color:%1;font-size:12px;background:transparent;").arg(TEXT_SEC));
    sz->setFixedWidth(76);

    auto* dt = new QLabel(timestamp.left(10));
    dt->setStyleSheet(QString("color:%1;font-size:12px;background:transparent;").arg(TEXT_SEC));
    dt->setFixedWidth(90);

    auto* btnView     = new QPushButton("View");
    auto* btnDownload = new QPushButton("Download");
    auto* btnReport   = new QPushButton("Report");
    btnView->setFixedHeight(26);
    btnDownload->setFixedHeight(26);
    btnReport->setFixedHeight(26);
    btnView->setStyleSheet(btnStyle(ACCENT,ACCENT2));
    btnDownload->setStyleSheet(btnStyle("#1d4ed8","#2563eb"));
    btnReport->setStyleSheet(btnStyle("#374151","#4b5563"));

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
    setStyleSheet(QString("background:%1;").arg(BG_DEEP));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(32,24,32,24);
    root->setSpacing(16);

    // back button
    auto* btnBack = new QPushButton("← Back to Files");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(QString("color:%1;font-size:13px;background:transparent;").arg(TEXT_SEC));
    btnBack->setFixedWidth(160);

    // header
    iconLabel_ = new QLabel;
    iconLabel_->setAlignment(Qt::AlignCenter);
    iconLabel_->setStyleSheet("font-size:52px;background:transparent;");

    nameLabel_ = new QLabel;
    nameLabel_->setAlignment(Qt::AlignCenter);
    nameLabel_->setStyleSheet(QString(
        "color:%1;font-size:18px;font-weight:bold;background:transparent;").arg(TEXT_PRI));

    urlLabel_ = new QLabel;
    urlLabel_->setAlignment(Qt::AlignCenter);
    urlLabel_->setStyleSheet(QString("color:%1;font-size:11px;background:transparent;").arg(TEXT_SEC));

    // content view
    contentView_ = new QTextEdit;
    contentView_->setReadOnly(true);
    contentView_->setStyleSheet(QString(
        "QTextEdit{background:%1;color:%2;border:1px solid #1e293b;"
        "border-radius:8px;padding:12px;font-family:monospace;font-size:13px;}")
        .arg(BG_PANEL, TEXT_PRI));

    // image/binary placeholder
    previewLabel_ = new QLabel("Binary file — download to view");
    previewLabel_->setAlignment(Qt::AlignCenter);
    previewLabel_->setStyleSheet(QString(
        "color:%1;font-size:14px;background:%2;border-radius:8px;padding:40px;")
        .arg(TEXT_SEC, BG_PANEL));

    root->addWidget(btnBack, 0, Qt::AlignLeft);
    root->addSpacing(4);
    root->addWidget(iconLabel_);
    root->addWidget(nameLabel_);
    root->addWidget(urlLabel_);
    root->addWidget(contentView_, 1);
    root->addWidget(previewLabel_, 1);

    connect(btnBack, &QPushButton::clicked, this, &ViewPanel::backClicked);
}

void ViewPanel::load(const QString& filename, const QString& content, const QString& url)
{
    iconLabel_->setText(fileIcon(filename));
    nameLabel_->setText(filename);
    urlLabel_->setText("Server: " + url);

    if (isTextFile(filename) && !content.isEmpty()) {
        // decode base64 and show as text
        QByteArray decoded = QByteArray::fromBase64(content.toUtf8());
        contentView_->setPlainText(QString::fromUtf8(decoded));
        contentView_->setVisible(true);
        previewLabel_->setVisible(false);
    } else {
        contentView_->setVisible(false);
        previewLabel_->setText(fileIcon(filename) + "\n\n" + filename +
                               "\n\nDownload to open this file.");
        previewLabel_->setVisible(true);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  ReportPanel
// ─────────────────────────────────────────────────────────────────────────────
ReportPanel::ReportPanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("background:%1;").arg(BG_DEEP));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    // card centred
    auto* outer = new QVBoxLayout;
    outer->setContentsMargins(40,24,40,24);
    outer->setAlignment(Qt::AlignTop);

    auto* btnBack = new QPushButton("← Back to Files");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(QString("color:%1;font-size:13px;background:transparent;").arg(TEXT_SEC));
    btnBack->setFixedWidth(160);

    auto* card = new QWidget;
    card->setMaximumWidth(620);
    card->setStyleSheet(QString("background:%1;border-radius:12px;").arg(BG_PANEL));
    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(32,28,32,28);
    cardLayout->setSpacing(14);

    auto* title = new QLabel("Report File");
    title->setStyleSheet(QString("color:%1;font-size:20px;font-weight:bold;").arg(TEXT_PRI));

    fileLabel_ = new QLabel;
    fileLabel_->setStyleSheet(QString("color:%1;font-size:13px;").arg(TEXT_SEC));

    auto lbl = [](const QString& t) {
        auto* l = new QLabel(t);
        l->setStyleSheet("color:#64748b;font-size:11px;font-weight:bold;letter-spacing:1px;");
        return l;
    };

    // category dropdown styled like input
    auto* categoryLbl = lbl("CATEGORY");
    auto* category = new QComboBox;
    category->addItems({"Inappropriate Content", "Copyright Violation",
                        "Spam / Misleading", "Malware / Harmful", "Other"});
    category->setFixedHeight(42);
    category->setStyleSheet(QString(
        "QComboBox{background:%1;color:%2;border:1px solid #1e293b;"
        "border-radius:8px;padding:8px 12px;font-size:13px;}"
        "QComboBox::drop-down{border:none;}"
        "QComboBox QAbstractItemView{background:%1;color:%2;border:1px solid #1e293b;}")
        .arg(BG_INPUT, TEXT_PRI));

    // reason
    reasonInput_ = new QLineEdit;
    reasonInput_->setPlaceholderText("Brief reason...");
    reasonInput_->setFixedHeight(42);
    reasonInput_->setStyleSheet(inputStyle());

    // details
    detailsInput_ = new QTextEdit;
    detailsInput_->setPlaceholderText("Additional details (optional)...");
    detailsInput_->setFixedHeight(100);
    detailsInput_->setStyleSheet(textAreaStyle());

    auto* btnRow = new QHBoxLayout;
    auto* btnCancel = new QPushButton("Cancel");
    auto* btnSubmit = new QPushButton("Submit Report");
    btnCancel->setFixedHeight(42);
    btnSubmit->setFixedHeight(42);
    btnCancel->setStyleSheet(btnStyle("#374151","#4b5563"));
    btnSubmit->setStyleSheet(btnStyle(DANGER,"#dc2626"));
    btnRow->addWidget(btnCancel);
    btnRow->addWidget(btnSubmit);

    cardLayout->addWidget(title);
    cardLayout->addWidget(fileLabel_);
    cardLayout->addSpacing(8);
    cardLayout->addWidget(categoryLbl);
    cardLayout->addWidget(category);
    cardLayout->addWidget(lbl("REASON *"));
    cardLayout->addWidget(reasonInput_);
    cardLayout->addWidget(lbl("DETAILS"));
    cardLayout->addWidget(detailsInput_);
    cardLayout->addSpacing(6);
    cardLayout->addLayout(btnRow);

    outer->addWidget(btnBack, 0, Qt::AlignLeft);
    outer->addSpacing(12);
    outer->addWidget(card);

    root->addLayout(outer);
    root->addStretch();

    // keep category value for submit
    connect(category, &QComboBox::currentTextChanged, this, [this, category](){
        // stored inline in onSubmit via capture
    });

    // store category pointer for submit
    connect(btnSubmit, &QPushButton::clicked, this, [this, category](){
        if (reasonInput_->text().trimmed().isEmpty()) return;
        QString fullReason = category->currentText() + " — " +
                             reasonInput_->text().trimmed();
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
    fileLabel_->setText("File: " + filename);
}

// ─────────────────────────────────────────────────────────────────────────────
//  FilesPanel
// ─────────────────────────────────────────────────────────────────────────────
FilesPanel::FilesPanel(QWidget* parent) : QWidget(parent)
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
    auto* pageTitle = new QLabel("Files");
    pageTitle->setStyleSheet(QString(
        "color:%1;font-size:22px;font-weight:bold;background:transparent;").arg(TEXT_PRI));

    searchBar_ = new QLineEdit;
    searchBar_->setPlaceholderText("🔍  Search files...");
    searchBar_->setFixedHeight(38);
    searchBar_->setMaximumWidth(280);
    searchBar_->setStyleSheet(inputStyle());

    auto* btnUpload = new QPushButton("⬆  Upload File");
    btnUpload->setFixedHeight(38);
    btnUpload->setStyleSheet(btnStyle(ACCENT, ACCENT2));

    topBar->addWidget(pageTitle);
    topBar->addStretch();
    topBar->addWidget(searchBar_);
    topBar->addSpacing(10);
    topBar->addWidget(btnUpload);

    // column header row
    auto* header = new QWidget;
    header->setAttribute(Qt::WA_StyledBackground, true);
    header->setFixedHeight(34);
    header->setStyleSheet(QString("background:%1;border-radius:6px;").arg(BG_PANEL));
    auto* hLayout = new QHBoxLayout(header);
    hLayout->setContentsMargins(60,0,14,0);
    hLayout->setSpacing(14);

    auto makeHdr = [](const QString& t, int minW=0) {
        auto* l = new QLabel(t);
        l->setStyleSheet("color:#475569;font-size:10px;font-weight:bold;"
                         "letter-spacing:1px;background:transparent;");
        if (minW) l->setMinimumWidth(minW);
        return l;
    };
    hLayout->addWidget(makeHdr("NAME"), 2);
    hLayout->addWidget(makeHdr("UPLOADED BY"), 1);
    hLayout->addWidget(makeHdr("SIZE"));
    hLayout->addWidget(makeHdr("DATE"));
    hLayout->addSpacing(230);

    // scroll area
    scrollArea_ = new QScrollArea;
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setStyleSheet(QString(
        "QScrollArea{border:none;background:%1;}"
        "QScrollBar:vertical{background:%2;width:5px;border-radius:3px;}"
        "QScrollBar::handle:vertical{background:#334155;border-radius:3px;}")
        .arg(BG_DEEP, BG_PANEL));

    listWidget_ = new QWidget;
    listWidget_->setAttribute(Qt::WA_StyledBackground, true);
    listWidget_->setStyleSheet(QString("background:%1;").arg(BG_DEEP));
    listLayout_ = new QVBoxLayout(listWidget_);
    listLayout_->setContentsMargins(0,4,0,4);
    listLayout_->setSpacing(4);
    listLayout_->setAlignment(Qt::AlignTop);

    emptyLabel_ = new QLabel("No files yet.\nUpload something to share with everyone.");
    emptyLabel_->setAlignment(Qt::AlignCenter);
    emptyLabel_->setStyleSheet(QString(
        "color:%1;font-size:15px;padding:60px;background:transparent;").arg(TEXT_SEC));
    listLayout_->addWidget(emptyLabel_);
    scrollArea_->setWidget(listWidget_);

    browseLayout->addLayout(topBar);
    browseLayout->addWidget(header);
    browseLayout->addWidget(scrollArea_, 1);

    // ── sub panels ────────────────────────────────────────────────────────
    viewPanel_   = new ViewPanel;
    reportPanel_ = new ReportPanel;

    stack_->addWidget(browsePage);   // 0
    stack_->addWidget(viewPanel_);   // 1
    stack_->addWidget(reportPanel_); // 2

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
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
        box->setWindowTitle("Reported");
        box->setText("File has been reported for admin review.");
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
        box->setText("Could not open file.");
        box->exec();
        return;
    }

    QByteArray fileData = file.readAll();
    if (fileData.size() > 5 * 1024 * 1024) {
        auto* box = new QMessageBox(this);
        box->setStyleSheet(msgBoxStyle());
        box->setWindowTitle("File Too Large");
        box->setText("Maximum file size is 5MB.");
        box->exec();
        return;
    }

    QFileInfo info(path);
    QString b64 = fileData.toBase64();

    // cache content locally so View/Download work immediately after upload
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
    // MATERIAL_REPORT handled by ReportPanel directly — no dialog here
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
    // fetch content if we have it stored
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
    // get stored content
    QString content = files_.contains(id) ? files_[id].content : "";

    if (content.isEmpty()) {
        auto* box = new QMessageBox(this);
        box->setStyleSheet(msgBoxStyle());
        box->setWindowTitle("Download");
        box->setText("File content not available locally.\n"
                     "HTTP download will be added when the server serves files over HTTP.");
        box->exec();
        return;
    }

    QString savePath = QFileDialog::getSaveFileName(this, "Save File", filename);
    if (savePath.isEmpty()) return;

    QByteArray data = QByteArray::fromBase64(content.toUtf8());
    QFile file(savePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data);
        file.close();
        auto* box = new QMessageBox(this);
        box->setStyleSheet(msgBoxStyle());
        box->setWindowTitle("Downloaded");
        box->setText("File saved to:\n" + savePath);
        box->exec();
    }
}