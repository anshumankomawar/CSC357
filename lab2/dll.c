#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct dll {
  struct dll *next, *prev;
  char text[1000];
  

} dll;

dll *head;

dll *getLastElement() {
  dll *curr = head;

  while(curr != NULL && curr->next != NULL) {
    curr = curr->next;
  }

  return curr;
}

void pushString() {
  char str[1000];
  printf("Enter new string: ");
  scanf("%s", str);
  printf("You entered: %s\n", str);

  dll *lastNode = getLastElement();
  
  if(lastNode) {
    dll *node = (dll*) malloc(sizeof(dll));
    strcpy(node->text, str);
    node->prev = lastNode;
    node->next = NULL;
    lastNode->next = node;
  } else {
    head = (dll*) malloc(sizeof(dll));
    strcpy(head->text, str);
    head->next = NULL;
    head->prev = NULL;
  }

  printf("\n");
}

void printString() {
  for(dll *curr = head; curr != NULL; curr = curr->next) {
    printf("%s\n", curr->text);
  }
}

void deleteItem() {
  int index;
  printf("Enter delete index: ");
  scanf("%d", &index);

  if(index == 1) {
    if(head != NULL) {
      head = head->next;
      return;
    }
  } else {
    dll *node = head;
    int curr = 1;
    while(node != NULL) {
      if(curr == index) {
        dll *after = node->next;
        dll *prev = node->prev;

        if(after != NULL) {
          after->prev = prev;
        } 
        prev->next = after;
        free(node);

      }
      curr++;
      node = node->next;
    }
  }
}

void clearString() {
  free(head);
  head = NULL;
}

int main() {
  head = NULL;
  int status, option;

  while(status) {
    printf("\nSelect an Option\n");
    printf("1) Push String\n");
    printf("2) Print String\n");
    printf("3) Delete Item\n");
    printf("4) End Program\n");

    printf(">>> ");
    scanf("%d", &option);

    switch(option) {
      case 1: pushString(); break;
      case 2: printString(); break;
      case 3: deleteItem(); break;
      case 4: clearString(); status = 0; break;
      default: printf("Please enter a valid option.\n");
    }
  }

  return 0;
}
