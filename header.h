#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <errno.h>
#include <openssl/md5.h>
#include <fcntl.h>

#define PERMS 0600

#define P1_ENC1_SHMKEY 1111
#define CHAN_ENC1_SHMKEY 2222
#define CHAN_ENC2_SHMKEY 3333
#define P2_ENC2_SHMKEY 4444

#define P1_SEM_FNAME "/p1"
#define ENC1_P1_SEM_FNAME "/enc1p1"
#define ENC1_CHAN_SEM_FNAME "/enc1chan"
#define CHAN_ENC1_SEM_FNAME "/chanenc1"
#define CHAN_ENC2_SEM_FNAME "/chanenc2"
#define	ENC2_CHAN_SEM_FNAME "/enc2chan"
#define ENC2_P2_SEM_FNAME "/enc2p2"
#define P2_SEM_FNAME "/p2"

typedef struct{
	char message[300];
	char checksum[MD5_DIGEST_LENGTH];
	int  corruption_status;
} memory_data;
