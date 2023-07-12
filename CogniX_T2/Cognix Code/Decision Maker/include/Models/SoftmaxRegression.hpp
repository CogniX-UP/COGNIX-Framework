#pragma once
#include "BaseModel.h"
#include <mlpack/methods/softmax_regression/softmax_regression.hpp>
namespace DecisionMaking {
	class SoftmaxRegression : public BaseModel {
	public:
		void Load(const std::string& path) override;
		void Save(const std::string& path) override;
		std::vector<size_t> Predict(std::vector<double>& features, std::vector<size_t>& labels) override;
		virtual void Train(std::vector<std::vector<double>>& observations, std::vector<size_t>& labels, size_t numClasses) override;
		virtual double CrossValidate(size_t k, std::vector<std::vector<double>>& observations, std::vector<size_t>& labels, size_t numClasses, Metric metric) override;
	private:
		mlpack::SoftmaxRegression model;
	};
}