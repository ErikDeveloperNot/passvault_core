#include "passwordgenerator.h"

#include <random>

#include <QDebug>


PasswordGenerator::PasswordGenerator(Generator &_generator) : generator_struct{_generator}
{
//        for (auto it = generator_struct.properties.allowedCharacter.begin();
//             it != generator_struct.properties.allowedCharacter.end(); it++) {
//            int c = static_cast<int>(*it);

//            if (c >= aLower && c <= zLower)
//                lower = true;
//            else if (c >= aUpper && c <= zUpper)
//                upper = true;
//            else if (c >= zero && c <= nine)
//                digits = true;
//            else
//                special = true;


//            if (lower && upper && digits && special)
//                break;
//        }
    lower = true;
    upper = true;
    digits = true;
    special = true;

    for (char i = static_cast<char>(aLower); i <= static_cast<char>(zLower); i++)
        if (generator_struct.properties.allowedCharacter.find(i) ==
                generator_struct.properties.allowedCharacter.end()) {
            lower = false;
            break;
        }

    for (char i = static_cast<char>(aUpper); i <= static_cast<char>(zUpper); i++)
        if (generator_struct.properties.allowedCharacter.find(i) ==
                generator_struct.properties.allowedCharacter.end()) {
            upper = false;
            break;
        }

    for (char i = static_cast<char>(zero); i <= static_cast<char>(nine); i++)
        if (generator_struct.properties.allowedCharacter.find(i) ==
                generator_struct.properties.allowedCharacter.end()) {
            digits = false;
            break;
        }

    for (char c : specials)
        if (generator_struct.properties.allowedCharacter.find(c) ==
                generator_struct.properties.allowedCharacter.end()) {
            special = false;
            break;
        }
}


void PasswordGenerator::enableLower(bool allow)
{
    if (allow)
        lower = true;
    else
        lower = false;

    modifyCharacters(allow, aLower, zLower);
}


void PasswordGenerator::enableUpper(bool allow)
{
    if (allow)
        upper = true;
    else
        upper = false;

    modifyCharacters(allow, aUpper, zUpper);
}


void PasswordGenerator::enableDigits(bool allow)
{
    if (allow)
        digits = true;
    else
        digits = false;

    modifyCharacters(allow, zero, nine);
}


void PasswordGenerator::enableSpecial(bool allow)
{
    if (allow) {
        special = true;

        for (auto val : specials)
            generator_struct.properties.allowedCharacter.emplace(val);
    } else {
        special = false;

        //make sure to remove all special, not just default
        for (auto it = generator_struct.properties.allowedCharacter.begin();
             it != generator_struct.properties.allowedCharacter.end(); ) {
            int i_val = static_cast<int>(*it);

            if ((!(i_val >= aLower && i_val <= zLower)) &&
               (!(i_val >= aUpper && i_val <= zUpper)) &&
               (!(i_val >= zero && i_val <= nine))) {
                    it = generator_struct.properties.allowedCharacter.erase(it);
            } else {
                it++;
            }
        }
    }
}


void PasswordGenerator::addChar(char c)
{
    generator_struct.properties.allowedCharacter.emplace(c);
}


void PasswordGenerator::removeChar(char c)
{
    generator_struct.properties.allowedCharacter.erase(c);
}


int PasswordGenerator::getPasswordLength()
{
    return generator_struct.properties.length;
}


void PasswordGenerator::setPasswordLength(int l)
{
     generator_struct.properties.length = l;
}


std::set<char> PasswordGenerator::getAllowedCharacters()
{
    return generator_struct.properties.allowedCharacter;
}


QString PasswordGenerator::generate()
{
    bool lowerUsed=!lower, upperUsed=!upper, digitUsed=!digits, specialUsed=!special;
    int size = generator_struct.properties.allowedCharacter.size();
    char chars[size];
    int i{0};

    for (char val : generator_struct.properties.allowedCharacter)
        chars[i++] = val;

    std::random_device seed;
    std::mt19937 gen(seed());
    std::uniform_int_distribution<> uniformDist(0, 1020000);

    //make sure settins password length is long enough in settings (defaults 32)
    int needed{0};
    if (lower)
        needed++;
    if (upper)
        needed++;
    if (digits)
        needed++;
    if (special)
        needed++;

    if (needed > generator_struct.properties.length) {
        throw GeneratorException{"The current setting for the password length is too short"};
    }

    QString password{};

    //start by making sure all the neededs are met
    while (!lowerUsed) {
        int i = chars[uniformDist(gen) % size];

        if (i >= aLower && i <= zLower) {
            password.append(static_cast<char>(i));
            lowerUsed = true;
        }
    }

    while (!upperUsed) {
        int i = chars[uniformDist(gen) % size];

        if (i >= aUpper && i <= zUpper) {
            password.append(static_cast<char>(i));
            upperUsed = true;
        }
    }

    while (!digitUsed) {
        int i = chars[uniformDist(gen) % size];

        if (i >= zero && i <= nine) {
            password.append(static_cast<char>(i));
            digitUsed = true;
        }
    }

    while (!specialUsed) {
        int i = chars[uniformDist(gen) % size];

        if ((!(i >= aLower && i <= zLower)) &&
           (!(i >= aUpper && i <= zUpper)) &&
           (!(i >= zero && i <= nine))) {
            password.append(static_cast<char>(i));
            specialUsed = true;
        }
    }

    //fill in the remainder
    int stillNeeded = generator_struct.properties.length - password.size();

    for (int i=0; i< stillNeeded; i++) {
        int add = chars[uniformDist(gen) % size];
        password.append(static_cast<char>(add));
    }

    //last shuffle it up since the first x number of chars will be in the order of needed
    for (int i=0; i<1000; i++) {
        int x = uniformDist(gen) % password.size();
        int y = uniformDist(gen) % password.size();
        QChar temp = password.at(y);
        password[y] = password[x];
        password[x] = temp;
    }

    return password;
}


void PasswordGenerator::modifyCharacters(bool allow, int low, int high)
{
    if (allow) {
        for (int i=low; i<=high; i++)
            generator_struct.properties.allowedCharacter.emplace(static_cast<char>(i));
    } else {
        for (int i=low; i<=high; i++)
            generator_struct.properties.allowedCharacter.erase(static_cast<char>(i));
    }
}


