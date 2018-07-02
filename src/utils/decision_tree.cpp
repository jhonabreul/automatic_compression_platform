/**
 *  AutoComp Decision Tree
 *  decision_tree.cpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 08/20/2018
 */

#include "utils/decision_tree.hpp"

namespace autocomp
{

DecisionTree::DecisionTree(const std::string & filename)
{
  std::ifstream input(filename);

  if (!input) {
    throw exceptions::IOError("File " + filename + " does not exist");
  }

  // Read compressor labels
  int nCompressors;
  input >> nCompressors;
  checkInputStream(input, " 1");
  std::vector<std::string> compressorLabels(nCompressors);

  for (int i = 0; i < nCompressors; i++) {
    input >> compressorLabels[i];
    checkInputStream(input, " 2");
  }

  if (not this->parseCompressors(compressorLabels)) {
    throw exceptions::IOError("An error occured while reading a compressor "
                              "label from file " + filename);
  }

  // Read number of features
  input >> this->nFeatures;
  checkInputStream(input, " 3");

  // Read number of nodes
  std::size_t nNodes;
  input >> nNodes;
  checkInputStream(input, " 4");
  this->nodes = std::vector<Node>(nNodes);

  input.ignore(2);

  // Read node info
  for (int i = 0; i < nNodes; i++) {
    std::string line;
    int leftChild, rightChild, feature, value;
    float threshold;

    std::getline(input, line);
    checkInputStream(input, " 5");

    std::istringstream stream(line);
    stream >> leftChild >> rightChild >> feature >> threshold >> value;
    checkInputStream(stream, " 6");

    this->nodes[i] = {leftChild, rightChild, feature, threshold, value};
  }

  input.close();
}

std::pair<Compressor, int>
DecisionTree::classify(const std::vector<int> & point) const
{
  if (point.size() != this->nFeatures) {
    throw std::invalid_argument("Invalid point dimensionality. Shape of " +
                                std::to_string(point.size()) +
                                " was given but must be " +
                                std::to_string(this->nFeatures));
  }

  int currentNodeIndex = 0;

  while (not isLeaf(this->nodes[currentNodeIndex])) {
    const auto & currentNode = this->nodes[currentNodeIndex];
    currentNodeIndex = (point[currentNode.feature] <= currentNode.threshold)
                          ? currentNode.leftChild : currentNode.rightChild;
  }

  return this->compressors[this->nodes[currentNodeIndex].value];
}

inline bool DecisionTree::isLeaf(const Node & node) const
{
  return (node.leftChild == node.rightChild);
}

inline bool
DecisionTree::parseCompressors(const std::vector<std::string> &
                                  compressorLabels)
{
  Compressor compressor;
  std::string compressorName;
  int compressionLevel;

  for (const std::string & label : compressorLabels) {
    std::size_t delimiterPosition = label.find("_");

    compressorName = label.substr(0, delimiterPosition);
    std::transform(compressorName.begin(), compressorName.end(),
                   compressorName.begin(), ::toupper);
    
    try {
      compressionLevel = (delimiterPosition == std::string::npos)
                          ? -1 : std::stoi(label.substr(delimiterPosition + 1));
    }
    catch (...) {
      return false;
    }

    if (not Compressor_Parse(compressorName, &compressor)) {
      return false;
    }

    this->compressors.push_back(std::make_pair(compressor, compressionLevel));
  }

  return true;
}


} // namespace autocomp