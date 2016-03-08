#include "helper.hpp"

#include <sys/types.h>
#include <sys/unistd.h>

// Default flag of a directory is 7 5 5
#define DEFAULT_FLAG (S_IRWXU|(S_IRGRP|S_IWGRP)|(S_IROTH|S_IWOTH))

void print_usage(void) {
    std::cout<<"Usage: sudo ./kage [-h] [-n net_conf] [-u] [-m] dir_name\n";
    std::cout<<"-h show this help.\n";
    std::cout<<"-u unmount existing container.\n";
    std::cout<<"-n read net_conf and setup netns.\n";
    std::cout<<"-m create independent mount ns.\n";
    }

void mkdir_helper(std::string folderName) {
  try {
    // Making directories for mounting
    std::cout<<getuid()<<std::endl;

    // sudo will expose the true uid & gid by env
    const char* env_uid = std::getenv("SUDO_UID");
    const char* env_gid = std::getenv("SUDO_GID");
    uid_t oUid = std::atoi(env_uid);
    uid_t oGid = std::atoi(env_gid);
    setuid(oUid);
    setgid(oGid);

    std::cout<<"Making dir:"<<folderName.c_str()<<std::endl;
    
    if ( mkdir( folderName.c_str(), DEFAULT_FLAG) ) {
      throw(std::strerror(errno));
      }

    }
  catch (const char* errmsg){
    std::cerr<<"Failed during creating folder: "<<folderName<<std::endl;
    std::cerr<<errmsg<<std::endl;
    exit(1);
    }
  }

int child_main(void *mergeDirName) {
  //uid_t oUid = std::atoi(std::getenv("SUDO_UID"));
  char *uidString = std::getenv("SUDO_UID");
  //char arg_su[] = "-u "+;
  chroot((const char*)mergeDirName);
  char arg_inter[] = "-i";
  char *argv_list[] = {uidString,NULL};
  execvp("su", argv_list);

  return 1;
  } 

void namespace_helper(kage::NamespaceType nsType) {

    } 

void mount_helper(kage::MountDetail _entry, kage::MountAction _act) {
  try {
    // Mounting directories
    std::string mountId     = std::get<0>(_entry); 
    std::string mountPoint  = std::get<1>(_entry); 
    std::string fsType      = std::get<2>(_entry);
    unsigned long int mOpt  = std::get<3>(_entry);
    std::string mountOpt    = std::get<4>(_entry);
    std::cout<<"Mounting: ";
    std::cout<<mountId<<" to ";  
    std::cout<<mountPoint<<" as";  
    std::cout<<fsType    <<" with options: "; 
    std::cout<<mountOpt  <<std::endl; 
    // Mount arguments are handled diffrently...
    // For instance, be careful of proc, which mount options are filled
    // in mount flags...
    switch(_act) {
      case kage::m:
        if( mount(mountId.c_str(), mountPoint.c_str(),
            fsType.c_str(), mOpt, mountOpt.c_str()) ) {
          std::cerr<<"Error occured when mounting: "<<mountPoint<<std::endl;
          throw(std::strerror(errno));
          }
        break;
      case kage::u:
        if( umount2(mountPoint.c_str(), 0) ) {
          std::cerr<<"Error occured when umounting: "<<mountPoint<<std::endl;
          throw(std::strerror(errno));
          }
        break;
      case kage::r:
        throw("Not supporting remount for now ...");
        break;

      default:
        throw("Internal error: invalid mount action.");
      }
    //if (0)
    //  throw(std::strerror(errno));
    }
  catch (const char* errmsg){
    std::cerr<<errmsg<<std::endl;
    exit(1);
    }
  }
