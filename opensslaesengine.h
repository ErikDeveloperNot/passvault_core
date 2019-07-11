#ifndef OPENSSLAESENGINE_H
#define OPENSSLAESENGINE_H

#include <string>
#include <exception>

class OpenSSLAESEngine
{
public:
    void setKey(std::string);
    std::string getKey();

    OpenSSLAESEngine(std::string);
    OpenSSLAESEngine();
    OpenSSLAESEngine(OpenSSLAESEngine&) = delete;
    OpenSSLAESEngine(OpenSSLAESEngine&&) = delete;

    std::string encryptPassword(std::string);
    std::string decryptPassword(std::string);
private:
    constexpr static int KEY_LENGTH = 32;
    constexpr static int IV_LENGTH = 16;
    const std::string DEFAULT_KEY{"Th1$15N0t5ecure"};

    std::string key;
    std::string iv;

    void finalizeSpec(std::string &, int);

    void handleErrors(void);
    int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
                unsigned char *iv, unsigned char *ciphertext);
    int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
                unsigned char *iv, unsigned char *plaintext);

};

class EncryptionException : public std::exception
{
public:
    EncryptionException(const char* _ex) : ex{_ex} {}
    ~EncryptionException() = default;

    virtual const char* what() const noexcept
    {
        return ex;
    }

private:
    const char* ex;
};


#endif // OPENSSLAESENGINE_H
