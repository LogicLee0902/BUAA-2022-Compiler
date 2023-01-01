//
// Created by Leo on 2022/11/26.
//

#ifndef COMPILER_BIGINTEGER_H
#define COMPILER_BIGINTEGER_H

#include <iostream>
#include <string>
#include <vector>

using namespace std;

class BigInteger {
    typedef bool Sign;

    friend ostream &operator<<(ostream &, const BigInteger &num);

    friend istream &operator<<(istream &, const BigInteger &num);

public:
    BigInteger() {
        numBits.clear();
    };

    BigInteger(const string &num);

    BigInteger(long long num);

    BigInteger(const BigInteger &num);

    BigInteger &operator=(const BigInteger &num);

    BigInteger operator-(const BigInteger &num) const;

    BigInteger operator+(const BigInteger &num) const;

    BigInteger operator*(const BigInteger &num) const;

    BigInteger operator/(const BigInteger &num) const;

    BigInteger operator%(const BigInteger &num) const;

    BigInteger operator^(const int num) const;

    long long toLongLong() const;

    bool operator==(const BigInteger &num) const;

    bool operator!=(const BigInteger &num) const;

    bool operator>(const BigInteger &num) const;

    bool operator<(const BigInteger &num) const;

    bool operator>=(const BigInteger &num) const;

    bool operator<=(const BigInteger &num) const;

private:
    static const int BASE = 10000;
    static const int BASE_LEN = 4;
    vector<int> numBits;

    static void fotmatting(BigInteger &num);
};

ostream &operator<<(ostream &os, const BigInteger &num);

istream &operator<<(istream &is, const BigInteger &num);

#endif //COMPILER_BIGINTEGER_H
