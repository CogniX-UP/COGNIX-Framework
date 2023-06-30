#pragma once
#include "BaseModel.h"
#include <mlpack/methods/softmax_regression/softmax_regression.hpp>
namespace DecisionMaking {
	class SoftmaxRegression : public BaseModel {
	public:
		void Load(const std::string& path) override;
		void Save(const std::string& path) override;
		std::vector<size_t> Predict(std::vector<double>& features) override;
	private:
		mlpack::SoftmaxRegression model;
	};
}