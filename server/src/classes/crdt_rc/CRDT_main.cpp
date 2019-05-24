#include <iostream>
#include "CRDT_Char.h"
#include "CRDT_identifier.h"
#include <stdlib.h>
#include <string>
#include "CRDT_Crdt.h"

int main_rtrrtrr() {
//
//   /* Test compare To
//    * int i;
//    identifier *a = new identifier(1,"polito");
//    identifier *b = new identifier(2,"polito");
//    //std::string siteid= "polito";
//    CRDT_Char *A = new CRDT_Char('A',"polito");
//    CRDT_Char *B= new CRDT_Char('A',"polito");
//
//    A->push_identifier(*a);
//    B->push_identifier(*b);
//
//
//    i=A->compare_to(*B);
//    std::cout<<i;
//*/
CRDT_Crdt *crdt = new CRDT_Crdt("Canguro",casuale);
crdt->local_insert('c',CRDT_pos(0,0));
crdt->local_insert('a',CRDT_pos(0,1));
crdt->local_insert('o',CRDT_pos(0,2));
crdt->local_insert('i',CRDT_pos(0,1));


    return 0;
}