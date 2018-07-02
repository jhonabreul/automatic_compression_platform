#include <iostream>
#include <vector>
#include <string>
#include <fcntl.h>
#include <cmath>

#include "utils/functions.hpp"
#include "common_functions.hpp"

int main()
{
  std::vector<unsigned char> as(1000000, 'a');

  std::cout << "Bytecounting for 1 mb of 'a' character: "
            << autocomp::bytecounting(as.data(), as.size())
            << std::endl;

  std::vector<std::string> fileList{
    "test/test_files/alice29.txt",
    "test/test_files/test_dir/test_file2.txt",
    "test/test_files/test_dir/test_file3.xml",
    "test/test_files/test_dir/test_file.txt",
    "test/test_files/test_dir/test_subdir2/test_file.html",
    "test/test_files/test_dir/test_subdir2/test_subsubdir/test_subsubfile.txt",
    "test/test_files/test_dir/test_subdir/test_subfile.txt",
    "test/test_files/test.trace"
  };

  for (auto file : fileList) {
    std::string data;
    data = autocomp::test::getDataFromFile(file);
    auto tmpData = reinterpret_cast<const unsigned char *>(data.data());
    
    std::cout << "Bytecounting for file " << file << ":\n"
              << "  100%: " << autocomp::bytecounting(tmpData, data.size())
              << "\n  10%: " 
              << autocomp::bytecounting(tmpData + (int) (data.size() * 0.45),
                                        data.size() * 0.10)
              << "\n  20%: " 
              << autocomp::bytecounting(tmpData + (int) (data.size() * 0.40),
                                        data.size() * 0.20)
              << "\n  30%: " 
              << autocomp::bytecounting(tmpData + (int) (data.size() * 0.35),
                                        data.size() * 0.30)
              << "\n  40%: " 
              << autocomp::bytecounting(tmpData + (int) (data.size() * 0.30),
                                        data.size() * 0.40)
              << "\n  mean(10% + 10% + 10%): " 
              << std::round(
                  (autocomp::bytecounting(tmpData + (int) (data.size() * 0.10),
                                           data.size() * 0.10) + 
                   autocomp::bytecounting(tmpData + (int) (data.size() * 0.45),
                                           data.size() * 0.10) +
                   autocomp::bytecounting(tmpData + (int) (data.size() * 0.80),
                                           data.size() * 0.10)) / 3.0
                 )
              << std::endl;
  }

  // Random data
  int randomDataFile = ::open("/dev/urandom", O_RDONLY);
  if (randomDataFile < 0) {
    std::cerr << "Error opening /dev/urandom" << std::endl;
  }
  else {
    std::vector<unsigned char> randomData(3000000);
    ssize_t result = ::read(randomDataFile, randomData.data(),
                            randomData.size());
    if (result < 0) {
      std::cerr << "Error reading data from /dev/urandom" << std::endl;
    }

    std::cout << "Bytecounting for 3 mb or random data:\n"
              << "  100%: "
              << autocomp::bytecounting(randomData.data(), randomData.size())
              << "\n  10%: "
              << autocomp::bytecounting(randomData.data() +
                                          (int) (randomData.size() * 0.45),
                                        randomData.size() * 0.10)
              << "\n  20%: "
              << autocomp::bytecounting(randomData.data() +
                                          (int) (randomData.size() * 0.40),
                                        randomData.size() * 0.20)
              << "\n  30%: "
              << autocomp::bytecounting(randomData.data() +
                                          (int) (randomData.size() * 0.35),
                                        randomData.size() * 0.30)
              << "\n  40%: "
              << autocomp::bytecounting(randomData.data() +
                                          (int) (randomData.size() * 0.30),
                                        randomData.size() * 0.40)
              << "\n  mean(10% + 10% + 10%): " 
              << std::round(
                  (autocomp::bytecounting(randomData.data() +
                                            (int) (randomData.size() * 0.10),
                                          randomData.size() * 0.10) + 
                   autocomp::bytecounting(randomData.data() + 
                                            (int) (randomData.size() * 0.45),
                                          randomData.size() * 0.10) +
                   autocomp::bytecounting(randomData.data() + 
                                            (int) (randomData.size() * 0.80),
                                          randomData.size() * 0.10)) / 3.0
                 )
              << std::endl;
  }

  return 0;
}