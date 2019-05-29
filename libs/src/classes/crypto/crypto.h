//
// Created by jsnow on 23/05/19.
//

#ifndef SERVER_CRYPTO_H
#define SERVER_CRYPTO_H

#include <iostream>
#include <cstring>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/err.h>


class crypto {

private:
    int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *aad,
                int aad_len, unsigned char *tag, unsigned char *key, unsigned char *iv,
                unsigned char *plaintext);

    int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *aad,
                int aad_len, unsigned char *key, unsigned char *iv,
                unsigned char *ciphertext, unsigned char *tag);

    void handleErrors(void);

public:
    explicit crypto(void);

    std::string _encrypt(std::string password, int *cifred_len);

    std::string _decrypt(std::string password, int *cifred_len);

    bool _isEqual(std::string key1, std::string key2, int lKey1, int lKey2);

};


#endif //SERVER_CRYPTO_H
