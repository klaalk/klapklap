//
// Created by Alberto Bruno on 2019-05-15.
//

#include <sys/time.h>
#include "kk_crdt.h"
#include "identifier/kk_identifier.h"
#include <cstdlib>
#include <ctime>
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility> 

#define CRDT_FORMAT "%1"
#define CRDT_FORMAT_LENGTH 3

using std::string;
using std::shared_ptr;


KKCrdt::KKCrdt(string siteid, strategy strategy) : siteid(std::move(siteid)), boundary(10), _strategy(strategy), base(32) {

};

KKCrdt::~KKCrdt() {
    text.clear();
    strategy_cache.clear();
}

KKCharPtr KKCrdt::localInsert(char val, KKPosition pos, QString _font, QString _color) {
    KKCharPtr newChar = this->generateChar(val, pos);
    newChar->setKKCharFont(_font);
    newChar->setKKCharColor(_color);
    this->insertChar(newChar, pos);
    return newChar;
}

KKCharPtr KKCrdt::generateChar(char val, KKPosition pos) {

    vector<KKIdentifierPtr> position_before; vector<KKIdentifierPtr> position_after; vector<KKIdentifierPtr> new_position;

    KKCharPtr new_Char = KKCharPtr(new KKChar(val, this->siteid));

    position_before = this->findPositionBefore(pos);
    position_after = this->findPositionAfter(pos);
    new_position = this->generatePositionBetween(position_before, position_after, &new_position, 0);
    new_Char->insertPosition(new_position);
    return new_Char;
}

vector<KKIdentifierPtr> KKCrdt::findPositionBefore(KKPosition pos) {

    if (pos.getCh() == 0 && pos.getLine() == 0) {
        vector<KKIdentifierPtr> vuoto;
        return vuoto;
    } if (pos.getCh() == 0 && pos.getLine() != 0) {
        unsigned long line = pos.getLine() - 1;
        unsigned long ch = static_cast<unsigned long>(text[line].size());
        return std::next(text[line].begin(), static_cast<long>(ch) - 1)->get()->getPosition();
    }
    return std::next(text[pos.getLine()].begin(), static_cast<long>(pos.getCh()) - 1)->get()->getPosition();
}

vector<KKIdentifierPtr> KKCrdt::findPositionAfter(KKPosition pos) {

    long num_lines; long num_chars;
    vector<KKIdentifierPtr> vuoto;


    num_lines = static_cast<long>(text.size()) - 1;


    if (text.empty() || text[pos.getLine()].empty()) {
        num_chars = 0;
    } else {
        num_chars = static_cast<long>(text[pos.getLine()].size());
    }

    if (static_cast<long>(pos.getLine()) > num_lines  && pos.getCh() == 0) { //sei alla prima lettera
        return vuoto;
    } if (static_cast<long>(pos.getLine()) == num_lines  && static_cast<long>(pos.getCh()) == num_chars) { //sei all'ultima riga e all'ultimo char
        return vuoto;
    } else if (static_cast<long>(pos.getLine()) < num_lines  && static_cast<long>(pos.getCh()) == num_chars) { // sei a fine riga ma non nell'ultima riga
        return text[pos.getLine() + 1].front()->getPosition();
    }

    vector<KKIdentifierPtr> positions_after(std::next(text[pos.getLine()].begin(),static_cast<long>(pos.getCh()))->get()->getPosition());

    return positions_after; // se non sei in nessun caso particolare DA RIVEDERE
}

