#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <algorithm>

#include "helper.hpp"

// Needed for namespace manipulation
// #define _GNU_SOURCE //clang complains abt it
#include <sched.h>

#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
  if (argc <2) {
    std::cout<<"usage: ./kage container_folder_name"<<std::endl;
    exit(1);
    }

  // mount namespace,
  // no arguments required, just detach from root namespace
  int mflag =0;
  // net namespace, read config for now
  int nflag =0; 
  // unmount flag to unmount existing container
  int uflag =0;
  // display help
  int hflag =0;
  // error flag to deal with exclusive options and wrong syntax
  int errflag =0;
  

  int index;
  std::string nvalue;
  std::string dirName;
  int c;

  opterr = 0;
  while ((c = getopt (argc, argv, "humn:")) != -1) {
    switch (c) {
        case 'h':
          if(uflag||hflag) 
            ++errflag;
          else
            hflag = 1;
          break;
        case 'u':
          if(uflag||hflag) 
            ++errflag;
          else
            uflag = 1;
          break;
        case 'm':
          if(uflag||hflag) 
            ++errflag;
          else
            mflag = 1;
          break;
        case 'n':
          if(uflag||hflag) 
            ++errflag;
          else {
            nflag = 1;
            nvalue = std::string(optarg);
            }
          break;
        case '?':
          if (optopt == 'n')
            std::cerr<<"Option -"<<optopt<<"requires an argument.\n";
          else if (isprint (optopt))
            std::cerr<<"Unknown option `-"<<optopt<<"'.\n";
          else
            std::cerr<<"Unknown option character `\\x="<<std::hex<<optopt<<"'.\n";
          return 1;
        default:
          abort ();
        }
      }

  int non_opt_count = 0;
  for (index = optind; index < argc; index++) {
    if(non_opt_count <= 0 ) {
      std::cout<<"Container dir name: "<<argv[index]<<"\n";
      dirName = std::string(argv[index]);
      ++non_opt_count;
      }
    else {
      ++errflag;
      std::cout<<"More than one non-option argument: "<<argv[index]<<"\n";
      print_usage();
      break;
      }
    }

  if(errflag||hflag) {
    std::cout<<"Invalid arguments."<<std::endl;
    print_usage();
    exit(1);
    }
  
  // Don't mess around me now, this is just a POC.
  std::string folderName = dirName;
  std::string workDirName(folderName  + "/work");
  std::string upperDirName(folderName + "/upper");
  std::string mergeDirName(folderName + "/new_root");

  // Setting up about-to-mount/create list
  kage::MountList mountList = {
      // First one is Overlay Root
      // (mount id, mount targe, fs type, flags, mount options)
      std::make_tuple(folderName ,
                      mergeDirName,
                      "overlay",
                      (MS_NODEV|MS_MGC_VAL), 
                      "lowerdir=/,upperdir="+upperDirName+",workdir="+workDirName),
      std::make_tuple("proc",
                       mergeDirName+"/proc",
                       "proc",
                       (MS_MGC_VAL|MS_NOSUID|MS_NODEV|MS_NOEXEC),
                       ""),
      std::make_tuple("sys",
                      mergeDirName+"/sys",
                      "sysfs",
                      (MS_MGC_VAL|MS_RDONLY|MS_NOSUID|MS_NODEV|MS_NOEXEC),
                      ""),
      std::make_tuple("udev",
                      mergeDirName+"/dev",
                      "devtmpfs",
                      (MS_MGC_VAL|MS_NOSUID),
                      "mode=0755"),
      std::make_tuple("devpts",
                      mergeDirName+"/dev/pts",
                      "devpts",
                      (MS_MGC_VAL|MS_NOSUID|MS_NOEXEC),
                      "mode=0620,gid=5"),
      std::make_tuple("run",
                      mergeDirName+"/run",
                      "tmpfs",
                      (MS_MGC_VAL|MS_NOSUID|MS_NODEV),
                      "mode=0755"),
      std::make_tuple("tmp",
                      mergeDirName+"/tmp",
                      "tmpfs",
                      (MS_NOSUID|MS_NODEV|MS_STRICTATIME),
                      "mode=1777")};

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
        // Terminate child process after creating directories !!!
        exit(0);
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
  kage::MountAction act = kage::m;
  std::for_each(mountList.begin(), mountList.end(),
                // This is a lambda trick to pass 2 args to for_each
                [act](kage::MountDetail md) {
                  mount_helper(md, act);
                  });
  

  // Dealing with namespace flags ;
  // Rewrite this part with for_each statement later...
  int cloneFlags = 0;
  static char childStack[kage::CHILD_STACK_SIZE];
  if( mflag == 1) {
    cloneFlags = cloneFlags | CLONE_NEWNS;
    }

  //namespace_helper();
  // Clone(fork) a process to chroot into overlay root.
  // Parent process will wait and clean up if child process exist.
  childPID = clone(child_main,
                   childStack + kage::CHILD_STACK_SIZE,
                   cloneFlags | SIGCHLD,
                   (void *)mergeDirName.c_str())
                   ;
  if(childPID >= 0) {
    if(childPID == 0) {
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
    std::string cmd = "./cleanup.sh "+mergeDirName;
    system(cmd.c_str());
    exit(1);
    }
    

  return 0;
  }
