#include <Models/RandomForest.h>
using namespace mlpack;
void DecisionMaking::RandomForestModel::Load(const std::string& path)
{
	mlpack::data::Load(path, "rForest", rForest);
}

void DecisionMaking::RandomForestModel::Save(const std::string& path)
{
	mlpack::data::Save(path, "rForest", rForest);
}


void DecisionMaking::RandomForestModel::Train(std::vector<double>& observations, size_t featureSize, std::vector<size_t>& labels, size_t numClasses)
{
	arma::mat data;
	FillArmaMat(observations, featureSize, data);
	rForest.Train(data, labels, numClasses);
}

double DecisionMaking::RandomForestModel::CrossValidate(size_t k, std::vector<double>& observations, size_t featureSize, std::vector<size_t>& labels, size_t numClasses, Metric metric)
{
	arma::mat data;
	FillArmaMat(observations, featureSize, data);

	if (k == 1) {
		switch (metric) {
		case BaseModel::Accuracy: {
			mlpack::SimpleCV<mlpack::RandomForest<>, mlpack::Accuracy> cv(0.2, data, labels, numClasses);
			return cv.Evaluate();
		}
		case BaseModel::F1:
		{
			auto cv = mlpack::SimpleCV<mlpack::RandomForest<>, mlpack::F1<mlpack::AverageStrategy::Macro>>(0.2, data, labels, numClasses);
			return cv.Evaluate();
		}
		case BaseModel::Precision:
		{
			SimpleCV<RandomForest<>, mlpack::Precision<mlpack::AverageStrategy::Macro>> cv(0.2, data, labels, numClasses);
			return cv.Evaluate();
		}
		case BaseModel::Recall: {
			SimpleCV<RandomForest<>, mlpack::Recall<mlpack::AverageStrategy::Macro>> cv(0.2, data, labels, numClasses);
			return cv.Evaluate();
		}
		}
	}
	else {
		switch (metric) {
		case BaseModel::Accuracy: {
			//mlpack::KFoldCV<mlpack::RandomForest<>, mlpack::Accuracy> cv(k, data, labels, numClasses, true);
			return 0;
		}
		case BaseModel::F1:
		{
			//mlpack::KFoldCV<mlpack::RandomForest<>, mlpack::F1<mlpack::AverageStrategy::Macro>> cv(k, data, labels);
			return 0;
		}
		case BaseModel::Precision:
		{
			//mlpack::KFoldCV<mlpack::RandomForest<>, mlpack::Precision<mlpack::AverageStrategy::Macro>> cv(k, data, labels);
			return 0;
		}
		case BaseModel::Recall: {
			//mlpack::KFoldCV<mlpack::RandomForest<>, mlpack::Recall<mlpack::AverageStrategy::Macro>> cv(k, data, labels);
			return 0;
		}
		}
		return 0.0;
	}
}

std::vector<size_t> DecisionMaking::RandomForestModel::Predict(std::vector<double>& features, std::vector<size_t>& labels)
{
	arma::colvec queryData;
	FillArmaVec(features, queryData);
	
	auto result = rForest.Classify(queryData);

	return std::vector<size_t> { result };
}

