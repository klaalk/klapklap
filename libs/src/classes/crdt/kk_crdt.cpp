//
// Created by Alberto Bruno on 2019-05-15.
//
#include "kk_crdt.h"

#define CRDT_FORMATT "%1"
#define CRDT_FORMAT_LENGTH 3

using std::string;
using std::shared_ptr;

KKCrdt::KKCrdt(QString siteid, KKStrategy strategy) : siteId(siteid), strategy(strategy), boundary(10), base(32) {
    linesAlignment.push_back(1);
}

KKCrdt::~KKCrdt() {
    clear();
}

void KKCrdt::clear() {
    text.clear();
    strategyCache.clear();
    linesAlignment.clear();
}

QStringList KKCrdt::encodeCrdt(){
    QStringList encodedCrdt;
    for (const auto& linePtr : text) {
        for (const auto& charPtr : linePtr) {
            QString crdtChar = encodeCrdtChar(charPtr);
            encodedCrdt.push_back(crdtChar);
        }
    }
    encodedCrdt.push_back("END");

    // Scrivo alignment
    for (int alignment : linesAlignment) {
        encodedCrdt.push_back(QVariant(alignment).toString());
    }

    return encodedCrdt;
}

QString KKCrdt::encodeCrdtChar(KKCharPtr charPtr)
{
    QString charValue;
    charValue.push_back(charPtr->getValue());
    QString identifiers = charPtr->encodeIdentifiers();

    QString encodedChar;
    encodedChar.append(QString(CRDT_FORMATT).arg(charValue.length(), CRDT_FORMAT_LENGTH, 10, QChar('0')) + charValue);
    encodedChar.append(QString(CRDT_FORMATT).arg(charPtr->getSiteId().length(), CRDT_FORMAT_LENGTH, 10, QChar('0')) + charPtr->getSiteId());
    encodedChar.append(QString(CRDT_FORMATT).arg(identifiers.length(), CRDT_FORMAT_LENGTH, 10, QChar('0')) + identifiers);
    encodedChar.append(QString(CRDT_FORMATT).arg(charPtr->getKKCharFont().length(), CRDT_FORMAT_LENGTH, 10, QChar('0')) + charPtr->getKKCharFont());
    encodedChar.append(QString(CRDT_FORMATT).arg(charPtr->getKKCharColor().length(), CRDT_FORMAT_LENGTH, 10, QChar('0')) + charPtr->getKKCharColor());
    return encodedChar;
}

void KKCrdt::decodeCrdt(QStringList encodedCrdt){
    bool isCrdtText = true;
    clear();
    text.insert(text.end(), list<KKCharPtr>());
    for(QString crdtChar : encodedCrdt) {
        if (crdtChar.isEmpty())
            continue;
        if (crdtChar == "END") {
            isCrdtText = false;
            continue;
        }
        if (isCrdtText) {
            KKCharPtr charPtr = decodeCrdtChar(crdtChar);
            if (charPtr != nullptr) {
                text[text.size()-1].push_back(charPtr);
                // Se il carattere e' un 'a capo' si inserisce una nuova riga nel vettore di liste (text)
                if (charPtr->getValue() == '\n') {
                    text.insert(text.end(), list<KKCharPtr>());
                }
            }
        } else {
            linesAlignment.push_back(crdtChar.toInt());
        }
    }
}

KKCharPtr KKCrdt::decodeCrdtChar(QString encodedChar)
{
    KKCharPtr charPtr = nullptr;
    QStringList fields;
    int start = 0, nextFieldLength = 0;
    do {
        nextFieldLength = encodedChar.midRef(start, CRDT_FORMAT_LENGTH).toInt();
        start += CRDT_FORMAT_LENGTH;
        fields.append(encodedChar.mid(start, nextFieldLength));
        start += nextFieldLength;
    } while (start < encodedChar.size());

    if (!fields.isEmpty()) {
        charPtr = KKCharPtr(new KKChar(fields[0].at(0), fields[1]));
        charPtr->decodeIdentifiers(fields[2]);
        charPtr->setKKCharFont(fields[3]);
        charPtr->setKKCharColor(fields[4]);
    }

    return charPtr;
}

KKCharPtr KKCrdt::localInsert(QChar val, KKPosition pos, QString font, QString color) {
    KKCharPtr newChar = generateChar(val, pos);
    newChar->setKKCharFont(font);
    newChar->setKKCharColor(color);
    insertChar(newChar, pos);
    return copyChar(newChar.get());
}

