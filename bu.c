#include <string.h> // for memset, etc.
#include <stdio.h>  // for printf

#include "bu.h"

// NOTE: In this code "word" always refers to a uint32_t

// Copy dest = src
void bu_cpy(bigunsigned *dest, bigunsigned *src) {
  uint16_t cnt = src->used;
  dest->used = cnt;
  dest->base = 0;

  // reset upper 0s in dest
  memset(dest->digit + cnt, 0, sizeof(uint32_t)*(BU_DIGITS-cnt));

  uint8_t i_dest = dest->base;
  uint8_t i_src = src->base;

  while (cnt-- > 0) {
    dest->digit[i_dest++] = src->digit[i_src++];
  }
}

// Set to 0
void bu_clear(bigunsigned *a_ptr) {
  memset(a_ptr->digit, 0, sizeof(uint32_t)*BU_DIGITS);
  a_ptr->used = 0;
  a_ptr->base = 0;
}

//  a = b<<cnt
void bu_shl(bigunsigned* a_ptr, bigunsigned* b_ptr, uint16_t cnt) {
  bu_clear(a_ptr);
  if (b_ptr->used == 0) return;
  uint16_t wrds = cnt >> 5; // # of whole words to shift
  uint16_t bits = cnt &0x1f;// number of bits in a word to shift
  a_ptr->used = b_ptr->used;

  uint32_t mask = ~ (0xffffffff >> bits);
  uint8_t pos_a = a_ptr->used;
  uint8_t pos_b = b_ptr->base + b_ptr->used;

  if ((b_ptr->digit[pos_b - 1] & mask) > 0) a_ptr->used++;

  while (pos_a--) {
    pos_b--;
    a_ptr->digit[pos_a + 1] ^= (b_ptr->digit[pos_b] & mask) >> (32 - bits);
    a_ptr->digit[pos_a] = b_ptr->digit[pos_b] << bits;
  }

  a_ptr->base -= wrds;
  a_ptr->used += wrds;
}

// Shift in place a bigunsigned by cnt bits to the left
// Example: beef shifted by 4 results in beef0
void bu_shl_ip(bigunsigned* a_ptr, uint16_t cnt) {
  if (a_ptr->used == 0) return;
  uint16_t wrds = cnt >> 5; // # of whole words to shift
  uint16_t bits = cnt &0x1f;// number of bits in a word to shift

  if (bits) {
    uint32_t mask = ~ (0xffffffff >> bits);

    uint8_t pos = a_ptr->base + a_ptr->used;

    if ((a_ptr->digit[pos - 1] & mask) > 0) a_ptr->used++;

    while (pos-- != a_ptr->base) {
      a_ptr->digit[pos + 1] ^= (a_ptr->digit[pos] & mask) >> (32 - bits);
      a_ptr->digit[pos] <<= bits;
    }
  }

  a_ptr->base -= wrds;
  a_ptr->used += wrds;
}

//  a = b>>cnt
void bu_shr(bigunsigned* a_ptr, bigunsigned* b_ptr, uint16_t cnt) {
  bu_clear(a_ptr);
  uint16_t wrds = cnt >> 5; // # of whole words to shift
  uint16_t bits = cnt &0x1f;// number of bits in a word to shift

  if (wrds >= b_ptr->used) {
    return;
  }

  a_ptr->used = b_ptr->used - wrds;

    uint32_t mask = ~ (0xffffffff << bits);

    uint8_t pos_a = 0;
    uint8_t pos_b = b_ptr->base;

    while (pos_a != a_ptr->used) {
      a_ptr->digit[pos_a] = b_ptr->digit[pos_b + wrds] >> bits;
      a_ptr->digit[pos_a] ^= (b_ptr->digit[pos_b + wrds + 1] & mask) << (32 - bits);
      pos_a++;
      pos_b++;
    }

  if (!a_ptr->digit[(a_ptr->base + a_ptr->used - 1) % BU_DIGITS]) a_ptr->used--;
}

//  a >>= cnt
// Shift in place a big unsigned by cnt bits to the left
// Example: beef shifted by 4 results in beef0
void bu_shr_ip(bigunsigned* a_ptr, uint16_t cnt) {
  uint16_t wrds = cnt >> 5; // # of whole words to shift
  uint16_t bits = cnt &0x1f;// number of bits in a word to shift

  if (wrds >= a_ptr->used) {
    bu_clear(a_ptr);
    return;
  }

  if (a_ptr->base + wrds > BU_DIGITS) {
    uint16_t wrds_till_end = BU_DIGITS - a_ptr->base;
    memset(a_ptr->digit + a_ptr->base, 0, sizeof(uint32_t)*wrds_till_end);
    memset(a_ptr->digit, 0, sizeof(uint32_t)*(wrds - wrds_till_end));
  } else {
    memset(a_ptr->digit + a_ptr->base, 0, sizeof(uint32_t)*wrds);
  }
  a_ptr->base += wrds;
  a_ptr->used -= wrds;

  if (bits) {
    uint32_t mask = ~ (0xffffffff << bits);

    uint8_t pos = a_ptr->base;

    while (pos != a_ptr->base + a_ptr->used) {
      a_ptr->digit[pos] >>= bits;
      a_ptr->digit[pos] ^= (a_ptr->digit[pos + 1] & mask) << (32 - bits);
      pos++;
    }
  }

  if (!a_ptr->digit[(a_ptr->base + a_ptr->used - 1) % BU_DIGITS]) a_ptr->used--;
}

