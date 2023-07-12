#pragma once
#include <mlpack ext/MlPackUtil.h>
#include <mlpack/mlpack.hpp>
#include <vector>

//Used to fill an armadillo matrix from a vector of observations
void FillArmaMat(std::vector<std::vector<double>>& observations, arma::mat& matrix) {	
	
	auto d = observations.data()->data(); // get the pointer to the first double, vectors are contiguous in memory
	auto fSize = observations[0].size(); // all the vectors should have the same size since they are feature vectors
	matrix = arma::mat(d, fSize, observations.size(), false);
}
//Used to fill an armadillo feature vec from a feature vector
void FillArmaVec(std::vector<double>& features, arma::rowvec& vec) {
	auto fPointer = features.data();
	vec = arma::rowvec(fPointer, features.size(), false);
}
//Used to fill an armadillo label vec from a label vector
void FillArmaVec(std::vector<size_t>& labels, arma::Row<size_t> vec) {
	auto fPointer = labels.data();
	vec = arma::Row<size_t>(fPointer, labels.size(), false);
}