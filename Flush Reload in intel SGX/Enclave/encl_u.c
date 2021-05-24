#include "encl_u.h"
#include <errno.h>

typedef struct ms_ecall_secret_lookup_t {
	char* ms_array;
	int ms_len;
	int ms_nn;
} ms_ecall_secret_lookup_t;

static const struct {
	size_t nr_ocall;
	void * table[1];
} ocall_table_encl = {
	0,
	{ NULL },
};
sgx_status_t ecall_secret_lookup(sgx_enclave_id_t eid, char* array, int len, int nn)
{
	sgx_status_t status;
	ms_ecall_secret_lookup_t ms;
	ms.ms_array = array;
	ms.ms_len = len;
	ms.ms_nn = nn;
	status = sgx_ecall(eid, 0, &ocall_table_encl, &ms);
	return status;
}

