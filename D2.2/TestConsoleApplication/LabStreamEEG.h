#pragma once
#include <lsl_cpp.h>

class LabStreamEEG {
	LabStreamEEG();
	~LabStreamEEG();
public:
	lsl::stream_info GetStreamInfo() { return streamInfo; }
private:
	lsl::stream_info streamInfo;
};