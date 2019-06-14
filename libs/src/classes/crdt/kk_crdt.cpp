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


kk_crdt::kk_crdt(string siteid, strategy strategy) : siteid(siteid), boundary(20), _strategy(strategy), base(32) {
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
            return this->generate_position_between(slice(position1, 1), vector<kk_identifier_ptr>(), new_position,
                                                   level + 1);

        } else if (id1->get_siteid() == id2->get_siteid()) {
            new_position->insert(new_position->end(), id1);
            return this->generate_position_between(slice(position1, 1), slice(position2, 1), new_position, level + 1);

        } else {
            std::cout << "ERRORE IN GETPOSITION" << std::endl;//gestire errore
            return vector<kk_identifier_ptr>();
        }
    }
    return vector<kk_identifier_ptr>();
}

strategy kk_crdt::find_strategy(unsigned long level/*FORSE DA TOGLIERE*/) {
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


        list<kk_char_ptr> line_after(std::next(text[pos.get_line()].begin(),static_cast<long>(pos.get_ch())),text[pos.get_line()].end());
        text[pos.get_line()].erase(std::next(text[pos.get_line()].begin(),static_cast<long>(pos.get_ch())),text[pos.get_line()].end());

          if (line_after.size() == 0) {
               list<kk_char_ptr> _list_char = {_char};
               text[pos.get_line()].splice(std::next(text[pos.get_line()].begin(), static_cast<long>(pos.get_ch())), _list_char);
           } else {

               text[pos.get_line()].push_back(_char);
               list<kk_char_ptr> line_before(text[pos.get_line()]);
               text.erase(std::next(text.begin(),static_cast<long>(pos.get_line())));
               text.insert(std::next(text.begin(),static_cast<long>(pos.get_line())),line_before);
               text.insert(std::next(text.begin(),static_cast<long>(pos.get_line()+1)),line_after);

           }


    } else {
        if (text.at(pos.get_line()).empty()) {
            text.insert(text.begin() + static_cast<long>(pos.get_line()), list<kk_char_ptr>());
        }
        list<kk_char_ptr> _list_char = {_char};
        text[pos.get_line()].splice(std::next(text[pos.get_line()].begin(), static_cast<long>(pos.get_ch())), _list_char);
    }
}

void kk_crdt::remote_insert(kk_char_ptr _char){
    kk_pos pos = find_insert_position(_char);
    insert_char(_char, pos);
    // this.controller.insertIntoEditor(char.value, pos, char.siteId); //inserimento nel text edit di qt
}

kk_pos kk_crdt::find_insert_position(kk_char_ptr _char){
       unsigned long min_line=0;
       unsigned long total_lines = text.size();
       unsigned long max_line = total_lines - 2;
       unsigned long mid_line;

       list<kk_char_ptr> last_line(text[max_line]);


       //      let currentLine, midLine, charIdx, minCurrentLine, lastChar,
       //            maxCurrentLine, minLastChar, maxLastChar;

     if(text.empty() || _char.get()->compare_to(*text[0].front().get()) <= 0) {
              return kk_pos(0,0);
          }


     kk_char last_char = *std::next(last_line.begin(),static_cast<long>(last_line.size()-1))->get();

     if(_char.get()->compare_to(last_char)>0){
        return find_end_position(last_char, last_line, total_lines);
     }

     while (min_line +1 < max_line) {
         mid_line = static_cast<unsigned long>(floor(min_line + (max_line - min_line)/2));

         list<kk_char_ptr> current_line (text[mid_line]);
         last_char = *std::next(current_line.begin(),static_cast<long>(last_line.size()-1))->get();

         if(_char.get()->compare_to(last_char)==0){
             return kk_pos(mid_line,current_line.size()-1);
         }else if(_char.get()->compare_to(last_char)<0) {
             max_line=mid_line;
         }else{
             min_line=mid_line;
         }
     }

     list<kk_char_ptr> min_current_line (text[min_line]);
     kk_char min_last_char = *std::next(min_current_line.begin(),static_cast<long>(min_current_line.size()-1))->get();
     list<kk_char_ptr> max_current_line (text[max_line]);
     kk_char max_last_char = *std::next(max_current_line.begin(),static_cast<long>(max_current_line.size()-1))->get();

     if(_char.get()->compare_to(min_last_char)<=0){
        unsigned long char_idx = find_insert_index_in_line(_char,min_current_line);
        return kk_pos(min_line,char_idx);
     }else {
         unsigned long char_idx = find_insert_index_in_line(_char,max_current_line);
        return kk_pos(max_line,char_idx);
     }
}

kk_pos kk_crdt::find_end_position (kk_char last_char, list<kk_char_ptr> last_line, unsigned long total_lines){
    if (last_char.get_value() == '\n') {
        return kk_pos(total_lines,0);
    } else {
        return kk_pos(total_lines-2,last_line.size());
  }
}

