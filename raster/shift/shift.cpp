#include<iostream>
#include <cstdint>
#include <bitset>

using namespace std;

uint16_t barrelShift(uint16_t num, uint8_t shift_right){
    return (num >> shift_right) | (num << ((16-shift_right)%16));
}

uint16_t rightShift(uint16_t num, uint8_t shift){
    uint16_t temp = barrelShift(num, shift);
    uint16_t x = (1 << (16 - shift)) - 1;
    temp= temp & x;
    return temp;
}

uint16_t leftShift(uint16_t num, uint8_t shift){
    uint16_t temp = barrelShift(num, 16 - shift);
    uint16_t x = ~((1 << shift) -1);
    temp= temp & x;
    return temp;
}

int main() {
    uint16_t num = 0b111011001, shift = 6;
    uint16_t a = barrelShift(num,shift);
    uint16_t b = leftShift(num,shift);
    uint16_t c = rightShift(num,shift);
    cout << bitset<16>(a) << endl;
    cout << bitset<16>(b) << endl;
    cout << bitset<16>(c) << endl;
    return 0;
}