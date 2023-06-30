#include "RandomForest.h"
void DecisionMaking::RandomForestModel::Load(const std::string& path)
{
	mlpack::data::Load(path, "rForest", rForest);
}

void DecisionMaking::RandomForestModel::Save(const std::string& path)
{
	mlpack::data::Save(path, "rForest", rForest);
}

std::vector<size_t> DecisionMaking::RandomForestModel::Predict(std::vector<double>& features)
{
	auto fPointer = features.data();
	auto queryData = arma::rowvec(fPointer, features.size());

	arma::Row<size_t> predictions;
	arma::mat probabilities;
	
	auto result = rForest.Classify(queryData);

	return std::vector<size_t> { result };
}
