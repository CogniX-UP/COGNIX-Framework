/**
 * @file EEG/LabStreamEEG.h
 * @author Georgios Papadoulis
 *
 * BioSemi acquisition with LSL
 *
 */
#pragma once
#include <lsl_cpp.h>
#include "BiosemiEEG.h"
#include<unordered_map>

/**
* The class LabStreamEEG for acquiring raw data from the BioSemi ActiveTwo, 
* rescaling and resampling if necessary and applying speficic EEG metadata.
* This class can be used to send BioSemi acquisition data through the Lab
* Streaming Layer (LSL).
*
* BioSemi acquisition with LSL
*
*/
class LabStreamEEG {
public:
	~LabStreamEEG();
	/**
	* Constructs the LSL stream, passing the total number of channels to the constructor.
	*/
	LabStreamEEG() :LabStreamEEG(eeg.AllChannelCount()) {};
	/**
	* Constructs the LSL stream, passing a specified number of channels to the consturctor.
	* @param channelCount Constructor argument for specifying the number of channels to be used.
	*/
	LabStreamEEG(int channelCount);
	/**
	* Returns the stream info, a collection of metadata describing the BioSemi ActiveTwo acquisition.
	*/
	lsl::stream_info GetStreamInfo() { return streamInfo; }
	/**
	* Returns the underlying BioSemi low level API interface.
	*/
	BiosemiEEG& GetBiosemiInterface() { return eeg; }
	/**
	* Sends data that has been buffered through LSL.
	*/
	void SendData();
	/**
	* Append channel metadata and locations. See EEG metadata template.
	* 
	* @param channels The name of the channels given as alphanumeric values.
	* @param types The type of the channel. A significant portion is EEG, but some of the types include AUX
	* for auxiliary.
	* @param locmap A hashmap giving the locations of each channel. If not given, an empty map should be passed.
	*/
	void AppendChannelMetadata(const std::vector<std::string> &channels, const std::vector<std::string> &types, const std::unordered_map<std::string, std::vector<std::string>> &locmap);
	/**
	* Append reference metadata to the stream.
	* 
	* @param refNames The reference names. In our case, the EXT channels are used.
	*/
	void AppendRefMetadata(const std::vector<std::string> &refNames);
	/**
	* Append acquisition metadata.
	* 
	* @param compensatedLag The lag that may occur due to signal processing
	* @param causalCorrection A time correction for synchronization purposes
	*/
	void AppendAcquisitionMetadata(float compensatedLag, float causalCorrection);

	//Synchronization meta-data
	const float 
		offsetMean = 0.00772f,
		offsetRms = 0.000070f,
		offsetMedian = 0.00772f,
		offset5Centile = 0.00764f,
		offset95Centile = 0.00783f;
private:
	lsl::stream_info streamInfo;
	lsl::stream_outlet *outlet;
	BiosemiEEG eeg;
	float compensatedLag;
};