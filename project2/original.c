#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#define PAGESIZE 4096

int heapsize = 0;
typedef unsigned char BYTE;
typedef struct chunkhead {
  unsigned int size;
  unsigned int info;
  unsigned char *next, *prev;
} chunkhead;
chunkhead *startofheap = NULL;

unsigned char *mymalloc(unsigned int size) {
  unsigned int smallest = 0;
  BYTE *p = 0;
  chunkhead *chc = 0;
  BYTE *s = sbrk(0);
  chunkhead *ch = startofheap;
  chunkhead *pch = 0;
  unsigned int pg = (size + sizeof(chunkhead)) / PAGESIZE;
  if (pg == 0) {
    size = PAGESIZE;
  } else {
    size = ((int)pg + 1) * PAGESIZE;
  }
  // size -= sizeof(chunkhead);
  if (heapsize == 0) {
    startofheap = (chunkhead *)sbrk(0);
    sbrk(size);
    ch = startofheap;
    ch->info = 1;
    ch->prev = 0;
    ch->size = size;
    ch->next = 0;
    heapsize = size;
    return (BYTE *)ch + (sizeof(chunkhead));
  }
  if (ch->info == 0 && ch->size == size) {
    ch->info = 1;
    return (BYTE *)ch + (sizeof(chunkhead));
  }
  for (; (ch->next) != 0; ch = (chunkhead *)(ch->next)) {
    if (ch->info == 1 || size > ch->size) {
      pch = ch;
      continue;
    }
    if (ch->info == 0 && ch->size == size) {
      ch->info = 1;
      return (BYTE *)ch + (sizeof(chunkhead));
    }
    if (ch->info == 0 && ch->size > size) {
      if (smallest == 0 || smallest > ch->size) {
        chc = (chunkhead *)((BYTE *)ch + (ch->size) + sizeof(chunkhead) -
                            (size + sizeof(chunkhead)));
        p = (BYTE *)ch;
        smallest = ch->size;
      }
    }
  }
  if (smallest != 0) {
    ch = (chunkhead *)p;
    pch = (chunkhead *)(ch->next);
    chc->info = 1;
    ch->info = 0;
    ch->size -= (size) + sizeof(chunkhead);
    chc->prev = (BYTE *)ch;
    chc->size = size;
    ch->next = (BYTE *)chc;
    chc->next = (BYTE *)pch;
    pch->prev = (BYTE *)chc;
    return (BYTE *)ch + sizeof(chunkhead);
  } else {
    sbrk(size);
    heapsize += size + sizeof(chunkhead);
    pch = ch;
    ch = (chunkhead *)((BYTE *)ch + ch->size);
    ch->info = 1;
    ch->size = size;
    ch->next = 0;
    ch->prev = (BYTE *)pch;
    pch->next = (BYTE *)ch;
    return (BYTE *)ch + sizeof(chunkhead);
  }
  return 0;
}

void myfree(unsigned char *address) {
  chunkhead *soh = startofheap;
  int hs = heapsize;
  chunkhead *ch = (chunkhead *)(address - sizeof(chunkhead));
  ch->info = 0;
  chunkhead *chn = ((chunkhead *)(ch->next));
  chunkhead *chp = ((chunkhead *)(ch->prev));
  if (chn == 0) {
    heapsize -= ch->size;
    BYTE *p = sbrk(0);
    brk((BYTE *)p - (ch->size - sizeof(chunkhead)));
    if (chp == 0 || heapsize == 0) {
      heapsize = 0;
      brk(startofheap);
      startofheap = 0;
    }
    if (chp->info == 0) {
      if (chp->prev != 0) {
        chp = (chunkhead *)(chp->prev);
        chp->next = 0;
        int z = brk(sbrk(0) - chp->size);
      } else {
        heapsize = 0;
        int y = brk(startofheap);
        startofheap = 0;
      }
    } else {
      chp->next = ch->next;
    }
  } else if (((chunkhead *)(ch->prev)) == 0 && ((chunkhead *)(ch->next))->info == 0) {
    ch->size += chn->size + sizeof(chunkhead);
    ch->next = chn->next;
    ch->prev = 0;
    if (((chunkhead *)(ch->next)) != 0) {
      ((chunkhead *)(ch->next))->prev = (BYTE *)ch;
    }
  } else if (((chunkhead *)(ch->prev)) == 0 &&
             ((chunkhead *)(ch->next))->info != 0) {
    ch->info = 0;
  } else if (((chunkhead *)(ch->next))->info == 0 &&
             ((chunkhead *)(ch->prev))->info == 0) {
    ch->size += (chn->size) + (chp->size) + sizeof(chunkhead) * 2;
    if (((chunkhead *)(chn->next)) != 0) {
      ((chunkhead *)(chn->next))->prev = (BYTE *)ch;
    }
    if (((chunkhead *)(chp->prev)) != 0) {
      ((chunkhead *)(chp->prev))->next = (BYTE *)ch;
    }
    ch->next = (chn->next);
    ch->prev = (chp->prev);
    chp->size = ch->size;
    chp->next = ch->next;
    chp->prev = ch->prev;
  } else if (chn->info == 0) {
    ch->size += chn->size + sizeof(chunkhead);
    ch->next = chn->next;
    if (((chunkhead *)(ch->next)) != 0) {
      ((chunkhead *)(ch->next))->prev = (BYTE *)ch;
    }
  } else if (chp->info == 0) {
    chp->size += ch->size;
    if (((chunkhead *)(chp->prev)) != 0) {
      ((chunkhead *)(chp->prev))->next = (BYTE *)chp;
    }
    if (((chunkhead *)(chn->next)) != 0) {
      ((chunkhead *)(ch->next))->prev = (BYTE *)chp;
    }
    chp->next = ch->next;
    chn->prev = (BYTE *)chp;
  }
}

chunkhead *get_last_chunk() {
  if (!startofheap) {
    return NULL;
  }
  chunkhead *ch = (chunkhead *)startofheap;
  for (; ch->next; ch = (chunkhead *)ch->next);
  return ch;
}

void analyze() {
  printf("\n--------------------------------------------------------------\n");
  if (!startofheap || heapsize == 0) {
    printf("no heap");
    printf("\nprogram break on address: %x\n", sbrk(0));
    return;
  }
  chunkhead *ch = (chunkhead *)startofheap;
  for (int no = 0; ch; ch = (chunkhead *)ch->next, no++) {
    printf("%d | current addr: %p |", no, (BYTE *)ch);
    printf("size: %d | ", ch->size);
    printf("info: %d | ", ch->info);
    printf("next: %p | ", ch->next);
    printf("prev: %p", ch->prev);
    printf(" \n");
  }
  printf("\nprogram break on address: %x\n", sbrk(0));
}

int main() {
  printf(" ");
  BYTE *a[100];
  clock_t ca, cb;
  ca = clock();
  analyze();

  for (int i = 0; i < 100; i++) {
    a[i] = mymalloc(1000);
  }

  for (int i = 0; i < 90; i++) {
    myfree(a[i]);
  }
  analyze();
  myfree(a[95]);
  a[95] = mymalloc(1000);
  analyze();

  for (int i = 90; i < 100; i++) {
    myfree(a[i]);
  }
  o
  cb = clock();
  analyze();
  printf("\nduration: %f\n", (double)(cb - ca));
}
