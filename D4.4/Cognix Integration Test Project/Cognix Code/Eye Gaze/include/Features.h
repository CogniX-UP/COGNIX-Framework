#pragma once
#include <vector>
#include <Fixation.h>
namespace EyeGaze {
	namespace Features{
		using namespace EyeGaze::Data;
		
		const Fixation& GetFixationMaxDuration(const std::vector<Fixation>& fixations);
		double CalcFixationMeanDuration(const std::vector<Fixation>& fixations);
		double CalcFixationSumDuration(const std::vector<Fixation>& fixations);
		double CalcFixationStandardDev(const std::vector<Fixation>& fixations);
//definitions that do not require external implementation
		double CalcFixationMaxDuration(const std::vector<Fixation>& fixations) {
			auto& result = GetFixationMaxDuration(fixations);
			return result.GetDuration();
		};
	}
}