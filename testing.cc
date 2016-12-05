#include "testing.h"

void check(bool test, std::string message) {
    if (test) {
        std::cout << "  Test ok. " << message << std::endl;
    } else {
        std::cout << "  TEST FAILED! " << message << std::endl;
    }
}

void checkFalse(bool test, std::string message) {
    check(!test, message);
}

