#ifndef TEST_UTILITIES_H
#define TEST_UTILITIES_H

/* C++ System Headers */
#include <string>
#include <fstream>
#include <streambuf>
#include <utility>

namespace autocomp {
  namespace test {

  	std::string getDataFromFile(const std::string & filename);

  } // test
} // autocomp

#endif // TEST_UTILITIES_H