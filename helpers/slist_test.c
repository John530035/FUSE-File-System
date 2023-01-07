
#include <stdio.h>

#include "slist.h"

void print_list(slist_t *list) {
  while (list != NULL) {
    printf("%s\n", list->data);
    list = list->next;
  }
}

int main(int argc, char **argv) {
  slist_t *list1 =
      s_cons("This", s_cons("is", s_cons("a", s_cons("list", NULL))));

  printf("List 1:\n");
  print_list(list1);

  // Try splitting
  const char *str =
      "Each|of|these|words|should|be|a|separate|entry|except these three";

  printf("\nExploding \"%s\":\n", str);
  slist_t *list2 = s_explode(str, '|');

  print_list(list2);

  s_free(list1);
  s_free(list2);
  return 0;
}
