#pragma once
#include <BaseModel.h>
#include <mlpack/methods/random_forest/random_forest.hpp>

namespace DecisionMaking {
	//A Class implementing the random forest algorithm using mlpack
	class RandomForestModel : public BaseModel {
	public:
		void Load(const std::string& path) override;
		void Save(const std::string& path) override;
		std::vector<size_t> Predict(std::vector<double>& features, std::vector<size_t>& labels) override;
		virtual void Train(std::vector<double>& observations, size_t featureSize, std::vector<size_t>& labels, size_t numClasses) override;
		virtual double CrossValidate(size_t k, std::vector<double>& observations, size_t featureSize, std::vector<size_t>& labels, size_t numClasses, Metric metric) override;
	private:
		mlpack::RandomForest<> rForest;
	};
}