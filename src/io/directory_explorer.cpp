/**
 *  AutoComp Directory Explorer
 *  directory_explorer.cpp
 *
 *  @author Jhonathan Abreu
 *  @version 1.0
 *  @date 07/20/2018
 */

#include "io/directory_explorer.hpp"

namespace autocomp {

const std::map<int, std::string> DirectoryExplorer::errorMessages = {
  {EACCES,       "Current system user does not have the correct permissions on "
                 "the given path"},
  {EMFILE,       "Current process has too many open file descriptors"},
  {ENFILE,       "Too many open file descriptors in the system"},
  {ENOENT,       "The given path does not exists"},
  {ENOMEM,       "Not enough memory to complete the operation"},
  {ENOTDIR,      "The given path does not correspond to a directory"},
  {ENOENT,       "A path component does not exist"},
  {ELOOP,        "To many symbolic links in the path"},
  {EFAULT,       "Wrong direction"},
  {ENAMETOOLONG, "The file name is too long"}
};

// Tests the path.
DirectoryExplorer::DirectoryExplorer(const std::string & path)
  : root(path),
    currentDirectory(path),
    directory(nullptr),
    rootIsFile(false),
    filesAvailable(true)
{

  if (isDirectory(path)) {
    this->directory = opendir(path.c_str());

    if (this->directory == nullptr) {
      std::string errorMessage("Error testing the path ");
      errorMessage.append(path)
                  .append(". Errno: ")
                  .append(std::to_string(errno))
                  .append(", ")
                  .append(errorMessages.at(errno));

      throw exceptions::IOError(errorMessage);
    }

    findNextFile();
  }
  else {
    struct stat stats;   
    if (stat(path.c_str(), &stats) == -1) {
      std::string errorMessage("The requested file ");
      errorMessage.append(path)
                  .append(" does not exist. Errno: ")
                  .append(std::to_string(errno))
                  .append(", ")
                  .append(errorMessages.at(errno));

      throw exceptions::IOError(errorMessage);
    } 

    this->nextFile = path;
    this->rootIsFile = true;
  }
}

DirectoryExplorer::~DirectoryExplorer()
{
  /*if (this->directory != nullptr) {
    closedir(this->directory);
  }*/
}

// Tests whether there is another available file in the path 
bool DirectoryExplorer::hasNextFile()
{
  return this->filesAvailable;
}

// Gets the name of the next available file relative to the root path given
// during instantiation
std::string DirectoryExplorer::getNextFileName()
{
  if (not this->rootIsFile) {
    std::string currentFile(this->nextFile);
    findNextFile();

    return currentFile;
  }
  else {
    this->filesAvailable = false;

    return this->nextFile;
  }
}

// Checks whether the path is a directory
bool DirectoryExplorer::isDirectory(const std::string & path)
{
  struct stat pathBuffer;

  if (stat(path.c_str(), &pathBuffer) != 0) {
    std::string errorMessage("Could not stat path ");
    errorMessage.append(path)
                .append(". Errno: ")
                .append(std::to_string(errno))
                .append(", ")
                .append(errorMessages.at(errno));

    throw exceptions::IOError(errorMessage);
  }

  return S_ISDIR(pathBuffer.st_mode);

  //return stat(path.c_str(), &pathBuffer) != 0 and S_ISDIR(pathBuffer.st_mode);
}

// Finds the next filename in the root path given during instantiation.
// This only changes the internal state of the object.
void DirectoryExplorer::findNextFile()
{
  while (true) {
    dirent * entry = readdir(this->directory);

    if (entry != nullptr) {
      std::string entryPath = this->currentDirectory + "/" + entry->d_name;

      if (not isDirectory(entryPath)) {
        this->nextFile = entryPath;
        break;
      }

      std::string directoryName(entry->d_name);
      if (directoryName != "." and directoryName != "..") {
        this->directoriesQueue.push(entryPath);
      }
    }
    else { // Get to the next directory
      closedir(this->directory);

      do {
        if (this->directoriesQueue.empty()) {
          this->nextFile = "";
          this->filesAvailable = false;

          return;
        }

        this->currentDirectory = this->directoriesQueue.front();
        this->directoriesQueue.pop();
        this->directory = opendir(this->currentDirectory.c_str());

        /*
        if (this->directory == nullptr) {
          std::string errorMessage("Skipping directory with path ")
          errorMessage.append(nextDirectory)
                      .append(". Errno: ")
                      .append(std::to_string(errno))
                      .append(", ")
                      .append(errorMessages[errno]);

          throw exceptions::IOError(errorMessage);
        }
        */
      } while (this->directory == nullptr);
    }
  }
}
  
} // namespace autocomp