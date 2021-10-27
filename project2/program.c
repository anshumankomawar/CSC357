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

typedef unsigned char BYTE;
typedef struct chunkhead {
  unsigned int size;
  unsigned int info;
  unsigned char *next, *prev;
} chunkhead;

int heapsize = 0;
chunkhead *heap = NULL;

unsigned char *mymalloc(unsigned int size) {
  unsigned int smallest = 0;
  BYTE *p = 0, *s = sbrk(0);
  chunkhead *chc = 0, *ch = heap, *pch = 0;
  unsigned int pg = (size + sizeof(chunkhead)) / PAGESIZE;
  size = pg == 0 ? PAGESIZE : ((int)pg + 1) * PAGESIZE;

  if (!heapsize) {
    heap = (chunkhead *)sbrk(0);
    sbrk(size);

    ch = heap;
    ch->size = size;
    ch->info = 1;
    ch->prev = 0;
    ch->next = 0;

    heapsize = size;

    return (BYTE *)ch + (sizeof(chunkhead));
  } else if (ch->size == size && ch->info == 0) {
    ch->info = 1;
    return (BYTE *)ch + (sizeof(chunkhead));
  }


  while(ch->next != 0) {
    if (ch->info == 1 || size > ch->size) {
      pch = ch;
      ch = (chunkhead *)(ch->next);
      continue;
    }
    if (ch->info == 0 && ch->size == size) {
      ch->info = 1;
      return (BYTE *)ch + (sizeof(chunkhead));
    }
    if ((ch->info == 0 && ch->size > size) && (smallest == 0 || smallest > ch->size)) {
        chc = (chunkhead *)((BYTE *)ch + (ch->size) + sizeof(chunkhead) -
                            (size + sizeof(chunkhead)));
        p = (BYTE *)ch;
        smallest = ch->size;
    }
    ch = (chunkhead *)(ch->next);
  }

  if(!smallest) {
    sbrk(size);
    heapsize += size;
    heapsize += sizeof(chunkhead);

    pch = ch;

    ch = (chunkhead *)((BYTE *)ch + ch->size);
    ch->size = size;
    ch->info = 1;
    ch->next = 0;
    ch->prev = (BYTE *)pch;

    pch->next = (BYTE *)ch;

    return (BYTE *)ch + sizeof(chunkhead);
  } else {
    ch = (chunkhead *)p;
    pch = (chunkhead *)(ch->next);

    chc->info = 1;
    ch->info = 0;

    ch->size -= (size) + sizeof(chunkhead);
    chc->size = size;

    chc->prev = (BYTE *)ch;

    ch->next = (BYTE *)chc;
    chc->next = (BYTE *)pch;
    pch->prev = (BYTE *)chc;
    return (BYTE *)ch + sizeof(chunkhead);
  }
}

void myfree(unsigned char *address) {
  chunkhead *soh = heap;
  int hs = heapsize;
  chunkhead *ch = (chunkhead *)(address - sizeof(chunkhead));
  ch->info = 0;
  chunkhead *next = ((chunkhead *)(ch->next));
  chunkhead *prev = ((chunkhead *)(ch->prev));

  if (!next) {
    BYTE *p = sbrk(0);
    heapsize = heapsize - ch->size;
    brk((BYTE *)p - (ch->size - sizeof(chunkhead)));

    if (!prev->info) {
      if (!prev->prev) {
        brk(heap);
        heap = 0;
        heapsize = 0;
      } else {
        prev = (chunkhead *)(prev->prev);
        prev->next = 0;
        brk(sbrk(0) - prev->size);
      }
    } else {
      prev->next = ch->next;
    }

    if (!heapsize || !prev) {
      brk(heap);
      heap = 0;
      heapsize = 0;
    }

  } else if (!((chunkhead *)(ch->prev)) && !((chunkhead *)(ch->next))->info) {
    ch->size = sizeof(chunkhead) + ch->size + next->size ;
    ch->next = next->next; ch->prev = 0;

    if (((chunkhead *)(ch->next))) ((chunkhead *)(ch->next))->prev = (BYTE *)ch;
  } else if (((chunkhead *)(ch->prev)) == 0 && ((chunkhead *)(ch->next))->info != 0) {
    ch->info = 0;
  } else if (((chunkhead *)(ch->next))->info == 0 && ((chunkhead *)(ch->prev))->info == 0) {
    ch->size += (next->size);
    ch->size += (prev->size);
    ch->size += sizeof(chunkhead) * 2;
    
    ((chunkhead *)(next->next))->prev = ((chunkhead *)(next->next)) ? (BYTE *)ch : ((chunkhead *)(next->next))->prev;
    ((chunkhead *)(prev->prev))->next = ((chunkhead *)(prev->prev)) ? (BYTE *)ch : ((chunkhead *)(prev->prev))->next;

    ch->next = (next->next);
    ch->prev = (prev->prev);
    prev->size = ch->size;
    prev->next = ch->next;
    prev->prev = ch->prev;

  } else if (!next->info) {
    ch->size += next->size;
    ch->size += sizeof(chunkhead);
    ch->next = next->next;

    if (((chunkhead *)(ch->next)) != 0) ((chunkhead *)(ch->next))->prev = (BYTE *)ch; 
  } else if (!prev->info) {
    prev->size += ch->size;
    if (((chunkhead *)(prev->prev)) != 0) ((chunkhead *)(prev->prev))->next = (BYTE *)prev;
    if (((chunkhead *)(prev->next)) != 0) ((chunkhead *)(prev->next))->prev = (BYTE *)prev;

    prev->next = ch->next;
    next->prev = (BYTE *)prev;
  }
}


void analyze() {
  printf("\n--------------------------------------------------------------\n");
  if (!heap) {
    printf("no heap");
printf("program break on address: %x\n", sbrk(0));
    return;
  }
  chunkhead *ch = (chunkhead *)heap;
  for (int no = 0; ch; ch = (chunkhead *)ch->next, no++) {
    printf("%d | current addr: %x |", no, ch);
    printf("size: %d | ", ch->size);
    printf("info: %d | ", ch->info);
    printf("next: %x | ", ch->next);
    printf("prev: %x", ch->prev);
    printf("      \n");
  }
  printf("program break on address: %x\n", sbrk(0));
}

chunkhead *get_last_chunk() {
  if (!heap) {
    return NULL;
  }

  chunkhead *ch = (chunkhead *)heap;
  for (; ch->next; ch = (chunkhead *)ch->next)
    ;
  return ch;
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

  cb = clock();

  analyze();
  printf("\nduration: %f\n", (double)(cb - ca));
}