// Produce a = b + c
void bu_add(bigunsigned *a_ptr, bigunsigned *b_ptr, bigunsigned *c_ptr) {
  uint8_t carry = 0;
  uint64_t nxt;
  uint16_t cnt = 0;
  uint16_t min_used = b_ptr->used <= c_ptr->used 
                      ? b_ptr->used : c_ptr->used;
  uint8_t  b_dig = b_ptr->base;
  uint8_t  c_dig = c_ptr->base;
  uint8_t  a_dig = 0;

  while (cnt < min_used) {
    nxt = ((uint64_t)b_ptr->digit[b_dig++]) 
          + (uint64_t)(c_ptr->digit[c_dig++]) + carry;
    carry = 0 != (nxt&0x100000000);
    a_ptr->digit[a_dig++] = (uint32_t)nxt;
    cnt++;
  }

  while (cnt < b_ptr->used && carry) {
    nxt = ((uint64_t)b_ptr->digit[b_dig++]) + carry;
    carry = 0 != (nxt&0x100000000);
    a_ptr->digit[a_dig++] = (uint32_t)nxt;
    cnt++;
  }

  while (cnt < b_ptr->used) {
    a_ptr->digit[a_dig++] = b_ptr->digit[b_dig++];
    cnt++;
  }  

  while (cnt < c_ptr->used && carry) {
    nxt = ((uint64_t)c_ptr->digit[c_dig++]) + carry;
    carry = 0 != (nxt&0x100000000);
    a_ptr->digit[a_dig++] = (uint32_t)nxt;
    cnt++;
  }

  while (cnt < c_ptr->used) {
    a_ptr->digit[a_dig++] = c_ptr->digit[c_dig++];
    cnt++;
  }

  while (cnt < BU_DIGITS && carry) {
    a_ptr->digit[a_dig++] = 1;
    carry = 0;
    cnt++;
  }

  a_ptr->base = 0;
  a_ptr->used = cnt;
}

// a = b*d
void bu_mul_digit(bigunsigned *a_ptr, bigunsigned *b_ptr, uint32_t d) {
  uint8_t pos = 0;
  uint32_t res_lo, res_hi;
  uint64_t res, d_64 = (uint64_t)d;
  bigunsigned carries, sum;
  bu_clear(&carries);
  bu_clear(&sum);
  while (pos != (uint8_t)(b_ptr->base + b_ptr->used)) {
    res = (uint64_t)b_ptr->digit[pos + b_ptr->base] * d_64;
    sum.digit[pos]= (uint32_t)res;
    carries.digit[pos] = (uint32_t)(res >> 32);
    pos++;
  }
  sum.used = carries.used = pos;

  bu_shl_ip(&carries, 32);
  bu_add(a_ptr, &sum, &carries);
}

// a *= d
void bu_mul_digit_ip(bigunsigned *a_ptr, uint32_t d) {
  uint8_t pos = 0;
  uint32_t res_lo, res_hi;
  uint64_t res, d_64 = (uint64_t)d;
  bigunsigned carries, sum;
  bu_clear(&carries);
  bu_clear(&sum);
  while (pos != (uint8_t)(a_ptr->base + a_ptr->used)) {
    res = (uint64_t)a_ptr->digit[pos + a_ptr->base] * d_64;
    sum.digit[pos]= (uint32_t)res;
    carries.digit[pos] = (uint32_t)(res >> 32);
    pos++;
  }
  sum.used = carries.used = pos;

  bu_shl_ip(&carries, 32);
  bu_add(a_ptr, &sum, &carries);
}

// a = b*c
void bu_mul(bigunsigned *a_ptr, bigunsigned *b_ptr, bigunsigned *c_ptr) {
  bigunsigned shorter, longer;
  if (b_ptr->used <= c_ptr->used) {
    bu_cpy(&shorter, b_ptr);
    bu_cpy(&longer, c_ptr);
  } else {
    bu_cpy(&shorter, c_ptr);
    bu_cpy(&longer, b_ptr);
  }

  bigunsigned carry, sum;
  bu_clear(&carry);
  bu_clear(&sum);
  while(shorter.used) {
    if(shorter.digit[shorter.base] & 0x1) {
      bu_carry_save_add(&carry, &sum, &longer, &carry, &sum);
    }
    bu_shr_ip(&shorter, 1);
    bu_shl_ip(&longer, 1);
  }

  bu_add(a_ptr, &carry, &sum);
}