vector<KKIdentifierPtr> KKCrdt::generatePositionBetween(vector<KKIdentifierPtr> position1, vector<KKIdentifierPtr> position2,
                                                        vector<KKIdentifierPtr> *new_position, unsigned long level) {

    strategy _strategy=casuale;
    auto elev = static_cast<unsigned long>(pow(2,level));
    unsigned long _base = elev * this->base;
    //    _strategy = this->find_strategy(level);

    KKIdentifierPtr id1; KKIdentifierPtr id2;


    if (position1.empty()) {
        id1 = shared_ptr<KKIdentifier>(new KKIdentifier(0, this->siteid));
    } else {
        id1 = shared_ptr<KKIdentifier>(new KKIdentifier(position1[0]->getDigit(), position1[0]->getSiteId()));
    }

    if (position2.empty()) {
        id2 = shared_ptr<KKIdentifier>(new KKIdentifier(_base, this->siteid));
    } else {
        id2 = shared_ptr<KKIdentifier>(new KKIdentifier(position2[0]->getDigit(), position2[0]->getSiteId()));
    }

    if (id2->getDigit() - id1->getDigit() > 1) {

        unsigned long new_digit;
        KKIdentifierPtr new_id;

        if(position2.empty()){
            new_digit = this->generateIdentifierBetween(id1->getDigit(), id2->getDigit(), _strategy, level);
        }
        else{
            new_digit = this->generateIdentifierBetween(id1->getDigit(), id2->getDigit()-1, _strategy, level);
        }

        new_id = shared_ptr<KKIdentifier>(new KKIdentifier(new_digit, this->siteid));
        new_position->insert(new_position->end(), new_id);
        return *new_position;

    } if (id2->getDigit() - id1->getDigit() == 1) {

        new_position->insert(new_position->end(), id1);

        return this->generatePositionBetween(slice(position1, 1), vector<KKIdentifierPtr>(), new_position, level + 1);

    }else if (id1->getDigit() == id2->getDigit()) {
        new_position->insert(new_position->end(), id1);
        return this->generatePositionBetween(slice(position1, 1), slice(position2, 1), new_position,
                                             level + 1);


    }
    return vector<KKIdentifierPtr>();
}

strategy KKCrdt::findStrategy() {
    //    if (!strategy_cache.empty() && strategy_cache[level]) {
    //        return strategy_cache[level];
    //    }
    strategy _local_strategy;
    switch (this->_strategy) {
    case plus:
        _local_strategy = plus;
        break;
    case minus:
        _local_strategy = minus;
        break;
    case casuale: {
        double _rand;
        _rand = rand()%100;
        _rand=_rand/100;
        _local_strategy = round(_rand) == 0 ? plus : minus;
        break;
    }
    }
    //    strategy_cache.insert(std::next(strategy_cache.begin(), static_cast<int>(level)), _local_strategy);
    return _local_strategy;
}

unsigned long KKCrdt::generateIdentifierBetween(unsigned long min, unsigned long max, strategy _strategy,unsigned long level) {
    //    unsigned long elev = static_cast<unsigned long>(pow(2,level));
    //    unsigned long _boundary = elev * this->boundary;
    unsigned long _boundary = this->boundary;
    if(level%2)_strategy = plus;
    else _strategy=minus;

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

    unsigned long random_number;
    std::random_device rd; // obtain a random number from hardware
    std::mt19937_64 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(static_cast<int>(min), static_cast<int>(max));
    random_number=static_cast<unsigned long>(distr(gen));

    //     std::cout<<_strategy<<std::endl;
    //    std::cout<<min<< "\t"<<random_number<<"\t"<< max<<std::endl;


    return random_number;
}

