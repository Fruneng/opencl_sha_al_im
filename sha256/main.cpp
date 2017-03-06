#include "sha256.h"


void crypt_and_print(char* input) 
{
  char result[65];
  sha256_crypt(input,result);
  printf("'%s':\n%s\n", input, result);
}

int main()
{
  char result[65];

  sha256_init(2048);

  crypt_and_print((char*)"1");
  crypt_and_print((char*)"12");
  crypt_and_print((char*)"123");
  crypt_and_print((char*)"1234567890123456789012345678901234567890123456789012345678901234567890");
}