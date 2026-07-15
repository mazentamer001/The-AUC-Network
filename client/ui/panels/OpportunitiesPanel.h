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

// ── single opportunity card widget ─────────────────────────────────────────
class OpportunityCard : public QWidget {
    Q_OBJECT
public:
    OpportunityCard(const QString& id, const QString& title,
                     const QString& organization, const QString& oppType,
                     const QString& deadline, const QString& description,
                     QWidget* parent = nullptr);
    QString opportunityId() const { return id_; }
signals:
    void clicked(const QString& id);
protected:
    void mousePressEvent(QMouseEvent* event) override;
private:
    QString id_;
};

// ── post opportunity form panel ─────────────────────────────────────────────
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
    QLineEdit* title_;
    QLineEdit* organization_;
    QComboBox* type_;
    QLineEdit* deadline_;
    QLineEdit* contactInfo_;
    QTextEdit* description_;
    QString    displayName_;
};

// ── opportunity detail panel ────────────────────────────────────────────────
class OpportunityDetailPanel : public QWidget {
    Q_OBJECT
public:
    explicit OpportunityDetailPanel(QWidget* parent = nullptr);
    void show(const QString& id, const QString& title, const QString& organization,
              const QString& oppType, const QString& deadline,
              const QString& posterId, const QString& description);
signals:
    void applyClicked(const Message& msg);
    void backClicked();
private:
    QLabel*  title_;
    QLabel*  organization_;
    QLabel*  deadline_;
    QTextEdit* description_;
    QString  currentId_;
    QString  currentPosterId_;
    QString  currentTitle_;
    QString  displayName_;
public:
    void setUser(const QString& n) { displayName_ = n; }
};

// ── main opportunities panel ────────────────────────────────────────────────
class OpportunitiesPanel : public QWidget {
    Q_OBJECT
public:
    explicit OpportunitiesPanel(QWidget* parent = nullptr);
    void setCurrentUser(const QString& displayName, const QString& userId);
    void setToken(const QString& token) { token_ = token; }
    void receiveMessage(const Message& msg);

signals:
    void sendMessage(const Message& msg);

private slots:
    void onSearch();
    void onCardClicked(const QString& id);
    void showPostForm();
    void showBrowse();

private:
    void addCard(const QString& id, const QString& title, const QString& organization,
                 const QString& oppType, const QString& deadline,
                 const QString& posterId, const QString& description);
    void clearGrid();

    QStackedWidget*    stack_;
    QGridLayout*       grid_;
    QWidget*           gridWidget_;
    QScrollArea*       scrollArea_;
    QLineEdit*         searchBar_;
    PostOpportunityPanel*   postPanel_;
    OpportunityDetailPanel* detailPanel_;

    struct OpportunityData {
        QString id, title, organization, oppType, deadline, posterId, description;
    };
    QMap<QString, OpportunityData> opportunities_;

    QString displayName_;
    QString userId_;
    QString token_;
};

