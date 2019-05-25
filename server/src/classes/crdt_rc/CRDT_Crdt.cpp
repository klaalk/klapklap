//
// Created by Alberto Bruno on 2019-05-15.
//

#include "CRDT_Crdt.h"
#include "CRDT_identifier.h"

using std::string;



CRDT_Crdt::CRDT_Crdt(string siteid,strategy strategy):siteid(siteid),boundary(10),_strategy(casuale),base(32){
};

void CRDT_Crdt::local_insert(char val,CRDT_pos pos ){
    CRDT_Char* new_Char = new CRDT_Char(val, this->siteid);
    *new_Char = this->generate_Char(val,pos);
    this->insert_char(*new_Char,pos);
    return;

}

CRDT_Char CRDT_Crdt::generate_Char(char val, CRDT_pos pos){

    vector<CRDT_identifier> position_before, position_after, new_position;
    CRDT_Char* new_Char = new CRDT_Char(val, this->siteid);

    position_before=this->find_position_before(pos);
    position_after=this->find_position_after(pos);
    new_position=this->generate_position_between(position_before,position_after,&new_position,0);
    new_Char->insert_position(new_position);
    return *new_Char;
}

vector<CRDT_identifier> CRDT_Crdt::find_position_before(CRDT_pos pos){

    if(pos.get_ch()==0 && pos.get_line()==0){
        vector<CRDT_identifier> vuoto;
        return vuoto;
    }else if(pos.get_ch()==0 && pos.get_line()!=0){
        int line = pos.get_line()-1;
        int ch = text[line].size();
        return std::next(text[line].begin(), ch -1)->get_position();
    }
    return std::next(text[pos.get_line()].begin(),pos.get_ch()-1)->get_position();
}

vector<CRDT_identifier> CRDT_Crdt::find_position_after(CRDT_pos pos){
    int num_lines, num_chars;
    vector<CRDT_identifier> vuoto;

    num_lines = text.size();

    if(text.empty() || text[pos.get_line()].empty()){
        num_chars=0;
    }else{
        num_chars=text[pos.get_line()].size();
    }

    if(pos.get_line() > num_lines - 1 && pos.get_ch() == 0){ //non dovrebbe mai succedere
        return vuoto;
    }else if(pos.get_line() == num_lines - 1 && pos.get_ch() == num_chars){ //sei all'ultima riga e all'ultimo char
        return vuoto;
    }else if(pos.get_line() < num_lines - 1 && pos.get_ch() == num_chars) { // sei a fine riga ma non nell'ultima riga
        return text[pos.get_line()].front().get_position();
    }
    return std::next(text[pos.get_line()].begin(),pos.get_ch())->get_position(); // se non sei in nessun caso particolare DA RIVEDERE
}

vector<CRDT_identifier> CRDT_Crdt::generate_position_between(vector<CRDT_identifier> position1,vector<CRDT_identifier> position2,vector<CRDT_identifier> *new_position, int level) {

    strategy _strategy;

    base = pow(2, level) * this->base;
    _strategy = this->find_strategy(level);
    CRDT_identifier *id1, *id2;


    if (position1.empty()) {
        id1 = new CRDT_identifier(0, this->siteid);
    } else {
        id1 = new CRDT_identifier(position1[0].get_digit(), position1[0].get_siteid());
    }

    if (position2.empty()) {
        id2 = new CRDT_identifier(base, this->siteid);
    } else {
        id2 = new CRDT_identifier(position2[0].get_digit(), position2[0].get_siteid());
    }

    if (id2->get_digit() - id1->get_digit() > 1) {

        int new_digit;
        CRDT_identifier *new_id;

        new_digit = this->generate_identifier_between(id1->get_digit(), id2->get_digit(), _strategy);
        new_id = new CRDT_identifier(new_digit, this->siteid);
        new_position->insert(new_position->end(), *new_id);
        return *new_position;

    } else if (id2->get_digit() - id1->get_digit() == 1) {

        new_position->insert(new_position->end(), *id1);

        return this->generate_position_between(slice(position1, 1),vector<CRDT_identifier>(),new_position,level+1);

    } else if(id1->get_digit()==id2->get_digit()){
        if(id1->get_siteid()<id2->get_siteid()){
            new_position->insert(new_position->end(), *id1);
            return this->generate_position_between(slice(position1, 1),vector<CRDT_identifier>(), new_position,level+1);

        }else if(id1->get_siteid()==id2->get_siteid()){
            new_position->insert(new_position->end(), *id1);
            return this->generate_position_between(slice(position1, 1),slice(position2, 1), new_position,level+1);

        }else{
           std::cout<<"ERRORE IN GETPOSITION"<<std::endl;//gestire errore
           return vector<CRDT_identifier>();
        }
    }
    return vector<CRDT_identifier>();
}

