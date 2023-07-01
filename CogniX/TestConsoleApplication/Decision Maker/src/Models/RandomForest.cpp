#include "RandomForest.h"
using namespace mlpack;
void DecisionMaking::RandomForestModel::Load(const std::string& path)
{
	mlpack::data::Load(path, "rForest", rForest);
}

void DecisionMaking::RandomForestModel::Save(const std::string& path)
{
	mlpack::data::Save(path, "rForest", rForest);
}


void DecisionMaking::RandomForestModel::Train(std::vector<std::vector<double>>& observations, std::vector<size_t>& labels, size_t numClasses)
{
	arma::mat data;
	FillArmaMat(observations, data);
	rForest.Train(data, labels, numClasses);
}

double DecisionMaking::RandomForestModel::CrossValidate(uint32_t k, std::vector<std::vector<double>>& observations, std::vector<size_t>& labels, size_t numClasses, BaseModel::Metric metric)
{
	arma::mat data;
	FillArmaMat(observations, data);

	if (k == 1) {
		arma::rowvec responses = arma::randu<arma::rowvec>(numClasses);
		switch (metric) {
		case BaseModel::Accuracy: {
			mlpack::SimpleCV<mlpack::RandomForest<>, mlpack::Accuracy> cv(0.2, data, responses);
		}
		case BaseModel::F1:
		{
			auto cv = mlpack::SimpleCV<mlpack::RandomForest<>, mlpack::F1<mlpack::AverageStrategy::Macro>>(0.2, data, responses);
			return cv.Evaluate();
		}
		case BaseModel::MSE: {
			SimpleCV<RandomForest<>, mlpack::MSE> cv(0.2, data, responses);
			return cv.Evaluate();
		}
		case BaseModel::Precision:
		{
			SimpleCV<RandomForest<>, mlpack::Precision<mlpack::AverageStrategy::Macro>> cv(0.2, data, responses);
			return cv.Evaluate();
		}
		case BaseModel::Recall: {
			SimpleCV<RandomForest<>, mlpack::Recall<mlpack::AverageStrategy::Macro>> cv(0.2, data, responses);
			return cv.Evaluate();
		}
		}
	}
	else {
		switch (metric) {
		case BaseModel::Accuracy: {
			mlpack::KFoldCV<mlpack::RandomForest<>, mlpack::Accuracy> cv(k, data, labels);
			return cv.Evaluate();
		}
		case BaseModel::F1:
		{
			mlpack::KFoldCV<mlpack::RandomForest<>, mlpack::F1<mlpack::AverageStrategy::Macro>> cv(k, data, labels);
			return cv.Evaluate();
		}
		case BaseModel::MSE: {
			mlpack::KFoldCV<mlpack::RandomForest<>, mlpack::MSE> cv(k, data, labels);
			return cv.Evaluate();
		}
		case BaseModel::Precision:
		{
			mlpack::KFoldCV<mlpack::RandomForest<>, mlpack::Precision<mlpack::AverageStrategy::Macro>> cv(k, data, labels);
			return cv.Evaluate();
		}
		case BaseModel::Recall: {
			mlpack::KFoldCV<mlpack::RandomForest<>, mlpack::Recall<mlpack::AverageStrategy::Macro>> cv(k, data, labels);
			return cv.Evaluate();
		}
		}
		return 0.0;
	}
}

std::vector<size_t> DecisionMaking::RandomForestModel::Predict(std::vector<double>& features, std::vector<size_t>& labels)
{
	arma::rowvec queryData;
	FillArmaVec(features, queryData);
	
	arma::Row<size_t> predictions;
	arma::mat probabilities;
	auto result = rForest.Classify(queryData);

	return std::vector<size_t> { result };
}

