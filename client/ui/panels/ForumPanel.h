#pragma once
#include <QWidget>
#include <QStackedWidget>
#include <QMap>
#include <QVBoxLayout>
#include <QSet>    
#include "Message.h"

class QLabel;
class QLineEdit;
class QTextEdit;
class QPushButton;
class QScrollArea;

// ── faq panel ─────────────────────────────────────────────────────────────────
class FaqPanel : public QWidget {
    Q_OBJECT
public:
    explicit FaqPanel(QWidget* parent = nullptr);
    void addFaqQuestion(const QString& id, const QString& title, const QString& text,
                        const QString& author, const QString& timestamp,
                        int upvotes, int downvotes, int answerCount);
    void clear();
signals:
    void backClicked();
    void questionClicked  (const QString& id);
    void upvoteClicked    (const QString& id);
    void downvoteClicked  (const QString& id);
private:
    QVBoxLayout* layout_;
    QLabel*      emptyLabel_;
};


// ── question card (shown in list view) ───────────────────────────────────────
class QuestionCard : public QWidget {
    Q_OBJECT
public:
    QuestionCard(const QString& id, const QString& title, const QString& text,
                 const QString& author, const QString& timestamp,
                 int upvotes, int downvotes, int answerCount,
                 QWidget* parent = nullptr);
    void updateVotes(int upvotes, int downvotes);
    void updateAnswerCount(int count);
    QString questionId() const { return id_; }
signals:
    void clicked(const QString& id);
    void upvoteClicked  (const QString& id);
    void downvoteClicked(const QString& id);
protected:
    void mousePressEvent(QMouseEvent* event) override;  
private:
    QString  id_;
    QLabel*  votesLabel_;
    QLabel*  answerCountLabel_;
};

// ── answer widget (shown in question detail view) ─────────────────────────────
class AnswerWidget : public QWidget {
    Q_OBJECT
public:
    AnswerWidget(const QString& questionId, const QString& answerId,
                 const QString& author, const QString& text,
                 int upvotes, int downvotes, bool isFaq,
                 QWidget* parent = nullptr);
    void updateVotes(int upvotes, int downvotes);
    QString answerId() const { return answerId_; }
signals:
    void upvoteClicked  (const QString& questionId, const QString& answerId);
    void downvoteClicked(const QString& questionId, const QString& answerId);
private:
    QString  questionId_, answerId_;
    QLabel*  votesLabel_;
};

// ── post question panel ───────────────────────────────────────────────────────
class PostQuestionPanel : public QWidget {
    Q_OBJECT
public:
    explicit PostQuestionPanel(QWidget* parent = nullptr);
    void setUser(const QString& u) { username_ = u; }
signals:
    void submitted(const Message& msg);
    void cancelled();
private slots:
    void onSubmit();
private:
    QLineEdit* title_;
    QTextEdit* body_;
    QString    username_;
};

// ── question detail panel ─────────────────────────────────────────────────────
class QuestionDetailPanel : public QWidget {
    Q_OBJECT
public:
    explicit QuestionDetailPanel(QWidget* parent = nullptr);
    void load(const QString& qId, const QString& title, const QString& text,
              const QString& author, const QString& timestamp,
              int upvotes, int downvotes);
    void addAnswer(const QString& qId, const QString& aId,
                   const QString& author, const QString& text,
                   int upvotes, int downvotes, bool isFaq);
    void clearAnswers();
    void setUser(const QString& u) { username_ = u; }
    void updateAnswerVotes(const QString& answerId, int up, int down);
    QString currentQuestionId() const { return questionId_; }
signals:
    void backClicked();
    void answerSubmitted(const Message& msg);
    void upvoteQuestion  (const QString& qId);
    void downvoteQuestion(const QString& qId);
    void upvoteAnswer    (const QString& qId, const QString& aId);
    void downvoteAnswer  (const QString& qId, const QString& aId);
private:
    QLabel*      titleLabel_;
    QLabel*      metaLabel_;
    QLabel*      bodyLabel_;
    QLabel*      votesLabel_;
    QVBoxLayout* answersLayout_;
    QTextEdit*   answerInput_;
    QString      questionId_;
    QString      username_;
    QMap<QString, AnswerWidget*> answerWidgets_;
};


// ── main forum panel ──────────────────────────────────────────────────────────
class ForumPanel : public QWidget {
    Q_OBJECT
public:
    explicit ForumPanel(QWidget* parent = nullptr);
    void setCurrentUser(const QString& displayName, const QString& userId,
                        const QString& token);
    void receiveMessage(const Message& msg);
signals:
    void sendMessage(const Message& msg);
private slots:
    void onSearch(const QString& query);
    void showPost();
    void showBrowse();
    void showFaq();
    void onQuestionClicked(const QString& id);
    void requestQuestions();
private:
    void addQuestionCard(const QString& id, const QString& title,
                         const QString& text, const QString& author,
                         const QString& timestamp, int up, int down, int answers);
    void clearCards();
    void sendVote(const QString& qId, const QString& aId, bool upvote);

    QStackedWidget*      stack_;       // 0=browse, 1=post, 2=detail, 3=faq
    QVBoxLayout*         cardsLayout_;
    QWidget*             cardsWidget_;
    QScrollArea*         scrollArea_;
    QLineEdit*           searchBar_;
    QLabel*              emptyLabel_;
    PostQuestionPanel*   postPanel_;
    QuestionDetailPanel* detailPanel_;
    FaqPanel*            faqPanel_;

    struct QuestionData {
        QString id, title, text, author, timestamp;
        int upvotes=0, downvotes=0, answerCount=0;
    };
    QMap<QString, QuestionData>   questions_;
    QMap<QString, QuestionCard*>  cards_;
    QSet<QString>                 knownAnswerIds_; // prevents double-counting

    QString displayName_, userId_, token_;
};