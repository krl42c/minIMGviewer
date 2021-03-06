#include "file_util.h"


std::string miv::get_wd(std::string path) {
  auto found = path.find_last_of("/\\");
  return(path.substr(0, found));
}

std::vector<std::string> miv::load_all(std::string dir) {
  std::vector<std::string> files;
  for(const auto & entry : std::filesystem::directory_iterator(dir))
    if(entry.path().string().find(".PNG") != std::string::npos)
      files.push_back(entry.path().string());

  return files;
}

bool miv::is_file(std::string path) {
  return path.find(".") != std::string::npos;
}