list<KKCharPtr> KKCrdt::localDelete(KKPosition startPos, KKPosition endPos){
    int alignFirstRow = linesAlignment[0];
    bool newLineRemoved = false;
    list<KKCharPtr> chars;

    if (startPos.getLine() > endPos.getLine())
        return chars;

    // Controlla se hai cancellato su piu di una riga
    if(startPos.getLine() != endPos.getLine()){
        newLineRemoved = true;
        chars = deleteMultipleLines(startPos, endPos);        
    } else {
        chars = deleteSingleLine(startPos, endPos);
        if (chars.size() > 0) {
            list<KKCharPtr>::iterator it;
            it=chars.begin();

            // Controlla se hai cancellato un \n
            for (it = chars.begin(); it!= chars.end(); it++) {
                if(it->get()->getValue() == '\n'){
                    newLineRemoved = true;
                }
            }
        }
    }
    deleteEmptyLines();

    if (newLineRemoved && !text[startPos.getLine()].empty()) {
        QChar ch = std::next(text[startPos.getLine()].begin(), static_cast<long>(text[startPos.getLine()].size()-1))->get()->getValue();

        if (!text[startPos.getLine()+1].empty() && ch != '\n') {
            mergeLines(startPos.getLine());
        }
    }

    if (text.empty()) {
        linesAlignment.push_back(alignFirstRow);
    }
    return chars;
}

KKCharPtr KKCrdt::localFormatChange(KKPosition pos, QString font, QString color, QChar* value){
    list<KKCharPtr>::iterator ch;
    ch = std::next(text[pos.getLine()].begin(), static_cast<long>(pos.getCh()));

    KKCharPtr charPtr;
    if (ch->get() != nullptr &&
            (ch->get()->getKKCharFont() != font || ch->get()->getKKCharColor() != color || ch->get()->getValue()=='\n')) {
        ch->get()->setKKCharFont(font);
        ch->get()->setKKCharColor(color);
        charPtr = copyChar(ch->get());
    }

    if (ch->get() != nullptr)
        *value = ch->get()->getValue();

    return charPtr;
}

KKPosition KKCrdt::remoteInsert(const KKCharPtr& charPtr){
    KKPosition position = findInsertPosition(charPtr);
    insertChar(charPtr, position);
    return position;
}

KKPosition KKCrdt::remoteDelete(const KKCharPtr& charPtr, bool *flag){
    *flag = true;
    int alignFirstRow = linesAlignment[0];

    // Trova la posizione nel crdt del carattere
    KKPosition position = findPosition(charPtr, flag);

    if (!*flag) {
        if (text.empty()) {
            linesAlignment.push_back(alignFirstRow);
        }
        return position;
    }

    text[position.getLine()].erase(std::next(text[position.getLine()].begin(),static_cast<long>(position.getCh())));//rimuove il carattere cancellato

    if(charPtr->getValue()=='\n'){
        // Devi rimuovere elem in posizione getline+1
        if(!text[position.getLine()+1].empty()) {
            mergeLines(position.getLine());
        }
    }

    deleteEmptyLines();

    if (text.empty())
        linesAlignment.push_back(alignFirstRow);

    return position;
}

KKPosition KKCrdt::remoteFormatChange(const KKCharPtr& charPtr){
    bool flag = true;
    KKPosition position = findPosition(charPtr, &flag);

    if (flag) {
        list<KKCharPtr>::iterator ch = std::next(text[position.getLine()].begin(), static_cast<long>(position.getCh()));
        if (ch->get() != nullptr &&
                (ch->get()->getKKCharFont() != charPtr->getKKCharFont() || ch->get()->getKKCharColor() != charPtr->getKKCharColor())
                ) {
            ch->get()->setKKCharColor(charPtr->getKKCharColor());
            ch->get()->setKKCharFont(charPtr->getKKCharFont());
        } else
            logger("[CRDT - remoteFormatChange] Char non modificabile");

    } else
        logger("[CRDT - remoteFormatChange] Posizione non valida");

    return position;
}

bool KKCrdt::remoteAlignmentChange(unsigned long idx, int align)
{
    if (idx < linesAlignment.size()) {
        if (linesAlignment.at(idx) != align) {
            linesAlignment.at(idx) = align;
            return true;
        }
    }
    else
        logger("[CRDT - remoteAlignmentChange] Index out of range");

    return false;
}