void KKCrdt::insertChar(const KKCharPtr& _char, KKPosition pos) {
    int flag =0;


    //qDebug()<< "POSIZIONEEEE->"<< pos.getLine()<<pos.getCh();
    // Inizializzo la prima riga.
    if (pos.getLine() == text.size()) {
        text.insert(text.end(), list<KKCharPtr>());
        flag=1;
    }

    // if inserting a newline, split line into two lines
    if (_char->getValue() == '\n') {
        //qDebug()<< "insert \n";
        list<KKCharPtr> line_after(std::next(text[pos.getLine()].begin(),static_cast<long>(pos.getCh())),text[pos.getLine()].end());//salvo cio che c'è dopo il \n
        text[pos.getLine()].erase(std::next(text[pos.getLine()].begin(),static_cast<long>(pos.getCh())),text[pos.getLine()].end()); //cancello cio che c'era dopo il \n
        if (line_after.empty()) { //se il /n è l'ultimo carattere del testo
            list<KKCharPtr> _list_char = {_char};
            text[pos.getLine()].splice(std::next(text[pos.getLine()].begin(), static_cast<long>(pos.getCh())), _list_char);
            list<KKCharPtr> new_last_line = {};
            text.insert(std::next(text.begin(),static_cast<long>(pos.getLine()+1)),new_last_line); //aggiungi una riga vuota

        } else { //il \n NON è l'ultimo carattere del testo
            text[pos.getLine()].push_back(_char); //inserisco il \n
            list<KKCharPtr> line_before(text[pos.getLine()]); //salvo la riga col \n
            text.erase(std::next(text.begin(),static_cast<long>(pos.getLine()))); //cancello la riga col \n
            text.insert(std::next(text.begin(),static_cast<long>(pos.getLine())),line_before); //aggiungo la riga che finisce col \n
            text.insert(std::next(text.begin(),static_cast<long>(pos.getLine()+1)),line_after); //aggiungo la riga di cio che sta dopo il \n
        }
    } else { //se hai scritto un carattere
//        if (text.at(pos.getLine()).empty() && flag==0) { //se la riga dove hai scritto il carattere è vuota, crea un altra riga
//            text.insert(text.begin() + static_cast<long>(pos.getLine()), list<KKCharPtr>());
//        }
        list<KKCharPtr> _list_char = {_char};
        text[pos.getLine()].splice(std::next(text[pos.getLine()].begin(), static_cast<long>(pos.getCh())), _list_char);



    }
}

unsigned long KKCrdt::remoteInsert(const KKCharPtr& _char){
    unsigned long global_pos;

    KKPosition pos = findInsertPosition(_char);
    insertChar(_char, pos);
    global_pos = generateGlobalPos(pos);
    this->print();
    return global_pos;
}

KKPosition KKCrdt::findInsertPosition(const KKCharPtr& _char){
    unsigned long min_line=0;
    unsigned long total_lines = static_cast<unsigned long>(text.size());
    unsigned long mid_line;
    unsigned long max_line = total_lines-1;


    //controlla se la char va messa come primo carattere della prima riga
    if(text.empty() || text.at(0).empty() ||_char.get()->compareTo(*text[0].front().get()) <= 0) {
        return {0,0};
    }


    if(text[max_line].empty()){
        max_line--;
    }

    list<KKCharPtr> last_line(text[max_line]);

    //controlla se la char va messa come primo carattere dell'ultima riga DA RIVEDERE
    if(last_line.empty() && std::next(text[max_line-1].begin(),static_cast<long>(text[max_line-1].size()-1))->get()->getValue()=='\n' && _char.get()->compareTo(**std::next(text[max_line-1].begin(),static_cast<long>(text[max_line-1].size()-1)))>0){
        return KKPosition(max_line,0);
    }

    KKChar last_char = **std::next(last_line.begin(),static_cast<long>(last_line.size()-1));


    //controlla se la char va messa come ultimo carattere dell'ultima riga
    if(_char.get()->compareTo(last_char)>0){
        //return findEndPosition(last_char, last_line, total_lines);
        return findEndPosition(last_char);
    }

    while (min_line +1 < max_line) {
        mid_line = static_cast<unsigned long>(floor(min_line + (max_line - min_line)/2));

        list<KKCharPtr> current_line (text[mid_line]);

        last_char = **std::next(current_line.begin(),static_cast<long>(current_line.size()-1));

        if(_char.get()->compareTo(last_char)==0){
            return KKPosition(mid_line, static_cast<unsigned long>(current_line.size()-1));
        }if(_char.get()->compareTo(last_char)<0) {
            max_line=mid_line;
        }else{
            min_line=mid_line;
        }
    }

    //la scelta ora sarà tra massimo due linee
    list<KKCharPtr> min_current_line (text[min_line]);
    KKChar min_last_char = **std::next(min_current_line.begin(),static_cast<long>(min_current_line.size()-1));
    list<KKCharPtr> max_current_line (text[max_line]);
    KKChar max_last_char = **std::next(max_current_line.begin(),static_cast<long>(max_current_line.size()-1));

    //vede se sta nella linea1 o linea2 comparando con l'ultima Char delle linea1
    if(_char.get()->compareTo(min_last_char)<=0){
        unsigned long char_idx = findInsertIndexInLine(_char,min_current_line);
        return KKPosition(min_line,char_idx);
    }
    unsigned long char_idx = findInsertIndexInLine(_char,max_current_line);
    return KKPosition(max_line,char_idx);
    
}