unsigned long kk_crdt::find_insert_index_in_line(kk_char_ptr _char, list<kk_char_ptr> line){
    unsigned long left=0;
    unsigned long right = line.size()-1;


    if(line.size()==0 || _char.get()->compare_to(*line.begin()->get())<0){
        return left;
    } else if ( _char.get()->compare_to(*std::next(line.begin(),static_cast<long>(line.size()-1))->get())>0){
        return right;
    }

    list<kk_char_ptr>::iterator it;
    it=line.begin();
    unsigned long cnt=0;

    for (it = line.begin(); it!= line.end(); it++) {
        if(_char.get()->compare_to(*it->get())<0){
            return cnt;
         };
        cnt++;
    }
}

void kk_crdt::print() {
    for (unsigned long i = 0; i < text.size(); i++) {
        for (auto x : text[i]) {
            if(x->get_value() =='\n'){
                 std::cout <<"/n"<< "[ ";
            }else {
                std::cout << x->get_value() << "[ ";
            }
            for (auto y: x->get_position()) {
                std::cout << y->get_digit() << " ";
            }
            std::cout << "] ";
        }
        std::cout << std::endl;
    }
}

//forse da eliminare
vector<kk_identifier_ptr> kk_crdt::slice(vector<kk_identifier_ptr> const &v, int i) {
    if(v.cbegin() == v.cend()) {
        return std::vector<kk_identifier_ptr>();
    }
    auto first = v.cbegin() + i;
    auto last = v.cend();

    std::vector<kk_identifier_ptr> list(first, last);
    return list;
}

void kk_crdt::local_delete(kk_pos start_pos, kk_pos end_pos){
      bool new_line_removed=false;


      if(start_pos.get_line() != end_pos.get_line()){
              new_line_removed = true;
              list<kk_char_ptr> chars(delete_multiple_lines(start_pos,end_pos));
      }else{
          list<kk_char_ptr> chars = delete_single_line(start_pos, end_pos);

          list<kk_char_ptr>::iterator it;
          it=chars.begin();

          for (it = chars.begin(); it!= chars.end(); it++) {
              if(it->get()->get_value() == '\n'){
                  new_line_removed=true;
               }
          }
      }
          //    this.broadcast(chars); ???

          remove_empty_lines();
          if(new_line_removed && !text[start_pos.get_line()+1].empty()){
              merge_lines(start_pos.get_line());

      }
}



list<kk_char_ptr> kk_crdt::delete_multiple_lines(kk_pos start_pos, kk_pos end_pos){

    list<kk_char_ptr> chars(std::next(text[start_pos.get_line()].begin(),static_cast<long>(start_pos.get_ch())),text[start_pos.get_line()].end());
    text[start_pos.get_line()].erase(std::next(text[start_pos.get_line()].begin(),static_cast<long>(start_pos.get_ch())),text[start_pos.get_line()].end());

    unsigned long line;

//    vector<list<kk_char_ptr>>::iterator it;
//    it=text.begin();
//    advance(it,static_cast<long>(start_pos.get_line()+1));

    for(line=start_pos.get_line()+1;line<end_pos.get_line();line++){
        chars.splice(chars.end(),text[line]);
        text[line].erase(text[line].begin(),std::next(text[line].begin(),text[line].size()));
//        it++;
        }

    if(!text[end_pos.get_line()].empty()){
        //azioni ottimizabili
        list<kk_char_ptr> tmp(text[end_pos.get_line()].begin(),std::next(text[end_pos.get_line()].begin(),static_cast<long>(end_pos.get_ch())));
        text[end_pos.get_line()].erase(text[end_pos.get_line()].begin(),std::next(text[end_pos.get_line()].begin(),static_cast<long>(end_pos.get_ch())));
        chars.splice(chars.end(),tmp);
    }

    return chars;
}

list<kk_char_ptr> kk_crdt::delete_single_line(kk_pos start_pos, kk_pos end_pos){
    unsigned long char_num = end_pos.get_ch() - start_pos.get_ch();
    list<kk_char_ptr> chars(std::next(text[start_pos.get_line()].begin(),static_cast<long>(start_pos.get_ch())),std::next(text[start_pos.get_line()].begin(),static_cast<long>(start_pos.get_ch() + char_num)));
    text[start_pos.get_line()].erase(std::next(text[start_pos.get_line()].begin(),static_cast<long>(start_pos.get_ch())),std::next(text[start_pos.get_line()].begin(),static_cast<long>(start_pos.get_ch() + char_num)));
    return chars;
}

void kk_crdt::remove_empty_lines(){

    vector<list<kk_char_ptr>>::iterator it;
    it=text.begin();

    for(unsigned long line=0; line<text.size(); line++){
            if(text[line].size()==0){
            text.erase(it);
            line --;
            it--;
        }
        it++;
     }
    if(text.size()==0){
        text[0] = list<kk_char_ptr>(); //occhio

    }
}

void kk_crdt::merge_lines(unsigned long line){

    text[line].merge(text[line+1]);
    list<kk_char_ptr> merged_line(text[line]);

    //devi togliere line e line+1 e metter mergelined
    vector<list<kk_char_ptr>>::iterator it;
    it=text.begin();
    std::advance(it,static_cast<long>(line));
    text.erase(it);
    text.erase(it);
    text.insert(it,merged_line);
    return;
}
















