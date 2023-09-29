#include <LabStreamEEG.h>
#include <string.h>
#include <iostream>
#include <mlpack ext/MlPackUtil.h>
#include <mlpack/methods/linear_svm/linear_svm.hpp>
#include <Models/LinearSvm.hpp>

//This is a basic program to test the implemented
//machine learning algorithms. It is a synthetic test that
//tests if the wrapper works

int main()
{
    //Load hard coded data in armadillo format
    arma::mat data("0.339406815,0.843176636,0.472701471; \
                  0.212587646,0.351174901,0.81056695;  \
                  0.160147626,0.255047893,0.04072469;  \
                  0.564535197,0.943435462,0.597070812");

    data = data.t();
    std::cout << "Matrix through armadillo is:" << std::endl;
    std::cout << data << std::endl;

    //Do the same but using an std::vector and the CogniX API
    std::vector<double> vecData = {
        0.339406815,0.843176636,0.472701471,
        0.212587646,0.351174901,0.81056695,
        0.160147626,0.255047893,0.04072469,
        0.564535197,0.943435462,0.597070812
    };

    //Load to an arma mat from a vector of vectors using CogniX
    arma::mat data2;
    FillArmaMat(vecData, 3, data2);
    std::cout << "Matrix through CogniX to armadillo is:" << std::endl;
    std::cout << data2;
    
    
    //Load hard coded labels from armadillo
    arma::u64_rowvec labels("3, 2, 3, 2");
    std::cout << "Labels through armadillo are:" << std::endl;
    std::cout << labels;

    //Load labels through cognix;
    std::vector<size_t> labelData = { 3, 2, 3, 2 };
    arma::u64_rowvec labels2;
    FillArmaVec(labelData, labels2);
    std::cout << "Labels through Cognix to armadillo is:" << std::endl;
    std::cout << labels2;

    //Check if labels are equal
    auto result = data - data2;
    //Check if matrices are equal
    auto labelResult = labels - labels2;
    
    //Here we are validating the conversion
    if (result.is_zero() && labelResult.is_zero()) {
        std::cout << "CONVERSION VALIDATED" << std::endl;
    }
    else
    {
        std::cout << "CONVERSION FAILED";
        return 0;
    }

    //Train and evaluate through armadillo.
    //arma::rowvec dataPoint("0.514535197, 0.843435462, 0.497070812");
    //arma::rowvec dataPoint("0.212587646,0.351174901,0.81056695"); //label should be 3
    arma::colvec dataPoint("0.264535197,0.143435462,0.697070812"); //label should be 2
    auto dataVecPoint = std::vector<double>{ 0.264535197,0.143435462,0.697070812 };
    try {

        //Train  and eusing only armadillo
        mlpack::LinearSVM<> model; // pre-warm model
        model.Train(data, labels, 2);

        mlpack::LinearSVM<> model2;
        model2.Train(data, labels, 2);
        
        std::cout << "Data point is:\n" << dataPoint << std::endl;
        auto labelPredictionIndex = model2.Classify(dataPoint);
        auto predictedLabel = labels[labelPredictionIndex];
        std::cout << "Label Result only armadillo at index: " << labelPredictionIndex << " is: " << predictedLabel << std::endl;
        
        //Train using CogniX API
        DecisionMaking::LinearSvm svm;
        svm.Train(vecData, 3, labelData, 2);
        auto predictions = svm.Predict(dataVecPoint, labelData);
        
        std::cout << "Label Result using CogniX at index: " << predictions[0] << " is: " << labelData[predictions[0]] << std::endl;
    }
    catch (std::exception& e) {
        std::cout << e.what();
    }
    
    std::cout << "Press enter exit" << std::endl;
    std::cin.get();
    return 0;
}
