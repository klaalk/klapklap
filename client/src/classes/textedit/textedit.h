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

    void setLabelsStyle(QString fontColor, int fontSize) {
        this->fontSize = fontSize;
        this->backgorundColor = fontColor;
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
};

class TextEdit : public QMainWindow
{
Q_OBJECT

public:
    TextEdit(QWidget *parent = 0);
    QTextEdit *textEdit;
    bool load(const QString &f);
    void applyRemoteChanges(QString operation, QString name, QString text, int line, int col);
    void movekk_cursor(int targetCol, int targetLine, int line, QTextCursor *curs);
    void modifyLabels();
signals:
    void insertTextToCRDT(QString text, int line, int col);
    void removeTextFromCRDT(int startLine, int startCol, int endLine, int endCol);

public slots:
    void fileNew();

protected:
    void closeEvent(QCloseEvent *e) override;
    void resizeEvent(QResizeEvent* event);
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
    void setCurrentFileName(const QString &fileName);

    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void fontChanged(const QFont &f);
    void colorChanged(const QColor &c);
    void alignmentChanged(Qt::Alignment a);

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
    int lastLength = 0, cursorPos=0, lastCursorPos=0, fontSize=0;
    QMap <QString,kk_cursor*> cursors_;
    QComboBox *comboStyle;
    QFontComboBox *comboFont;
    QComboBox *comboSize;
    QString lastText="",diffText="",fileName;
    QToolBar *tb;
};
typedef QSharedPointer<TextEdit> textedit_ptr;
#endif // TEXTEDIT_H
