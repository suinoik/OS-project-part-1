shm_proc: shm_processes.c
	gcc shm_processes.c -D_SVID_SOURCE -pthread -std=c99 -lpthread  -o shm_proc
example: example.c
	gcc example.c -pthread -std=c99 -lpthread  -o example
shd_mem: shd_mem.c
	gcc shd_mem.c -D_SVID_SOURCE -pthread -std=c99 -lpthread  -o shd_mem
