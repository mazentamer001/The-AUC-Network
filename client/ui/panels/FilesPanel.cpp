#include "FilesPanel.h"
#include "ui/theme/Theme.h"
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

static QString fileIcon(const QString& name) {
    QString ext = QFileInfo(name).suffix().toLower();
    if (ext=="pdf") return "PDF";
    if (ext=="png"||ext=="jpg"||ext=="jpeg"||ext=="gif") return "IMG";
    if (ext=="mp4"||ext=="avi"||ext=="mov") return "VID";
    if (ext=="mp3"||ext=="wav") return "AUD";
    if (ext=="zip"||ext=="rar") return "ZIP";
    if (ext=="doc"||ext=="docx") return "DOC";
    if (ext=="ppt"||ext=="pptx") return "PPT";
    if (ext=="xls"||ext=="xlsx") return "XLS";
    if (ext=="cpp"||ext=="h"||ext=="py"||ext=="js"||ext=="txt"||ext=="md") return "SRC";
    return "FILE";
}

static bool isTextFile(const QString& name) {
    QString ext = QFileInfo(name).suffix().toLower();
    return ext=="txt"||ext=="md"||ext=="cpp"||ext=="h"||ext=="py"
          ||ext=="js"||ext=="json"||ext=="html"||ext=="css"||ext=="xml";
}

static QString smallBtnStyle(bool danger = false) {
    QString color = danger ? Theme::DANGER : Theme::TEXT_SECONDARY;
    return QString(
        "QPushButton { background: transparent; color: %1; border: 1px solid %2; border-radius: 6px; padding: 4px 10px; font-size: 11px; font-weight: 500; }"
        "QPushButton:hover { background: %3; }"
    ).arg(color, Theme::BORDER, Theme::SURFACE_ALT);
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
    setFixedHeight(52);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString(
        "FileRow { background: transparent; border-bottom: 1px solid %1; }"
        "FileRow:hover { background: %2; }"
    ).arg(Theme::BORDER, Theme::SURFACE_ALT));

    auto* l = new QHBoxLayout(this);
    l->setContentsMargins(16, 0, 16, 0);
    l->setSpacing(14);

    auto* icon = new QLabel(fileIcon(filename));
    icon->setFixedSize(48, 28);
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet(QString(
        "background: %1; color: %2; border-radius: 6px; font-size: 10px; font-weight: 600;"
    ).arg(Theme::SURFACE_ALT, Theme::ACCENT));

    auto* name = new QLabel(filename);
    name->setStyleSheet(QString("border: none; background: transparent; color: %1; font-size: 13px; font-weight: 500;").arg(Theme::TEXT_PRIMARY));
    name->setMinimumWidth(180);

    auto* upl = new QLabel(uploader);
    upl->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));
    upl->setMinimumWidth(110);

    auto* sz = new QLabel(size);
    sz->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));
    sz->setFixedWidth(70);

    auto* dt = new QLabel(timestamp.left(10));
    dt->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));
    dt->setFixedWidth(90);

    auto* btnView     = new QPushButton("View");
    auto* btnDownload = new QPushButton("Download");
    auto* btnReport   = new QPushButton("Report");

    btnView->setFixedHeight(28);
    btnDownload->setFixedHeight(28);
    btnReport->setFixedHeight(28);

    btnView->setStyleSheet(smallBtnStyle());
    btnDownload->setStyleSheet(smallBtnStyle());
    btnReport->setStyleSheet(smallBtnStyle(true));

    l->addWidget(icon);
    l->addWidget(name, 2);
    l->addWidget(upl, 1);
    l->addWidget(sz);
    l->addWidget(dt);
    l->addWidget(btnView);
    l->addWidget(btnDownload);
    l->addWidget(btnReport);

    connect(btnView,     &QPushButton::clicked, this, [this](){ emit viewClicked(id_, url_, filename_, content_); });
    connect(btnDownload, &QPushButton::clicked, this, [this](){ emit downloadClicked(id_, url_, filename_); });
    connect(btnReport,   &QPushButton::clicked, this, [this](){ emit reportClicked(id_, filename_); });
}

