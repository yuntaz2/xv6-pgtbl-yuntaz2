#include "kernel/param.h"
#include "kernel/fcntl.h"
#include "kernel/types.h"
#include "kernel/riscv.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    char *buffer;
    unsigned int ubits;
    if (argc != 2)
    {
        printf("uperm needs one bit mask.\n");
        exit(1);
    }

    ubits = atoi(argv[1]);
    printf("Allocating pages.\n");
    buffer = malloc(2 * PGSIZE); // allocating 2 page of memory

    printf("Accessing umem.\n");
    buffer[PGSIZE * 0] += 1;
    buffer[PGSIZE * 1] += 1;

    // assign 1 to the first bit of data -> access bit to user pages.
    printf("Setting user page permission.\n");
    uperm(buffer, ubits);

    printf("Access umem again to check result.\n");
    buffer[PGSIZE * 0] += 1;
    buffer[PGSIZE * 1] += 1;
    exit(0);
}