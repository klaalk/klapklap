//
// Created by Klaus on 06/05/2019.
//

#ifndef CLIENT_CHAT_MESSAGE_H
#define CLIENT_CHAT_MESSAGE_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../../constants/kk_constants.h"

class kk_payload {
public:
    enum {
        header_length = 10
    };
    enum {
        max_body_length = 512
    };

    /// Costruttuore. Inizializza il body length a 0.
    kk_payload();

    /// Ritorna il messaggio come costante.
    const char *data() const;

    /// Ritorna il messaggio modificabile.
    char *data();

    /// Ritorna la grandezza di tutto il messaggio.
    /// Header + Body
    size_t length() const;

    /// Ritorna il corpo del messaggio come costante.
    const char *body() const;

    /// Ritorna il corpo del messaggio modificabile.
    char *body();

    /// Lunghezza del corpo del messaggio.
    size_t body_length() const;

    /// Imposta il body length in base al massimo lenght possibile.
    void body_length(size_t length);

    kk_payload_type type();

    kk_payload_result_type result_type();

    kk_payload_type decode_header();

    void encode_header(kk_payload_type type, kk_payload_result_type result);

    void delete_data();

private:
    char data_[header_length + max_body_length];
    kk_payload_type type_;
    kk_payload_result_type result_;
    size_t body_length_;
};

#endif //CLIENT_CHAT_MESSAGE_H