// ─────────────────────────────────────────────────────────────────────────────
//  ViewPanel
// ─────────────────────────────────────────────────────────────────────────────
ViewPanel::ViewPanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("ViewPanel { %1 }").arg(Theme::pageBackground()));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(40, 32, 40, 32);
    root->setSpacing(12);

    auto* btnBack = new QPushButton("Back to files");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(QString(
        "QPushButton { background: transparent; color: %1; border: none; font-size: 12px; }"
        "QPushButton:hover { color: %2; }"
    ).arg(Theme::TEXT_SECONDARY, Theme::ACCENT));

    iconLabel_ = new QLabel;
    iconLabel_->setAlignment(Qt::AlignCenter);
    iconLabel_->setStyleSheet(QString("border: none; background: transparent; color: %1; font-size: 14px; font-weight: 600;").arg(Theme::ACCENT));

    nameLabel_ = new QLabel;
    nameLabel_->setAlignment(Qt::AlignCenter);
    nameLabel_->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::heading()));

    urlLabel_ = new QLabel;
    urlLabel_->setAlignment(Qt::AlignCenter);
    urlLabel_->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));

    contentView_ = new QTextEdit;
    contentView_->setReadOnly(true);
    contentView_->setStyleSheet(Theme::textArea());

    previewLabel_ = new QLabel("Binary file — download to view");
    previewLabel_->setAlignment(Qt::AlignCenter);
    previewLabel_->setStyleSheet(QString(
        "background: %1; border: 1px solid %2; border-radius: 12px; padding: 40px; color: %3; font-size: 14px;"
    ).arg(Theme::SURFACE_ALT, Theme::BORDER, Theme::TEXT_SECONDARY));

    root->addWidget(btnBack, 0, Qt::AlignLeft);
    root->addSpacing(8);
    root->addWidget(iconLabel_);
    root->addWidget(nameLabel_);
    root->addWidget(urlLabel_);
    root->addSpacing(8);
    root->addWidget(contentView_, 1);
    root->addWidget(previewLabel_, 1);

    connect(btnBack, &QPushButton::clicked, this, &ViewPanel::backClicked);
}

