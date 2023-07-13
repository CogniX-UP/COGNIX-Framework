/**
 * @file DecisionMaker/include/ModelFactory.h
 * @author Georgios Papadoulis
 *
 * Classification mechanism model factory.
 *
 */
#pragma once
#include <BaseModel.h>
#include <LinearSvm.hpp>
#include <SoftmaxRegression.hpp>
#include <RandomForest.h>
#include<string>
namespace DecisionMaking {
	/**
	 * The class model factory helps resolve a path to the model.
	 */
	class ModelFactory {
	public:
		/**
		 * Gets a pointer to a created model. The model is resolved through a combination of
		 * model name and library name. Resolving this might return a nullptr if the model
		 * doesn't exist.
		 * 
		 * @param modelName The unique name of the model as an std::string
		 * @param library The unique name of the library as an std::string
		 */
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