// a *= b
void bu_mul_ip(bigunsigned *a_ptr, bigunsigned *b_ptr) {
  bu_mul(a_ptr, a_ptr, b_ptr);
}

// sum = x1 ^ x2 ^ x3
// carry = (x1 & x2) | (x1 & x3) | (x2 & x3)
// Performs a carry-save addition
void bu_carry_save_add(bigunsigned *carry, bigunsigned *sum, bigunsigned *x1, bigunsigned *x2, bigunsigned *x3) {
  uint8_t pos = 0;
  // uint32_t x1, x2, x3;
  while (pos < x1->used && pos < x2->used && pos < x3->used) {
    uint32_t x1_32 = x1->digit[x1->base + pos], x2_32 = x2->digit[x2->base + pos], x3_32 = x3->digit[x3->base + pos];
    sum->digit[sum->base + pos] = x1_32 ^ x2_32 ^ x3_32;
    carry->digit[carry->base + pos] = (x1_32 & x2_32) | (x1_32 & x3_32) | (x2_32 & x3_32);
    pos++;
  }
  while (pos < x1->used && pos < x2->used) {
    uint32_t x1_32 = x1->digit[x1->base + pos], x2_32 = x2->digit[x2->base + pos];
    sum->digit[sum->base + pos] = x1_32 ^ x2_32;
    carry->digit[carry->base + pos] = x1_32 & x2_32;
    pos++;
  }
  while (pos < x1->used && pos < x3->used) {
    uint32_t x1_32 = x1->digit[x1->base + pos], x3_32 = x3->digit[x3->base + pos];
    sum->digit[sum->base + pos] = x1_32 ^ x3_32;
    carry->digit[carry->base + pos] = x1_32 & x3_32;
    pos++;
  }
  while (pos < x2->used && pos < x3->used) {
    uint32_t x2_32 = x2->digit[x2->base + pos], x3_32 = x3->digit[x3->base + pos];
    sum->digit[sum->base + pos] = x2_32 ^ x3_32;
    carry->digit[carry->base + pos] = x2_32 & x3_32;
    pos++;
  }
  while (pos < x1->used) {
    sum->digit[sum->base + pos] = x1->digit[x1->base + pos];
    pos++;
  }
  while (pos < x2->used) {
    sum->digit[sum->base + pos] = x2->digit[x2->base + pos];
    pos++;
  }
  while (pos < x3->used) {
    sum->digit[sum->base + pos] = x3->digit[x3->base + pos];
    pos++;
  }
  carry->used = pos;
  sum->used = pos;
  bu_shl_ip(carry, 1);
}

// a = b^2
void bu_sqr(bigunsigned *a_ptr, bigunsigned *b_ptr) {
  bu_mul(a_ptr, b_ptr, b_ptr);
}

// Return the length in bits (should always be less or equal to 32*a->used)
uint16_t bu_len(bigunsigned *a_ptr) {
  if (a_ptr->used== 0) return 0;
  uint16_t res = a_ptr->used<<5;
  uint32_t bit_mask = 0x80000000;
  uint32_t last_wrd = a_ptr->digit[(a_ptr->base+a_ptr->used-1) % BU_DIGITS];

  while (bit_mask && !(last_wrd&bit_mask)) {
    bit_mask >>= 1;
    res--;
  }
  return res;
}

// Read from a string of hex digits
void bu_readhex(bigunsigned * a_ptr, char *s) {
  bu_clear(a_ptr);

  unsigned pos = 0;
  char *s_ptr = s + strlen(s) - 1;
  while (s_ptr >= s && pos < BU_MAX_HEX) {
    if(*s_ptr == ' ') {
      s_ptr--;
      continue;
    }
    a_ptr->digit[pos>>3] |= (((uint32_t)hex2bin(*s_ptr)) << ((pos & 0x7)<<2));
    pos++;
    s_ptr--;
  }
  a_ptr->used = (pos>>3) + ((pos&0x7)!=0);
}

// Print to stdout in hex
void bu_dbg_printf(bigunsigned *a_ptr) {
  printf("Used %x\n", a_ptr->used);
  printf("Base %x\n", a_ptr->base);
  uint16_t i = a_ptr->used;
  printf("Digits: ");
  while (i-- > 0)
    printf("%08x ", a_ptr->digit[(a_ptr->base+i) % BU_DIGITS]);
  printf("Length: %x\n", bu_len(a_ptr));
}
