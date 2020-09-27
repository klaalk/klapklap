//
// Created by Alberto Bruno on 2019-05-15.
//

#include "kk_char.h"

#include <QVariant>

KKChar::KKChar(QChar value, QString siteId) : siteId(std::move(siteId)), value(value) {

};


KKChar::KKChar(QChar value, QString siteId, QString KKCharFont, QString KKCharColor) : siteId(std::move(siteId)), value(value),  KKCharFont(KKCharFont),KKCharColor(KKCharColor){

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

QChar KKChar::getValue() {
    return value;
}

QString KKChar::getSiteId() {
    return siteId;
}

void KKChar::insertSiteId(QString siteId) {
    this->siteId = siteId;
}

void KKChar::insertPosition(vector<KKIdentifierPtr> position) {
    this->position = std::move(position);
}

vector<KKIdentifierPtr> KKChar::getPosition() {
    return this->position;
}

QString KKChar::encodeIdentifiers() {
    QString identifiers;
    stringstream strstream;

    for (const auto& pos : position) {
        identifiers.append(QVariant(static_cast<qulonglong>(pos->getDigit())).toString()).append(";");
    }
    identifiers.remove(identifiers.size()-1, 1);
    return identifiers;
}

void KKChar::decodeIdentifiers(QString encodedIds)
{
    QStringList ids = encodedIds.split(";");
    for(QString id : ids) {
        unsigned long val = id.toULong();
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


