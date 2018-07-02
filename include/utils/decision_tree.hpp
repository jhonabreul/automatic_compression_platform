/**
 *  AutoComp Decision Tree
 *  decision_tree.hpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 08/20/2018
 */

#ifndef AC_DECISION_TREE_HPP
#define AC_DECISION_TREE_HPP

#include <vector>
#include <string>
#include <utility>
#include <fstream>
#include <sstream>

#include "messaging/compressor.pb.h"
#include "utils/exceptions.hpp"

namespace autocomp {


class DecisionTree
{
  struct Node
  {
    int leftChild;
    int rightChild;
    int feature;
    float threshold;
    int value;
  };

  std::vector<Node> nodes;

  std::vector<std::pair<Compressor, int>> compressors;

  unsigned int nFeatures;

public:

  DecisionTree(const std::string & filename);

  DecisionTree(const DecisionTree &) = delete;
  DecisionTree(DecisionTree &&) = delete;
  DecisionTree & operator=(const DecisionTree &) = delete;
  DecisionTree & operator=(DecisionTree &&) = delete;

  std::pair<Compressor, int> classify(const std::vector<int> & point) const;

private:

  bool isLeaf(const Node & node) const;

  template<class T>
  inline void checkInputStream(const T & stream, const std::string & a) const
  {
    if (not stream.good()) {
      throw exceptions::IOError("Input file is not a valid decision tree" + a);
    }
  }

  bool parseCompressors(const std::vector<std::string> & compressor_labels);

}; // class DecisionTree

} // namespace autocomp

#endif // AC_DECISION_TREE_HPP