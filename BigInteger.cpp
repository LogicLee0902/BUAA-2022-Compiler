//
// Created by Leo on 2022/11/26.
//

#include <iostream>
#include <algorithm>
#include <cstring>
#include "include/BigInteger.h"

using namespace std;

BigInteger::BigInteger(long long num) {
    numBits.clear();
    do {
        this->numBits.push_back((int) (num % BASE));
        num /= BASE;
    } while (num != 0);
}

BigInteger::BigInteger(const string &num) {
    numBits.clear();
    int len = (int) num.length();
   // cout << "len " << len << endl;
    int i = 0;
    if (num[0] == '-') {
        i = 1;
    }
    for (; i < len; i += 4) {
        int tmp = 0;
        for (int j = i; j < i + 4; j++) {
            //cout << i * 4 + j << ": " << num[i * 4 + j] << endl;
           if(j >= len) break;
            tmp = tmp * 10 + num[j] - '0';
        }
        this->numBits.push_back(tmp);
    }
}

BigInteger::BigInteger(const BigInteger &num) {
    this->numBits.clear();
    for (int numBit: num.numBits) {
        this->numBits.push_back(numBit);
    }
}

BigInteger &BigInteger::operator=(const BigInteger &num) {
    this->numBits.clear();
    for (int numBit: num.numBits) {
        this->numBits.push_back(numBit);
    }
    return *this;
}

BigInteger BigInteger::operator+(const BigInteger &num) const {
    BigInteger sum;
    for(int i = 0, carry = 0; ; ++i) {
        if (i >= num.numBits.size() && i >= this->numBits.size() && carry == 0) {
            break;
        }
        int number = carry;
        if (i < num.numBits.size()) {
            number += num.numBits[i];
        }
        if (i < this->numBits.size()) {
            number += this->numBits[i];
        }
        sum.numBits.push_back(number % BASE);
        carry = number / BASE;
    }
    return sum;
}

BigInteger BigInteger::operator-(const BigInteger &num) const {
    BigInteger diff;
    for (int i = 0, carry = 0;; ++i) {
        if (i >= num.numBits.size() && i >= this->numBits.size() && carry == 0) {
            break;
        }
        int number = carry;
        if (i < num.numBits.size()) {
            number -= num.numBits[i];
        }
        if (i < this->numBits.size()) {
            number += this->numBits[i];
        }
        if (number < 0) {
            number += BASE;
            carry = -1;
        } else {
            carry = 0;
        }
        diff.numBits.push_back(number);
    }
    return diff;
}

BigInteger BigInteger::operator * (const BigInteger &num) const {
    BigInteger product;
    product.numBits.resize(this->numBits.size() + num.numBits.size());
    for (int i = 0; i < this->numBits.size(); ++i) {
        for (int j = 0; j < num.numBits.size(); ++j) {
            product.numBits[i + j] += this->numBits[i] * num.numBits[j];
        }
    }
    for (int i = 0; i < product.numBits.size() - 1; ++i) {
        product.numBits[i + 1] += product.numBits[i] / BASE;
        product.numBits[i] %= BASE;
    }
    while (product.numBits.size() > 1 && product.numBits.back() == 0) {
        product.numBits.pop_back();
    }
    return product;
}

BigInteger BigInteger::operator/(const BigInteger &num) const {
    BigInteger quotient;
    BigInteger remainder;
    for (int i = this->numBits.size() - 1; i >= 0; --i) {
        remainder = remainder * BASE + this->numBits[i];
        int x = 0, l = 0, r = BASE;
        while (l <= r) {
            int m = (l + r) / 2;
            BigInteger tmp = num * m;
            if (tmp <= remainder) {
                x = m;
                l = m + 1;
            } else {
                r = m - 1;
            }
        }
        quotient.numBits.push_back(x);
        remainder = remainder - num * x;
    }
    reverse(quotient.numBits.begin(), quotient.numBits.end());
    return quotient;
}

BigInteger BigInteger::operator^(const int num) const {
    if (num == 0) {
        return 1;
    }
    if (num == 1) {
        return *this;
    }
    int n = num, i;
    // quick power
    BigInteger power(1), tmp;
    while (n > 1) {
        tmp = *this;
        for ( i = 1; (i<<1) < n; i <<= 1) {
            tmp = tmp * tmp;
        }
        n -= i;
        power = power * tmp;
        if (n == 1)
            power = power * (*this);
    }
    return power;
}

bool BigInteger::operator<=(const BigInteger &num) const {
    if (this->numBits.size() != num.numBits.size()) {
        return this->numBits.size() < num.numBits.size();
    }
    for (int i = this->numBits.size() - 1; i >= 0; --i) {
        if (this->numBits[i] != num.numBits[i]) {
            return this->numBits[i] < num.numBits[i];
        }
    }
    return true;
}

long long BigInteger::toLongLong() const {
    long long result = 0;
    for (int i = (int)this->numBits.size() - 1; i >= 0; --i) {
        result = result * BASE + this->numBits[i];
    }
    return result;
}

bool BigInteger::operator>(const BigInteger &num) const {
    return !(*this <= num);
}

ostream& operator<<(ostream& out, const BigInteger& num) {
    out << num.numBits.back();
    for (int i = (int)num.numBits.size() - 2; i >= 0; --i) {
        char buf[10];
        sprintf(buf, "%04d", num.numBits[i]);
        for (int j = 0; j < strlen(buf); ++j) {
            out << buf[j];
        }
    }
    return out;
}

