//
// Created by Alberto Bruno on 2019-05-15.
//
#include "../identifier/kk_identifier.h"
#include <QTextCharFormat>
#include <utility>
#include "kk_char.h"

using std::string;

KKChar::KKChar(char value, string siteId) : siteId(std::move(siteId)), value(value) {

};
KKChar::~KKChar() {
    position.clear();
}
void KKChar::pushIdentifier(const KKIdentifierPtr& id) {
    position.push_back(id);
}

/*void kk_char::insert_identifier(std::vector<identifier>::iterator it,identifier x) {
    this->position.insert(it,x);
    return;
}*/

int KKChar::compareTo(const KKChar &other) {
    //    int comp;
    unsigned long min;

    if (this->position.size() <= other.position.size()) {
        min = this->position.size();
    } else {
        min = other.position.size();
    }

    for (unsigned long i = 0; i < min; i++) {

        KKIdentifier id1(*this->position[i].get());
        KKIdentifier id2(*other.position[i].get());


        if(id1.getDigit()>id2.getDigit()) return 1;
        if (id1.getDigit()<id2.getDigit()) return -1;

    }



    if (this->position.size() < other.position.size()) {
        return -1;
    } if (this->position.size() > other.position.size()) {
        return 1;
    } else {
        if(this->getSiteId()> other.siteId) return 1;
        if(this->getSiteId()< other.siteId) return -1;
        else return 0;
    }
}

char KKChar::getValue() {
    return value;
}

string KKChar::getSiteId() {
    return siteId;
}

void KKChar::insertSiteId(string siteId) {
    this->siteId = std::move(siteId);
}

void KKChar::insertPosition(vector<KKIdentifierPtr> position) {
    this->position = std::move(position);
}

vector<KKIdentifierPtr> KKChar::getPosition() {
    return this->position;
}

std::string KKChar::getIdentifiersString() {
    std::string identifiers_;
    std::for_each(position.begin(), position.end(),[&](const KKIdentifierPtr& i){
        char str[sizeof(unsigned long)+1];
        sprintf(str, "%lu", i->getDigit());
        identifiers_= identifiers_ + str + " ";
    });
    return identifiers_;
}
QString KKChar::getKKCharFont(){
    return KKCharFont;
}
void KKChar::setKKCharFont(QString font){
    this->KKCharFont=std::move(font);
}

QString KKChar::getKKCharColor(){
    return KKCharColor;
}
void KKChar::setKKCharColor(QString color){
    this->KKCharColor=std::move(color);
}


