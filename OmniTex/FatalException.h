#pragma once
#include <exception>
#include <string>

class FatalException : public std::exception {

public:
	FatalException(const std::string_view& m);
	const char* what() const override;
private:
	const std::string_view msg; 
};