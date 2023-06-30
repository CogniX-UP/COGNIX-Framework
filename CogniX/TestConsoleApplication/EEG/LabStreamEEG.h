#pragma once
#include <lsl_cpp.h>

class LabStreamEEG {
	LabStreamEEG();
	LabStreamEEG(lsl::stream_info info);
	~LabStreamEEG();
public:
	lsl::stream_info GetStreamInfo() { return streamInfo; }
	void SendData(std::vector<int32_t>);
private:
	lsl::stream_info streamInfo;
};