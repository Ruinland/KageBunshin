#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <algorithm>

#include "helper.hpp"

#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
  if (argc <2) {
    std::cout<<"usage: ./kage container_folder_name"<<std::endl;
    exit(1);
    }
  
  // Don't mess around me now, this is just a POC.
  std::string folderName(argv[1]);
  std::string workDirName(folderName  + "/work");
  std::string upperDirName(folderName + "/upper");
  std::string mergeDirName(folderName + "/new_root");

  // Setting up about-to-mount/create list
  kage::MountList mountList = {
      // First one is Overlay Root
      // (mount id, mount targe, fs type, flags, mount options)
      std::make_tuple(folderName ,mergeDirName, "overlay", (MS_NODEV|MS_MGC_VAL), 
      "lowerdir=/,upperdir="+upperDirName+",workdir="+workDirName),
      std::make_tuple("proc", mergeDirName+"/proc","proc",(MS_MGC_VAL|MS_NOSUID|MS_NODEV|MS_NOEXEC),""),
      std::make_tuple("sys", mergeDirName+"/sys","sysfs", (MS_MGC_VAL|MS_RDONLY|MS_NOSUID|MS_NODEV|MS_NOEXEC), ""),
      std::make_tuple("udev", mergeDirName+"/dev","devtmpfs", (MS_MGC_VAL|MS_NOSUID), "mode=0755"),
      std::make_tuple("devpts", mergeDirName+"/dev/pts","devpts", (MS_MGC_VAL|MS_NOSUID|MS_NOEXEC), "mode=0620,gid=5"),
      std::make_tuple("run", mergeDirName+"/run", "tmpfs" , (MS_MGC_VAL|MS_NOSUID|MS_NODEV), "mode=0755"),
      std::make_tuple("tmp", mergeDirName+"/tmp","tmpfs", (MS_NOSUID|MS_NODEV|MS_STRICTATIME), "mode=1777")};
  std::cout<<"Mount List Size: "<<mountList.size()<<std::endl;

  kage::ContainerFolders folderList = {folderName,
                                       upperDirName,
                                       workDirName,
                                       mergeDirName};

  

  // Making directories, since we need to make it accessible by 
  // unprevileged users, fork to setuid();
  pid_t childPID;
  childPID = fork();
  if(childPID >= 0) {
    if(childPID == 0) {
        std::for_each(folderList.begin(), folderList.end(), mkdir_helper);
        }
    else {
        int status;
        waitpid(childPID, &status, WUNTRACED);
        }
    }
  else {
    std::cerr<<"Fork failed before creating direcotries..."<<std::endl;
    exit(1);
    }

  // Mounting OverlayFS and other necessary virtual file systems
  std::for_each(mountList.begin(), mountList.end(), mount_helper);
  
  childPID = fork();
  if(childPID >= 0) {
    if(childPID == 0) {
        chroot(mergeDirName.c_str());
        char arg_inter[] = "-i";
        //uid_t oUid = std::atoi(std::getenv("SUDO_UID"));
        //char arg_su[] = "-u "+;
        char *argv_list[] = {arg_inter,NULL};
        execvp("sh", argv_list);
        }
    else {
        int status;
        waitpid(childPID, &status, 0);

        //Clean up code ... will be rewrite soon...
        std::string cmd = "./cleanup.sh "+mergeDirName;
        system(cmd.c_str());
        }
    }
  else {
    std::cerr<<"Fork failed before chroot..."<<std::endl;
    exit(1);
    }
    

  return 0;
  }
