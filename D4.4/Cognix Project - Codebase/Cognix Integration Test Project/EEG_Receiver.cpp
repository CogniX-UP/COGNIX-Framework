#include <iostream>
#include <lsl_cpp.h>
#include <string>
#include <thread>
#include <fstream>
#include <chrono>

//¡Write a string padded with spaces
void writePaddedString(std::ofstream& file, const std::string& str, int length) {
	std::string paddedStr = str;
	paddedStr.resize(length, ' ');
	file.write(paddedStr.c_str(), length);
}

//Write the BDF metadata, the first 256 bytes.
void writeBDFMetadata(std::ofstream& file, const std::string& patient, const std::string& date,
	const std::string& time, const std::vector<std::string> &labels, int sampleRate) {
	file.write("255", 1);
	writePaddedString(file, "BIOSEMI", 7);
	writePaddedString(file, patient, 80);
	writePaddedString(file, "None", 80);
	file.write(date.c_str(), date.size());
	file.write(time.c_str(), time.size());
	writePaddedString(file, "24BIT", 44);
	writePaddedString(file, "32", 8);
	writePaddedString(file, "1", 8);
	writePaddedString(file, "32", 4);

	for (auto& label : labels) {
		writePaddedString(file, label, 16);
	}

	for (int i = 0; i < labels.size(); i++) {
		writePaddedString(file, "active electrode", 80);
	}

	for (int i = 0; i < labels.size(); i++) {
		writePaddedString(file, "uV", 8);
	}

	for (int i = 0; i < labels.size(); i++) {
		writePaddedString(file, "-262144", 8);
	}

	for (int i = 0; i < labels.size(); i++) {
		writePaddedString(file, "262143", 8);
	}

	for (int i = 0; i < labels.size(); i++) {
		writePaddedString(file, "-8388608", 8);
	}

	for (int i = 0; i < labels.size(); i++) {
		writePaddedString(file, "8388607", 8);
	}

	for (int i = 0; i < labels.size(); i++) {
		writePaddedString(file, "", 80);
	}

	auto sRateString = std::to_string(sampleRate);
	for (int i = 0; i < labels.size(); i++) {
		writePaddedString(file, sRateString, 8);
	}

	writePaddedString(file, "", 32);
}

/**
 *Program that aims to receive EEG data and save it when it's closed
 * This application must be run after the BioSemi Link Console Application has started sending data
 * This was used as a test to validate the CogniX API.
 */
int main1(int argc, char* argv[]) {
	std::string field, value;

	field = "BioSemi";
	value = "EEG";
	// resolve the stream of interest
	std::cout << "Now resolving streams... Timeout at 10 seconds" << std::endl;
	auto results = lsl::resolve_stream(field, value, 0, 10);

	//Exit because a stream was not found in 10 seconds
	if (results.empty()) {
		std::cout << "There's no stream with topic EEG" << std::endl << "Press enter to exit.";
		std::cin.get();
	}

	auto result = results[0];
	
	std::cout << "Here is what was resolved: " << std::endl;
	std::cout << results[0].as_xml() << std::endl;

	//Read the patients name
	std::cout << "Input Subject ID:";
	std::string subjectId;
	std::cin >> subjectId;
	std::cout << std::endl;
	
	//Create a bdf file
	std::ofstream bdfFile("output.bdf", std::ios::binary);

	if (!bdfFile) {
		std::cout << "Failed to created BDF file." << std::endl;
		return 1;
	}

	//Get start date and start time
	auto start = std::chrono::system_clock::now();
	auto currentTime = std::chrono::system_clock::to_time_t(start);


	tm localTime;
	localtime_s(&localTime, &currentTime);
	int day = localTime.tm_mday;
	int month = localTime.tm_mon + 1; // month is 0 indexed
	int year = localTime.tm_year + 1900; // years since 1900
	int hour = localTime.tm_hour;
	int minute = localTime.tm_min;
	int second = localTime.tm_sec;

	// Format date as "dd.mm.yy"
	std::string formattedDate = std::to_string(day) + "." 
								+ std::to_string(month) + "." 
								+ std::to_string(year % 100);
	
	// Format time as "hh.mm.ss"
	std::string formattedTime = std::to_string(hour) + "."
								+ std::to_string(minute) + "." 
								+ std::to_string(second);
	//Append HEADER to bdf file
	auto channels = std::vector<std::string>{
		"Fp1", "Af3", "F7", "F3", "Fc1", "Fc5", "T7", "C3", "Cp1",
		"Cp5", "P7", "P3", "Pz", "Po3", "O1", "Oz", "O2", "Po4", "P4",
		"P8", "Cp6", "Cp2", "C4", "T8", "Fc6", "Fc2", "F4", "F8", "Af4",
		"Fp2", "Fz", "Cz"
	};

	writeBDFMetadata(bdfFile, subjectId, formattedDate, formattedTime, channels, result.nominal_srate());

	// make an inlet to get data from it
	std::cout << "Now creating the inlet..." << std::endl;
	lsl::stream_inlet inlet(results[0]);

	// start receiving & displaying the data
	std::cout << "Now pulling samples..." << std::endl;

	//thread for capturing data
	auto receive = [](bool &keepGoing, std::ofstream& file, lsl::stream_inlet &inlet, size_t numChannels)
	{
		std::vector<std::vector<float>> currentSamples;
		std::vector<std::vector<float>> fullSamples;

		while (keepGoing) {
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			inlet.pull_chunk(fullSamples);

			for (auto& sample : currentSamples)
				fullSamples.push_back(sample);
		}

		//For each channel append the data
		for (int i = 0; i < numChannels; i++) {
			for (auto &sample : fullSamples) {
				auto value = sample[i] * (256 * 0.03125);
				file.write(std::to_string(value).c_str(), 3);
			}
		}

		file.close();
	};

	bool keepGoing = true;
	//Start the acquisition thread. Stop the aplpication if someone presses enter.
	std::thread receiver(receive, std::ref(keepGoing), std::ref(bdfFile), std::ref(inlet), channels.size());

	std::cout << "Currently receiving samples. Press enter exit" << std::endl;
	std::cin.get();

	keepGoing = false;
	receiver.join();

	return 0;
}