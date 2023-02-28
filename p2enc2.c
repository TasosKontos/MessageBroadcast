#include "header.h"

int main(){
	//CREATE SHARED MEM FOR P2_ENC2
	int p2_enc2_shmid;
	if((p2_enc2_shmid = shmget(P2_ENC2_SHMKEY, 300*sizeof(char), PERMS|IPC_CREAT)) == -1){
		perror("shmget()");
		exit(EXIT_FAILURE);
	}
	
	char* p2_enc2_shm;
	if((p2_enc2_shm = (char*)shmat(p2_enc2_shmid,0 ,0)) == (char*)-1){
		perror("shmat()");
		exit(EXIT_FAILURE);
	}
	
	//CONNECT TO P2 AND ENC2 COMMON SEMAPHORES
	sem_t *p2_sem = sem_open(P2_SEM_FNAME, 0);
	if(p2_sem == SEM_FAILED){
		perror("sem_open/p2");
		exit(EXIT_FAILURE);
	}
		
	sem_t * enc2_p2_sem = sem_open(ENC2_P2_SEM_FNAME, 0);
	if(enc2_p2_sem == SEM_FAILED){
		perror("sem_open/enc2p2");
		exit(EXIT_FAILURE);
	}
	
	//FORK FOR ENC2
	int pid = fork();
	if(pid<0){
		printf("Fork error!\n");
		exit(1);
	}
	else if(pid>0){
		//p2
		char str[300];
		while(1){
			sem_wait(p2_sem);
			
			if(memcmp(p2_enc2_shm, "TERM", 4)==0){
				sem_close(p2_sem);
				
				if(shmdt(p2_enc2_shm)){
					perror("shmdt()");
					exit(EXIT_FAILURE);	
				}
				
				shmctl(p2_enc2_shmid, IPC_RMID, NULL);
				
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
			
			printf("User 1:\n	%s\n", p2_enc2_shm);
			
			
			printf("User 2:\n	");
			fgets(str, 300, stdin);
			strcpy(p2_enc2_shm,str);
			
			sem_post(enc2_p2_sem);
			
			if(memcmp(str, "TERM", 4)==0){
				sem_close(p2_sem);
				break;
			}
		}
	}
	else{
		//enc2
		int chan_enc2_shmid;
		if((chan_enc2_shmid = shmget(CHAN_ENC2_SHMKEY, sizeof(memory_data), PERMS|IPC_CREAT)) == -1){
			perror("shmget()");
			exit(EXIT_FAILURE);
		}
		
		memory_data* chan_enc2_shm;
		if((chan_enc2_shm = (memory_data*)shmat(chan_enc2_shmid, 0, 0))== (memory_data*)-1){
			perror("shmat()");
			exit(EXIT_FAILURE);
		}
			
		//CONNECT TO ENC2 RELATED SEMAPHORES
		sem_t *enc2_chan_sem = sem_open(ENC2_CHAN_SEM_FNAME, 0);
		if(enc2_chan_sem == SEM_FAILED){
			perror("sem_open/enc2chan");
			exit(EXIT_FAILURE);
		}
		
		sem_t * chan_enc2_sem = sem_open(CHAN_ENC2_SEM_FNAME, 0);
		if(chan_enc2_sem == SEM_FAILED){
			perror("sem_open/chanenc2");
			exit(EXIT_FAILURE);
		}
		
		char msg[300];
		char hash[MD5_DIGEST_LENGTH];
		int i = 1;
		while(1){
			if(i==1){
				sem_wait(chan_enc2_sem);
				i = 0;				
			}
			
			strcpy(msg, chan_enc2_shm->message);
			MD5(msg, strlen(msg), hash);
			
			while(memcmp(hash, chan_enc2_shm->checksum, MD5_DIGEST_LENGTH)!=0){
				//if checksums are the different
				chan_enc2_shm->corruption_status = 1;
				sem_post(enc2_chan_sem);
				sem_wait(chan_enc2_sem);
				strcpy(msg, chan_enc2_shm->message);
				MD5(msg, strlen(msg), hash);				
			}
		
			//checksums are the same
			strcpy(p2_enc2_shm, chan_enc2_shm->message);
			sem_post(p2_sem);
			
			if(memcmp(chan_enc2_shm->message, "TERM", 4)==0){
				sem_close(enc2_chan_sem);
				sem_close(chan_enc2_sem);
				sem_close(enc2_p2_sem);
				
				if(shmdt(chan_enc2_shm)){
					perror("shmdt()");
					exit(EXIT_FAILURE);
				}
				
				shmctl(chan_enc2_shmid, IPC_RMID, NULL);
				break;
			}
			
			sem_wait(enc2_p2_sem);
			
			//create md5 hash
			strcpy(msg, p2_enc2_shm);
			MD5(msg, strlen(msg), hash);
			
			//pass it to enc2_chan_shm			
			strcpy(chan_enc2_shm->message, msg);
			memcpy(chan_enc2_shm->checksum, hash, MD5_DIGEST_LENGTH);
			chan_enc2_shm->corruption_status = 0;
			
			sem_post(enc2_chan_sem);
			
			if(memcmp(msg, "TERM", 4)==0){
				sem_close(enc2_p2_sem);
				sem_close(chan_enc2_sem);
				sem_close(enc2_chan_sem);
				
				if(shmdt(p2_enc2_shm)){
					perror("shmdt()");
					exit(EXIT_FAILURE);
				}


				shmctl(p2_enc2_shmid, IPC_RMID, NULL);
				break;
			}
			
			sem_wait(chan_enc2_sem);	
			while(chan_enc2_shm->corruption_status == 1){
				strcpy(chan_enc2_shm->message, msg);
				memcpy(chan_enc2_shm->checksum, hash, MD5_DIGEST_LENGTH);
				chan_enc2_shm->corruption_status=0;
				
				sem_post(enc2_chan_sem);
				sem_wait(chan_enc2_sem);
			}
		}
		
	}
}