list<KKCharPtr> KKCrdt::changeMultipleKKCharFormat(KKPosition start, KKPosition end, QString font, QString color){
    list<KKCharPtr> charsChanged;
    list<KKCharPtr>::iterator currentIter, startIter, endIter;

    for(auto line = start.getLine(); line <= end.getLine(); line++){
        // prima riga
        if (line == start.getLine()) {

            if (line == end.getLine()){
                // La selezione inizia e finisce su una stessa riga: cambia tutte le kkchar da start.getch() a end.getch()
                startIter = std::next(text[line].begin(), static_cast<long>(start.getCh()));
                endIter = std::next(text[line].begin(), static_cast<long>(end.getCh()) + 1);

            } else {
                // La selezione ?? su piu righe diverse: cambia tutte le kkchar da start.getch() fino alla fine della riga
                startIter = std::next(text[line].begin(), static_cast<long>(start.getCh()));
                endIter = text[line].end();
            }

        } else if (line == end.getLine()) {
            // Sei all'ultima riga cambia tutte le kkchar dall'inizio della riga fino a end.getch()
            startIter = text[line].begin();
            endIter = std::next(text[line].begin(), static_cast<long>(end.getCh() + 1));

        } else {
            // La selezione ?? almeno su 3 righe, sei in una riga centrale: cambia tutte le KKchar di questa riga
            startIter = text[line].begin();
            endIter = text[line].end();
        }

        for(currentIter = startIter; currentIter != endIter; currentIter++) {
            currentIter->get()->setKKCharFont(font);
            currentIter->get()->setKKCharColor(color);

            KKCharPtr charPtr = copyChar(currentIter->get());
            if (charPtr != nullptr)
                charsChanged.push_back(charPtr);
        }
    }

    return charsChanged;
}

bool KKCrdt::calculateLineCol(unsigned long position, unsigned long startLine, unsigned long *line, unsigned long *col){
    unsigned long tot = 0, succ = 0;
    *line = 0;
    *col = 0;

    if (position <= 0)
        return true;

    for(unsigned long i = startLine; i<text.size(); i++){
        succ+=text[i].size();

        if (position<=succ) {

            if(position==succ && std::next(text[i].begin(), static_cast<long>(text[i].size()-1))->get()->getValue()=='\n'){
                *line=i+1;
                *col=0; //per calcolare il carattere \n
            } else {
                *col=position-tot;
                *line=i;
            }
            return true;
        }
        tot+=text[i].size();
    }

    logger(QString("[calculateLineCol] Invalid line >%1< and col >%2<").arg(QVariant(static_cast<long long>(*line)).toString(), QVariant(static_cast<long long>(*col)).toString()));
    return false;
}

int KKCrdt::calculateGlobalPosition(KKPosition position){
    int globalPosition = 0;
    for (unsigned long i=0; i < position.getLine(); i++) {
        globalPosition = globalPosition + static_cast<int>(text[i].size());
    }
    return globalPosition + static_cast<int>(position.getCh());
}

int KKCrdt::getLineAlignment(unsigned long idx)
{
    int alignment = -1;
    if (idx < linesAlignment.size())
        alignment = linesAlignment.at(idx);
    else
        logger("[CRDT - getLineAlignment] Index out of range");
    return alignment;
}

bool KKCrdt::checkLine(unsigned long lineIdx)
{
    return text.size() > lineIdx;
}

bool KKCrdt::checkPosition(unsigned long start, unsigned long end)
{
    long line = static_cast<long>(start);
    long col = static_cast<long>(end);
    long lines = static_cast<long>(text.size());
    long cols = static_cast<long>(text[start].size());
    if (line < 0 || col < 0) return false;
    if (line > lines) return false;
    if (line <= lines && col > cols) return false;

    return true;
}

vector<int> KKCrdt::getLinesAlignment() const
{
    return linesAlignment;
}

vector<list<KKCharPtr>> KKCrdt::getText() const
{
    return text;
}

QString KKCrdt::getSiteId() const
{
    return siteId;
}


bool KKCrdt::isTextEmpty() {
    return text.empty();
}

void KKCrdt::print() {
    qDebug() << "[CRDT - print] Testo:";
    for (auto& i : text) {
        for (const auto& x : i) {
            if(x->getValue() =='\n'){
                qDebug() << "/n"<< "[";
            } else {
                qDebug() << x->getValue() << "[";
            }
            for (const auto& y: x->getPosition()) {
                qDebug() << y->getDigit() << ";";
            }
            qDebug() << "] ";
        }
    }
}

void KKCrdt::printText() {
    qDebug() << "[CRDT - printText] Testo:";
    for (auto& i : text) {
        for (const auto& x : i) {
            qDebug() << x->getValue();
        }
    }
}

