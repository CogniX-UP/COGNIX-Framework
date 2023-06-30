#include "Features.h"
#include <cmath>
const EyeGaze::Data::Fixation& EyeGaze::Features::GetFixationMaxDuration(const std::vector<Fixation>& fixations)
{
	double durationResult = -1;
	const EyeGaze::Data::Fixation* fixPointer;
	for (auto& fixation : fixations) {
		auto curDur = fixation.GetDuration();
		if (curDur > durationResult) {
			fixPointer = &fixation;
			durationResult = curDur;
		}
	}
	return *fixPointer;
}

double EyeGaze::Features::CalcFixationMeanDuration(const std::vector<Fixation>& fixations)
{
	if (fixations.size() == 0)
		return 0;
	double result = CalcFixationSumDuration(fixations);
	return result / fixations.size();
}

double EyeGaze::Features::CalcFixationSumDuration(const std::vector<Fixation>& fixations)
{
	double result = 0;
	for (auto& fixation : fixations)
		result += fixation.GetDuration();
	return result;
}

double EyeGaze::Features::CalcFixationStandardDev(const std::vector<Fixation>& fixations)
{
	float fixLength = fixations.size();
	if (fixLength == 0)
		return 0;
	float mean = CalcFixationMeanDuration(fixations);
	float std;
	for (int i = 0; i < fixLength; i++)
		std += pow(fixations[i].GetDuration() - mean, 2);

	return sqrt(std / fixLength);
}
