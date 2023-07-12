#include <iostream>
#include <mlpack/mlpack.hpp>
#include <BiosemiEEG.h>
#include <LabStreamEEG.h>
#include <string.h>
#include <unordered_map>

//This is a basic programm that uses the CogniX API to send
//BioSemi EEG data to LSL
int mainElse()
{
    try {
        LabStreamEEG eegLab;
        //Append channel metadata for the device
        auto channels = std::vector<std::string>{
            "Fp1", "Af3", "F7", "F3", "Fc1", "Fc5", "T7", "C3", "Cp1",
            "Cp5", "P7", "P3", "Pz", "Po3", "O1", "Oz", "O2", "Po4", "P4",
            "P8", "Cp6", "Cp2", "C4", "T8", "Fc6", "Fc2", "F4", "F8", "Af4",
            "Fp2", "Fz", "Cz"
        };
        auto types = std::vector<std::string>(channels.size());
        for (int i = 0; i < types.size(); i++) {
            types[i] = "EEG";
        }
        std::unordered_map<std::string, std::vector<std::string>> locaMaps;
        eegLab.AppendChannelMetadata(channels, types, locaMaps);

        auto refChannels = std::vector<std::string>{ "EX1", "EX2" };
        eegLab.AppendRefMetadata(refChannels);
        //Append 0 lag, we don't care for now
        eegLab.AppendAcquisitionMetadata(0, 0);

        //Start sending the data
        while (1) {
            eegLab.SendData();
        }
    }
    catch (const std::exception&) {
        std::cout << "Failed to connect to BioSemi Device." << std::endl << "Press enter to exit";
        std::cin.get();
    }
    return 0;
}
