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
    const char* env_uid = std::getenv("SUDO_UID");
    const char* env_gid = std::getenv("SUDO_GID");
    uid_t oUid = std::atoi(env_uid);
    uid_t oGid = std::atoi(env_gid);
    std::cout<<oUid<<" "<<oGid<<std::endl;
    setuid(oUid);
    setgid(oGid);
    std::cout<<getuid()<<std::endl;

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

void mount_helper(kage::MountDetail _entry) {
  try {
    // Mounting directories
    std::string mountId     = std::get<0>(_entry); 
    std::string mountPoint  = std::get<1>(_entry); 
    std::string fsType      = std::get<2>(_entry);
    unsigned long int mOpt  = std::get<3>(_entry);
    std::string mountOpt    = std::get<4>(_entry);
    std::cout<<mountId<<" ";  
    std::cout<<mountPoint<<" ";  
    std::cout<<fsType    <<" "; 
    std::cout<<mountOpt  <<std::endl; 
    // Mount arguments are handled diffrently...
    // For instance, be careful of proc, which mount options are filled
    // in mount flags...
    if ( mount(mountId.c_str(), mountPoint.c_str(), fsType.c_str(),
               mOpt, mountOpt.c_str()) ) {
      throw(std::strerror(errno));
      }
    //if (0)
    //  throw(std::strerror(errno));
    }
  catch (const char* errmsg){
    std::cerr<<errmsg<<std::endl;
    exit(1);
    }
  }

/* Obsoleted code ... remove if go release
  try {
    //Making directories for mounting
    if ( mkdir( folderName.c_str(), DEFAULT_FLAG) )
      throw(std::strerror(errno));
    if ( mkdir( workDirName.c_str(), DEFAULT_FLAG) )
      throw(std::strerror(errno));
    if ( mkdir( upperDirName.c_str(), DEFAULT_FLAG) )
      throw(std::strerror(errno));
    if ( mkdir( mergeDirName.c_str(), DEFAULT_FLAG) )
      throw(std::strerror(errno));
    }
  catch (const std::string& errmsg){
    std::cerr<<"Failed during creating folder: "<<folderName<<std::endl;
    std::cerr<<errmsg<<std::endl;
    exit(1);
    }
*/
