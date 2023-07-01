#pragma once
#include<string>
#include<vector>
#include<mlpack/core/cv/cv.hpp>
#include "MlPackUtil.cpp"

namespace DecisionMaking {
	
	class BaseModel {
	public:
		enum Metric { Accuracy, F1, MSE, Precision, Recall };
		//Loading from a file. Extensions could be anything. Currently support .bin, .json, .xml
		virtual void Load(const std::string& path) = 0;
		//Saving from a file. Extensions could be anything. Currently support .bin, .json, .xml
		virtual void Save(const std::string& path) = 0;
		//Predict a data point's label given a feature vector and a label vector
		virtual std::vector<size_t> Predict(std::vector<double>& features, std::vector<size_t>& labels) = 0;
		//Train the model
		virtual void Train(std::vector<std::vector<double>>& observations, std::vector<size_t>& labels, size_t numClasses) = 0;
		//Cross validate the model using a performance metric
		virtual double CrossValidate(size_t k, std::vector<std::vector<double>>& observations, std::vector<size_t> &labels, size_t numClasses, Metric metric) = 0;
	private:

	};
}