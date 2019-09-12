#include <stdio.h>
#include <string.h>

#include "bu.h"


int main() {
  bigunsigned a,b,c,d;

  printf("Reading in string...\n");
  bu_readhex(&a,"CAB5 1AFF BEEF");
  bu_readhex(&b,"111111111111");
  bu_dbg_printf(&a);

  printf("Adding bigunsigneds...\n");
  bu_add(&c, &a, &b);
  bu_dbg_printf(&c);

  printf("Copying bigunsigned...\n");
  bu_cpy(&d, &a);
  bu_dbg_printf(&d);

  printf("Clearing bigunsigned...\n");
  bu_clear(&d);
  bu_dbg_printf(&d);

  return 0;
}