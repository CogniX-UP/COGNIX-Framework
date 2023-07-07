#include "LinearSvm.hpp"

using namespace mlpack;

void DecisionMaking::LinearSvm::Load(const std::string& path)
{
	mlpack::data::Load(path, "svm", model);
}

void DecisionMaking::LinearSvm::Save(const std::string& path)
{
	mlpack::data::Save(path, "svm", model);
}

void DecisionMaking::LinearSvm::Train(std::vector<std::vector<double>>& observations, std::vector<size_t>& labels, size_t numClasses)
{
	arma::mat data;
	FillArmaMat(observations, data);
	model.Train(data, labels, numClasses);
}

double DecisionMaking::LinearSvm::CrossValidate(size_t k, std::vector<std::vector<double>>& observations, std::vector<size_t>& labels, size_t numClasses, Metric metric)
{
	arma::mat data;
	FillArmaMat(observations, data);

	if (k == 1) {
		arma::rowvec responses = arma::randu<arma::rowvec>(numClasses);
		switch (metric) {
		case BaseModel::Accuracy: {
			mlpack::SimpleCV<mlpack::LinearSVM<>, mlpack::Accuracy> cv(0.2, data, responses);
		}
		case BaseModel::F1:
		{
			auto cv = mlpack::SimpleCV<mlpack::LinearSVM<>, mlpack::F1<mlpack::AverageStrategy::Macro>>(0.2, data, responses);
			return cv.Evaluate();
		}
		case BaseModel::Precision:
		{
			SimpleCV<mlpack::LinearSVM<>, mlpack::Precision<mlpack::AverageStrategy::Macro>> cv(0.2, data, responses);
			return cv.Evaluate();
		}
		case BaseModel::Recall: {
			SimpleCV<mlpack::LinearSVM<>, mlpack::Recall<mlpack::AverageStrategy::Macro>> cv(0.2, data, responses);
			return cv.Evaluate();
		}
		}
	}
	else {
		switch (metric) {
		case BaseModel::Accuracy: {
			mlpack::KFoldCV<mlpack::LinearSVM<>, mlpack::Accuracy> cv(k, data, labels, numClasses);
			return cv.Evaluate();
		}
		case BaseModel::F1:
		{
			mlpack::KFoldCV<mlpack::LinearSVM<>, mlpack::F1<mlpack::AverageStrategy::Macro>> cv(k, data, labels, numClasses);
			return cv.Evaluate();
		}
		case BaseModel::Precision:
		{
			mlpack::KFoldCV<mlpack::LinearSVM<>, mlpack::Precision<mlpack::AverageStrategy::Macro>> cv(k, data, labels, numClasses);
			return cv.Evaluate();
		}
		case BaseModel::Recall: {
			mlpack::KFoldCV<mlpack::LinearSVM<>, mlpack::Recall<mlpack::AverageStrategy::Macro>> cv(k, data, labels, numClasses);
			return cv.Evaluate();
		}
		}
		return 0.0;
	}
}

std::vector<size_t> DecisionMaking::LinearSvm::Predict(std::vector<double>& features, std::vector<size_t>& labels)
{
	arma::rowvec queryData;
	FillArmaVec(features, queryData);

	arma::Row<size_t> predictions;
	arma::mat probabilities;
	auto result = model.Classify(queryData);

	return std::vector<size_t> { result };
}

