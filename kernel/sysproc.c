#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

#define UPPER 512 // maximum value to check by pgaccess

uint64
sys_exit(void)
{
  int n;
  if (argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if (argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if (argint(0, &n) < 0)
    return -1;

  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if (argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (myproc()->killed)
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

unsigned int pgaccess_helper(pagetable_t pagetable, uint64 page_addr)
{
  pte_t *pte;

  pte = walk(pagetable, page_addr, 0);

  if ((*pte & PTE_A))
  {
    *pte &= (~PTE_A); // clear PTE_A after checking if it exist
    return 1;
  }
  else
    return 0;
}

#ifdef LAB_PGTBL
int sys_pgaccess(void)
{
  // lab pgtbl: your code here.
  // printf("sys_pgaccess is called.\n");
  // First, it takes a user page to check.
  // Second, it takes the number of pages to check.
  // Third, it takes a user address to a buffer to store the results into a bitmask.
  uint64 start_addr;
  int pages;
  uint64 user_addr;
  if (argaddr(0, &start_addr) < 0)
    return -1;
  if (argint(1, &pages) < 0)
    return -1;
  if (0 > pages || UPPER < pages)
    return -1;
  if (argaddr(2, &user_addr) < 0)
    return -1;

  struct proc *p = myproc();
  int bitmask = 0;

  for (int i = 0; i < pages; ++i)
  {
    int page_addr = start_addr + PGSIZE * i;
    unsigned int abit = pgaccess_helper(p->pagetable, page_addr); // in vm.c
    bitmask |= abit << i;
  }
  // copy it to the user via copyout
  if (copyout(myproc()->pagetable, user_addr, (char *)&bitmask, sizeof(bitmask)) < 0)
    return -1;

  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  if (argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_uperm(void)
{
  uint64 uva;
  int ubits;
  if (argaddr(0, &uva) < 0)
    return -1;
  if (argint(1, &ubits) < 0)
    return -1;

  pagetable_t pgtbl = myproc()->pagetable;

  // get pgtbl entry from a virtual addr -> walk()
  pte_t *pte = walk(pgtbl, uva, 0); // find a page without allocate one if not found.

  // check if it valid
  if ((*pte & PTE_V) == 0) // pte_v cheack the permission bit is valid or not.
    return -1;

  // set lower bits
  uint64 perm_bits = 0x1FF;
  *pte &= (~perm_bits); // clear all the permission bits
  *pte |= ubits;        // assigned user bit to permission bits
  return 0;
}