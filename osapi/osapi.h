/* osapi.h */

#include <os.h>
#include <sys/stat.h>
#include <unistd.h>

#define getposixfd(x) fds[(x)]
private
bool isopen(file_discriptor fd);
