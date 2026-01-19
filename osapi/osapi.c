#include "osapi.h"

Error errno;

private
file_discriptor fds[256] = {
    [0] = 1,
    [1] = 2,
};

private
bool isopen(file_discriptor fd)
{
    if (fd < 2)
        return false;

    signed int posixfd = getposixfd(fd);

    if (posixfd == -1)
        return false;

    struct stat _;
    if ((fstat(posixfd, &_)) == false)
        return false;

    return true;
}

/* write 1 char */
public
bool store(file_discriptor fd, uint8 c)
{
    if (fd > 1 && !isopen(fd))
        reterr(ERR_BAD_FILE_DESC);

    signed int posixfd = getposixfd(fd);

    if (!posixfd)
        reterr(ERR_BAD_FILE_DESC);

    posixfd      = (posixfd == 1 || posixfd == 2) ? (posixfd - 1) : (posixfd);

    uint8 buf[2] = { [0] = c };

    if (write(posixfd, (char*)buf, 1) != 1)
        reterr(ERR_BAD_IO);

    return true;
}

/* read 1 char */
public
uint8 load(file_discriptor fd)
{
    if (fd > 2 && !isopen(fd))
        reterr(ERR_BAD_FILE_DESC);

    signed int posixfd = getposixfd(fd);

    if (!posixfd)
        reterr(ERR_BAD_FILE_DESC);

    posixfd      = (posixfd == 1 || posixfd == 2) ? (posixfd - 1) : (posixfd);

    uint8 buf[2] = { 0 };

    if (read(posixfd, (char*)buf, 1) != 1)
        reterr(ERR_BAD_IO);

    return buf[0];
}