KKPosition KKCrdt::findEndPosition(KKChar last_char){
    if (last_char.getValue() == '\n') {
        return KKPosition(text.size()-1,0);
        }else{
        return KKPosition(text.size()-1,text[text.size()-1].size());
    }
}
//KKPosition KKCrdt::findEndPosition (KKChar last_char, const list<KKCharPtr>& last_line, unsigned long total_lines){
//    if (last_char.getValue() == '\n') {
//        return {total_lines,0};
//    }
//    return KKPosition(total_lines-1, last_line.size());
    
//}

unsigned long KKCrdt::findInsertIndexInLine(const KKCharPtr& _char, list<KKCharPtr> line){
    unsigned long left=0;
    unsigned long right = static_cast<unsigned long>(line.size()-1);


    if(line.empty() || _char.get()->compareTo(**line.begin())<0){
        return left;
    } if ( _char.get()->compareTo(**std::next(line.begin(),static_cast<long>(line.size()-1)))>0){
        return right;
    }

    list<KKCharPtr>::iterator it;
    it=line.begin();
    unsigned long cnt=0;

    for (it = line.begin(); it!= line.end(); it++) {
        if(_char.get()->compareTo(**it)<0){
            return cnt;
        }
        //caso non possibile (una remote insert di una Char esattamente identica a una esistente)
        if(_char.get()->compareTo(**it)==0){
            if(_char.get()->getValue()==it->get()->getValue()){
                if(_char.get()->getSiteId()==it->get()->getSiteId()){
                    return cnt;}}}
        cnt++;
    }
    return 0;
}

/// Scrive il crdt su un file, esempio formato: d(albo)[10;90;2;34]f(klaus)[2;3;89]
QStringList KKCrdt::saveCrdt(){
    QStringList crdt;
    for (const auto& line : text) {
        for (const auto& word : line) {
            QString crdtChar;
            string charValue;
            charValue.push_back(word->getValue());

            crdtChar.append(QString(CRDT_FORMAT).arg(charValue.length(), CRDT_FORMAT_LENGTH, 10, QChar('0')) + QString::fromStdString(charValue));
            crdtChar.append(QString(CRDT_FORMAT).arg(word->getSiteId().length(), CRDT_FORMAT_LENGTH, 10, QChar('0')) + QString::fromStdString(word->getSiteId()));

            string identifiers;
            std::stringstream strstream;
            for (const auto& position : word->getPosition()) {
                strstream << position->getDigit() << ";";
            }
            strstream >> identifiers;
            identifiers.pop_back();
            crdtChar.append(QString(CRDT_FORMAT).arg(identifiers.length(), CRDT_FORMAT_LENGTH, 10, QChar('0')) + QString::fromStdString(identifiers));
            crdtChar.append(QString(CRDT_FORMAT).arg(word->getKKCharFont().length(), CRDT_FORMAT_LENGTH, 10, QChar('0')) + word->getKKCharFont());
            crdtChar.append(QString(CRDT_FORMAT).arg(word->getKKCharColor().length(), CRDT_FORMAT_LENGTH, 10, QChar('0')) + word->getKKCharColor());
            crdt.push_back(crdtChar);
        }
    }
    return crdt;
}

void KKCrdt::loadCrdt(QStringList crdt){
    text.insert(text.end(), list<KKCharPtr>());

    for(QString crdtChar : crdt) {
        if (crdtChar.isEmpty())
            continue;

        QStringList fields;
        int start = 0;
        int nextFieldLength = 0;
        do {
            nextFieldLength = crdtChar.midRef(start, CRDT_FORMAT_LENGTH).toInt();
            start += CRDT_FORMAT_LENGTH;
            fields.append(crdtChar.mid(start, nextFieldLength));
            start += nextFieldLength;
        } while (start < crdtChar.size());

        if (fields.isEmpty())
            continue;

        KKCharPtr charPtr = KKCharPtr(new KKChar(fields[0].at(0).toLatin1(), fields[1].toStdString()));

        QStringList ids = fields[2].split(";");
        for(QString id : ids) {
            unsigned long val = stoul(id.toStdString(), nullptr, 0);
            charPtr->pushIdentifier(shared_ptr<KKIdentifier>(new KKIdentifier(val, fields[1].toStdString())));
        }

        charPtr->setKKCharFont(fields[3]);
        charPtr->setKKCharColor(fields[4]);
        text[text.size()-1].push_back(charPtr);

        // Se il carattere e' un 'a capo' si inserisce una nuova riga nel vettore di liste (text)
        if (charPtr->getValue() == '\n') {
            text.insert(text.end(), list<KKCharPtr>());
        }
    }
}

