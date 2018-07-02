/**
 *  AutoComp Directory Explorer
 *  directory_explorer.hpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/20/2018
 */

#ifndef AC_DIRECTORY_EXPLORER_HPP
#define AC_DIRECTORY_EXPLORER_HPP

#include <string>
#include <queue>
#include <map>

extern "C" {
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <dirent.h>
}

#include "utils/exceptions.hpp"

namespace autocomp {

/** 
 * This is a helper class for recursively traversing directories.
 */
class DirectoryExplorer
{

  std::string root;

  std::string nextFile;

  std::string currentDirectory;

  std::queue<std::string> directoriesQueue;

  DIR * directory;

  bool rootIsFile;

  bool filesAvailable;

  static const std::map<int, std::string> errorMessages;

public:

  /**
   * Tests the path.
   *
   * @param path The path to be tested
   *
   * @throws exeptions::IOError If the path does not exist
   */
  DirectoryExplorer(const std::string & path);

  ~DirectoryExplorer();

  /**
   * Tests whether there is another available file in the path 
   */
  bool hasNextFile();

  /**
   * Gets the name of the next available file relative to the root path given
   * during instantiation
   */
  std::string getNextFileName();

private:

  /**
   * Checks whether the path is a directory
   *
   * @param path The path to be tested
   *
   * @throws exeptions::IOError If the path does not exist
   */
  bool isDirectory(const std::string & path);

  /**
   * Finds the next filename in the root path given during instantiation.
   * This only changes the internal state of the object.
   *
   * @throws exeptions::IOError If the path does not exist
   */
  void findNextFile();


}; // class DirectoryExplorer

} // namespace autocomp

#endif // AC_DIRECTORY_EXPLORER_HPP