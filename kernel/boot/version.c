#include <version.h>
#include <string.h>

char * __kernel_git_hash = GITHASH;
char * __kernel_git_date = GITDATE;
int __kernel_git_dirty = GITDIRTY;

char * __kernel_build_date = __DATE__;
char * __kernel_build_time = __TIME__;