void KKCrdt::print() {
    std::cout << std::endl << "INIZIO STAMPA CRDT" << std::endl;
    for (auto& i : text) {
        for (const auto& x : i) {
            if(x->getValue() =='\n'){
                std::cout <<"/n"<< "[";
            } else {
                std::cout << x->getValue() << "[";
            }
            for (const auto& y: x->getPosition()) {
                std::cout << y->getDigit() << ";";
            }
            std::cout << "] ";
        }
        std::cout << std::endl;
    }
    std::cout << "FINE STAMPA CRDT" << std::endl;
}

void KKCrdt::printform() {
    std::cout << std::endl << "INIZIO STAMPA CRDT" << std::endl;
    for (auto& i : text) {
        for (const auto& x : i) {
            if(x->getValue() =='\n'){
                std::cout <<"/n"<< "[";
            } else {
                std::cout << x->getValue() << "[";
            }
            for (const auto& y: x->getPosition()) {
                std::cout << y->getDigit() << ";";
            }
            std::cout << "] ";
            std::cout << x->getKKCharFont().toUtf8().constData() <<"  "<< x->getKKCharColor().toUtf8().constData();
        }
        std::cout << std::endl;

    }
    std::cout << "FINE STAMPA CRDT" << std::endl;
}
//forse da eliminare
vector<KKIdentifierPtr> KKCrdt::slice(vector<KKIdentifierPtr> const &v, int i) {
    if(v.cbegin() == v.cend()) {
        return std::vector<KKIdentifierPtr>();
    }
    auto first = v.cbegin() + i;
    auto last = v.cend();

    std::vector<KKIdentifierPtr> list(first, last);
    return list;
}

list<KKCharPtr> KKCrdt::localDelete(KKPosition start_pos, KKPosition end_pos){
    bool new_line_removed=false;
    list<KKCharPtr> chars;

    if(start_pos.getLine() != end_pos.getLine()){ //controlla se hai cancellato piu di una riga
        new_line_removed = true;
        chars = deleteMultipleLines(start_pos,end_pos);
    }else{
        chars = deleteSingleLine(start_pos, end_pos);
        list<KKCharPtr>::iterator it;
        it=chars.begin();
        for (it = chars.begin(); it!= chars.end(); it++) { //controlla se hai cancellato un \n
            if(it->get()->getValue() == '\n'){
                new_line_removed=true;
            }
        }
    }
    removeEmptyLines();
    if(new_line_removed && !text[start_pos.getLine()+1].empty()){
        mergeLines(start_pos.getLine());
    }
    //text.emplace_back();
    this->print();
    return chars;
}



list<KKCharPtr> KKCrdt::deleteMultipleLines(KKPosition start_pos, KKPosition end_pos){
    list<KKCharPtr> chars(std::next(text[start_pos.getLine()].begin(),static_cast<long>(start_pos.getCh())),text[start_pos.getLine()].end());
    text[start_pos.getLine()].erase(std::next(text[start_pos.getLine()].begin(),static_cast<long>(start_pos.getCh())),text[start_pos.getLine()].end());

    unsigned long line;


    for(line=start_pos.getLine()+1;line<end_pos.getLine();line++){
        chars.splice(chars.end(),text[line]);
        text[line].erase(text[line].begin(),std::next(text[line].begin(),static_cast<long>(text[line].size())));

    }

    if(!text[end_pos.getLine()].empty()){
        //azioni ottimizabili
        list<KKCharPtr> tmp(text[end_pos.getLine()].begin(),std::next(text[end_pos.getLine()].begin(),static_cast<long>(end_pos.getCh())));
        text[end_pos.getLine()].erase(text[end_pos.getLine()].begin(),std::next(text[end_pos.getLine()].begin(),static_cast<long>(end_pos.getCh())));
        chars.splice(chars.end(),tmp);
    }

    return chars;
}

