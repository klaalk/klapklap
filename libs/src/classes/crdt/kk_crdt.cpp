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
        unsigned long ch = text[line].size();
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

    if (static_cast<long>(pos.getLine()) > num_lines - 1 && pos.getCh() == 0) {
        return vuoto;
    } if (static_cast<long>(pos.getLine()) == num_lines - 1 && static_cast<long>(pos.getCh()) == num_chars) { //sei all'ultima riga e all'ultimo char
        return vuoto;
    } else if (static_cast<long>(pos.getLine()) < num_lines - 1 && static_cast<long>(pos.getCh()) == num_chars) { // sei a fine riga ma non nell'ultima riga
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
    // Inizializzo la prima riga.
    if (pos.getLine() == text.size()) {
        text.insert(text.end(), list<KKCharPtr>());
    }

    // if inserting a newline, split line into two lines
    if (_char->getValue() == '\n') {
        list<KKCharPtr> line_after(std::next(text[pos.getLine()].begin(),static_cast<long>(pos.getCh())),text[pos.getLine()].end());
        text[pos.getLine()].erase(std::next(text[pos.getLine()].begin(),static_cast<long>(pos.getCh())),text[pos.getLine()].end());
        if (line_after.empty()) {
            list<KKCharPtr> _list_char = {_char};
            text[pos.getLine()].splice(std::next(text[pos.getLine()].begin(), static_cast<long>(pos.getCh())), _list_char);
        } else {
            text[pos.getLine()].push_back(_char);
            list<KKCharPtr> line_before(text[pos.getLine()]);
            text.erase(std::next(text.begin(),static_cast<long>(pos.getLine())));
            text.insert(std::next(text.begin(),static_cast<long>(pos.getLine())),line_before);
            text.insert(std::next(text.begin(),static_cast<long>(pos.getLine()+1)),line_after);
        }
    } else {
        if (text.at(pos.getLine()).empty()) {
            text.insert(text.begin() + static_cast<long>(pos.getLine()), list<KKCharPtr>());
        }
        list<KKCharPtr> _list_char = {_char};
        text[pos.getLine()].splice(std::next(text[pos.getLine()].begin(), static_cast<long>(pos.getCh())), _list_char);
    }
}

unsigned long KKCrdt::remoteInsert(const KKCharPtr& _char){
    unsigned long global_pos;

    KKPosition pos = findInsertPosition(_char);
    insertChar(_char, pos);
    global_pos= generateGlobalPos(pos);
    return global_pos;
}

