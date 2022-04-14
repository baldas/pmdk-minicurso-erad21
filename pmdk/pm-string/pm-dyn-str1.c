#include <stdio.h>
#include <string.h>
#include <libpmemobj.h>                                                              

/*
 Create the pool for this app with:
 pmempool create obj --layout=strdynamic <pool-name>
*/

#define LAYOUT_NAME "strdynamic"
#define MAX_BUF_LEN 100

POBJ_LAYOUT_BEGIN(string_dynamic);
POBJ_LAYOUT_ROOT(string_dynamic, struct my_root);
POBJ_LAYOUT_TOID(string_dynamic, char);
POBJ_LAYOUT_END(string_dynamic);


struct my_root {
  TOID(char) my_string;
};


/*
 * Auxiliary functions to allocate/print the string.
 * */
static int string_constructor(PMEMobjpool *pop, void *ptr, void *arg)
{
  int size = strlen(arg)+1;

  memcpy(ptr, arg, size);
  pmemobj_persist(pop, ptr, size);

  return 0;
}

static int string_new(PMEMobjpool *pop, TOID(char) *str, char *buf) 
{
  return POBJ_ALLOC(pop, str, char, strlen(buf)+1, string_constructor, buf);
}


static void string_print(const TOID(char) str)
{
  printf("%s\n", D_RO(str));
}


  
int main(int argc, char *argv[])
{
  if (argc != 2) {
    printf("usage: %s file-name\n", argv[0]);
    return 1;
  }

  PMEMobjpool *pop = pmemobj_open(argv[1], LAYOUT_NAME);
  if (pop == NULL) {
    perror("pmemobj_open");
    return 1;
  }

  TOID(struct my_root) root = POBJ_ROOT(pop, struct my_root);
  
  if (!TOID_IS_NULL(D_RO(root)->my_string)) {
    printf("Last string: "); string_print(D_RO(root)->my_string);

    POBJ_FREE(&D_RW(root)->my_string);
  }

  char buf[MAX_BUF_LEN] = {0};
  if (scanf("%99[0-9a-zA-Z ]", buf) == EOF) {
    fprintf(stderr, "EOF\n");
    return 1;
  }


  if (string_new(pop, &D_RW(root)->my_string, buf))
    perror("pmemobj_alloc");



  pmemobj_close(pop);

  return 0;
}
