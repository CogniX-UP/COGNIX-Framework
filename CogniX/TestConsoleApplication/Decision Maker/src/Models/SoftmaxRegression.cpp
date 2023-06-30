#include "SoftmaxRegression.hpp"

void DecisionMaking::SoftmaxRegression::Load(const std::string& path)
{
	mlpack::data::Load(path, "softmaxRegression", model);
}

void DecisionMaking::SoftmaxRegression::Save(const std::string& path)
{
	mlpack::data::Save(path, "softmaxRegression", model);
}

std::vector<size_t> DecisionMaking::SoftmaxRegression::Predict(std::vector<double>& features)
{
	auto fPointer = features.data();
	auto queryData = arma::rowvec(fPointer, features.size());

	arma::Row<size_t> predictions;
	arma::mat probabilities;

	auto result = model.Classify(queryData);

	return std::vector<size_t> { result };
}
