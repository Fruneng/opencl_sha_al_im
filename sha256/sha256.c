#include "sha256.h"

static cl_platform_id platform_id = NULL;
static cl_device_id device_id = NULL;  
static cl_uint ret_num_devices;
static cl_uint ret_num_platforms;
static cl_context context;

static cl_int ret;

static char* source_str;
static size_t source_size;

static cl_program program;
static cl_kernel kernel;
static cl_command_queue command_queue;


static cl_mem pinned_saved_keys, pinned_partial_hashes, buffer_out, buffer_keys, data_info;
static cl_uint *partial_hashes;
static cl_uint *res_hashes;
static char *saved_plain;
static unsigned int datai[3];
static int have_full_hashes;

static size_t kpc = 4;

static size_t global_work_size=1;
static size_t local_work_size=1;
static size_t string_len;

void load_source();
void createDevice();
void createkernel();
void create_clobj();

void crypt_all();


void sha256_init(size_t user_kpc)
{
	kpc = user_kpc;
	load_source();
	createDevice();
	createkernel();
	create_clobj();
}

void sha256_crypt(char* input, char* output)
{
	int i;
	string_len = strlen(input);
	global_work_size = 1;
	datai[0] = SHA256_PLAINTEXT_LENGTH;
	datai[1] = global_work_size;
	datai[2] = string_len;
	memcpy(saved_plain, input, string_len+1);

	crypt_all();

	for(i=0; i<SHA256_RESULT_SIZE; i++)
	{
		sprintf(output+i*8,"%08x", partial_hashes[i]);

	}
}

void crypt_all()
{
	//printf("%s\n",saved_plain);
	ret = clEnqueueWriteBuffer(command_queue, data_info, CL_TRUE, 0, sizeof(unsigned int) * 3, datai, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, buffer_keys, CL_TRUE, 0, SHA256_PLAINTEXT_LENGTH * kpc, saved_plain, 0, NULL, NULL);
	// printf("%s\n",buffer_keys);
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_work_size, &local_work_size, 0, NULL, NULL);

	ret = clFinish(command_queue);
	// read back partial hashes
	ret = clEnqueueReadBuffer(command_queue, buffer_out, CL_TRUE, 0, sizeof(cl_uint) * SHA256_RESULT_SIZE, partial_hashes, 0, NULL, NULL);
	have_full_hashes = 0;
}

void load_source()
{
	FILE *fp;

	fp = fopen("sha256.cl", "r");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		exit(1);
	}
	source_str = (char*)malloc(MAX_SOURCE_SIZE);
	source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose( fp );
}

void create_clobj(){
	pinned_saved_keys = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, (SHA256_PLAINTEXT_LENGTH)*kpc, NULL, &ret);
	saved_plain = (char*)clEnqueueMapBuffer(command_queue, pinned_saved_keys, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, (SHA256_PLAINTEXT_LENGTH)*kpc, 0, NULL, NULL, &ret);
	memset(saved_plain, 0, SHA256_PLAINTEXT_LENGTH * kpc);
	res_hashes = (cl_uint *)malloc(sizeof(cl_uint) * SHA256_RESULT_SIZE);
	memset(res_hashes, 0, sizeof(cl_uint) * SHA256_RESULT_SIZE);
	pinned_partial_hashes = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, sizeof(cl_uint) * SHA256_RESULT_SIZE, NULL, &ret);
	partial_hashes = (cl_uint *) clEnqueueMapBuffer(command_queue, pinned_partial_hashes, CL_TRUE, CL_MAP_READ, 0, sizeof(cl_uint) * SHA256_RESULT_SIZE, 0, NULL, NULL, &ret);
	memset(partial_hashes, 0, sizeof(cl_uint) * SHA256_RESULT_SIZE);

	buffer_keys = clCreateBuffer(context, CL_MEM_READ_ONLY, (SHA256_PLAINTEXT_LENGTH) * kpc, NULL, &ret);
	buffer_out = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_uint) * SHA256_RESULT_SIZE, NULL, &ret);
	data_info = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(unsigned int) * 3, NULL, &ret);

	clSetKernelArg(kernel, 0, sizeof(data_info), (void *) &data_info);
	clSetKernelArg(kernel, 1, sizeof(buffer_keys), (void *) &buffer_keys);
	clSetKernelArg(kernel, 2, sizeof(buffer_out), (void *) &buffer_out);
}

void createDevice()
{
	ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_ALL, 1, &device_id, &ret_num_devices);

	context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);
}

void createkernel()
{
	program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	kernel = clCreateKernel(program, "sha256_crypt_kernel", &ret);
	command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
}
