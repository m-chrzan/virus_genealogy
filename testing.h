#ifndef TESTING_H_
#define TESTING_H_

#include <functional>
#include <iostream>
#include <string>

#define beginTest() std::cout << "Starting " << __FUNCTION__ << ".\n"

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

template <class T>
void checkEqual(T const& p1, T const& p2, std::string message) {
    check(p1 == p2, message);
}

template <class T>
void checkNotEqual(T const& p1, T const& p2, std::string message) {
    checkFalse(p1 == p2, message);
}

void checkNoExceptionThrown(std::function<void(void)> function,
        std::string message) {
    bool thrown = false;

    try {
        function();
    } catch (...) {
        thrown = true;
    }

    checkFalse(thrown, message);
}

template <class E>
void checkExceptionThrown(std::function<void(void)> function, std::string message) {
    bool thrown = false;

    try {
        function();
    } catch (E e) {
        thrown = true;
    } catch (...) {
        thrown = false;
    }

    check(thrown, message);
}

#endif
