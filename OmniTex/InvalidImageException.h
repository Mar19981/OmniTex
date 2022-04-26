#pragma once
#include <exception>
#include <string>

class InvalidImageException: std::exception
{
public:
	InvalidImageException(const std::string_view& m); 	
	const char* what() const override;
private:
	const std::string_view msg; 
};

