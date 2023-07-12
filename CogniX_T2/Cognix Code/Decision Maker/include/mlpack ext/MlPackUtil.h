#pragma once
#include<mlpack/mlpack.hpp>
#include<vector>

//Used to fill an armadillo matrix from a vector of observations
void FillArmaMat(std::vector<std::vector<double>>& observations, arma::mat& matrix);
//Used to fill an armadillo feature vec from a feature vector
void FillArmaVec(std::vector<double>& features, arma::rowvec& vec);
//Used to fill an armadillo label vec from a label vector
void FillArmaVec(std::vector<size_t>& labels, arma::Row<size_t> vec);