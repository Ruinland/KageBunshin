#include <iostream>
#include <string>
#include <vector>
#include <tuple>

// For getchar() in debugging, remove after realese ...
#include <cstdio>

// For exit() && getenv()
#include <cstdlib>
// For strerror()
#include <cstring>
// For errno
#include <cerrno>
// Platform operations
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/capability.h>

namespace kage {
  typedef std::vector<std::string> ContainerFolders;
  // (mount id, mount targe, fs type, flags, mount options)
  typedef std::tuple<std::string,
                     std::string,
                     std::string,
                     unsigned long int,
                     std::string> MountDetail;
  typedef std::vector<MountDetail> MountList;
  }

void mkdir_helper(std::string); 
void mount_helper(kage::MountDetail); 
void print_usage(void);
