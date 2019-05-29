//
// Created by Paola on 24/05/2019.
//
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

using std::vector;

#ifndef SERVER_CRDT_UTILITY_H
#define SERVER_CRDT_UTILITY_H


class Utility {

public:
    template<typename T>
    std::vector<T> slice(std::vector<T> const &v, int i);

    template<typename T>
    std::vector<T> slice(std::vector<T> const &v, int start, int end);

    template<typename T>
    std::vector<T> splice(std::vector<T> const &v, int i);

};


#endif //SERVER_CRDT_UTILITY_H
