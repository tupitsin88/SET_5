#include <iostream>
#include <string>

int main() {
    std::string str = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM1234567890";
    for (int p = 0; p <= 31; ++p) {
        std::cout << "p = " << p << ": ";
        for (char c1 : str) {
            int f1 = (int)c1 - 'a' + 1;
            int f0 = -p * f1;
            for (char c0 : str) {
                if ((int)c0 - 'a' + 1 == f0) {
                    std::cout << c0 << c1 << " ";
                }
            }
        }
        std::cout << "\n";
    }
    return 0;
}