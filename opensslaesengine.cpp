#include "opensslaesengine.h"

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include <QDebug>


void OpenSSLAESEngine::setKey(std::string key_)
{
    key = key_;
    finalizeSpec(key, KEY_LENGTH);
//    qDebug() << "key: " << QString::fromStdString(key);
    iv = key.substr(20, 4);
    finalizeSpec(iv, IV_LENGTH);
    //    qDebug() << "iv: " << QString::fromStdString(iv);
}


std::string OpenSSLAESEngine::getKey()
{
    return key;
}


OpenSSLAESEngine::OpenSSLAESEngine(std::string key_)
{
    setKey(key_);
}


OpenSSLAESEngine::OpenSSLAESEngine()
{
    setKey(DEFAULT_KEY);
}


std::string OpenSSLAESEngine::encryptPassword(std::string toEncrypt)
{
    unsigned char *toEncryptPtr = (unsigned char*)strdup(toEncrypt.c_str());
    unsigned char *keyPtr = (unsigned char*)strdup(key.c_str());
    unsigned char *ivPtr = (unsigned char*)strdup(iv.c_str());

    unsigned char cipherText[toEncrypt.size()*2];
    int cipherLength = encrypt(toEncryptPtr, toEncrypt.size(), keyPtr, ivPtr, cipherText);

    std::string toReturn;

    if (cipherLength != -1)
        toReturn = std::string{(const char*)cipherText, strlen((const char*)cipherText)};
    else
        toReturn = "";

    free(toEncryptPtr);
    free(keyPtr);
    free(ivPtr);

    return toReturn;
}


std::string OpenSSLAESEngine::decryptPassword(std::string toDecrypt)
{
    unsigned char *toDecryptPtr = (unsigned char*)strdup(toDecrypt.c_str());
    unsigned char *keyPtr = (unsigned char*)strdup(key.c_str());
    unsigned char *ivPtr = (unsigned char*)strdup(iv.c_str());

    unsigned char plainText[toDecrypt.size()*2];
    int plaintext_len = decrypt(toDecryptPtr, toDecrypt.size(), keyPtr, ivPtr, plainText);

    free(toDecryptPtr);
    free(keyPtr);
    free(ivPtr);

    if (plaintext_len != -1)
        return std::string{(const char*)plainText, strlen((const char*)plainText)};
    else
//        return "";
        throw EncryptionException{"Error decrypting password with supplied key"};
}


void OpenSSLAESEngine::finalizeSpec(std::string &spec, int length)
{
    int specSize = spec.size();
    int amountToPad = length - spec.size();

    if (amountToPad > 0) {
        int mod = 256;

        for (int i=1; i<= amountToPad; i++) {
            int i1 = i * static_cast<int>(spec[i%specSize]);
            int i2 = (i+3*i) * spec[(i+2)%specSize];
            int i3 = ((i1 * i2) % mod);
            int x = 3;

            while ((i3 < 65 || i3 > 122) && (i3 < 34 || i3 > 57))
                i3 = ((i1 * x++) % mod--);

            spec.append(1, i3);
        }

    } else if (amountToPad < 0) {
        spec = spec.substr(0, length);
    }
}



/*
 * OpenSSL calls
 */

void OpenSSLAESEngine::handleErrors()
{
//    qDebug() << "OpenSSL Error Handler...";
//    ERR_print_errors_fp(stderr);

}


int OpenSSLAESEngine::encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
                              unsigned char *iv, unsigned char *ciphertext)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int ciphertext_len;

    unsigned char tmpCipherText[plaintext_len*2];

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new())) {
        handleErrors();
        return -1;
    }

    /*
     * Initialise the encryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        handleErrors();
        return -1;
    }

    /*
     * Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    if(1 != EVP_EncryptUpdate(ctx, tmpCipherText, &len, plaintext, plaintext_len)) {
        handleErrors();
        return -1;
    }

    ciphertext_len = len;

    /*
     * Finalise the encryption. Further ciphertext bytes may be written at
     * this stage.
     */
    if(1 != EVP_EncryptFinal_ex(ctx, tmpCipherText + len, &len)) {
        handleErrors();
        return -1;
    }

    ciphertext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    ciphertext_len = EVP_EncodeBlock(ciphertext, tmpCipherText, ciphertext_len);

    return ciphertext_len;
}


int OpenSSLAESEngine::decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
                              unsigned char *iv, unsigned char *plaintext)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int plaintext_len;

    unsigned char decodedtext[ciphertext_len*2];
    int decodedtext_len = EVP_DecodeBlock(decodedtext, ciphertext, ciphertext_len);

    // work aroudn to get rid of trailing =, == padding if present
    int trim = 0;
    if (ciphertext[ciphertext_len-1] == '=') {
        trim++;
        if (ciphertext[ciphertext_len-2] == '=')
            trim++;
    }
    decodedtext_len -= trim;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new())) {
        handleErrors();
        return -1;
    }

    /*
     * Initialise the decryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        handleErrors();
        return -1;
    }

    /*
     * Provide the message to be decrypted, and obtain the plaintext output.
     * EVP_DecryptUpdate can be called multiple times if necessary.
     */
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, decodedtext, decodedtext_len)) {
        handleErrors();
        return -1;
    }

    plaintext_len = len;

    /*
     * Finalise the decryption. Further plaintext bytes may be written at
     * this stage.
     */
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) {
        handleErrors();
        return -1;
    }

    plaintext_len += len;
    plaintext[plaintext_len] = '\0';

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}
