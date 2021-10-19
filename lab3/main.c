#include <stdio.h>
#define PAGESIZE 1024

unsigned char myheap[1048576];
  typedef unsigned char BYTE;
  typedef struct chunkhead {
    unsigned int size;
    unsigned int info;
    unsigned char *next, *prev;
} chunkhead;

unsigned char *mymalloc(unsigned int size) {
  chunkhead *ch = (chunkhead *)myheap;

  int hi;
  unsigned int pg = size / PAGESIZE;
  if (size / PAGESIZE == 0) {
    size = PAGESIZE;
  } else {
    size = ((int)pg + 1) * PAGESIZE;
  }
  if (ch->info == 0 && ch->size > size) {
    ch->info = 1;
    ch->prev = 0;
    BYTE *p = (BYTE *)ch + size + sizeof(chunkhead);
    chunkhead *nch = (chunkhead *)p;
    nch->info = 0;
    nch->prev = (BYTE *)ch;
    nch->size = (ch->size) - size - (sizeof(chunkhead));
    ch->size = size;
    ch->next = (BYTE *)nch;
    return (BYTE *)ch + (sizeof(chunkhead));
  } else if (ch->info == 0 && ch->size == size) {
    ch->info = 1;
    return (BYTE *)ch + (sizeof(chunkhead));
  }
  chunkhead *pch = 0;
  for (; ch != 0; ch = (chunkhead *)(ch->next)) {
    if (ch->info == 1 || size > ch->size) {
      pch = ch;
      continue;
    }
    if (ch->info == 0 && ch->size > size) {
      BYTE *p = (BYTE *)ch + size + sizeof(chunkhead);
      chunkhead *nch = (chunkhead *)p;
      nch->info = 0;
      nch->size = (ch->size) - size - (sizeof(chunkhead));
      nch->prev = (BYTE *)ch;
      ch->info = 1;
      ch->size = size;
      ch->next = (BYTE *)nch;
      ch->prev = (BYTE *)pch;
      return (BYTE *)ch + sizeof(chunkhead);
    } else if (ch->info == 0 && ch->size == size) {
      ch->info = 1;
      return (BYTE *)ch + (sizeof(chunkhead));
    }
    return 0;
  }
  return 0;
}

void myfree(unsigned char *address) {
  chunkhead *ch = (chunkhead *)(address - sizeof(chunkhead));
  ch->info = 0;
  chunkhead *chn = ((chunkhead *)(ch->next));
  chunkhead *chp = ((chunkhead *)(ch->prev));
  if (((chunkhead *)(ch->prev)) == 0 && ((chunkhead *)(ch->next))->info == 0) {
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
    ch->size += chp->size + sizeof(chunkhead);
    if (((chunkhead *)(ch->prev)) != 0) {
      ((chunkhead *)(ch->prev))->next = (BYTE *)chp;
    }
    if (((chunkhead *)(ch->next)) != 0) {
      ((chunkhead *)(ch->next))->prev = (BYTE *)chp;
    }
    ch->prev = chp->prev;
    chp->size = ch->size;
    chp->next = ch->next;
    chp->prev = ch->prev;
  }
}
void analyze() {
  chunkhead *ch = (chunkhead *)myheap;
  int count = 1;

  while (ch != 0) {
    printf("\nChunk #%d\n", count);
    printf("Size = %d bytes\n", ch->size);
    if (ch->info == 1) {
      printf("Occupied\n");
    } else {
      printf("Free\n");
    }
    printf("Next = %p\n", (ch->next));
    printf("Prev = %p\n", (ch->prev));
    count++;
    ch = (chunkhead *)(ch->next);
  }
}

int main() {
  unsigned char *a, *b, *c, *d, *e;
  chunkhead *ch = (chunkhead *)myheap;
  ch->size = 1048576 - sizeof(chunkhead);
  ch->info = 0;
  ch->prev = 0;
  ch->next = 0;
  a = mymalloc(1000);
  b = mymalloc(1000);
  c = mymalloc(1000);
  myfree(b);
  myfree(c);
  analyze();
  return 0;
}
