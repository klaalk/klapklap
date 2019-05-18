//
// Created by Michele Luigi Greco on 2019-05-18.
//

#ifndef SERVER_DB_CRYPTO_H
#define SERVER_DB_CRYPTO_H

#include "../classes_include.h"


class db_crypto {
private:
//    /* A 256 bit key */
//    constexpr static const unsigned char key[] =  "01234567890123456789012345678901";
//
//    /* A 128 bit IV */
//    constexpr static const unsigned char iv[] = "0123456789012345"; ;
//
//    /* Some additional data to be authenticated */
//    constexpr static const unsigned char aad[] = "Some AAD data";

    int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *aad,
                int aad_len, unsigned char *tag, unsigned char *key, unsigned char *iv,
                unsigned char *plaintext);

    int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *aad,
                int aad_len, unsigned char *key, unsigned char *iv,
                unsigned char *ciphertext, unsigned char *tag);

    void handleErrors(void);

public:
    explicit db_crypto(void);

    std::string db_encrypt(std::string password, unsigned char *tag, int *cifred_len);

    std::string db_decrypt(std::string password, unsigned char *tag, int *cifred_len);

};


#endif //SERVER_DB_CRYPTO_H
