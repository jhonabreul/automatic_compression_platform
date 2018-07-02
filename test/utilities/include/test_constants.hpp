#ifndef TEST_CONSTANTS_H
#define TEST_CONSTANTS_H

namespace autocomp
{
  namespace test
  {
    namespace constants
    {

      const std::string compressionTestFilename("./test/test_files/"
                                                "alice29.txt");

      const std::string fpcTestFilename("./test/test_files/test.trace");

      const std::string testDirectory("test/test_files");

      const std::string testOutputDirectory("test/test_output_dir");

      const unsigned short testPortOne = 25111;

      const unsigned short testPortTwo = 25112;

      const unsigned short testPortThree = 25113;

      const unsigned short testPortFour = 25114;

      const unsigned short testPortFive = 25115;

      const std::string invalidDecisionTreeFile("test/utils_test/include/"
                                                "decision_tree_classifier_invalid.txt");

      const std::string validDecisionTreeFile("test/utils_test/include/"
                                              "decision_tree_classifier.txt");
    
    } // constants
  } // test
} // autocomp

#endif // TEST_CONSTANTS_H