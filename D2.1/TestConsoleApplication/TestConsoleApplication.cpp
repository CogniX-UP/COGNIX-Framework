#include <iostream>
#include "biosemi_io.h"
int main()
{
    std::cout << "Hello World!\n";
    auto b = biosemi_io();
    std::cout << b.battery_low() << "\n";
    std::cout << b.is_mk2() << "\n";
    std::cout << b.speed_mode() << "\n";
    
    for (auto str : b.channel_labels())
        std::cout << str << " ";

    auto buffer = biosemi_io::chunk_t();

    while (1) {
        b.get_chunk(buffer);
        if (buffer.size() > 0) {
            for (auto sample : buffer)
                for (auto rawValue : sample)
                    std::cout << rawValue << "\n";
        }
    }
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu