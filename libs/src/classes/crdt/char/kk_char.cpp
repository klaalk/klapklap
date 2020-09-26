//
// Created by Alberto Bruno on 2019-05-15.
//

#include "kk_char.h"

KKChar::KKChar(char value, string siteId) : siteId(std::move(siteId)), value(value) {

};


KKChar::KKChar(char value, string siteId, QString KKCharFont, QString KKCharColor) : siteId(std::move(siteId)), value(value),  KKCharFont(KKCharFont),KKCharColor(KKCharColor){

};

KKChar::~KKChar() {
    position.clear();
}
void KKChar::pushIdentifier(const KKIdentifierPtr& id) {
    position.push_back(id);
}

int KKChar::compareTo(const KKChar &other) {
    //    int comp;
    unsigned long min;

    if (this->position.size() <= other.position.size()) {
        min = static_cast<unsigned long>(position.size());
    } else {
        min = static_cast<unsigned long>(other.position.size());
    }

    for (unsigned long i = 0; i < min; i++) {
        KKIdentifier id1(*position[i].get());
        KKIdentifier id2(*other.position[i].get());

        if (id1.getDigit()>id2.getDigit()) return 1;
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

string KKChar::encodeIdentifiers() {
    string identifiers;
    stringstream strstream;
    for (const auto& pos : position) {
        strstream << pos->getDigit() << ";";
    }
    strstream >> identifiers;
    identifiers.pop_back();
    return identifiers;
}

void KKChar::decodeIdentifiers(QString encodedIds)
{
    QStringList ids = encodedIds.split(";");
    for(QString id : ids) {
        unsigned long val = stoul(id.toStdString(), nullptr, 0);
        pushIdentifier(std::shared_ptr<KKIdentifier>(new KKIdentifier(val, siteId)));
    }
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