list<KKCharPtr> KKCrdt::deleteSingleLine(KKPosition start_pos, KKPosition end_pos){
    unsigned long char_num = end_pos.getCh() - start_pos.getCh();
    list<KKCharPtr> chars(std::next(text[start_pos.getLine()].begin(),static_cast<long>(start_pos.getCh())),std::next(text[start_pos.getLine()].begin(),static_cast<long>(start_pos.getCh() + char_num)));
    text[start_pos.getLine()].erase(std::next(text[start_pos.getLine()].begin(),static_cast<long>(start_pos.getCh())),std::next(text[start_pos.getLine()].begin(),static_cast<long>(start_pos.getCh() + char_num)));
    return chars;
}

void KKCrdt::removeEmptyLines(){

    vector<list<KKCharPtr>>::iterator it;
    it=text.begin();

    for(unsigned long line=0; line<text.size(); line++){
        if(text[line].empty()){
            text.erase(it);
            line --;
            it--;
        }
        it++;
    }
    if(text.empty()){
        text[0] = list<KKCharPtr>(); //occhio

    }
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
}

unsigned long KKCrdt::remoteDelete(const KKCharPtr& _Char){
    bool flag = true;
    unsigned long global_pos;

    KKPosition pos(findPos(_Char, &flag));//trova la posizione nel crdt del carattere

    if(!flag){
        return generateGlobalPos(pos);
    }


    text[pos.getLine()].erase(std::next(text[pos.getLine()].begin(),static_cast<long>(pos.getCh())));//rimuove il carattere cancellato

    if(_Char->getValue()=='\n' && !text[pos.getLine()+1].empty()){ //se il carattere era un \n ricollega le linee
        mergeLines(pos.getLine());
    }

    removeEmptyLines();
    //text.emplace_back();
    global_pos= generateGlobalPos(pos);
    this->print();
    return global_pos;
}

KKPosition KKCrdt::findPos (const KKCharPtr& _Char, bool *flag){

    //qDebug()<<"[findPos] char_value="<<_Char.get()->getValue();
    unsigned long min_line=0;
    unsigned long total_lines = static_cast<unsigned long>(text.size());
    unsigned long max_line = total_lines - 1;
    unsigned long mid_line;


    //se il testo è vuoto o la char ha una position inferiore alla prima in assoluto
    if (text.empty() || _Char.get()->compareTo(*text[0].front().get()) < 0) {
        *flag=false;
        return {0,0};
    }

    if(text[max_line].empty()){
        max_line--;
    }
   list<KKCharPtr> last_line(text[max_line]);
    KKChar last_char (**std::next(last_line.begin(),static_cast<long>(last_line.size()-1)));

    //se la char ha position maggiore dell'ultima char nel testo
    if(_Char.get()->compareTo(last_char)>0){
        *flag=false;
        return KKPosition(0,0);
    }
    //ricerca la linea dove si trova la char
    while (min_line +1 < max_line) {
        mid_line = static_cast<unsigned long>(floor(min_line + (max_line - min_line)/2));
        list<KKCharPtr> current_line (text[mid_line]);
        last_char = **std::next(current_line.begin(),static_cast<long>(current_line.size()-1));
        if(_Char.get()->compareTo(last_char)==0){
            return KKPosition(mid_line, static_cast<unsigned long>(current_line.size()-1));
        }if(_Char.get()->compareTo(last_char)<0) {
            max_line=mid_line;
        }else{
            min_line=mid_line;
        }
    }

    //ora la scelta è tra una o massimo due righe
    list<KKCharPtr> min_current_line (text[min_line]);
    KKChar min_last_char = **std::next(min_current_line.begin(),static_cast<long>(min_current_line.size()-1));
    list<KKCharPtr> max_current_line (text[max_line]);
    KKChar max_last_char = **std::next(max_current_line.begin(),static_cast<long>(max_current_line.size()-1));

    //confronta con l'ultima char della riga1 (per capire se è su quella riga o sulla riga2(se c'è))
    if(_Char.get()->compareTo(min_last_char)<=0){
        unsigned long char_idx = findIndexInLine(_Char,min_current_line,flag);
        return KKPosition(min_line,char_idx);
    }
    unsigned long char_idx = findIndexInLine(_Char,max_current_line,flag);
    return KKPosition(max_line,char_idx);
    
}

