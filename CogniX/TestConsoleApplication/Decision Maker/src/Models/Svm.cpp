#include "Svm.hpp"

void DecisionMaking::Svm::Load(const std::string& path)
{
	mlpack::data::Load(path, "svm", model);
}

void DecisionMaking::Svm::Save(const std::string& path)
{
	mlpack::data::Save(path, "svm", model);
}

std::vector<size_t> DecisionMaking::Svm::Predict(std::vector<double>& features)
{
	auto fPointer = features.data();
	auto queryData = arma::rowvec(fPointer, features.size());

	arma::Row<size_t> predictions;
	arma::mat probabilities;

	auto result = model.Classify(queryData);

	return std::vector<size_t> { result };
}
