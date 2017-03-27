
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <termios.h>

int same_file(char *src, char *dst) {
    struct stat s1;
    struct stat s2;
    int f1;
    int f2;

    f1 = open(src, O_RDONLY);
    f2 = open(dst, O_RDONLY);

    fstat(f1, &s1);
    fstat(f2, &s2);

    close(f1);
    close(f2);

    return s1.st_ino == s2.st_ino && s1.st_dev == s2.st_dev;
}


int getonechar() {
    int c;
    static struct termios oldt, newt;

    /*tcgetattr gets the parameters of the current terminal
    STDIN_FILENO will tell tcgetattr that it should write the settings
    of stdin to oldt*/
    tcgetattr(STDIN_FILENO, &oldt);

    newt = oldt;

    /*ICANON normally takes care that one line at a time will be processed
    that means it will return if it sees a "\n" or an EOF or an EOL*/
    newt.c_lflag &= ~(ICANON);

    /*Those new settings will be set to STDIN
    TCSANOW tells tcsetattr to change attributes immediately. */
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    c = getchar();

    /*restore the old settings*/
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return c;
}
