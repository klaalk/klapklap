//
// Created by Alberto Bruno on 2019-05-12.
//

#include "identifier.h"
#include <stdio.h>
#include <stdlib.h>

int identifier::compareTo(const identifier &other) {
    if(this->digit<other.digit){
        return -1;
    }
    else if(this->digit > other.digit){
        return 1;
    }
    else if(this->siteid < other.siteid){
        return -1;
    }
    return 1;
}