void KKCrdt::printForm() {
    qDebug() << "[CRDT - printForm] Testo:";
    for (auto& i : text) {
        for (const auto& x : i) {
            if(x->getValue() =='\n'){
                qDebug() <<"/n"<< "[";
            } else {
                qDebug() << x->getValue() << "[";
            }
            for (const auto& y: x->getPosition()) {
                qDebug() << y->getDigit() << ";";
            }
            qDebug() << "] ";
            qDebug() << x->getKKCharFont().toUtf8().constData() <<"  "<< x->getKKCharColor().toUtf8().constData();
        }
    }
}

void KKCrdt::printLinesAlignment(){
    qDebug() << "[CRDT - printLinesAlignment] Testo:";
    for(unsigned long i = 0; i<linesAlignment.size();i++) {
        qDebug() << linesAlignment[i];
    }
}

// PRIVATE

void KKCrdt::insertChar(const KKCharPtr& charPtr, KKPosition pos) {
    int alignBefore;

    // Inizializzo la prima riga.
    if (pos.getLine() == text.size()) {
        text.insert(text.end(), list<KKCharPtr>());
    }

    // Split line into two lines
    if (charPtr->getValue() == '\n') {
        // Salvo cio che c'?? dopo il \n
        list<KKCharPtr> line_after(std::next(text[pos.getLine()].begin(),static_cast<long>(pos.getCh())),text[pos.getLine()].end());

        // Cancello cio che c'era dopo il \n
        text[pos.getLine()].erase(std::next(text[pos.getLine()].begin(),static_cast<long>(pos.getCh())),text[pos.getLine()].end());
        if (line_after.empty()) {
            // Se il /n ?? l'ultimo carattere del testo
            list<KKCharPtr> charsPtr = {charPtr};
            text[pos.getLine()].splice(std::next(text[pos.getLine()].begin(), static_cast<long>(pos.getCh())), charsPtr);
            list<KKCharPtr> newLastLine = {};
            // Aggiungi una riga vuota
            text.insert(std::next(text.begin(),static_cast<long>(pos.getLine()+1)),newLastLine);
            // Inserisco nella lista align un elemento in posizione getLine+1 e gli assegno il valore di allineamento che aveva l'elemento in getLine
            linesAlignment.insert(linesAlignment.begin()+static_cast<int>(pos.getLine()+1),linesAlignment[pos.getLine()]);

        } else {
            // Il \n NON ?? l'ultimo carattere del testo
            // Inserisco il \n
            text[pos.getLine()].push_back(charPtr);

            alignBefore = linesAlignment[pos.getLine()];

            // Salvo la riga col \n
            list<KKCharPtr> line_before(text[pos.getLine()]);

            // Cancello la riga col \n
            text.erase(std::next(text.begin(),static_cast<long>(pos.getLine())));
            linesAlignment.erase(linesAlignment.begin()+static_cast<int>(pos.getLine()));

            // Aggiungo la riga che finisce col \n
            text.insert(std::next(text.begin(),static_cast<long>(pos.getLine())),line_before);

            // Aggiungo la riga di cio che sta dopo il \n
            text.insert(std::next(text.begin(),static_cast<long>(pos.getLine()+1)),line_after);

            // Inserisco nella lista align un elemento in posizione getLine+1 e gli assegno il valore di allineamento che aveva l'elemento in getLine
            linesAlignment.insert(linesAlignment.begin()+static_cast<int>(pos.getLine()),alignBefore);
            linesAlignment.insert(linesAlignment.begin()+static_cast<int>(pos.getLine()+1),alignBefore);
        }
    } else {
        // Se hai scritto un carattere
        list<KKCharPtr> charsPtr = {charPtr};
        text[pos.getLine()].splice(std::next(text[pos.getLine()].begin(), static_cast<long>(pos.getCh())), charsPtr);
    }
}

KKCharPtr KKCrdt::generateChar(QChar val, KKPosition pos) {
    vector<KKIdentifierPtr> posBefore,  posAfter, posNew;
    KKCharPtr charPtr = KKCharPtr(new KKChar(val, siteId));
    posBefore = findPositionBefore(pos);
    posAfter = findPositionAfter(pos);
    posNew = generatePositionBetween(posBefore, posAfter, &posNew, 0);
    charPtr->insertPosition(posNew);
    return charPtr;
}

