#ifndef KK_CURSOR_H
#define KK_CURSOR_H

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



class KKCursor {
private:
    int globalPositon=0;
    int fontSize=0;

    QString backgorundColor;
    QString siteId;

    QLabel* name;
    QLabel* earpiece;

    void setLabelsStyleSheet(QString fontColor, int fontSize) {
        QString size=QString::number(1.15*fontSize);
        QString styleName;
        QString styleEarpiece;
        styleEarpiece = "font: 75 "+size+"pt \"Calibri\";\n";
        styleName = "background-color: " + fontColor + ";\nfont: 10pt \"Calibri\";\nfont: bold;";

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
#ifndef Q_OS_MACOS
        earpiece->move(qRect.x()-static_cast<int>(0.5*fontSize), qRect.y()-static_cast<int>(0.35*fontSize));
        name->move(qRect.x()-static_cast<int>(0.1*fontSize), qRect.y()-17);
#else
         earpiece->move(qRect.x()-static_cast<int>(0.5*fontSize), qRect.y()-static_cast<int>(0.2*fontSize));
         name->move(qRect.x(),qRect.y()-10);
#endif
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

#endif // KK_CURSOR_H
