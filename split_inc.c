#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <shmem.h>
#include <sys/time.h>
double *pWrk;
long *pSync;
struct timeval ts,te;
int counter;
#define max(a,b) ((a)>(b)?(a):(b))
double totalusec,myusec,request_usec,test_usec;
double difftime(struct timeval ts, struct timeval te)
{
	return  (te.tv_sec*1000000+te.tv_usec-ts.tv_sec*1000000-ts.tv_usec);
}
int main(int argc, char *argv[])
{
	start_pes(0);

	int me=shmem_my_pe();
	int npe=shmem_n_pes();
	int inc_time,i;
	printf("%d:Initialized\n",me);
	pWrk=(double*)shmalloc(sizeof(double)*max(1/2 + 1, _SHMEM_REDUCE_MIN_WRKDATA_SIZE));
	pSync=(long*)shmalloc(sizeof(long)*_SHMEM_REDUCE_SYNC_SIZE);
	for(i=0;i<SHMEM_REDUCE_SYNC_SIZE;i++)
	{
		pSync[i] = _SHMEM_SYNC_VALUE;
	}

	sscanf(argv[1],"%d", &inc_time);
	if(npe<2 && me==0){
		printf("Please run under num_pes >= 2\n");
		exit(-1);
	}
	if(me!=0){
		split_handle_t* *handlelist=(split_handle_t**)malloc(sizeof(void*)*inc_time);
		if(handlelist==NULL){
			fprintf(stderr, "%d: handle list allocation failed\n", me);
			exit(-1);
		}
		memset(handlelist,0,sizeof(void*)*inc_time);
		gettimeofday(&ts,NULL);
		for(i=0;i<inc_time;i++){
			handlelist[i]=shmem_int_inc_nb(&counter, (me+1)%npe);
			//handlelist[i]=shmem_int_inc_nb(&counter, 0);
			/*
			if(handlelist[i]==NULL){
				fprintf(stderr, "%d: handlelist[%d] allocation failed\n", me, i);
				exit(-1);
			}
			*/
		}
		gettimeofday(&te,NULL);
		request_usec=difftime(ts,te);

		sleep(5);

		printf("%d:%d inc requests sent in %f us\n",me,inc_time,request_usec);
		gettimeofday(&ts,NULL);
		int nDone=0;
		i=0;
		while(nDone<npe-1){
			if(handlelist[i]!=NULL){
				if(shmem_split_test(handlelist[i])){
					shmem_split_release_handle(handlelist[i]);

					handlelist[i]=NULL;
					nDone++;
				}
			}
			i=(i+1)%npe;
		}
		gettimeofday (&te, NULL);
		test_usec = difftime(ts,te);
		printf ("%d:%d inc requests finished,test cost %f us\n",me,inc_time,test_usec);
		myusec = request_usec + test_usec;
	}

	shmem_barrier_all();
	shmem_double_sum_to_all(&totalusec, &myusec, 1, 0, 0, npe, pWrk, pSync);
	shmem_barrier_all();
	if(me==0){
		printf("totalused=%f\n", totalusec);
		printf("%d: counter is %d, %s\n", me, counter, (counter==(npe-1)*inc_time?"Success":"Failed"));
		printf("Evenly communication time is %f us\n", totalusec/(npe-1));
	}
	return 0;
}