strategy CRDT_Crdt::find_strategy(int level){
    if (!strategy_cache.empty() && strategy_cache[level]) {
        return strategy_cache[level];
    }
    strategy _local_strategy;
    switch (this->_strategy) {
        case plus:
            _local_strategy = plus;
        case minus:
            _local_strategy = minus;
        case casuale:
            _local_strategy = round(rand()) == 0 ? plus : minus;
        default:
            _local_strategy = (level%2) == 0 ? plus : minus;
    }
    this->strategy_cache.insert(strategy_cache.begin() + level, _local_strategy);
    return _local_strategy;
}

int CRDT_Crdt::generate_identifier_between(int min, int max, strategy _strategy) {
    if((max-min<this->boundary)){
        min=min+1;
    }else{
        if(_strategy== minus) {
            min = max - this->boundary;
        }else{
            min=min+1;
            max=min + this->boundary;
        }
    }
    double _rand = ((double) rand() / RAND_MAX);
return floor( _rand * (max - min)) + min;
}
//
//void CRDT_Crdt::handle_remote_insert(CRDT_Char _char){
//    CRDT_pos pos = find_insert_position(_char);
//    insert_char(_char, pos);
//    //    this.controller.insertIntoEditor(char.value, pos, char.siteId);
//}
//
//CRDT_pos CRDT_Crdt::find_insert_position(CRDT_Char _char){
////    int min_line = 0;
//    int total_lines = text.size();
//    int maxLine = total_lines - 1;
//    list<CRDT_Char> last_line = text[maxLine];
//
//    int char_idx, mid_line;
//    list<CRDT_Char> current_line, min_current_line, max_current_line;
//    CRDT_Char min_last_char, max_last_char, last_char;
//
//    if(is_empty() || _char.compare_to(text[0][0]) <= 0) {
//        return CRDT_pos(0,0);
//    }
//    last_char = last_line[last_line.size() - 1];
//
//    if(_char.compare_to(last_char) > 0) {
//        return find_end_position(last_char, last_line, total_lines);
//    }
//
//    // binary search
//    while (min_line + 1 < maxLine) {
//        mid_line = floor(min_line + (maxLine - min_line) / 2);
//        current_line = text[mid_line];
//        last_char = current_line[current_line.size() - 1];
//
//        if (_char.compare_to(last_char) == 0) {
//            return CRDT_pos(mid_line, current_line.size() - 1);
//        } else if (_char.compare_to(last_char) < 0) {
//            maxLine = mid_line;
//        } else {
//            min_line = mid_line;
//        }
//    }
//
//    // Check between min and max line.
//    min_current_line = text[min_line];
//    min_last_char = min_current_line[min_current_line.size() - 1];
//    max_current_line = text[maxLine];
//
//    if (_char.compare_to(min_last_char) <= 0) {
//        char_idx = find_insert_index_in_line(_char, min_current_line);
//        return CRDT_pos(min_line, char_idx);
//    } else {
//        char_idx = find_insert_index_in_line(_char, max_current_line);
//        return CRDT_pos(maxLine, char_idx);
//    }
//}
//
//int CRDT_Crdt::is_empty(){
//    return text.size() == 1 && text[0].size() == 0;
//}
//
//CRDT_pos CRDT_Crdt::find_end_position (CRDT_Char last_char, list<CRDT_Char> last_line, int total_lines){
//
//}
//
//int CRDT_Crdt::find_insert_index_in_line(CRDT_Char _char, list<CRDT_Char> line){
//
//}

void CRDT_Crdt::insert_char(CRDT_Char _char, CRDT_pos pos){
    if (pos.get_line() == text.size()) {
        text.insert(text.end(), list<CRDT_Char>());
    }

    // if inserting a newline, split line into two lines
    if (_char.get_value() == '\n') {
//        list<CRDT_Char> pos_line = text[pos.get_line()];
//        list<CRDT_Char> new_pos_line = Utility::splice<CRDT_Char>(pos_line, pos.get_ch());
//        text[pos.get_line()] = new_pos_line;
//
//        if (new_pos_line.size() == 0) {
//            list<CRDT_Char> pos_line = text[pos.get_line()];
//            text[pos.get_line()].insert(_char);
//        } else {
//            const lineBefore = this.struct[pos.get_line()].concat(char);
//            this.struct.splice(pos.get_line(), 1, lineBefore, lineAfter);
//        }
    } else {
        if(text.at(pos.get_line()).empty()) {
            text.insert(text.begin() + pos.get_line(),list<CRDT_Char>());
        }
        list<CRDT_Char> _list_char = {_char};
        text[pos.get_line()].splice(std::next(text[pos.get_line()].begin(), pos.get_ch()), _list_char);
    }
}

void CRDT_Crdt::print() {
    for(int i = 0; i < text.size(); i++) {
        for(auto x : text[i]) {
            std::cout<<x.get_value() << "[ ";
            for(auto y: x.get_position()) {
                std::cout << y.get_digit() << " ";
            }
            std::cout<<"] ";
        }
        std::cout<<std::endl;
    }
}

vector<CRDT_identifier> CRDT_Crdt::slice(vector<CRDT_identifier> const &v, int i) {
    auto first = v.cbegin() +1 ;
    auto last = v.cend();

    std::vector<CRDT_identifier> list(first, last);
    return list;
}