unsigned long KKCrdt::findIndexInLine(const KKCharPtr& _Char, list<KKCharPtr> line, bool *flag){

    unsigned long left=0;
    unsigned long right = static_cast<unsigned long>(line.size()-1);

    //controlla se la linea è vuota o il Char è minore del primo della linea
    if(line.empty() || _Char.get()->compareTo(**line.begin())<0){
        return left;
        //controlla se il Char è maggiore dell'ultimo della linea
    } if ( _Char.get()->compareTo(**std::next(line.begin(),static_cast<long>(line.size()-1)))>0){
        return right;
    }

    list<KKCharPtr>::iterator it;
    it=line.begin();
    unsigned long cnt=0;

    for (it = line.begin(); it!= line.end(); it++) {
        if(_Char.get()->compareTo(**it)==0 &&
                _Char.get()->getValue()==it->get()->getValue() &&
                _Char.get()->getSiteId()==it->get()->getSiteId()) {
            return cnt;
        }
        cnt++;
    }
    *flag=false;
    return 0;
}

unsigned long KKCrdt::generateGlobalPos(KKPosition pos){
    unsigned long global_pos=0;

    for (unsigned long i=0; i < pos.getLine(); i++) {
        global_pos = global_pos + static_cast<unsigned long>(text[i].size());
    }

    global_pos = global_pos + pos.getCh();
    return global_pos;
}

void KKCrdt::calculateLineCol(unsigned long global_pos, unsigned long *line, unsigned long *col){
    unsigned long tot=0; unsigned long succ=0;

    if(global_pos==0){
        *line=0;
        *col=0;
        return;
    }

    for(unsigned long i=0;i<text.size();i++){

        succ+=text[i].size();
        if(global_pos<=succ){

            if(global_pos==succ && std::next(text[i].begin(), static_cast<long>(text[i].size()-1))->get()->getValue()=='\n'){
                *line=i+1; *col=0;//per calcolare il carattere \n
            }else {
                *col=global_pos-tot;
                *line=i;
            }
            return;
        }
        tot+=text[i].size();
    }
    //qDebug() << "[calculateLineCol] line: " << *line << " - col: " << *col;

}
list<KKCharPtr> KKCrdt::changeMultipleKKCharFormat(KKPosition start, KKPosition end, QString font_, QString color_){

    list<KKCharPtr> changed;
    list<KKCharPtr>::iterator ch;

    for(auto line=start.getLine();line<=end.getLine();line++){
        if(line==start.getLine()){ //prima riga
            if(line==end.getLine()){
                //la selezione inizia e finisce su una stessa riga: cambia tutte le kkchar da start.getch() a end.getch()
                for(ch = std::next(text[line].begin(),static_cast<long>(start.getCh())); ch != std::next(text[line].begin(),static_cast<long>(end.getCh()+1)); ch++){
                    ch->get()->setKKCharFont(font_);
                    ch->get()->setKKCharColor(color_);

                    KKCharPtr new_Char = KKCharPtr(new KKChar(ch->get()->getValue(), this->siteid, font_, color_));
                    vector<KKIdentifierPtr> new_ids;
                    for (KKIdentifierPtr id:ch->get()->getPosition()){
                        KKIdentifierPtr newid  = KKIdentifierPtr(new KKIdentifier(id->getDigit(),id->getSiteId()));
                        new_ids.push_back(newid);
                    }
                    new_Char.get()->insertPosition(new_ids);

                    changed.push_back(new_Char);
                }
            }else{
                //la selezione è su piu righe diverse: cambia tutte le kkchar da start.getch() fino alla fine della riga
                for(ch = std::next(text[line].begin(), static_cast<long>(start.getCh())); ch != text[line].end(); ch++){
                    ch->get()->setKKCharFont(font_);
                    ch->get()->setKKCharColor(color_);

                    KKCharPtr new_Char = KKCharPtr(new KKChar(ch->get()->getValue(), this->siteid, font_, color_));
                    vector<KKIdentifierPtr> new_ids;
                    for (KKIdentifierPtr id:ch->get()->getPosition()){
                        KKIdentifierPtr newid  = KKIdentifierPtr(new KKIdentifier(id->getDigit(),id->getSiteId()));
                        new_ids.push_back(newid);
                    }
                    new_Char.get()->insertPosition(new_ids);

                    changed.push_back(new_Char);
                }
            }

        }else if(line==end.getLine()){
            //sei all'ultima riga cambia tutte le kkchar dall'inizio della riga fino a end.getch()
            for(auto ch = text[line].begin(); ch != std::next(text[line].begin(),static_cast<long>(end.getCh()+1)); ch++){
                ch->get()->setKKCharFont(font_);
                ch->get()->setKKCharColor(color_);

                KKCharPtr new_Char = KKCharPtr(new KKChar(ch->get()->getValue(), this->siteid, font_, color_));
                vector<KKIdentifierPtr> new_ids;
                for (KKIdentifierPtr id:ch->get()->getPosition()){
                    KKIdentifierPtr newid  = KKIdentifierPtr(new KKIdentifier(id->getDigit(),id->getSiteId()));
                    new_ids.push_back(newid);
                }
                new_Char.get()->insertPosition(new_ids);

                changed.push_back(new_Char);
            }
        }else{
        //la selezione è almeno su 3 righe, sei in una riga centrale: cambia tutte le KKchar di questa riga
        for(ch = text[line].begin(); ch != text[line].end(); ch++){
            ch->get()->setKKCharFont(font_);
             ch->get()->setKKCharColor(color_);

             KKCharPtr new_Char = KKCharPtr(new KKChar(ch->get()->getValue(), this->siteid, font_, color_));
             vector<KKIdentifierPtr> new_ids;
             for (KKIdentifierPtr id:ch->get()->getPosition()){
                 KKIdentifierPtr newid  = KKIdentifierPtr(new KKIdentifier(id->getDigit(),id->getSiteId()));
                 new_ids.push_back(newid);
             }
             new_Char.get()->insertPosition(new_ids);

             changed.push_back(new_Char);
        }
        }
    }

    return  changed;
}