vector<KKIdentifierPtr> KKCrdt::generatePositionBetween(vector<KKIdentifierPtr> positionOne,
                                                        vector<KKIdentifierPtr> positionTwo,
                                                        vector<KKIdentifierPtr> *positionNew,
                                                        unsigned long level) {

    KKStrategy _strategy = casuale;
    unsigned long elev = static_cast<unsigned long>(pow(2,level));
    unsigned long _base = elev * base;

    KKIdentifierPtr idOne;
    KKIdentifierPtr idTwo;

    if (positionOne.empty()) {
        idOne = shared_ptr<KKIdentifier>(new KKIdentifier(0, siteId));
    } else {
        idOne = shared_ptr<KKIdentifier>(new KKIdentifier(positionOne[0]->getDigit(), positionOne[0]->getSiteId()));
    }

    if (positionTwo.empty()) {
        idTwo = shared_ptr<KKIdentifier>(new KKIdentifier(_base, siteId));
    } else {
        idTwo = shared_ptr<KKIdentifier>(new KKIdentifier(positionTwo[0]->getDigit(), positionTwo[0]->getSiteId()));
    }

    if (idTwo->getDigit() - idOne->getDigit() > 1) {
        unsigned long digitNew;
        KKIdentifierPtr idNew;

        if (positionTwo.empty()){
            digitNew = generateIdentifierBetween(idOne->getDigit(), idTwo->getDigit(), _strategy, level);
        } else{
            digitNew = generateIdentifierBetween(idOne->getDigit(), idTwo->getDigit()-1, _strategy, level);
        }

        idNew = shared_ptr<KKIdentifier>(new KKIdentifier(digitNew, siteId));
        positionNew->insert(positionNew->end(), idNew);
        return *positionNew;

    }

    if (idTwo->getDigit() - idOne->getDigit() == 1) {
        positionNew->insert(positionNew->end(), idOne);
        return generatePositionBetween(slice(positionOne, 1), vector<KKIdentifierPtr>(), positionNew, level + 1);

    } else if (idOne->getDigit() == idTwo->getDigit()) {
        positionNew->insert(positionNew->end(), idOne);
        return generatePositionBetween(slice(positionOne, 1), slice(positionTwo, 1), positionNew, level + 1);
    }
    return vector<KKIdentifierPtr>();
}

unsigned long KKCrdt::generateIdentifierBetween(unsigned long min, unsigned long max, KKStrategy _strategy,unsigned long level) {
    unsigned long _boundary = this->boundary;

    if (level%2)
        _strategy = plus;
    else
        _strategy = minus;

    if ((max - min < _boundary)) {
        min = min + 1;
    } else {
        if (_strategy == minus) {
            min=min+1;
            min = max - _boundary;
        } else {
            min = min + 1;
            max = min + _boundary;
        }
    }

    // Obtain a random number from hardware
    std::random_device rd;
    // Seed the generator
    std::mt19937_64 gen(rd());

    std::uniform_int_distribution<> distr(static_cast<int>(min), static_cast<int>(max));
    return static_cast<unsigned long>(distr(gen));
}

KKPosition KKCrdt::findPosition(const KKCharPtr& charPtr, bool *flag) {
    unsigned long totalLines = static_cast<unsigned long>(text.size());
    unsigned long maxLine = totalLines - 1;
    unsigned long minLine = 0;
    unsigned long midLine = 0;


    // Se il testo ?? vuoto o la char ha una position inferiore alla prima in assoluto
    if (text.empty() || charPtr.get()->compareTo(*text[0].front().get()) < 0) {
        *flag=false;
        return {0,0};
    }

    if (text[maxLine].empty()) {
        maxLine--;
    }

    list<KKCharPtr> lastLine(text[maxLine]);
    KKChar lastChar (**std::next(lastLine.begin(), static_cast<long>(lastLine.size()-1)));

    // Se la char ha position maggiore dell'ultima char nel testo
    if (charPtr.get()->compareTo(lastChar) > 0){
        *flag = false;
        return KKPosition(0,0);
    }

    // Ricerca la linea dove si trova la char
    while (minLine + 1 < maxLine) {
        midLine = static_cast<unsigned long>(floor(minLine + (maxLine - minLine)/2));
        list<KKCharPtr> currentLine (text[midLine]);
        lastChar = **std::next(currentLine.begin(), static_cast<long>(currentLine.size()-1));

        if (charPtr.get()->compareTo(lastChar) == 0) {
            return KKPosition(midLine, static_cast<unsigned long>(currentLine.size()-1));

        } if (charPtr.get()->compareTo(lastChar) < 0) {
            maxLine = midLine;

        } else {
            minLine = midLine;
        }
    }

    // Ora la scelta ?? tra una o massimo due righe
    list<KKCharPtr> minCurrentLine (text[minLine]);
    KKChar minLastChar = **std::next(minCurrentLine.begin(), static_cast<long>(minCurrentLine.size()-1));

    list<KKCharPtr> maxCurrentLine (text[maxLine]);
    KKChar maxLastChar = **std::next(maxCurrentLine.begin(), static_cast<long>(maxCurrentLine.size()-1));

    // Confronta con l'ultima char della riga 1 (per capire se ?? su quella riga o sulla riga 2 (se c'??))
    if (charPtr.get()->compareTo(minLastChar) <=0 ) {
        unsigned long charIdx = findIndexInLine(charPtr, minCurrentLine, flag);
        return KKPosition(minLine, charIdx);
    }
    unsigned long charIdx = findIndexInLine(charPtr, maxCurrentLine, flag);
    return KKPosition(maxLine, charIdx);

}

