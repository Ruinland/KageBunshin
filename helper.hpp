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
  enum MountAction { m, r, u };
  enum NamespaceType { User, Pic, Net, Mount };
  typedef std::vector<std::string> ContainerFolders;
  // (mount id, mount targe, fs type, flags, mount options)
  typedef std::tuple<std::string,
                     std::string,
                     std::string,
                     unsigned long int,
                     std::string> MountDetail;
  typedef std::vector<MountDetail> MountList;

  constexpr int CHILD_STACK_SIZE = 1024*1024;
  }

void mkdir_helper(std::string); 
void mount_helper(kage::MountDetail, kage::MountAction); 
void namespace_helper(kage::NamespaceType);
int child_main(void *);
void print_usage(void);
