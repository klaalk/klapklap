//
// Created by Klaus on 26/05/2019.
//

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <vector>
#include <list>

#include <QMainWindow>
#include <QMap>
#include <QPointer>
#include <QTextCursor>
#include <QLabel>
#include <QListWidgetItem>

#include "../../../../libs/src/classes/crdt/char/kk_char.h"
#include "../../../../libs/src/classes/crdt/pos/kk_pos.h"
#include "../../../../libs/src/classes/crdt/identifier/kk_identifier.h"
#include "../../../../libs/src/constants/kk_constants.h"
#include "../chat/kk_chat.h"
#include "./kk_textedit.h"
#include "./kk_cursor.h"

class QAction;
class QComboBox;
class QFontComboBox;
class QTextEdit;
class QTextCharFormat;
class QMenu;
class QPrinter;
class QLabel;

class KKEditor : public QMainWindow
{
    Q_OBJECT

signals:
    void insertTextToCrdt(unsigned long position, QList<QChar> values, QStringList fonts, QStringList colors);
    void removeTextFromCrdt(unsigned long start, unsigned long end, QString value);
    void saveCrdtTtoFile();
    void alignChange(int alignment, int alignStart, int alignEnd);
    void charFormatChange(unsigned long start, unsigned long end, QString font, QString color);
    void updateSiteIdsPositions(QString siteId);
    void openFileDialog();
    void editorClosed();
    void notifyAlignment(int startAling,int endAlign);
    void printCrdt();
public:
    KKEditor(QWidget *parent = nullptr);
    void clear();
    void loading(bool enable);
    void load(std::vector<std::list<KKCharPtr>> crdt, std::vector<int> alignments);
    void applyRemoteAlignmentChange(int alignment, int alignPos);
    void applyRemoteFormatChange(int position, QString siteId, QString font, QString color);
    void applyRemoteTextChange(const QString& operation, int position, const QString& siteId, const QChar& value, const QString& font, const QString& colorRecived);
    void applyRemoteCursorChange(const QString& siteId, int position);
    void applySiteIdsPositions(const QString& siteId, const QSharedPointer<QList<int>>& list);
    QBrush applySiteIdClicked(const QString& name);


    void updateCursors(QString siteId, int position, int value);
    void updateLabels();

    int getCurrentAlignment(int pos);
    void setLink(QString link);
    void setCurrentFileName(const QString &fileName);
    void setParticipantAlias(QStringList participants);
    void addParticipant(const QString &username, const QString &nick);
    void removeParticipant(const QString &username);
    void setChatDialog(KKChat *value);
    void setMySiteId(QString mySiteId);

    QString getMySiteId();
    KKTextEdit* getTextEdit();

    bool clickedOne(const QString& siteId);
    bool clickedAny();

public slots:
    void fileNew();

protected:
    void closeEvent(QCloseEvent *e) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void fileOpen();
    bool fileSave();
    bool fileSaveAs();
    void filePrint();
    void filePrintPreview();
    void filePrintPdf();

    void textBold();
    void textUnderline();
    void textItalic();
    void textFamily(const QString &f);
    void textSize(const QString &p);
    void textColor();
    void textAlign(QAction *a);

    void onAbout();
    void onPrintPreview(QPrinter *);
    void onClipboardDataChanged();
    void onFormatChanged(const QTextCharFormat &format);
    void onTextChange(QString operation, QString diff, int start, int end);
private:
    void resetState();
    void setupFileActions();
    void setupEditActions();
    void setupTextActions();

    void mergeFormat(const QTextCharFormat &format);
    void fontChanged(const QFont &f);
    void colorChanged(const QColor &c);
    void alignmentChanged(Qt::Alignment a);

    void colorText(const QString& siteId, QBrush color);

    void createCursorAndLabel(KKCursor*& remoteCurs, const QString& name, int postion);

    QBrush selectRandomColor();

    QAction *actionSave{};
    QAction *actionTextBold{};
    QAction *actionTextUnderline{};
    QAction *actionTextItalic{};
    QAction *actionTextColor{};
    QAction *actionAlignLeft{};
    QAction *actionAlignCenter{};
    QAction *actionAlignRight{};
    QAction *actionAlignJustify{};
    QAction *actionUndo{};
    QAction *actionRedo{};
#ifndef QT_NO_CLIPBOARD
    QAction *actionCut{};
    QAction *actionCopy{};
    QAction *actionPaste{};
#endif
    int fontSize=0;
    int maxFontSize=0;
    QString siteId;
    QString fileName;
    QString link;
    QMap <QString, KKCursor*> cursors;
    QFontComboBox *comboFont{};
    QComboBox *comboSize{};
    QToolBar *tb{};
    QMap<QString, QSharedPointer<QList<int>>> siteIdsPositions;
    QMap<QString, QBrush> siteIdsColors;
    QList<QString> siteIdsClicked;
    QMap<QString, QString> participantsAlias;
    QMovie* loaderGif;
    QLabel* loader;

    KKChat* chatDialog;
    KKTextEdit *textEdit;
    QList<QBrush> primaryColors_={QColor(255,179,216,255),
                                    QColor(240,98,146,71),
                                    QColor(140,158,255,127),
                                    QColor(143,207,255,255),
                                    QColor(29,233,182,96),
                                    QColor(255,235,59,153),
                                    QColor(245,124,0,140),
                                    QColor(255,106,116,214),
                                    QColor(172,255,154,255),
                                    QColor(0,214,127,201)
    };
    QList<QBrush> secondaryColors_={QColor(244,67,54,127),
                           QColor(156,39,176,71),
                           QColor(94,53,177,96),
                           QColor(21,101,192,89),
                           QColor(0,188,212,102),
                           QColor(0,150,136,107),
                           QColor(76,175,80,104),
                           QColor(156,204,101,99),
                           QColor(205,220,57,153),
                           QColor(255,193,7,165),
                           QColor(255,87,34,102),
                           QColor(121,85,72,114),
                           QColor(158,158,158,102),
                           QColor(3,169,244,76),
                           QColor(255,193,140,216),
                           QColor(96,125,139,102)
                          };

};
typedef QSharedPointer<KKEditor> textedit_ptr;
#endif // TEXTEDIT_H
