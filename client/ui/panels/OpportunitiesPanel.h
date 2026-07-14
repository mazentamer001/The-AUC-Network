#pragma once
#include <QWidget>
#include <QStackedWidget>
#include <QMap>
#include "Message.h"

class QLineEdit;
class QTextEdit;
class QLabel;
class QScrollArea;
class QGridLayout;
class QPushButton;
class QComboBox;

// ── single opportunity card widget ────────────────────────────────────────────
class OpportunityCard : public QWidget {
    Q_OBJECT
public:
    OpportunityCard(const QString& id, const QString& title,
                     const QString& category, const QString& poster,
                     const QString& description, QWidget* parent = nullptr);
    QString opportunityId() const { return id_; }
signals:
    void clicked(const QString& id);
protected:
    void mousePressEvent(QMouseEvent* event) override;
private:
    QString id_;
};

// ── post opportunity form panel ───────────────────────────────────────────────
class PostOpportunityPanel : public QWidget {
    Q_OBJECT
public:
    explicit PostOpportunityPanel(QWidget* parent = nullptr);
    void setUser(const QString& displayName) { displayName_ = displayName; }
signals:
    void submitted(const Message& msg);
    void cancelled();
private slots:
    void onSubmit();
private:
    QLineEdit*  title_;
    QComboBox*  category_;
    QLineEdit*  location_;
    QLineEdit*  mediaUrl_;
    QTextEdit*  description_;
    QString     displayName_;
};

// ── opportunity detail panel ──────────────────────────────────────────────────
class OpportunityDetailPanel : public QWidget {
    Q_OBJECT
public:
    explicit OpportunityDetailPanel(QWidget* parent = nullptr);
    void show(const QString& id, const QString& title, const QString& category,
              const QString& location, const QString& poster, const QString& posterId,
              const QString& description);
signals:
    void inquiryClicked(const Message& msg);
    void backClicked();
private:
    QLabel*    title_;
    QLabel*    category_;
    QLabel*    poster_;
    QTextEdit* description_;
    QString    currentId_;
    QString    currentPosterId_;
    QString    currentTitle_;
    QString    displayName_;
public:
    void setUser(const QString& n) { displayName_ = n; }
};

// ── main opportunities panel ──────────────────────────────────────────────────
class OpportunitiesPanel : public QWidget {
    Q_OBJECT
public:
    explicit OpportunitiesPanel(QWidget* parent = nullptr);
    void setCurrentUser(const QString& displayName, const QString& userId);
    void setToken(const QString& token);
    void receiveMessage(const Message& msg);

signals:
    void sendMessage(const Message& msg);
    void openRoom(const QString& roomId);

private slots:
    void onSearch();
    void onCardClicked(const QString& id);
    void showPostForm();
    void showBrowse();

private:
    void addCard(const QString& id, const QString& title, const QString& category,
                 const QString& location, const QString& poster, const QString& posterId,
                 const QString& description);
    void clearGrid();

    QStackedWidget*         stack_;        // 0=browse, 1=post form, 2=detail
    QGridLayout*             grid_;
    QWidget*                 gridWidget_;
    QScrollArea*             scrollArea_;
    QLineEdit*               searchBar_;
    PostOpportunityPanel*    postPanel_;
    OpportunityDetailPanel*  detailPanel_;

    struct OpportunityData {
        QString id, title, category, location, poster, posterId, description;
    };
    QMap<QString, OpportunityData> opportunities_;

    QString displayName_;
    QString userId_;
    QString token_;
};