#pragma once
#include "BaseModel.h"
#include <mlpack/methods/random_forest/random_forest.hpp>

namespace DecisionMaking {
	//A Class implementing the random forest algorithm using mlpack
	class RandomForestModel : public BaseModel {
	public:
		void Load(const std::string& path) override;
		void Save(const std::string& path) override;
		std::vector<size_t> Predict(std::vector<double>& features, std::vector<size_t>& labels) override;
		void Train(std::vector<std::vector<double>>& observations, std::vector<size_t>& labels, size_t numClasses) override;
		double CrossValidate(uint32_t k, std::vector<std::vector<double>>& observations, std::vector<size_t>& labels, size_t numClasses, BaseModel::Metric metric) override;
	private:
		mlpack::RandomForest<> rForest;
	};
}