/*--------------------------------------------------------------------------
 * LuaSec 0.6a
 * Copyright (C) 2006-2015 Bruno Silvestre
 *
 *--------------------------------------------------------------------------*/

#ifndef LSEC_EC_H
#define LSEC_EC_H

#include <openssl/objects.h>

typedef struct t_ec_ {
  char *name;
  int nid;
} t_ec;
typedef t_ec* p_ec;

/* Elliptic curves supported */
static t_ec curves[] = {
  /* SECG */
  {"secp112r1", NID_secp112r1},
  {"secp112r2", NID_secp112r2},
  {"secp128r1", NID_secp128r1},
  {"secp128r2", NID_secp128r2},
  {"secp160k1", NID_secp160k1},
  {"secp160r1", NID_secp160r1},
  {"secp160r2", NID_secp160r2},
  {"secp192k1", NID_secp192k1},
  {"secp224k1", NID_secp224k1},
  {"secp224r1", NID_secp224r1},
  {"secp256k1", NID_secp256k1},
  {"secp384r1", NID_secp384r1},
  {"secp521r1", NID_secp521r1},
  {"sect113r1", NID_sect113r1},
  {"sect113r2", NID_sect113r2},
  {"sect131r1", NID_sect131r1},
  {"sect131r2", NID_sect131r2},
  {"sect163k1", NID_sect163k1},
  {"sect163r1", NID_sect163r1},
  {"sect163r2", NID_sect163r2},
  {"sect193r1", NID_sect193r1},
  {"sect193r2", NID_sect193r2},
  {"sect233k1", NID_sect233k1},
  {"sect233r1", NID_sect233r1},
  {"sect239k1", NID_sect239k1},
  {"sect283k1", NID_sect283k1},
  {"sect283r1", NID_sect283r1},
  {"sect409k1", NID_sect409k1},
  {"sect409r1", NID_sect409r1},
  {"sect571k1", NID_sect571k1},
  {"sect571r1", NID_sect571r1},
  /* ANSI X9.62 */
  {"prime192v1", NID_X9_62_prime192v1},
  {"prime192v2", NID_X9_62_prime192v2},
  {"prime192v3", NID_X9_62_prime192v3},
  {"prime239v1", NID_X9_62_prime239v1},
  {"prime239v2", NID_X9_62_prime239v2},
  {"prime239v3", NID_X9_62_prime239v3},
  {"prime256v1", NID_X9_62_prime256v1},
  /* End */
  {NULL,        0U}
};

#endif
