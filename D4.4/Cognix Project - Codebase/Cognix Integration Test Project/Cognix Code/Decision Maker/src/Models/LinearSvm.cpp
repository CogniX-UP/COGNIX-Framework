#include <Models/LinearSvm.hpp>
#include <mlpack ext/MlPackUtil.h>

using namespace mlpack;

void DecisionMaking::LinearSvm::Load(const std::string& path)
{
	mlpack::data::Load(path, "svm", model);
}

void DecisionMaking::LinearSvm::Save(const std::string& path)
{
	mlpack::data::Save(path, "svm", model);
}

void DecisionMaking::LinearSvm::Train(std::vector<double>& observations, size_t featureSize, std::vector<size_t>& labels, size_t numClasses)
{
	arma::mat data;
	FillArmaMat(observations, featureSize, data);
	model.Train(data, labels, numClasses);
}

double DecisionMaking::LinearSvm::CrossValidate(size_t k, std::vector<double>& observations, size_t featureSize, std::vector<size_t>& labels, size_t numClasses, Metric metric)
{
	arma::mat data;
	FillArmaMat(observations, featureSize, data);

	if (k == 1) {
		switch (metric) {
		case BaseModel::Accuracy: {
			mlpack::SimpleCV<mlpack::LinearSVM<>, mlpack::Accuracy> cv(0.2, data, labels, numClasses);
			return cv.Evaluate();
		}
		case BaseModel::F1:
		{
			mlpack::SimpleCV<mlpack::LinearSVM<>, mlpack::F1<mlpack::AverageStrategy::Macro>> cv(0.2, data, labels, numClasses);
			return cv.Evaluate();
		}
		case BaseModel::Precision:
		{
			SimpleCV<mlpack::LinearSVM<>, mlpack::Precision<mlpack::AverageStrategy::Macro>> cv(0.2, data, labels, numClasses);
			return cv.Evaluate();
		}
		case BaseModel::Recall: {
			SimpleCV<mlpack::LinearSVM<>, mlpack::Recall<mlpack::AverageStrategy::Macro>> cv(0.2, data, labels, numClasses);
			return cv.Evaluate();
		}
		}
	}
	else {
		auto labelsRow = arma::Row<size_t>(labels.data(), labels.size(), false);
		switch (metric) {
		case BaseModel::Accuracy: {
			mlpack::KFoldCV<mlpack::LinearSVM<>, mlpack::Accuracy> cv(k, data, labelsRow, numClasses);
			return cv.Evaluate();
		}
		case BaseModel::F1:
		{
			mlpack::KFoldCV<mlpack::LinearSVM<>, mlpack::F1<mlpack::AverageStrategy::Macro>> cv(k, data, labelsRow, numClasses);
			return cv.Evaluate();
		}
		case BaseModel::Precision:
		{
			mlpack::KFoldCV<mlpack::LinearSVM<>, mlpack::Precision<mlpack::AverageStrategy::Macro>> cv(k, data, labelsRow, numClasses);
			return cv.Evaluate();
		}
		case BaseModel::Recall: {
			mlpack::KFoldCV<mlpack::LinearSVM<>, mlpack::Recall<mlpack::AverageStrategy::Macro>> cv(k, data, labelsRow, numClasses);
			return cv.Evaluate();
		}
		}
		return 0.0;
	}
}

std::vector<size_t> DecisionMaking::LinearSvm::Predict(std::vector<double>& features, std::vector<size_t>& labels)
{
	arma::colvec queryData;
	FillArmaVec(features, queryData);

	auto result = model.Classify(queryData);

	return std::vector<size_t> { result };
}

