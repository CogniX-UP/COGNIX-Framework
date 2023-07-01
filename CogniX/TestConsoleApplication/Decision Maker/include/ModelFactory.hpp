#pragma once
#include <BaseModel.h>
#include <LinearSvm.hpp>
#include <SoftmaxRegression.hpp>
#include <RandomForest.h>
#include<string>
namespace DecisionMaking {
	class ModelFactory {
	public:
		DecisionMaking::BaseModel* CreateModel(std::string modelName, std::string library) {
			if (modelName == LSVM) {
				if (library == MLPACK)
					return new LinearSvm;
			}
			else if (modelName == SOFTMAX_REG) {
				if (library == MLPACK)
					return new SoftmaxRegression;
			}
			else if (modelName == RANDOM_FOREST) {
				return new RandomForestModel;
			}

			return nullptr;
		}
		//Model Names
		const std::string LSVM = "lsvm";
		const std::string SOFTMAX_REG = "softmax";
		const std::string RANDOM_FOREST = "random_forest";
		
		//Libraries
		const std::string MLPACK = "mlpack";
	};
}