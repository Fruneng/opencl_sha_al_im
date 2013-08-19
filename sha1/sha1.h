#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define SHA1_FORMAT_LABEL			"sha1-opencl"
#define SHA1_FORMAT_NAME				"SHA-1"
#define SHA1_ALGORITHM_NAME			"sha1-opencl develop only"

#define SHA1_PLAINTEXT_LENGTH		64

#define SHA1_BINARY_SIZE			20
#define SHA1_RESULT_SIZE			5

#ifndef uint32_t
#define uint32_t unsigned int
#endif

#define MAX_SOURCE_SIZE 0x10000000


#ifdef __cplusplus
extern "C"
{
#endif

void sha1_init(size_t);
void sha1_crypt(char*, char*);

#ifdef __cplusplus
}
#endif
