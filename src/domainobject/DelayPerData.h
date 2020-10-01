//
// Created by Chris on 10.09.20.
//

#include <string>

#ifndef POSE_ESTIMATOR_DELAYPERDATA_H
#define POSE_ESTIMATOR_DELAYPERDATA_H

class DelayPerData {
public:
    std::string data;
    double delay;
    const std::string delimiter = " ";

    DelayPerData(double delay, std::string data) : delay(delay), data(data) {};
	DelayPerData(std::string input) {
		int position = 0;
		position = input.find(delimiter, position);
		std::string delayString = input.substr(0, position);
		std::string dataString = input.substr(position + delimiter.length(), input.length() - 1);
		delay = std::stod(delayString);
		data = dataString;
	};

    const std::string toString() { return std::to_string(delay) + delimiter + data; };
};

#endif //POSE_ESTIMATOR_DELAYPERDATA_H
