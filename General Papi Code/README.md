# This repository explain how to use PAPI tool

1. Papi tool is generally a hardware performance counter used to analyse the behaviour of cache, memory etc.

2. Direction to use :
a. g++ -o papi_general_code papi_general_code.c
b. ./papi_general_code

3. You can enter your events in PAPI_add_event() function.
4. Papi various events can be found here : http://icl.cs.utk.edu/projects/papi/presets.html