KKPosition KKCrdt::findInsertPosition(const KKCharPtr& charPtr){
    unsigned long totalLines = static_cast<unsigned long>(text.size());
    unsigned long maxLine = totalLines - 1;
    unsigned long minLine = 0;
    unsigned long midLine = 0;


    // Controlla se la char va messa come primo carattere della prima riga
    if (text.empty() ||
            text.at(0).empty() ||
            charPtr.get()->compareTo(*text[0].front().get()) <= 0) {
        return {0,0};
    }


    if (text[maxLine].empty() ) {
        maxLine--;
    }

    list<KKCharPtr> lasLine(text[maxLine]);

    // Controlla se la char va messa come primo carattere dell'ultima riga DA RIVEDERE
    if (lasLine.empty() && std::next(text[maxLine-1].begin(), static_cast<long>(text[maxLine-1].size()-1))->get()->getValue()=='\n' &&
            charPtr.get()->compareTo(**std::next(text[maxLine-1].begin(), static_cast<long>(text[maxLine-1].size()-1))) > 0) {
        return KKPosition(maxLine,0);
    }

    KKChar lastChar = **std::next(lasLine.begin(), static_cast<long>(lasLine.size() - 1));
    // Controlla se la char va messa come ultimo carattere dell'ultima riga
    if(charPtr.get()->compareTo(lastChar)>0){
        return findEndPosition(lastChar);
    }

    while (minLine +1 < maxLine) {
        midLine = static_cast<unsigned long>(floor(minLine + (maxLine - minLine)/2));

        list<KKCharPtr> currentLine (text[midLine]);

        lastChar = **std::next(currentLine.begin(), static_cast<long>(currentLine.size()-1));

        if (charPtr.get()->compareTo(lastChar) == 0) {
            return KKPosition(midLine, static_cast<unsigned long>(currentLine.size()-1));
        }
        if (charPtr.get()->compareTo(lastChar)<0) {
            maxLine = midLine;
        } else {
            minLine = midLine;
        }
    }

    // La scelta ora sar?? tra massimo due linee
    list<KKCharPtr> minCurrentLine (text[minLine]);
    KKChar minLastChar = **std::next(minCurrentLine.begin(), static_cast<long>(minCurrentLine.size()-1));

    list<KKCharPtr> maxCurrentLine (text[maxLine]);
    KKChar maxLastChar = **std::next(maxCurrentLine.begin(), static_cast<long>(maxCurrentLine.size()-1));

    // Vede se sta nella linea1 o linea2 comparando con l'ultima Char delle linea1
    if(charPtr.get()->compareTo(minLastChar) <= 0) {
        unsigned long charIdx = findInsertIndexInLine(charPtr, minCurrentLine);
        return KKPosition(minLine,charIdx);
    }

    unsigned long charIdx = findInsertIndexInLine(charPtr, maxCurrentLine);
    return KKPosition(maxLine,charIdx);
}

KKPosition KKCrdt::findEndPosition(KKChar lastChar){
    if (lastChar.getValue() == '\n') {
        return KKPosition(static_cast<unsigned long>(text.size()-1), 0);
    } else {
        return KKPosition(static_cast<unsigned long>(text.size()-1), static_cast<unsigned long>(text[text.size()-1].size()));
    }
}