KKPosition KKCrdt::findInsertPosition(const KKCharPtr& _char){
    unsigned long min_line=0;
    unsigned long total_lines = text.size();
    unsigned long mid_line;
    unsigned long max_line = total_lines-2;
    //controlla se la char va messa come primo carattere della prima riga
    if(text.empty() || text.at(0).empty() ||_char.get()->compareTo(*text[0].front().get()) <= 0) {
        return {0,0};
    }

    list<KKCharPtr> last_line(text[max_line]);

    KKChar last_char = **std::next(last_line.begin(),static_cast<long>(last_line.size()-1));
    //controlla se la char va messa come ultimo carattere dell'a prima'ultima riga
    if(_char.get()->compareTo(last_char)>0){
        return findEndPosition(last_char, last_line, total_lines);
    }

    while (min_line +1 < max_line) {
        mid_line = static_cast<unsigned long>(floor(min_line + (max_line - min_line)/2));

        list<KKCharPtr> current_line (text[mid_line]);

        last_char = **std::next(current_line.begin(),static_cast<long>(current_line.size()-1));

        if(_char.get()->compareTo(last_char)==0){
            return KKPosition(mid_line,current_line.size()-1);
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

KKPosition KKCrdt::findEndPosition (KKChar last_char, const list<KKCharPtr>& last_line, unsigned long total_lines){
    if (last_char.getValue() == '\n') {
        return {total_lines-1,0};
    }
    return KKPosition(total_lines-2,last_line.size());
    
}

unsigned long KKCrdt::findInsertIndexInLine(const KKCharPtr& _char, list<KKCharPtr> line){
    unsigned long left=0;
    unsigned long right = line.size()-1;


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
QString KKCrdt::saveCrdt(){
    string strTmp;

    for (const auto &line : text) {

        for (const auto&word : line) {
            strTmp.push_back(word->getValue());
            strTmp.push_back('(');
            strTmp.append(word->getSiteId());
            strTmp.push_back(')');
            strTmp.push_back('[');

            for (const auto&position : word->getPosition()) {
                std::stringstream strstream;
                string app;
                strstream<<position->getDigit();
                strstream>>app;
                strTmp.append(app);
                strTmp.push_back(';');
            }

            strTmp.pop_back();
            strTmp.push_back(']');
            strTmp.append(word->getKKCharFont().toStdString());
            strTmp.push_back('*');
        }
    }

    QString stringCrdt = QString::fromStdString(strTmp);
    return stringCrdt;
}

void KKCrdt::loadCrdt(string stdStringCrdt){
    // Flag per quando stai prendendo gli identifier/i siteId
    int gettingSeq=1;

    KKIdentifierPtr new_id;
    text.insert(text.end(), list<KKCharPtr>());

    char ch;
    for(unsigned long i=0; i < stdStringCrdt.length(); i++){
        ch = stdStringCrdt[i];

        // Creo KKChar partendo dal carattere e siteId nullo
        KKCharPtr new_Char = KKCharPtr(new KKChar(ch, ""));

        gettingSeq = 1;
        string tmpSiteId;
        while(gettingSeq){
            i++;
            ch=stdStringCrdt[i];
            if(ch=='('){
                continue;
            } if(ch!='(' && ch!=')'){
                tmpSiteId.push_back(ch); //metto valore nella string di siteId

                continue;

            } else if(ch==')'){
                new_Char->insertSiteId(tmpSiteId);
                gettingSeq=0;
            }
        }

        gettingSeq=1;

        // Inizia a prendere gli identifiers
        while(gettingSeq) {
            i++;
            ch = stdStringCrdt[i];
            if (ch=='[' || ch==';') {
                continue;
            } if (stdStringCrdt[i]==']') {
                gettingSeq=0;
                break;
            }else {
                // Prendo l'identifier
                string num;
                while(stdStringCrdt[i]!=';' && stdStringCrdt[i]!=']') {
                    ch=stdStringCrdt[i];
                    num.push_back(ch);
                    i++;
                }

                unsigned long val;
                val=stoul(num, nullptr, 0);
                new_id = shared_ptr<KKIdentifier>(new KKIdentifier(val, tmpSiteId));
                new_Char->pushIdentifier(new_id);
                i--;
            }
        }

        gettingSeq=1;
        QString font;

        // Inizia a prendere il font
        while(gettingSeq) {
            i++;
            ch = stdStringCrdt[i];
            if (ch == '*') {
                unsigned long size = static_cast<unsigned long>(text.size());
                new_Char->setKKCharFont(font);
                text[size-1].push_back(new_Char);

                // Se il carattere e' un 'a capo' si inserisce una nuova riga nel vettore di liste (text)
                if (new_Char->getValue()=='\n') {
                    text.insert(text.end(), list<KKCharPtr>());
                }
                gettingSeq=0;
            } else {
                font.push_back(ch);
            }
        }
    }

    this->print();
}

void KKCrdt::print() {
    for (auto & i : text) {
        for (const auto& x : i) {
            if(x->getValue() =='\n'){
                std::cout <<"/n"<< "[ ";
            }else {
                std::cout << x->getValue() << "[ ";
            }
            for (const auto& y: x->getPosition()) {
                std::cout << y->getDigit() << " ";
            }
            std::cout << "] ";
        }
        std::cout << std::endl;

    }
    std::cout << std::endl;
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
    if(start_pos.getLine() != end_pos.getLine()){
        new_line_removed = true;
        chars = deleteMultipleLines(start_pos,end_pos);
    }else{
        chars = deleteSingleLine(start_pos, end_pos);
        list<KKCharPtr>::iterator it;
        it=chars.begin();
        for (it = chars.begin(); it!= chars.end(); it++) {
            if(it->get()->getValue() == '\n'){
                new_line_removed=true;
            }
        }
    }
    removeEmptyLines();
    if(new_line_removed && !text[start_pos.getLine()+1].empty()){
        mergeLines(start_pos.getLine());
    }
    text.emplace_back();
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
    KKPosition pos(findPos(_Char, &flag));

    if(!flag){
        return generateGlobalPos(pos);
    }

    text[pos.getLine()].erase(std::next(text[pos.getLine()].begin(),static_cast<long>(pos.getCh())));

    if(_Char->getValue()=='\n' && !text[pos.getLine()+1].empty()){
        mergeLines(pos.getLine());
    }

    removeEmptyLines();
    text.emplace_back();
    global_pos= generateGlobalPos(pos);
    return global_pos;
}

KKPosition KKCrdt::findPos (const KKCharPtr& _Char, bool *flag){
    unsigned long min_line=0;
    unsigned long total_lines = text.size();
    unsigned long max_line = total_lines - 2;
    unsigned long mid_line;
    list<KKCharPtr> last_line(text[max_line]);

    //se il testo è vuoto o la char ha una position inferiore alla prima in assoluto
    if (text.empty() || _Char.get()->compareTo(*text[0].front().get()) < 0) {
        *flag=false;
        return {0,0};
    }

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
            return KKPosition(mid_line,current_line.size()-1);
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
    unsigned long right = line.size()-1;

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

    for (unsigned long i=0; i <  pos.getLine(); i++) {
        global_pos = global_pos + text[i].size();
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
        if(global_pos<=succ){ //linea trovata
            *line=i;
            *col=global_pos-tot;
            return;
        }
        tot+=text[i].size();
    }

}
list<KKCharPtr> KKCrdt::changeMultipleKKCharFormat(KKPosition start, KKPosition end, QString font_, QString color_){

    list<KKCharPtr> changed;
    for(auto line=start.getLine();line<=end.getLine();line++){
        if(line==start.getLine()){ //prima riga
            if(line==end.getLine()){
                //la selezione inizia e finisce su una stessa riga: cambia tutte le kkchar da start.getch() a end.getch()
                for(auto ch = std::next(text[line].begin(),static_cast<long>(start.getCh())); ch != std::next(text[line].begin(),static_cast<long>(end.getCh()+1)); ch++){
                    ch->get()->setKKCharFont(font_);
                    ch->get()->setKKCharColor(color_);
                    changed.push_back(KKCharPtr(ch->get()));
                }
            }else{
                //la selezione è su piu righe diverse: cambia tutte le kkchar da start.getch() fino alla fine della riga
                for(auto ch = std::next(text[line].begin(), static_cast<long>(start.getCh())); ch != text[line].end(); ch++){
                    ch->get()->setKKCharFont(font_);
                    ch->get()->setKKCharColor(color_);
                    changed.push_back(KKCharPtr(ch->get()));
                }
            }

        }else if(line==end.getLine()){
            //sei all'ultima riga cambia tutte le kkchar dall'inizio della riga fino a end.getch()
            for(auto ch = text[line].begin(); ch != std::next(text[line].begin(),static_cast<long>(end.getCh()+1)); ch++){
                ch->get()->setKKCharFont(font_);
                ch->get()->setKKCharColor(color_);
                changed.push_back(KKCharPtr(ch->get()));
            }
        }else{
        //la selezione è almeno su 3 righe, sei in una riga centrale: cambia tutte le KKchar di questa riga
        for(auto ch = text[line].begin(); ch != text[line].end(); ch++){
            ch->get()->setKKCharFont(font_);
             ch->get()->setKKCharColor(color_);
             changed.push_back(KKCharPtr(ch->get()));
        }
        }
    }
    return  changed;
}

unsigned long KKCrdt::remoteFormatChange(const KKCharPtr& _char,QString font_, QString color_){
    bool flag = true;
    unsigned long global_pos;
    KKPosition pos(findPos(_char, &flag));

    if(!flag){
        return -1;
    }

    std::next(text[pos.getLine()].begin(),static_cast<long>(pos.getCh()))->get()->setKKCharFont(font_);
    std::next(text[pos.getLine()].begin(),static_cast<long>(pos.getCh()))->get()->setKKCharColor(color_);

    global_pos= generateGlobalPos(pos);
    return global_pos;

}










