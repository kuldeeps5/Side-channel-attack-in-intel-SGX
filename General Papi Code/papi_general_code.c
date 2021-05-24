#include <stdio.h>
#include <stdlib.h>
#include <papi.h>

void foo() {
  int c = 0;
  for(int i= 0; i < 1e7; ++i) {
    c = (c+i)%10000;
  }
  printf("Foo's output is %d\n",c);
}

void handle_error(int line) {
  printf("Something wrong happened on line %d\n", line);
  exit(1);
}

int main() {

  int event_set = PAPI_NULL;
  long long values[2] = {0,0};
  int ret;

  ret = PAPI_library_init(PAPI_VER_CURRENT);
  if(ret != PAPI_VER_CURRENT) handle_error(__LINE__);

  ret = PAPI_create_eventset(&event_set);
  if(ret != PAPI_OK) handle_error(__LINE__);

  ret = PAPI_add_event(event_set, PAPI_L1_DCM);
  if(ret != PAPI_OK)  handle_error(__LINE__);

  ret = PAPI_add_event(event_set, PAPI_L1_ICM);
  if(ret != PAPI_OK)  handle_error(__LINE__);

  ret = PAPI_start(event_set);
  if(ret != PAPI_OK)  handle_error(__LINE__);

  foo();

  ret = PAPI_stop(event_set, values);
  if(ret != PAPI_OK) handle_error(__LINE__);

  printf("%lld -- %lld\n", values[0], values[1]);
}

