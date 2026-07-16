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

// ── single listing card widget ────────────────────────────────────────────────
class ListingCard : public QWidget {
    Q_OBJECT
public:
ListingCard(const QString& id, const QString& title,
const QString& price, const QString& seller,
const QString& description, const QString& imageData,
QWidget* parent = nullptr);
QString listingId() const { return id_; }
signals:
void clicked(const QString& id);
protected:
void mousePressEvent(QMouseEvent* event) override;
private:
    QString id_;
};

// ── post listing form panel ───────────────────────────────────────────────────
class PostListingPanel : public QWidget {
    Q_OBJECT
public:
explicit PostListingPanel(QWidget* parent = nullptr);
void setUser(const QString& displayName) { displayName_ = displayName; }
signals:
void submitted(const Message& msg);
void cancelled();
private slots:
void onSubmit();
void onChoosePhoto();
private:
    QLineEdit* title_;
    QLineEdit* price_;
    QLabel*    photoStatusLabel_;
    QTextEdit* description_;
    QString    displayName_;
    QString    pendingImageBase64_;
};

// ── listing detail panel ──────────────────────────────────────────────────────
class ListingDetailPanel : public QWidget {
    Q_OBJECT
public:
explicit ListingDetailPanel(QWidget* parent = nullptr);
void show(const QString& id, const QString& title, const QString& price,
const QString& seller, const QString& sellerId, const QString& description,
const QString& imageData);
signals:
void inquiryClicked(const Message& msg);
void backClicked();
private:
    QLabel*  img_;
    QLabel*  title_;
    QLabel*  price_;
    QLabel*  seller_;
    QTextEdit* description_;
    QString  currentId_;
    QString  currentSellerId_;
    QString  currentTitle_;
    QString  currentPrice_;
    QString  displayName_;
public:
void setUser(const QString& n) { displayName_ = n; }
};

// ── main marketplace panel ────────────────────────────────────────────────────
class MarketplacePanel : public QWidget {
    Q_OBJECT
public:
explicit MarketplacePanel(QWidget* parent = nullptr);
void setCurrentUser(const QString& displayName, const QString& userId);
void setToken(const QString& token) { token_ = token; }
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
void addCard(const QString& id, const QString& title,
const QString& price, const QString& seller,
const QString& sellerId, const QString& description,
const QString& imageData);
void clearGrid();

    QStackedWidget*    stack_;        // 0=browse, 1=post form, 2=detail
    QGridLayout*       grid_;
    QWidget*           gridWidget_;
    QScrollArea*       scrollArea_;
    QLineEdit*         searchBar_;
    PostListingPanel*  postPanel_;
    ListingDetailPanel* detailPanel_;

    // store all listings locally for search
struct ListingData {
        QString id, title, price, seller, sellerId, description, imageData;
    };
    QMap<QString, ListingData> listings_;

    QString displayName_;
    QString userId_;
    QString token_;
};