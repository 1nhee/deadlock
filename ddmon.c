#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <sys/syscall.h>

// gcc -shared -fPIC -o ddmon.so ddmon.c -ldl
//LD_PRELOAD=./ddmon.so ./dinning_deadlock

//int (*pthread_create_new)(pthread_t *, const pthread_attr_t *, void *(*) (void *), void *);
//int (*pthread_mutex_init_new)(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrictattr);
int (*pthread_mutex_lock_new)(pthread_mutex_t *mutex);
int (*pthread_mutex_unlock_new)(pthread_mutex_t *mutex);

typedef struct send_element{
	int id;
	int mutex_owner;
	pthread_mutex_t *mutex;
	//check it is lock-1 or unlock-0 
	int lock;
}send;
//id_mutex_lock

int pthread_mutex_lock(pthread_mutex_t *mutex){
	send *new =  malloc(sizeof(send));
	char address[256], id[256], s[256], owner[256];
    pthread_mutex_lock_new = dlsym(RTLD_NEXT, "pthread_mutex_lock") ;

    new->id = syscall(SYS_gettid);
	new->mutex = mutex;
	new->mutex_owner = mutex->__data.__owner;
	sprintf(id, "%d", new->id);
	sprintf(owner, "%d", new->mutex_owner);
	sprintf(address, "%p", mutex);
	new->lock = 1;

	strcat(s, id);
	strcat(s, " ");
	strcat(s, owner);
	strcat(s, " ");
	strcat(s, address);
	strcat(s, " ");
	strcat(s, " 1");
	strcat(s, "\n");

	//printf("%s\n", s);
	//printf("lock id: %d mutex: %s lock: %d\n", new->id, address, new->lock);
	
	int fd = open(".ddtrace", O_WRONLY | O_SYNC) ;
	for (int i = 0 ; i < 256 ; ) {
			i += write(fd, s + i, 256) ;
		} 

	free(new);
	close(fd);
	

    return pthread_mutex_lock_new(mutex);
}

int pthread_mutex_unlock(pthread_mutex_t *mutex){
	send *new = malloc(sizeof(send));
	char address[256], id[256], s[256], owner[256];

    pthread_mutex_unlock_new = dlsym(RTLD_NEXT, "pthread_mutex_unlock") ;

    new->id = syscall(SYS_gettid);
	new->mutex = mutex;
	new->mutex_owner = mutex->__data.__owner;
	sprintf(id, "%d", new->id);
	sprintf(owner, "%d", new->mutex_owner);
	sprintf(address, "%p", mutex);
	new->lock = 0;

	strcat(s, id);
	strcat(s, " ");
	strcat(s, owner);
	strcat(s, " ");
	strcat(s, address);
	strcat(s, " ");
	strcat(s, " 0");
	strcat(s, "\n");

	//printf("%s\n", s);
	//printf("unlock id: %d mutex: %s lock: %d\n", new->id, address, new->lock);

	int fd = open(".ddtrace", O_WRONLY | O_SYNC) ;
	for (int i = 0 ; i < 256 ; ) {
			i += write(fd, s + i, 256) ;
		} 
	
	free(new);
	close(fd);

    return pthread_mutex_unlock_new(mutex);
}