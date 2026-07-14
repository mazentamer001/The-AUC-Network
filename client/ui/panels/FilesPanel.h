#pragma once
#include <QWidget>
#include <QMap>
#include <QStackedWidget>
#include "Message.h"

class QVBoxLayout;
class QLabel;
class QLineEdit;
class QScrollArea;
class QTextEdit;
class QPushButton;

// ── single file row ───────────────────────────────────────────────────────────
class FileRow : public QWidget {
    Q_OBJECT
public:
    FileRow(const QString& id, const QString& filename,
            const QString& uploader, const QString& size,
            const QString& timestamp, const QString& url,
            const QString& content,
            QWidget* parent = nullptr);
    QString fileId() const { return id_; }
signals:
    void viewClicked    (const QString& id, const QString& url,
                         const QString& filename, const QString& content);
    void downloadClicked(const QString& id, const QString& url, const QString& filename);
    void reportClicked  (const QString& id, const QString& filename);
private:
    QString id_, url_, filename_, content_;
};

// ── report panel ──────────────────────────────────────────────────────────────
class ReportPanel : public QWidget {
    Q_OBJECT
public:
    explicit ReportPanel(QWidget* parent = nullptr);
    void setFile(const QString& id, const QString& filename);
signals:
    void submitted(const Message& msg);
    void cancelled();
public:
    void setToken(const QString& t)       { token_ = t; }
    void setUser (const QString& u)       { username_ = u; }
private:
    QLabel* fileLabel_;
    QLineEdit* reasonInput_;
    QTextEdit* detailsInput_;
    QLineEdit* categoryInput_;
    QString    fileId_, token_, username_;
};

// ── view panel ────────────────────────────────────────────────────────────────
class ViewPanel : public QWidget {
    Q_OBJECT
public:
    explicit ViewPanel(QWidget* parent = nullptr);
    void load(const QString& filename, const QString& content, const QString& url);
signals:
    void backClicked();
private:
    QLabel* iconLabel_;
    QLabel* nameLabel_;
    QLabel* urlLabel_;
    QTextEdit* contentView_;
    QLabel* previewLabel_;
};

// ── files panel ───────────────────────────────────────────────────────────────
class FilesPanel : public QWidget {
    Q_OBJECT
public:
    explicit FilesPanel(QWidget* parent = nullptr);

    void setCurrentUser(const QString& displayName, const QString& userId,
                        const QString& token);
    void receiveMessage(const Message& msg);

signals:
    void sendMessage(const Message& msg);

private slots:
    void onUpload();
    void onSearch(const QString& query);

private:
    void addFileRow(const QString& id, const QString& filename,
                    const QString& uploader, const QString& size,
                    const QString& timestamp, const QString& url,
                    const QString& content = "");
    void clearList();
    void requestFileList();
    void showView    (const QString& id, const QString& url,
                      const QString& filename, const QString& content);
    void showReport  (const QString& id, const QString& filename);
    void downloadFile(const QString& id, const QString& url, const QString& filename);
    void showBrowse  ();

    QStackedWidget* stack_;          // 0=browse, 1=view, 2=report
    QVBoxLayout* listLayout_;
    QWidget* listWidget_;
    QScrollArea* scrollArea_;
    QLineEdit* searchBar_;
    QLabel* emptyLabel_;
    ViewPanel* viewPanel_;
    ReportPanel* reportPanel_;

    struct FileData {
        QString id, filename, uploader, size, timestamp, url, content;
    };
    QMap<QString, FileData> files_;

    QString displayName_, userId_, token_;
    QMap<QString, QString> pendingContent_; // filename → base64, cached on upload
};