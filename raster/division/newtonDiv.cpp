#include <iostream>
#include <cstdint>
#include <bitset>
#include <cmath>

using namespace std;

void newtonDiv(uint16_t dividend, uint16_t divisor, uint16_t &quotient, uint16_t &remainder, int &steps_taken) {
    steps_taken = 0;

    if (divisor == 0) {
        cerr << "Division by zero error!" << endl;
        quotient = remainder = 0;
        return;
    }

    float tempdivisor = static_cast<float>(divisor);
    float tempdividend = static_cast<float>(dividend);

    // Normalize to bring divisor in range [0.5, 1]
    while (tempdivisor > 1) {
        tempdivisor /= 2;
        tempdividend /= 2;
    }

    // Initial estimate of 1 / divisor using a linear approximation
    float inverseDivisor = 48.0f / 17.0f - (32.0f / 17.0f) * tempdivisor;
    float tol = 1e-3f;
    float prev = 0.0f;

    // Newton-Raphson iteration
    while (fabs(inverseDivisor - 1/tempdivisor) > tol) {
        inverseDivisor = inverseDivisor * (2.0f - tempdivisor * inverseDivisor);
        steps_taken++;
    }

    float q = inverseDivisor * tempdividend;
    quotient = static_cast<uint16_t>(q);
    remainder = dividend - quotient * divisor;
}

int main() {
    uint16_t dividend = 12345, divisor = 321;
    uint16_t quotient = 0, remainder = 0;
    int steps_taken = 0;

    newtonDiv(dividend, divisor, quotient, remainder, steps_taken);
    cout << "Steps taken: " << steps_taken << ", " 
         << dividend << " / " << divisor << " = " 
         << quotient << ", remainder = " << remainder << endl;

    return 0;
}
