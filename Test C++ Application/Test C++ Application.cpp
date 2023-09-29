// Test C++ Application.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <chrono>
#include <vector>
#include <thread>

int main_no_threads(uint32_t length);
int main_threads(uint32_t length, int split);

void g_test(std::vector<uint32_t>& vec, uint32_t start, uint32_t end) {
    for (auto i = start; i < end; i++)
        vec[i] = i;
}

int main()
{
    uint32_t length = 500000000;
    main_no_threads(length);
    std::cout << "\n";
    main_threads(length, 16);

    getchar();
    return 0;
}


int main_no_threads(uint32_t length) {

    std::vector<uint32_t> vec(length);

    auto start = std::chrono::steady_clock::now();
    for (uint32_t i = 0; i < length; i++) {
        vec[i] = i;
    }

    auto end = std::chrono::steady_clock::now();

    auto result = end - start;
    std::cout << " k=" << length << " " << result.count() << " ms\n";

    return 0;
}

int main_threads(uint32_t length, int split) {
    

    std::vector<uint32_t> vec(length);

    std::vector<std::thread*> threads;

    auto d = split;
    auto s = length / d;
    int i = 0;

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < d; i++) {
        std::thread *t = new std::thread(g_test, std::ref(vec), s* i, s* i + s);
        threads.push_back(t);
    }

    for (auto t : threads) {
        t->join();
        delete t;
    }

    auto end = std::chrono::steady_clock::now();

    auto result = end - start;
    std::cout << " k=" << length << " " << result.count() << " ms\n";

    return 0;
}



// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