unsigned long KKCrdt::remoteFormatChange(const KKCharPtr& _char,QString font_, QString color_){

    bool flag = true;
    unsigned long global_pos;
    KKPosition pos(findPos(_char, &flag));

    //qDebug()<<"[remoteFormatChange] flag="<<flag;

    if(!flag){
        return static_cast<unsigned long>(-1);
    }

    std::next(text[pos.getLine()].begin(),static_cast<long>(pos.getCh()))->get()->setKKCharFont(font_);
    std::next(text[pos.getLine()].begin(),static_cast<long>(pos.getCh()))->get()->setKKCharColor(color_);

    global_pos= generateGlobalPos(pos);
    // qDebug()<<"global_pos:"<<global_pos;
    return global_pos;

}


KKCharPtr KKCrdt::changeSingleKKCharFormat(KKPosition pos, QString font_, QString color_){
    std::next(text[pos.getLine()].begin(),static_cast<long>(pos.getCh()))->get()->setKKCharFont(font_);
    std::next(text[pos.getLine()].begin(),static_cast<long>(pos.getCh()))->get()->setKKCharColor(color_);
    list<KKCharPtr>::iterator ch;
    ch = std::next(text[pos.getLine()].begin(),static_cast<long>(pos.getCh()));

    KKCharPtr new_Char = KKCharPtr(new KKChar(ch->get()->getValue(), ch->get()->getSiteId(), font_, color_));
    vector<KKIdentifierPtr> new_ids;
    for (KKIdentifierPtr id:ch->get()->getPosition()){
        KKIdentifierPtr newid  = KKIdentifierPtr(new KKIdentifier(id->getDigit(),id->getSiteId()));
        new_ids.push_back(newid);
    }
    new_Char.get()->insertPosition(new_ids);
    return new_Char;
}

void KKCrdt::checkBackSlashN(){

}