vector<KKIdentifierPtr> KKCrdt::findPositionBefore(KKPosition pos) {

    if (pos.getCh() == 0 && pos.getLine() == 0) {
        vector<KKIdentifierPtr> vuoto;
        return vuoto;
    }

    if (pos.getCh() == 0 && pos.getLine() != 0) {
        unsigned long line = pos.getLine() - 1;
        unsigned long ch = static_cast<unsigned long>(text[line].size());
        return std::next(text[line].begin(), static_cast<long long>(ch - 1))->get()->getPosition();
    }

    return std::next(text[pos.getLine()].begin(), static_cast<long long>(pos.getCh() - 1))->get()->getPosition();
}

vector<KKIdentifierPtr> KKCrdt::findPositionAfter(KKPosition pos) {
    long numLines, numChars;
    vector<KKIdentifierPtr> vuoto;
    numLines = static_cast<long>(text.size()) - 1;

    if (text.empty() || text[pos.getLine()].empty()) {
        numChars = 0;
    } else {
        numChars = static_cast<long>(text[pos.getLine()].size());
    }

    if (static_cast<long>(pos.getLine()) > numLines  && pos.getCh() == 0) { //sei alla prima lettera
        return vuoto;
    } if (static_cast<long>(pos.getLine()) == numLines  && static_cast<long>(pos.getCh()) == numChars) { //sei all'ultima riga e all'ultimo char
        return vuoto;
    } else if (static_cast<long>(pos.getLine()) < numLines  && static_cast<long>(pos.getCh()) == numChars) { // sei a fine riga ma non nell'ultima riga
        return text[pos.getLine() + 1].front()->getPosition();
    }

    vector<KKIdentifierPtr> posAfter(std::next(text[pos.getLine()].begin(), static_cast<long>(pos.getCh()))->get()->getPosition());
    // se non sei in nessun caso particolare
    return posAfter;
}

unsigned long KKCrdt::findIndexInLine(const KKCharPtr& charPtr, list<KKCharPtr> line, bool *flag){
    unsigned long cnt = 0;
    unsigned long left = 0;
    unsigned long right = static_cast<unsigned long>(line.size()-1);

    // Controlla se la linea ?? vuota o il Char ?? minore del primo della linea
    if(line.empty() || charPtr.get()->compareTo(**line.begin()) < 0) {
        return left;
    }
    // Controlla se il Char ?? maggiore dell'ultimo della linea
    if ( charPtr.get()->compareTo(**std::next(line.begin(), static_cast<long>(line.size()-1))) > 0) {
        return right;
    }

    list<KKCharPtr>::iterator lineIterator = line.begin();
    for (lineIterator = line.begin(); lineIterator!= line.end(); lineIterator++) {
        if(charPtr.get()->compareTo(**lineIterator)==0 &&
                charPtr.get()->getValue()==lineIterator->get()->getValue() &&
                charPtr.get()->getSiteId()==lineIterator->get()->getSiteId()) {
            return cnt;
        }
        cnt++;
    }

    *flag = false;
    return 0;
}

unsigned long KKCrdt::findInsertIndexInLine(const KKCharPtr& charPtr, list<KKCharPtr> line){
    unsigned long cnt=0;
    unsigned long left=0;
    unsigned long right = static_cast<unsigned long>(line.size()-1);

    if (line.empty() || charPtr.get()->compareTo(**line.begin())<0){
        return left;
    }

    if (charPtr.get()->compareTo(**std::next(line.begin(), static_cast<long>(line.size()-1))) > 0){
        return right;
    }

    list<KKCharPtr>::iterator it = line.begin();
    for (it = line.begin(); it!= line.end(); it++) {
        if(charPtr.get()->compareTo(**it)<0){
            return cnt;
        }
        //Caso non possibile (una remote insert di una Char esattamente identica a una esistente)
        if(charPtr.get()->compareTo(**it) == 0 &&
                charPtr.get()->getValue() == it->get()->getValue() &&
                charPtr.get()->getSiteId() == it->get()->getSiteId()) {
            return cnt;
        }

        cnt++;
    }

    return 0;
}

KKStrategy KKCrdt::findStrategy() {
    KKStrategy localStrategy;
    switch (strategy) {
    case plus:
        localStrategy = plus;
        break;
    case minus:
        localStrategy = minus;
        break;
    case casuale: {
        double _rand;
        _rand = rand()%100;
        _rand=_rand/100;
        localStrategy = round(_rand) == 0 ? plus : minus;
        break;
    }
    }
    return localStrategy;
}

