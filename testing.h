#ifndef TESTING_H_
#define TESTING_H_

#include <iostream>
#include <string>

#define beginTest() std::cout << "Starting " << __FUNCTION__ << ".\n"

void check(bool test, std::string message);
void checkFalse(bool test, std::string message);
template <class T>
void checkEqual(T const& p1, T const& p2, std::string message);
template <class T>
void checkNotEqual(T const& p1, T const& p2, std::string message);

template <class T>
void checkEqual(T const& p1, T const& p2, std::string message) {
    check(p1 == p2, message);
}

template <class T>
void checkNotEqual(T const& p1, T const& p2, std::string message) {
    checkFalse(p1 == p2, message);
}

#endif
