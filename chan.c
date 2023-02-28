#include "header.h"
#include <time.h>


int main(int argc,char *argv[]){
	
	//GET THE PROBABILITY AS ARGUMENT
	double prob;
	if(argc!=2){
		printf("1 argument must be given! \n");
		return 2;
	}
	else{
		prob = atof(argv[1]);
		if(prob>0.7 || prob<0){
			printf("Probability must be >=0 and <=0.7\n");
			return 3;
		}
	}
	
	//CREATE ALL SEMAPHORES FOR THE APPLICATION
	sem_unlink(P1_SEM_FNAME);
	if(sem_open(P1_SEM_FNAME, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)==SEM_FAILED){
		perror("sem_open/p1");
		exit(EXIT_FAILURE);
	}
	
	sem_unlink(ENC1_P1_SEM_FNAME);
	if(sem_open(ENC1_P1_SEM_FNAME, O_CREAT|O_EXCL, S_IRUSR | S_IWUSR, 0)==SEM_FAILED){
		perror("sem_open/enc1p1");
		exit(EXIT_FAILURE);
	}
	
	sem_unlink(ENC1_CHAN_SEM_FNAME);
	if(sem_open(ENC1_CHAN_SEM_FNAME, O_CREAT|O_EXCL, S_IRUSR | S_IWUSR, 0)==SEM_FAILED){
		perror("sem_open/enc1chan");
		exit(EXIT_FAILURE);
	}
	
	sem_unlink(CHAN_ENC1_SEM_FNAME);
	if(sem_open(CHAN_ENC1_SEM_FNAME, O_CREAT|O_EXCL, S_IRUSR | S_IWUSR, 0)==SEM_FAILED){
		perror("sem_open/chanenc1");
		exit(EXIT_FAILURE);
	}
	
	sem_unlink(CHAN_ENC2_SEM_FNAME);
	if(sem_open(CHAN_ENC2_SEM_FNAME, O_CREAT|O_EXCL, S_IRUSR | S_IWUSR, 0)==SEM_FAILED){
		perror("sem_open/chanenc2");
		exit(EXIT_FAILURE);
	}
	
	sem_unlink(ENC2_P2_SEM_FNAME);
	if(sem_open(ENC2_P2_SEM_FNAME, O_CREAT|O_EXCL, S_IRUSR | S_IWUSR, 0)==SEM_FAILED){
		perror("sem_open/enc2p2");
		exit(EXIT_FAILURE);
	}
	
	sem_unlink(ENC2_CHAN_SEM_FNAME);
	if(sem_open(ENC2_CHAN_SEM_FNAME, O_CREAT|O_EXCL, S_IRUSR | S_IWUSR, 0)==SEM_FAILED){
		perror("sem_open/enc2chan");
		exit(EXIT_FAILURE);
	}
	
	sem_unlink(P2_SEM_FNAME);
	if(sem_open(P2_SEM_FNAME, O_CREAT|O_EXCL, S_IRUSR | S_IWUSR, 0)==SEM_FAILED){
		perror("sem_open/p2");
		exit(EXIT_FAILURE);
	}
	
	//CREATE CHAN-ENC1 AND CHAN-ENC2 SHARED MEMORY
	
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
	
	int chan_enc2_shmid;
	if((chan_enc2_shmid = shmget(CHAN_ENC2_SHMKEY, sizeof(memory_data), PERMS|IPC_CREAT)) == -1){
		perror("shmget()");
		exit(EXIT_FAILURE);
	}
	
	memory_data* chan_enc2_shm;
	if((chan_enc2_shm = (memory_data*)shmat(chan_enc2_shmid,0 ,0)) == (memory_data*)-1){
		perror("shmat()");
		exit(EXIT_FAILURE);
	}
	
	
	//ATTACH TO SEMAPHORES USED BY CHAN
	sem_t *enc1_chan_sem = sem_open(ENC1_CHAN_SEM_FNAME, 0);
	if(enc1_chan_sem == SEM_FAILED){
		perror("sem_open/enc1chan");
		exit(EXIT_FAILURE);
	}
	
	sem_t *chan_enc1_sem = sem_open(CHAN_ENC1_SEM_FNAME, 0);
	if(chan_enc1_sem == SEM_FAILED){
		perror("sem_open/chanenc1");
		exit(EXIT_FAILURE);
	}

	sem_t *chan_enc2_sem = sem_open(CHAN_ENC2_SEM_FNAME, 0);
	if(chan_enc2_sem == SEM_FAILED){
		perror("sem_open/chanenc2");
		exit(EXIT_FAILURE);
	}	
	
	sem_t *enc2_chan_sem = sem_open(ENC2_CHAN_SEM_FNAME, 0);
	if(enc2_chan_sem == SEM_FAILED){
		perror("sem_open/enc2chan");
		exit(EXIT_FAILURE);
	}
	
	srand(time(NULL));
	char random[] = "!@#$%^&*()_+1234567890qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM,.<>/;'?:~";
	char msg[300];
	int i, k=1;
	
	while(prob != (double)abs(prob)){
		prob*=10;
		k*=10;	
	}
	
	while(1){
		sem_wait(enc1_chan_sem);
		if(chan_enc1_shm->corruption_status == 0){
		//if message is normal
		//do the rng things chan is supposed to do
			strcpy(msg, chan_enc1_shm->message);
			if(memcmp(msg, "TERM", 4)!=0){
				for(i=0; i<strlen(msg); i++){
					if(rand()%k<prob){
						msg[i]=random[rand()%strlen(random)];
					}
				}
			}
				
			strcpy(chan_enc2_shm->message, msg);
			memcpy(chan_enc2_shm->checksum, chan_enc1_shm->checksum, MD5_DIGEST_LENGTH);
			chan_enc2_shm->corruption_status=0;
		}
		else{
			//if message is to be retransmitted
			strcpy(chan_enc2_shm->message, chan_enc1_shm->message);
			memcpy(chan_enc2_shm->checksum, chan_enc1_shm->checksum, MD5_DIGEST_LENGTH);
			chan_enc2_shm->corruption_status = 1;
		}

		sem_post(chan_enc2_sem);
		
		if(memcmp(msg, "TERM", 4)==0){
			sem_close(enc1_chan_sem);
			sem_close(enc2_chan_sem);
			sem_close(chan_enc1_sem);
			sem_close(chan_enc2_sem);
			
			if(shmdt(chan_enc1_shm)){
				perror("shmdt()");
				exit(EXIT_FAILURE);
			}
			
			shmctl(chan_enc1_shmid, IPC_RMID, NULL);
			break;
		}
		
		sem_wait(enc2_chan_sem);
		
		if(chan_enc2_shm->corruption_status == 0){
			//do the things chan is supposed to do
			strcpy(msg, chan_enc2_shm->message);
			if(memcmp(msg, "TERM", 4)!=0){
				for(i=0; i<strlen(msg); i++){
					if(rand()%k<prob){
						msg[i]=random[rand()%strlen(random)];
					}
				}
			}
			strcpy(chan_enc1_shm->message, msg);
			memcpy(chan_enc1_shm->checksum, chan_enc2_shm->checksum, MD5_DIGEST_LENGTH);
			chan_enc1_shm->corruption_status=0;
		}
		else{
			strcpy(chan_enc1_shm->message, chan_enc2_shm->message);
			memcpy(chan_enc1_shm->checksum, chan_enc2_shm->checksum, MD5_DIGEST_LENGTH);
			chan_enc1_shm->corruption_status = 1;
		}
		sem_post(chan_enc1_sem);
		
		if(memcmp(msg, "TERM", 4)==0){
			sem_close(enc1_chan_sem);
			sem_close(enc2_chan_sem);
			sem_close(chan_enc1_sem);
			sem_close(chan_enc2_sem);
			
			if(shmdt(chan_enc1_shm)){
				perror("shmdt()");
				exit(EXIT_FAILURE);
			}
			
			shmctl(chan_enc2_shmid, IPC_RMID, NULL);
			break;
		}
	}
}

