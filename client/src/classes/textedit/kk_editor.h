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

#include "../../../../libs/src/classes/dtl/dtl.hpp"
#include "../../../../libs/src/classes/crdt/char/kk_char.h"
#include "../../../../libs/src/classes/crdt/pos/kk_pos.h"
#include "../../../../libs/src/classes/crdt/identifier/kk_identifier.h"
#include "../../../../libs/src/constants/kk_constants.h"
#include "../chat/chatdialog.h"
#include "./kk_textedit.h"

class QAction;
class QComboBox;
class QFontComboBox;
class QTextEdit;
class QTextCharFormat;
class QMenu;
class QPrinter;
class QLabel;

class KKCursor {
private:
    int globalPositon=0;
    int fontSize=0;


    QString backgorundColor;
    QString siteId;

    QLabel* name;
    QLabel* earpiece;

    void setLabelsStyleSheet(QString fontColor, int fontSize) {
        QString size=QString::number(fontSize);
        QString styleName;
        QString styleEarpiece;
        styleName = "background-color: " + fontColor + ";\n"
                                                       "font: 75 "+size+"pt \"Calibri\";\n"
                                                                        "font: bold;";

        styleEarpiece = "font: 75 "+size+"pt \"Calibri\";\n";
        //Aggiorno le label
        name->setStyleSheet(styleName);
        earpiece->setStyleSheet(styleEarpiece);
    }

public:
    KKCursor(QString siteId, int position): globalPositon(position), siteId(siteId){}

    void setLabels(QLabel *name_, QLabel *earpiece_) {
        name = name_;
        earpiece = earpiece_;
    }

    void setLabelsStyle(QBrush fontColor, int fontSize) {
        this->fontSize = fontSize;
        this->backgorundColor = fontColor.color().name(QColor::HexArgb);
        setLabelsStyleSheet(this->backgorundColor, this->fontSize);
    }

    void setLabelsSize(int fontSize) {
        this->fontSize = fontSize;
        setLabelsStyleSheet(this->backgorundColor, this->fontSize);
        name->adjustSize();
        earpiece->adjustSize();
    }

    void showLabels() {
        name->show();
        earpiece->show();
    }

    void moveLabels(QRect qRect) {
        name->move(qRect.x(),qRect.y()-static_cast<int>(1.15*fontSize));
        earpiece->move(qRect.x()-static_cast<int>(0.35*fontSize), qRect.y());
    }
    int getGlobalPositon() {
        return this->globalPositon;
    }

    void setGlobalPositon(int position) {
        this->globalPositon = position;
    }

    QString getLabelName(){
        return this->name->text();
    }

    QString getSiteId() {
        return siteId;
    }
};

class KKEditor : public QMainWindow
{
    Q_OBJECT

signals:
    void insertTextToCRDT(char value, unsigned long position, QString font_, QString color_);
    void removeTextFromCRDT(unsigned long start, unsigned long end);
    void saveCRDTtoFile();
    void alignChange(QString alignment);
    void charFormatChange(unsigned long pos, QString font_, QString color_);
    void updateSiteIdsPositions(QString siteId);
    void openFileDialog();
    void editorClosed();

public:
    KKEditor(QWidget *parent = nullptr);
    bool load(const QString &f);
    void loadCrdt(std::vector<std::list<KKCharPtr>> crdt);
    void applyRemoteAlignmentChange(QString alignment);
    void applyRemoteFormatChange(int position, QString font, QString color);
    void applyRemoteChanges(const QString& operation, const QString& name, const QString& text, int globalPos,const QString& font, const QString& colorRecived);
    void applySiteIdsPositions(const QString& siteId, const QSharedPointer<QList<int>>& list);
    void applySiteIdClicked(const QString& name);

    void setCurrentFileName(const QString &fileName);
    void setParticipantAlias(QStringList participants);
    void addParticipant(const QString &username, const QString &nick);
    void removeParticipant(const QString &username);
    void setChatDialog(ChatDialog *value);
    void setMySiteId(QString mySiteId);

    QString getMySiteId();
    KKTextEdit* getTextEdit();

    bool clickedOne(const QString& siteId);
    bool clickedAny();

    KKTextEdit *textEdit;
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
    void textStyle(int styleIndex);
    void textColor();
    void textAlign(QAction *a);

    void onAbout();
    void onPrintPreview(QPrinter *);
    void onClipboardDataChanged();
    void onFormatChanged(const QTextCharFormat &format);
    void onCursorPositionChanged();
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

    void colorText(const QString& siteId);
    void clearColorText(const QString& name);

    void updateCursors(QString siteId, int position, int value);
    void updateLabels();
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
    bool isColored = false;
    bool blockCursor=false;
    int fontSize=0;
    int maxFontSize=0;
    QString siteId;
    QString fileName;
    QMap <QString,KKCursor*> cursors;
    QComboBox *comboStyle{};
    QFontComboBox *comboFont{};
    QComboBox *comboSize{};
    QToolBar *tb{};
    ChatDialog* chatDialog;
    QMap<QString, QSharedPointer<QList<int>>> siteIdsPositions;
    QMap<QString, QBrush> siteIdsColors;
    QList<QString> siteIdsClicked;
    QMap<QString, QString> participantsAlias;
    QList<QBrush> colors_={QColor(244,67,54,127),
                           QColor(240,98,146,71),
                           QColor(156,39,176,71),
                           QColor(94,53,177,96),
                           QColor(140,158,255,127),
                           QColor(21,101,192,89),
                           QColor(3,169,244,76),
                           QColor(0,188,212,102),
                           QColor(29,233,182,96),
                           QColor(0,150,136,107),
                           QColor(76,175,80,104),
                           QColor(156,204,101,99),
                           QColor(205,220,57,153),
                           QColor(255,235,59,153),
                           QColor(245,124,0,140),
                           QColor(255,193,7,165),
                           QColor(255,87,34,102),
                           QColor(121,85,72,114),
                           QColor(158,158,158,102),
                           QColor(96,125,139,102)
                          };

};
typedef QSharedPointer<KKEditor> textedit_ptr;
#endif // TEXTEDIT_H
