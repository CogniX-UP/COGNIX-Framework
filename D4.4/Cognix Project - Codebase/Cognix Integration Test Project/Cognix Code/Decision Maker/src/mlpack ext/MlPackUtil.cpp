#pragma once
#include <mlpack ext/MlPackUtil.h>
#include <mlpack/mlpack.hpp>
#include <vector>

//Used to fill an armadillo matrix from a vector of observations
void FillArmaMat(std::vector<double>& observations, size_t featureLength, arma::mat& matrix) {	
	
	auto d = observations.data();
	auto obsNum = observations.size() / featureLength;
	matrix = arma::mat(d, featureLength, obsNum, false);
}
//Used to fill an armadillo feature vec from a feature vector
void FillArmaVec(std::vector<double>& features, arma::colvec& vec) {
	auto fPointer = features.data();
	vec = arma::colvec(fPointer, features.size(), false);
}
//Used to fill an armadillo label vec from a label vector
void FillArmaVec(std::vector<size_t>& labels, arma::Row<size_t> &vec) {
	auto fPointer = labels.data();
	vec = arma::Row<size_t>(fPointer, labels.size(), false);
}