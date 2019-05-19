//
// Created by Michele Luigi Greco on 2019-05-18.
//

#include "db_crypto.h"


void db_crypto::handleErrors(void) {
    unsigned long errCode;

    printf("An error occurred\n");
    while ((errCode = ERR_get_error())) {
        char *err = ERR_error_string(errCode, NULL);
        printf("%s\n", err);
    }
    abort();
}

int db_crypto::encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *aad,
                       int aad_len, unsigned char *key, unsigned char *iv,
                       unsigned char *ciphertext, unsigned char *tag) {
    EVP_CIPHER_CTX *ctx = NULL;
    int len = 0, ciphertext_len = 0;

    /* Create and initialise the context */
    if (!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

    /* Initialise the encryption operation. */
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL))
        handleErrors();

    /* Set IV length if default 12 bytes (96 bits) is not appropriate */
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 16, NULL))
        handleErrors();

    /* Initialise key and IV */
    if (1 != EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv)) handleErrors();

    /* Provide any AAD data. This can be called zero or more times as
     * required
     */
    if (aad && aad_len > 0) {
        if (1 != EVP_EncryptUpdate(ctx, NULL, &len, aad, aad_len))
            handleErrors();
    }

    /* Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    if (plaintext) {
        if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
            handleErrors();

        ciphertext_len = len;
    }

    /* Finalise the encryption. Normally ciphertext bytes may be written at
     * this stage, but this does not occur in GCM mode
     */
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handleErrors();
    ciphertext_len += len;

    /* Get the tag */
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag))
        handleErrors();

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

int db_crypto::decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *aad,
                       int aad_len, unsigned char *tag, unsigned char *key, unsigned char *iv,
                       unsigned char *plaintext) {
    EVP_CIPHER_CTX *ctx = NULL;
    int len = 0, plaintext_len = 0, ret;

    /* Create and initialise the context */
    if (!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

    /* Initialise the decryption operation. */
    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL))
        handleErrors();

    /* Set IV length. Not necessary if this is 12 bytes (96 bits) */
    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 16, NULL))
        handleErrors();

    /* Initialise key and IV */
    if (!EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv)) handleErrors();

    /* Provide any AAD data. This can be called zero or more times as
     * required
     */
    if (aad && aad_len > 0) {
        if (!EVP_DecryptUpdate(ctx, NULL, &len, aad, aad_len))
            handleErrors();
    }

    /* Provide the message to be decrypted, and obtain the plaintext output.
     * EVP_DecryptUpdate can be called multiple times if necessary
     */
    if (ciphertext) {
        if (!EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
            handleErrors();

        plaintext_len = len;
    }

    /* Set expected tag value. Works in OpenSSL 1.0.1d and later */
    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag))
        handleErrors();

    /* Finalise the decryption. A positive return value indicates success,
     * anything else is a failure - the plaintext is not trustworthy.
     */
    ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    if (ret > 0) {
        /* Success */
        plaintext_len += len;
        return plaintext_len;
    } else {
        /* Verify failed */
        return -1;
    }
}

db_crypto::db_crypto(void) {
//    OpenSSL_add_all_algorithms();
//    ERR_load_crypto_strings();
}

std::string db_crypto::db_encrypt(std::string password,int *cifred_len) {



    /* A 256 bit key */
    constexpr static const unsigned char key[] = "+KbPeShVmYp3s6v9y$B&E)H@McQfTjWn";

    /* A 128 bit IV */
    constexpr static const unsigned char iv[] = "eThWmZq3t6w9z$C&";;

    /* Some additional data to be authenticated */
    constexpr static const unsigned char aad[] = "khfgdsdjkgdsjkkd";

    std::string c_password;
    int len = *cifred_len;
    unsigned char ciphertext[128];
    unsigned char tag2[len];
    std::strcpy((char*)tag2,"");

//    unsigned char tag[16];
    unsigned char plaintext[password.length()];
    std::strcpy((char *) plaintext, password.c_str());

    *cifred_len = encrypt(plaintext, std::strlen((char *) plaintext), const_cast<unsigned char *>(aad),
                          std::strlen((char *) aad),
                          const_cast<unsigned char *>(key), const_cast<unsigned char *>(iv), ciphertext, tag2);

    c_password.append(reinterpret_cast<const char *>(ciphertext));
    return c_password;

}
std::string db_crypto::db_decrypt(std::string password,int *cifred_len) {

//std::string db_crypto::db_decrypt(std::string password, unsigned char *tag, int *cifred_len) {

    /* A 256 bit key */
    constexpr static const unsigned char key[] = "+KbPeShVmYp3s6v9y$B&E)H@McQfTjWn";

    /* A 128 bit IV */
    constexpr static const unsigned char iv[] = "eThWmZq3t6w9z$C&";;

    /* Some additional data to be authenticated */
    constexpr static const unsigned char aad[] = "khfgdsdjkgdsjkkd";

    std::string c_password;
    int len = *cifred_len;
    unsigned char decryptedtext[128];
    unsigned char tag2[len];
    std::strcpy((char*)tag2,"");

//    unsigned char tag[16];
    unsigned char plaintext[password.length()];
    std::strcpy((char *) plaintext, password.c_str());

    *cifred_len = decrypt(plaintext, *cifred_len, const_cast<unsigned char *>(aad), std::strlen((char *) aad), tag2,
                          const_cast<unsigned char *>(key), const_cast<unsigned char *>(iv), decryptedtext);

    decryptedtext[*cifred_len] = '\0';

    c_password.append(reinterpret_cast<const char *>(decryptedtext));
    return c_password;

}