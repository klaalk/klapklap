//
// Created by Klaus on 26/05/2019.
//

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QMainWindow>
#include <QMap>
#include <QPointer>
#include <QTextCursor>
#include <QLabel>
#include <QListWidgetItem>

#include "../../../../libs/src/constants/kk_constants.h"

class QAction;
class QComboBox;
class QFontComboBox;
class QTextEdit;
class QTextCharFormat;
class QMenu;
class QPrinter;
class QLabel;

class kk_cursor{
private:
    int globalPositon=0;
    int fontSize=0;


    QString backgorundColor;

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
    kk_cursor(int position): globalPositon(position){}
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
        name->move(qRect.x(),qRect.y()-fontSize);
        earpiece->move(qRect.x()-1, qRect.y());
    }
    int getGlobalPositon() {
        return this->globalPositon;
    }
    void setGlobalPositon(int position) {
        this->globalPositon = position;
    }
   // DA TOGLIERE
//    int getSelectionStart(){
//        return this->selection_start;
//      }

//    int getSelectionEnd(){
//        return this->selection_end;
//      }

//    void setSelectionStart(int start){
//        this->selection_start=start;
//      }

//    void setSelectionEnd(int end){
//        this->selection_end=end;
//      }
};

//DEPRECATED
//class kk_couple{
//private:
//    int startPos=0;
//    int endPos=0;

//public:
//    kk_couple(int startPos, int endPos): startPos(startPos), endPos(endPos){}

//    int getStartPos() {
//        return this->startPos;
//    }
//    void setStartPos(int position) {
//        this->startPos = position;
//    }
//    int getEndPos() {
//        return this->endPos;
//    }
//    void setEndPos(int position) {
//        this->endPos = position;
//    }
//};

class TextEdit : public QMainWindow
{
Q_OBJECT

public:
    bool getIfIsClicked(QString siteId);
    void setCurrentFileName(const QString &fileName);
    TextEdit(QWidget *parent = nullptr);
    QTextEdit *textEdit;
    bool load(const QString &f);
    void resetState();
    void applyRemoteChanges(QString operation, QString name, QString text, int globalPos,QString font,QString colorRecived);
    void movekk_cursor(int targetCol, int targetLine, int line, QTextCursor *curs);
    void modifyLabels();
    void updateSiteIdsMap(QString siteId, QSharedPointer<QList<int>> list);
    void siteIdClicked(QString name);
    void setMySiteId(QString mySiteId);
    QTextEdit* getTextEdit();
signals:
    void insertTextToCRDT(QString text, int position);
    void removeTextFromCRDT(int start, int end);
    void saveCRDTtoFile();
    void loadCRDTtoFile();

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

    void currentCharFormatChanged(const QTextCharFormat &format);
    void cursorPositionChanged();

    void clipboardDataChanged();
    void about();
    void printPreview(QPrinter *);
    void onTextChange();

private:
    void setupFileActions();
    void setupEditActions();
    void setupTextActions();
    bool maybeSave();


    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void fontChanged(const QFont &f);
    void colorChanged(const QColor &c);
    void alignmentChanged(Qt::Alignment a);

    void colorText(QString siteId);
    void clearColorText(QString name);

    QAction *actionSave;
    QAction *actionTextBold;
    QAction *actionTextUnderline;
    QAction *actionTextItalic;
    QAction *actionTextColor;
    QAction *actionAlignLeft;
    QAction *actionAlignCenter;
    QAction *actionAlignRight;
    QAction *actionAlignJustify;
    QAction *actionUndo;
    QAction *actionRedo;
#ifndef QT_NO_CLIPBOARD
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
#endif
    bool blockCursor = false;
    bool isTextSelected = false;
    int lastLength = 0;
    int cursorPos=0;
    int lastCursorPos=0;
    int fontSize=0;
    int selection_start=0;
    int selection_end=0;
    QString lastText;
    QString mySiteId_;
    QString diffText;
    QString fileName;
    QMap <QString,kk_cursor*> cursors_;
    QComboBox *comboStyle;
    QFontComboBox *comboFont;
    QComboBox *comboSize;
    QToolBar *tb;
    QMap<QString,QSharedPointer<QList<int>>> siteIds_;
    QMap<QString,QBrush> siteIdsColors_;
    QList<QString> siteIdsClicked_;
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
typedef QSharedPointer<TextEdit> textedit_ptr;
#endif // TEXTEDIT_H
