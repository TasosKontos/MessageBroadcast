#include "header.h"

int main(){
	//CREATE SHARED MEMORY FOR P1_ENC1
	int p1_enc1_shmid;
	if((p1_enc1_shmid = shmget(P1_ENC1_SHMKEY, 300*sizeof(char), PERMS|IPC_CREAT)) == -1){
		perror("shmget()");
		exit(EXIT_FAILURE);
	}
	
	char* p1_enc1_shm;
	if((p1_enc1_shm = (char*)shmat(p1_enc1_shmid,0 ,0)) == (char*)-1){
		perror("shmat()");
		exit(EXIT_FAILURE);
	}
	
	//CONNECT TO P1 AND ENC1 COMMON SEMAPHORES
	sem_t *p1_sem = sem_open(P1_SEM_FNAME, 0);
	if(p1_sem == SEM_FAILED){
		perror("sem_open/p1");
		exit(EXIT_FAILURE);
	}
		
	sem_t * enc1_p1_sem = sem_open(ENC1_P1_SEM_FNAME, 0);
	if(enc1_p1_sem == SEM_FAILED){
		perror("sem_open/enc1p1");
		exit(EXIT_FAILURE);
	}
			
	//fork for enc1
	int pid = fork();
	if(pid<0){
		printf("Fork error! \n");
		exit(1);
	}
	else if(pid>0){
		//p1	
		char str[300];
		while(1){
			printf("User 1:\n	");
			fgets(str, 300, stdin);
			strcpy(p1_enc1_shm,str);
				
			sem_post(enc1_p1_sem);
			
			if(memcmp(str, "TERM", 4)==0){
				sem_close(p1_sem);
				break;
			}
		
			sem_wait(p1_sem);
			
			if(memcmp(p1_enc1_shm, "TERM", 4)==0){
				sem_close(p1_sem);
				
				if(shmdt(p1_enc1_shm)){
					perror("shmdt()");
					exit(EXIT_FAILURE);	
				}
				
				shmctl(p1_enc1_shmid, IPC_RMID, NULL);
				
				sem_unlink(P1_SEM_FNAME);
				sem_unlink(ENC1_P1_SEM_FNAME);
				sem_unlink(ENC1_CHAN_SEM_FNAME);
				sem_unlink(CHAN_ENC1_SEM_FNAME);
				sem_unlink(CHAN_ENC2_SEM_FNAME);
				sem_unlink(ENC2_CHAN_SEM_FNAME);
				sem_unlink(ENC2_P2_SEM_FNAME);
				sem_unlink(P2_SEM_FNAME);	
							
				break;			
			}
			
			printf("User 2:\n	%s\n", p1_enc1_shm);
		
		}
	}
	else{
		//enc1
		
		int chan_enc1_shmid;
		if((chan_enc1_shmid = shmget(CHAN_ENC1_SHMKEY, sizeof(memory_data), PERMS|IPC_CREAT)) == -1){
			perror("shmget()");
			exit(EXIT_FAILURE);
		}
	
		memory_data* chan_enc1_shm;
		if((chan_enc1_shm = (memory_data*)shmat(chan_enc1_shmid, 0, 0))== (memory_data*)-1){
			perror("shmat()");
			exit(EXIT_FAILURE);
		}
			
		//CONNECT TO ENC1 RELATED SEMAPHORES
		sem_t *enc1_chan_sem = sem_open(ENC1_CHAN_SEM_FNAME, 0);
		if(enc1_chan_sem == SEM_FAILED){
			perror("sem_open/enc1chan");
			exit(EXIT_FAILURE);
		}
		
		sem_t * chan_enc1_sem = sem_open(CHAN_ENC1_SEM_FNAME, 0);
		if(chan_enc1_sem == SEM_FAILED){
			perror("sem_open/chanenc1");
			exit(EXIT_FAILURE);
		}		
			
		char hash[MD5_DIGEST_LENGTH];
		char msg[300];	
		while(1){
			sem_wait(enc1_p1_sem);
			//create md5 hash
			strcpy(msg, p1_enc1_shm);
			MD5(msg, strlen(msg), hash);
			
			
			//pass it to enc1_chan_shm			
			strcpy(chan_enc1_shm->message, msg);
			memcpy(chan_enc1_shm->checksum, hash, MD5_DIGEST_LENGTH);
			chan_enc1_shm->corruption_status = 0;
			
			sem_post(enc1_chan_sem);
			
			if(memcmp(msg, "TERM", 4)==0){
				sem_close(enc1_p1_sem);
				sem_close(chan_enc1_sem);
				sem_close(enc1_chan_sem);
				
				if(shmdt(p1_enc1_shm)){
					perror("shmdt()");
					exit(EXIT_FAILURE);
				}
				
				shmctl(p1_enc1_shmid, IPC_RMID, NULL);
				break;
			}
			
			sem_wait(chan_enc1_sem);
			
			while(chan_enc1_shm->corruption_status == 1){
				strcpy(chan_enc1_shm->message, msg);
				memcpy(chan_enc1_shm->checksum, hash, MD5_DIGEST_LENGTH);
				chan_enc1_shm->corruption_status=0;
				
				sem_post(enc1_chan_sem);
				sem_wait(chan_enc1_sem);
			}
			
			strcpy(msg, chan_enc1_shm->message);
			MD5(msg, strlen(msg), hash);
			
			while(memcmp(hash, chan_enc1_shm->checksum, MD5_DIGEST_LENGTH)!=0){
				//if checksums are the different
				
				chan_enc1_shm->corruption_status = 1;
				sem_post(enc1_chan_sem);
				sem_wait(chan_enc1_sem);
				
				strcpy(msg, chan_enc1_shm->message);
				MD5(msg, strlen(msg), hash);				
			}
		
			//if checksums are the same
			strcpy(p1_enc1_shm, chan_enc1_shm->message);
			sem_post(p1_sem);
			
			if(memcmp(chan_enc1_shm->message, "TERM", 4)==0){
				sem_close(enc1_chan_sem);
				sem_close(chan_enc1_sem);
				sem_close(enc1_p1_sem);
				
				if(shmdt(chan_enc1_shm)){
					perror("shmdt()");
					exit(EXIT_FAILURE);
				}
				
				shmctl(chan_enc1_shmid, IPC_RMID, NULL);
				break;
			}	
					
		}
	}
}


