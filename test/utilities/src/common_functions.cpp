#include "common_functions.hpp"

namespace autocomp {
  namespace test {

  	std::string getDataFromFile(const std::string & filename)
  	{
  		std::ifstream file;

  		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  		file.open(filename, std::ifstream::in | std::ifstream::binary);

  		std::string data((std::istreambuf_iterator<char>(file)),
		                 	std::istreambuf_iterator<char>());

  		file.close();

  		return std::move(data);
  	}

  } // test
} // autocomp