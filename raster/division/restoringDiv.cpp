#include<iostream>
#include <cstdint>
using namespace std;

void restoringDiv(uint16_t dividend, uint16_t divisor, uint16_t &quotient, uint16_t &remainder){
    const int n =16; //bit width
    int32_t A = 0;
    uint32_t Q = dividend;
    uint32_t M = divisor;
    quotient = 0;

    for(int i = n - 1; i >= 0; i--){
        // Shift AQ
        A = (A << 1) | ((Q >> (n-1)) & 1);
        Q = (Q << 1) & 0xFFFF;

        // Subtract M from A
        A = A - M;

        if(A < 0){
            //Restore A
            A = A + M;
            // Set LSB of Q to 0
            Q = Q & 0xFFFFFFFE;
        } else {
            // Set LSB of Q to 1
            Q = Q | 1;
        }
    }
    quotient = (uint16_t)Q;
    remainder = (uint16_t)A;
}

int main() {
    uint16_t dividend = 12345, divisor = 321;
    uint16_t quotient, remainder;
    restoringDiv(dividend, divisor, quotient, remainder);
    cout << dividend << " / " << divisor << " = " << quotient << ", remainder = " << remainder << endl;
    return 0;
}