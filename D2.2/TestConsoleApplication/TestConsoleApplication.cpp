#include <iostream>
#include "BiosemiEEG.h"
#include "LabStreamEEG.h"
//This is a test app to see if everything is working correctly
int main()
{
    //Connect to the biosemi io
    auto b = BiosemiEEG();
    std::cout << b.HasLowBattery() << "\n";
    std::cout << b.IsMk2() << "\n";
    std::cout << b.SpeedMode() << "\n";
    
    for (auto str : b.channel_labels())
        std::cout << str << " ";

    auto buffer = BiosemiEEG::Chunk();

    while (1) {
        b.GetChunk(buffer);
        if (buffer.size() > 0) {
            for (auto sample : buffer)
                for (auto rawValue : sample)
                    std::cout << rawValue << "\n";
        }
    }

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu