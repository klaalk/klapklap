//
// Created by Alberto Bruno on 2019-05-15.
//

#include <sys/time.h>
#include "kk_crdt.h"
#include "identifier/kk_identifier.h"
#include <cstdlib>
#include <ctime>
#include <random>




using std::string;
using std::shared_ptr;

kk_crdt::kk_crdt(string siteid, strategy strategy) : siteid(siteid), boundary(10), _strategy(strategy), base(32) {
};

void kk_crdt::local_insert(char val, kk_pos pos) {
    kk_char_ptr newChar = this->generate_Char(val, pos);
    this->insert_char(newChar, pos);
    return;
}

kk_char_ptr kk_crdt::generate_Char(char val, kk_pos pos) {
    vector<kk_identifier_ptr> position_before, position_after, new_position;
    kk_char_ptr new_Char = kk_char_ptr(new kk_char(val, this->siteid));
    position_before = this->find_position_before(pos);
    position_after = this->find_position_after(pos);
    new_position = this->generate_position_between(position_before, position_after, &new_position, 0);
    new_Char->insert_position(new_position);
    return new_Char;
}

vector<kk_identifier_ptr> kk_crdt::find_position_before(kk_pos pos) {
    if (pos.get_ch() == 0 && pos.get_line() == 0) {
        vector<kk_identifier_ptr> vuoto;
        return vuoto;
    } else if (pos.get_ch() == 0 && pos.get_line() != 0) {
        unsigned long line = pos.get_line() - 1;
        unsigned long ch = text[line].size();
        return std::next(text[line].begin(), static_cast<long>(ch) - 1)->get()->get_position();
    }
    return std::next(text[pos.get_line()].begin(), static_cast<long>(pos.get_ch()) - 1)->get()->get_position();
}

vector<kk_identifier_ptr> kk_crdt::find_position_after(kk_pos pos) {
    long num_lines, num_chars;
    vector<kk_identifier_ptr> vuoto;

    num_lines = static_cast<long>(text.size()) - 1;

    if (text.empty() || text[pos.get_line()].empty()) {
        num_chars = 0;
    } else {
        num_chars = static_cast<long>(text[pos.get_line()].size());
    }

    if (static_cast<long>(pos.get_line()) > num_lines - 1 && pos.get_ch() == 0) { //non dovrebbe mai succedere
        return vuoto;
    } else if (static_cast<long>(pos.get_line()) == num_lines - 1 && static_cast<long>(pos.get_ch()) == num_chars) { //sei all'ultima riga e all'ultimo char
        return vuoto;
    } else if (static_cast<long>(pos.get_line()) < num_lines - 1 && static_cast<long>(pos.get_ch()) == num_chars) { // sei a fine riga ma non nell'ultima riga
        return text[pos.get_line() + 1].front()->get_position();
    }
    return std::next(text[pos.get_line()].begin(),
                     static_cast<long>(pos.get_ch()))->get()->get_position(); // se non sei in nessun caso particolare DA RIVEDERE
}

vector<kk_identifier_ptr> kk_crdt::generate_position_between(vector<kk_identifier_ptr> position1, vector<kk_identifier_ptr> position2,
                                   vector<kk_identifier_ptr> *new_position, unsigned long level) {

    strategy _strategy;
    unsigned long elev = static_cast<unsigned long>(pow(2,level));
    unsigned long _base = elev * this->base;
    _strategy = this->find_strategy(level);

    kk_identifier_ptr id1, id2;


    if (position1.empty()) {
        id1 = shared_ptr<kk_identifier>(new kk_identifier(0, this->siteid));
    } else {
        id1 = shared_ptr<kk_identifier>(new kk_identifier(position1[0]->get_digit(), position1[0]->get_siteid()));
    }

    if (position2.empty()) {
        id2 = shared_ptr<kk_identifier>(new kk_identifier(_base, this->siteid));
    } else {
        id2 = shared_ptr<kk_identifier>(new kk_identifier(position2[0]->get_digit(), position2[0]->get_siteid()));
    }

    if (id2->get_digit() - id1->get_digit() > 1) {
        unsigned long new_digit;
        kk_identifier_ptr new_id;
        if(position2.empty()){
            new_digit = this->generate_identifier_between(id1->get_digit(), id2->get_digit(), _strategy);}
        else{
            new_digit = this->generate_identifier_between(id1->get_digit(), id2->get_digit()-1, _strategy);
        };

        new_id = shared_ptr<kk_identifier>(new kk_identifier(new_digit, this->siteid));
        new_position->insert(new_position->end(), new_id);
        return *new_position;
    } else if (id2->get_digit() - id1->get_digit() == 1) {
        new_position->insert(new_position->end(), id1);
        return this->generate_position_between(slice(position1, 1), vector<kk_identifier_ptr>(), new_position, level + 1);
    } else if (id1->get_digit() == id2->get_digit()) {
        if (id1->get_siteid() < id2->get_siteid()) {
            new_position->insert(new_position->end(), id1);
            return this->generate_position_between(slice(position1, 1), vector<kk_identifier_ptr>(), new_position, level + 1);
        } else if (id1->get_siteid() == id2->get_siteid()) {
            new_position->insert(new_position->end(), id1);
            return this->generate_position_between(slice(position1, 1), slice(position2, 1), new_position, level + 1);
        } else {
            std::cout << "ERRORE IN GETPOSITION" << std::endl;
            //gestire errore
            return vector<kk_identifier_ptr>();
        }
    }
    return vector<kk_identifier_ptr>();
}

strategy kk_crdt::find_strategy(unsigned long level) {
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
   //this->strategy_cache.insert(strategy_cache.begin() + static_cast<int>(level), _local_strategy);
    return _local_strategy;
}

unsigned long kk_crdt::generate_identifier_between(unsigned long min, unsigned long max, strategy _strategy) {

    if ((max - min < this->boundary)) {
        min = min + 1;
    } else {
        if (_strategy == minus) {
            min=min+1;
            min = max - this->boundary;
        } else {
            min = min + 1;
            max = min + this->boundary;
        }
    }

    unsigned long random_number;

    std::random_device rd; // obtain a random number from hardware
    std::mt19937 eng(rd()); // seed the generator
    std::uniform_int_distribution<> distr(static_cast<int>(min), static_cast<int>(max));
    random_number=static_cast<unsigned long>(distr(eng));

    if ((max - min < this->boundary)) {
        std::cout<<"non importa"<<std::endl;
    }else{
        if(_strategy==minus){
             std::cout<<"minus"<<std::endl;
        }else if(_strategy==plus){
            std::cout<<"plus"<<std::endl;
       } else if(_strategy==casuale){
            std::cout<<"casuale"<<std::endl;
       }

    }



    std::cout<<min <<"  "<<random_number <<"  "<<max <<std::endl;

    return random_number;

}

void kk_crdt::insert_char(kk_char_ptr _char, kk_pos pos) {
    // Inizializzo la prima riga.
    if (pos.get_line() == text.size()) {
        text.insert(text.end(), list<kk_char_ptr>());
    }

    // if inserting a newline, split line into two lines
    if (_char->get_value() == '\n') {


        list<kk_char_ptr> line_after={};
        line_after.splice(line_after.begin(), line_after, std::next(text[pos.get_line()].begin(), 2), text[pos.get_line()].end());

           if (line_after.size() == 0) {
               list<kk_char_ptr> _list_char = {_char};
               text[pos.get_line()].splice(std::next(text[pos.get_line()].begin(), static_cast<long>(pos.get_ch())), _list_char);
           } else {
               text[pos.get_line()].insert(text[pos.get_line()].end(),_char);
               list<kk_char_ptr> line_before(text[pos.get_line()]);
               text.erase(std::next(text.begin(),pos.get_line()));
               text.insert(std::next(text.begin(),pos.get_line()),line_before);
               text.insert(std::next(text.begin(),pos.get_line()+1),line_after);

           }


    } else {
        if (text.at(pos.get_line()).empty()) {
            text.insert(text.begin() + static_cast<long>(pos.get_line()), list<kk_char_ptr>());
        }
        list<kk_char_ptr> _list_char = {_char};
        text[pos.get_line()].splice(std::next(text[pos.get_line()].begin(), static_cast<long>(pos.get_ch())), _list_char);
    }
}
//
//void kk_crdt::handle_remote_insert(kk_char _char){
//    kk_pos pos = find_insert_position(_char);
//    insert_char(_char, pos);
//    //    this.controller.insertIntoEditor(char.value, pos, char.siteId);
//}
//
//kk_pos kk_crdt::find_insert_position(kk_char _char){
//    int min_line = 0;
//    int total_lines = text.size();
//    int maxLine = total_lines - 1;
//    list<kk_char> last_line = text[maxLine];
//
//    int char_idx, mid_line;
//    list<kk_char> current_line, min_current_line, max_current_line;
//    kk_char min_last_char, max_last_char, last_char;
//
//    if(is_empty() || _char.compare_to(text[0][0]) <= 0) {
//        return kk_pos(0,0);
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
//            return kk_pos(mid_line, current_line.size() - 1);
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
//        return kk_pos(min_line, char_idx);
//    } else {
//        char_idx = find_insert_index_in_line(_char, max_current_line);
//        return kk_pos(maxLine, char_idx);
//    }
//}
//
//int kk_crdt::is_empty(){
//    return text.size() == 1 && text[0].size() == 0;
//}
//
//kk_pos kk_crdt::find_end_position (kk_char last_char, list<kk_char> last_line, int total_lines){
//
//}
//
//int kk_crdt::find_insert_index_in_line(kk_char _char, list<kk_char> line){
//
//}



void kk_crdt::print() {
    for (unsigned long i = 0; i < text.size(); i++) {
        for (auto x : text[i]) {
            std::cout << x->get_value() << "[ ";
            for (auto y: x->get_position()) {
                std::cout << y->get_digit() << " ";
            }
            std::cout << "] ";
        }
        std::cout << std::endl;
    }
}

vector<kk_identifier_ptr> kk_crdt::slice(vector<kk_identifier_ptr> const &v, int i) {
    if(v.cbegin() == v.cend()) {
        return std::vector<kk_identifier_ptr>();
    }
    auto first = v.cbegin() + i;
    auto last = v.cend();

    std::vector<kk_identifier_ptr> list(first, last);
    return list;
}