void ViewPanel::load(const QString& filename, const QString& content, const QString& url)
{
    iconLabel_->setText(fileIcon(filename));
    nameLabel_->setText(filename);
    urlLabel_->setText(url);

    if (isTextFile(filename) && !content.isEmpty()) {
        QByteArray decoded = QByteArray::fromBase64(content.toUtf8());
        contentView_->setPlainText(QString::fromUtf8(decoded));
        contentView_->setVisible(true);
        previewLabel_->setVisible(false);
    } else {
        contentView_->setVisible(false);
        previewLabel_->setText(filename + "\n\nDownload to view this file.");
        previewLabel_->setVisible(true);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  ReportPanel
// ─────────────────────────────────────────────────────────────────────────────
ReportPanel::ReportPanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("ReportPanel { %1 }").arg(Theme::pageBackground()));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(40, 32, 40, 32);
    root->setAlignment(Qt::AlignTop);

    auto* btnBack = new QPushButton("Back to files");
    btnBack->setFlat(true);
    btnBack->setStyleSheet(QString(
        "QPushButton { background: transparent; color: %1; border: none; font-size: 12px; }"
        "QPushButton:hover { color: %2; }"
    ).arg(Theme::TEXT_SECONDARY, Theme::ACCENT));

    auto* card = new QWidget;
    card->setObjectName("reportCard");
    card->setMaximumWidth(560);
    card->setStyleSheet(QString("#reportCard { %1 }").arg(Theme::card()));

    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(32, 28, 32, 28);
    cardLayout->setSpacing(6);

    auto* title = new QLabel("Report this file");
    title->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::heading()));

    fileLabel_ = new QLabel;
    fileLabel_->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));

    auto lbl = [](const QString& t) {
        auto* l = new QLabel(t);
        l->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));
        return l;
    };

    auto* category = new QComboBox;
    category->addItems({"Inappropriate content", "Copyright violation", "Spam or misleading", "Malware or harmful", "Other"});
    category->setFixedHeight(38);
    category->setStyleSheet(QString(
        "QComboBox { background: %1; color: %2; border: 1px solid %3; border-radius: 8px; padding: 6px 12px; font-size: 13px; }"
        "QComboBox QAbstractItemView { background: %1; color: %2; border: 1px solid %3; selection-background-color: %4; }"
    ).arg(Theme::SURFACE_ALT, Theme::TEXT_PRIMARY, Theme::BORDER, Theme::ACCENT));

    reasonInput_ = new QLineEdit;
    reasonInput_->setPlaceholderText("Brief summary");
    reasonInput_->setFixedHeight(38);
    reasonInput_->setStyleSheet(Theme::textInput());

    detailsInput_ = new QTextEdit;
    detailsInput_->setPlaceholderText("Additional context (optional)");
    detailsInput_->setFixedHeight(100);
    detailsInput_->setStyleSheet(Theme::textArea());

    auto* btnRow = new QHBoxLayout;
    auto* btnCancel = new QPushButton("Cancel");
    auto* btnSubmit = new QPushButton("Submit report");

    btnCancel->setFixedHeight(40);
    btnSubmit->setFixedHeight(40);

    btnCancel->setStyleSheet(Theme::secondaryButton());
    btnSubmit->setStyleSheet(QString(
        "QPushButton { background: %1; color: white; border: none; border-radius: 8px; padding: 10px 20px; font-size: 13px; font-weight: 500; }"
        "QPushButton:hover { background: %1; }"
    ).arg(Theme::DANGER));

    btnRow->addWidget(btnCancel);
    btnRow->addWidget(btnSubmit);

    cardLayout->addWidget(title);
    cardLayout->addWidget(fileLabel_);
    cardLayout->addSpacing(12);
    cardLayout->addWidget(lbl("Category"));
    cardLayout->addWidget(category);
    cardLayout->addSpacing(8);
    cardLayout->addWidget(lbl("Reason"));
    cardLayout->addWidget(reasonInput_);
    cardLayout->addSpacing(8);
    cardLayout->addWidget(lbl("Details"));
    cardLayout->addWidget(detailsInput_);
    cardLayout->addSpacing(16);
    cardLayout->addLayout(btnRow);

    root->addWidget(btnBack, 0, Qt::AlignLeft);
    root->addSpacing(16);
    root->addWidget(card);

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
    fileLabel_->setText("Reporting: " + filename);
}

