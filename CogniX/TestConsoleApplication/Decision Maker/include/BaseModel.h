/**
 * @file DecisionMaker/include/BaseModel.h
 * @author Georgios Papadoulis
 *
 * Classification mechanism abstraction for interoperability and ease of use.
 *
 */
#pragma once
#include<string>
#include<vector>
#include<mlpack/core/cv/cv.hpp>
#include "MlPackUtil.cpp"

namespace DecisionMaking {
	
	/**
	* The class BaseModel represents any classification mechanism in the CogniX framework.
	* It captures the basic methods needed as pure virtual methods.
	*
	*/
	class BaseModel {
	public:
		/**
		* Typical Accuracy metrics utilized in classifcation problems. Includes Accuracy, F1-Score, Precision and Recall.
		*/
		enum Metric { Accuracy, F1, Precision, Recall };
		/**
		* Loading from a file. Extensions are specificed by the underlying system. Currently, implementations
		* include .bin, .json, and .xml via mlpack. ONNX models may be supported later.
		* 
		* @param path The path where the model file exists.
		*/
		virtual void Load(const std::string& path) = 0;
		/**
		* Saving from a file. Extensions are specificed by the underlying system. Currently, implementations
		* include .bin, .json, and .xml via mlpack. ONNX models may be supported later.
		*
		* @param path The path where the model file should be saved to.
		*/
		virtual void Save(const std::string& path) = 0;
		/**
		* Predict a data point's label given a feature set and a label set. These can be imported by 
		* utilizing various .csv loading libraries and utilities. Returns a set of predicated labels.
		* Typically there is only one predicted label, but the API provides more for extensibility.
		*
		* @param features The features vector to classify.
		* @param labels The labels associated with this classification problem.
		*/
		virtual std::vector<size_t> Predict(std::vector<double>& features, std::vector<size_t>& labels) = 0;
		/**
		* Train the model to imrove the classification results. 
		*
		* @param observations A matrix of observations, given in a typical STD format.
		* @param labels The labels associated with each data point in the feature set.
		* @param numClasses The number of classes, or labels in this classification problem.
		*/
		virtual void Train(std::vector<std::vector<double>>& observations, std::vector<size_t>& labels, size_t numClasses) = 0;
		/**
		* Cross-validate the performance of the classification. Should support k-Cross-Validation.
		*
		* @param k The value of k for cross-validation.
		* @param observations A matrix of observations, given in a typical STD format.
		* @param labels The labels associated with each data point in the feature set.
		* @param numClasses The number of classes, or labels in this classification problem.
		* @param metric The metric to evaluate the classsification on.
		*/
		virtual double CrossValidate(size_t k, std::vector<std::vector<double>>& observations, std::vector<size_t> &labels, size_t numClasses, Metric metric) = 0;
	private:

	};
}