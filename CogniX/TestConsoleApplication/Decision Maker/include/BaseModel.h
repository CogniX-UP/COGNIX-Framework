#pragma once
#include<string>
#include<vector>
namespace DecisionMaking {
	
	class BaseModel {
	public:
		virtual void Load(const std::string& path) = 0;
		virtual void Save(const std::string& path) = 0;
		virtual std::vector<size_t> Predict(std::vector<double>& features) = 0;
	private:

	};
}