// ─────────────────────────────────────────────────────────────────────────────
//  FilesPanel
// ─────────────────────────────────────────────────────────────────────────────
FilesPanel::FilesPanel(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QString("FilesPanel { %1 }").arg(Theme::pageBackground()));

    stack_ = new QStackedWidget(this);
    stack_->setStyleSheet("background: transparent;");

    auto* browsePage = new QWidget;
    browsePage->setStyleSheet("background: transparent;");

    auto* browseLayout = new QVBoxLayout(browsePage);
    browseLayout->setContentsMargins(40, 32, 40, 28);
    browseLayout->setSpacing(16);

    auto* topBar = new QHBoxLayout;
    auto* pageTitle = new QLabel("Course materials");
    pageTitle->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::heading()));

    searchBar_ = new QLineEdit;
    searchBar_->setPlaceholderText("Search files...");
    searchBar_->setFixedHeight(38);
    searchBar_->setMaximumWidth(280);
    searchBar_->setStyleSheet(Theme::textInput());

    auto* btnUpload = new QPushButton("Upload file");
    btnUpload->setFixedHeight(38);
    btnUpload->setStyleSheet(Theme::primaryButton());

    topBar->addWidget(pageTitle);
    topBar->addStretch();
    topBar->addWidget(searchBar_);
    topBar->addSpacing(12);
    topBar->addWidget(btnUpload);

    auto* header = new QWidget;
    header->setFixedHeight(32);
    header->setStyleSheet(QString("background: transparent; border-bottom: 1px solid %1;").arg(Theme::BORDER));

    auto* hLayout = new QHBoxLayout(header);
    hLayout->setContentsMargins(62, 0, 14, 0);
    hLayout->setSpacing(14);

    auto makeHdr = [](const QString& t) {
        auto* l = new QLabel(t);
        l->setStyleSheet(QString("border: none; background: transparent; color: %1; font-size: 11px; font-weight: 600;").arg(Theme::TEXT_SECONDARY));
        return l;
    };

    hLayout->addWidget(makeHdr("Name"), 2);
    hLayout->addWidget(makeHdr("Uploaded by"), 1);
    hLayout->addWidget(makeHdr("Size"));
    hLayout->addWidget(makeHdr("Date"));
    hLayout->addSpacing(230);

    scrollArea_ = new QScrollArea;
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setStyleSheet(QString(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { background: transparent; width: 6px; }"
        "QScrollBar::handle:vertical { background: %1; border-radius: 3px; }"
    ).arg(Theme::BORDER));

    listWidget_ = new QWidget;
    listWidget_->setStyleSheet("background: transparent;");

    listLayout_ = new QVBoxLayout(listWidget_);
    listLayout_->setContentsMargins(0, 0, 0, 0);
    listLayout_->setSpacing(0);
    listLayout_->setAlignment(Qt::AlignTop);

    emptyLabel_ = new QLabel("No files yet.");
    emptyLabel_->setAlignment(Qt::AlignCenter);
    emptyLabel_->setStyleSheet(QString("border: none; background: transparent; %1").arg(Theme::mutedText()));
    emptyLabel_->setContentsMargins(0, 40, 0, 40);

    listLayout_->addWidget(emptyLabel_);
    scrollArea_->setWidget(listWidget_);

    browseLayout->addLayout(topBar);
    browseLayout->addWidget(header);
    browseLayout->addWidget(scrollArea_, 1);

    viewPanel_   = new ViewPanel;
    reportPanel_ = new ReportPanel;

    stack_->addWidget(browsePage);
    stack_->addWidget(viewPanel_);
    stack_->addWidget(reportPanel_);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(stack_);

    connect(btnUpload,  &QPushButton::clicked,   this, &FilesPanel::onUpload);
    connect(searchBar_, &QLineEdit::textChanged, this, &FilesPanel::onSearch);
    connect(viewPanel_,   &ViewPanel::backClicked,   this, &FilesPanel::showBrowse);
    connect(reportPanel_, &ReportPanel::cancelled,   this, &FilesPanel::showBrowse);
    connect(reportPanel_, &ReportPanel::submitted,   this, [this](const Message& msg){
        emit sendMessage(msg);
        showBrowse();
        QMessageBox::information(this, "Report submitted", "Thanks — this file has been flagged for review.");
    });
}

void FilesPanel::setCurrentUser(const QString& displayName, const QString& userId, const QString& token)
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

void FilesPanel::onUpload()
{
    QString path = QFileDialog::getOpenFileName(this, "Select file to upload");
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Read error", "Could not read the selected file.");
        return;
    }

    QByteArray fileData = file.readAll();
    if (fileData.size() > 5 * 1024 * 1024) {
        QMessageBox::warning(this, "File too large", "Files must be under 5MB.");
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
        if (q.isEmpty() || f.filename.toLower().contains(q) || f.uploader.toLower().contains(q)) {
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

void FilesPanel::downloadFile(const QString& id, const QString& url, const QString& filename)
{
    QString content = files_.contains(id) ? files_[id].content : "";

    if (content.isEmpty()) {
        QMessageBox::warning(this, "Not available", "This file is not cached locally yet.");
        return;
    }

    QString savePath = QFileDialog::getSaveFileName(this, "Save file as", filename);
    if (savePath.isEmpty()) return;

    QByteArray data = QByteArray::fromBase64(content.toUtf8());
    QFile file(savePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data);
        file.close();
        QMessageBox::information(this, "Saved", "File saved to:\n" + savePath);
    }
}
