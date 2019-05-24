//
// Created by Paola on 24/05/2019.
//

#include "Utility.h"

template<typename T>
std::vector<T> slice(std::vector<T> const &v, int i) {
    auto first = v.cbegin() + i;
    auto last = v.cend();

    std::vector<T> vec(first, last);
    return vec;
}
template<typename T>
std::vector<T> slice(std::vector<T> const &v, int start, int end) {
    auto first = v.cbegin() + start;
    auto last = v.cbegin()+ end;

    std::vector<T> vec(first, last);
    return vec;
}



