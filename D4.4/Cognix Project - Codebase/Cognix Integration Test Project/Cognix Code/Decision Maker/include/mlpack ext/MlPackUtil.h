#pragma once
#include<mlpack/mlpack.hpp>
#include<vector>

//Used to fill an armadillo matrix from a vector of observations
void FillArmaMat(std::vector<double> &observations, size_t featureLength, arma::mat& matrix);
//Used to fill an armadillo feature vec from a feature vector
void FillArmaVec(std::vector<double>& features, arma::colvec& vec);
//Used to fill an armadillo label vec from a label vector
void FillArmaVec(std::vector<size_t>& labels, arma::Row<size_t> &vec);