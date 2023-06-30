#pragma once
#include "BaseModel.h"
#include <mlpack/methods/linear_svm.hpp>
namespace DecisionMaking {
	class Svm : public BaseModel {
	public:
		void Load(const std::string& path) override;
		void Save(const std::string& path) override;
		std::vector<size_t> Predict(std::vector<double>& features) override;
	private:
		mlpack::LinearSVM<> model;
	};
}