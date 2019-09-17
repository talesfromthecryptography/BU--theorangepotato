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

  printf("Shifting left in place by 1...\n");
  bu_readhex(&d, "7FFF");
  bu_shl_ip(&d, 1);
  bu_dbg_printf(&d);

  printf("Shifting left in place by 32...\n");
  bu_readhex(&d, "7FFF");
  bu_shl_ip(&d, 32);
  bu_dbg_printf(&d);

  printf("Shifting left in place by 33...\n");
  bu_readhex(&d, "7FFF");
  bu_shl_ip(&d, 33);
  bu_dbg_printf(&d);

  bu_readhex(&c, "7FFF");
  printf("Shifting left by 1...\n");
  bu_shl(&d, &c, 1);
  bu_dbg_printf(&d);

  printf("Shifting left by 32...\n");
  bu_shl(&d, &c, 32);
  bu_dbg_printf(&d);

  printf("Shifting left by 33...\n");
  bu_shl(&d, &c, 33);
  bu_dbg_printf(&d);

  printf("Shifting right in place by 1...\n");
  bu_readhex(&d, "0000FFFE 00000000");
  bu_shr_ip(&d, 1);
  bu_dbg_printf(&d);

  printf("Shifting right in place by 32...\n");
  bu_readhex(&d, "0000FFFE 00000000");
  bu_shr_ip(&d, 32);
  bu_dbg_printf(&d);

  printf("Shifting right in place by 33...\n");
  bu_readhex(&d, "0000FFFE 00000000");
  bu_shr_ip(&d, 33);
  bu_dbg_printf(&d);

  bu_readhex(&c, "0000FFFE 00000000");
  printf("Shifting right by 1...\n");
  bu_shr(&d, &c, 1);
  bu_dbg_printf(&d);

  printf("Shifting right by 32...\n");
  bu_shr(&d, &c, 32);
  bu_dbg_printf(&d);

  printf("Shifting right by 33...\n");
  bu_shr(&d, &c, 33);
  bu_dbg_printf(&d);

  printf("Multiplying by digit (2)...\n");
  bu_readhex(&a, "FFFFE001 DEADBEEF");
  bu_mul_digit(&b, &a, 2);
  bu_dbg_printf(&b);

  printf("Multiplying by digit (2) in place...\n");
  bu_readhex(&a, "FFFFE001 DEADBEEF");
  bu_mul_digit_ip(&a, 2);
  bu_dbg_printf(&a);

  return 0;
}