void KKCrdt::mergeLines(unsigned long line){
    text[line].splice(std::next(text[line].begin(), static_cast<long>(text[line].size())),text[line+1]);

    list<KKCharPtr> merged_line(text[line]);

    //devi togliere line e line+1 e metter mergelined
    vector<list<KKCharPtr>>::iterator it;
    it=text.begin();
    std::advance(it,static_cast<long>(line));
    text.erase(it);
    text.erase(it);
    text.insert(it,merged_line);
    linesAlignment.erase(linesAlignment.begin()+static_cast<int>(line)+1);
}

list<KKCharPtr> KKCrdt::deleteMultipleLines(KKPosition startPos, KKPosition endPos) {
    list<KKCharPtr> chars(std::next(text[startPos.getLine()].begin(), static_cast<long>(startPos.getCh())), text[startPos.getLine()].end());
    text[startPos.getLine()].erase(std::next(text[startPos.getLine()].begin(),static_cast<long>(startPos.getCh())), text[startPos.getLine()].end());

    unsigned long line;


    for(line=startPos.getLine()+1;line<endPos.getLine();line++){
        chars.splice(chars.end(),text[line]);
        text[line].erase(text[line].begin(),std::next(text[line].begin(),static_cast<long>(text[line].size())));
    }

    if(!text[endPos.getLine()].empty()){
        //azioni ottimizabili
        list<KKCharPtr> tmp(text[endPos.getLine()].begin(),std::next(text[endPos.getLine()].begin(),static_cast<long>(endPos.getCh())));
        text[endPos.getLine()].erase(text[endPos.getLine()].begin(),std::next(text[endPos.getLine()].begin(),static_cast<long>(endPos.getCh())));
        chars.splice(chars.end(),tmp);
    }

    return chars;
}

list<KKCharPtr> KKCrdt::deleteSingleLine(KKPosition startPos, KKPosition endPos){
    long char_num = static_cast<long>(endPos.getCh() - startPos.getCh());
    if (char_num >= 0) {
        list<KKCharPtr> chars(std::next(text[startPos.getLine()].begin(),static_cast<long>(startPos.getCh())),std::next(text[startPos.getLine()].begin(),static_cast<long>(startPos.getCh()) + char_num));
        text[startPos.getLine()].erase(std::next(text[startPos.getLine()].begin(),static_cast<long>(startPos.getCh())),std::next(text[startPos.getLine()].begin(),static_cast<long>(startPos.getCh()) + char_num));
        return chars;
    }
    return {};
}

void KKCrdt::deleteEmptyLines(){
    vector<list<KKCharPtr>>::iterator it=text.begin();
    int it2=0;

    bool flag=false;
    // Controlla se l'ultimo carattere scritto ?? un \n
    for(auto it3 = text.end()-1; it3>=text.begin();it3--) {
        if(!it3->empty()){
            if(std::next(it3->begin(),static_cast<long>(it3->size()-1))->get()->getValue()=='\n'){
                flag=true;
            }
            break;

        }
    }


    for(unsigned long line=0; line < text.size(); line++){
        if(text[line].empty()){
            text.erase(it);
            linesAlignment.erase(linesAlignment.begin()+it2);
            line--;
            it--;
            it2--;
        }
        it++;
        it2++;
    }

    if(flag){
        // Aggiungi una riga vuota
        list<KKCharPtr> lastLine = {};
        text.insert(std::next(text.begin(),static_cast<long>(text.size())), lastLine);
        linesAlignment.insert(linesAlignment.begin()+static_cast<int>(text.size()-1),linesAlignment[text.size()-1]);
    }
}

vector<KKIdentifierPtr> KKCrdt::slice(vector<KKIdentifierPtr> const &v, int i) {
    if (v.cbegin() == v.cend()) {
        return std::vector<KKIdentifierPtr>();
    }
    return std::vector<KKIdentifierPtr>(v.cbegin() + i, v.cend());
}

KKCharPtr KKCrdt::copyChar(KKChar* source)
{
    KKCharPtr dest = nullptr;
    if (source != nullptr) {
        dest = KKCharPtr(new KKChar(source->getValue(), source->getSiteId(), source->getKKCharFont(), source->getKKCharColor()));
        vector<KKIdentifierPtr> idsPtr;

        for (KKIdentifierPtr id : source->getPosition()) {
            KKIdentifierPtr newid  = KKIdentifierPtr(new KKIdentifier(id->getDigit(), id->getSiteId()));
            idsPtr.push_back(newid);
        }

        dest.get()->insertPosition(idsPtr);
    }
    return dest;
}

void KKCrdt::logger(QString message)
{
    KKLogger::log(message, "CRDT");
}

