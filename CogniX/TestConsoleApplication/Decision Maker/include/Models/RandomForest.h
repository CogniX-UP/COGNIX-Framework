#pragma once
#include "BaseModel.h"
#include <mlpack/methods/random_forest/random_forest.hpp>

namespace DecisionMaking {
	class RandomForestModel : public BaseModel {
	public:
		void Load(const std::string& path) override;
		void Save(const std::string& path) override;
		std::vector<size_t> Predict(std::vector<double>& features) override;
	private:
		mlpack::RandomForest<> rForest;
	};
}