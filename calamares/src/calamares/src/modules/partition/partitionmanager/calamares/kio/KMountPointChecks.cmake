include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckStructHasMember)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckSymbolExists)

check_function_exists(getmntinfo  HAVE_GETMNTINFO)
check_function_exists(setmntent   HAVE_SETMNTENT)

check_include_files(mntent.h      HAVE_MNTENT_H)
check_include_files("stdio.h;sys/mnttab.h"  HAVE_SYS_MNTTAB_H)
check_include_files(sys/mntent.h  HAVE_SYS_MNTENT_H)
check_include_files("sys/param.h;sys/mount.h"  HAVE_SYS_MOUNT_H)

check_include_files(sys/types.h   HAVE_SYS_TYPES_H)
check_include_files(fstab.h       HAVE_FSTAB_H)
check_include_files(sys/param.h   HAVE_SYS_PARAM_H)

check_library_exists(volmgt volmgt_running "" HAVE_VOLMGT)

check_cxx_source_compiles("
  #include <sys/types.h>
  #include <sys/statvfs.h>
  int main(){
    struct statvfs *mntbufp;
    int flags;
    return getmntinfo(&mntbufp, flags);
  }
" GETMNTINFO_USES_STATVFS )
