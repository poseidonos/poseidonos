#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <inttypes.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

void show_elapsed_time(struct timeval* start, struct timeval* end, char* msg, int repeat_count, uint64_t io_size);

void show_elapsed_time(struct timeval* start, struct timeval* end, char* msg, int repeat_count, uint64_t io_size){
	long secs_used;
	long micros_used;
	secs_used=(end->tv_sec - start->tv_sec); //avoid overflow by subtracting first
	micros_used= ((secs_used*1000000) + end->tv_usec) - (start->tv_usec);

	printf("======================================================\n");
	printf("%s start: %lds.%ldus end: %lds.%ldus\n", (msg) ? msg : "", start->tv_sec, start->tv_usec, end->tv_sec, end->tv_usec);
	printf("%s total elapsed: %ldus, %.3fms\n", (msg) ? msg : "", micros_used, ((float)micros_used) / 1000);
	if(repeat_count) {
		float latency = (float)micros_used / repeat_count;
		printf("%s latency: %.3f us per operation\n", (msg) ? msg : "", latency);
		float ops = 1000000 / latency;
		printf("%s ops: %.3f\n", (msg) ? msg : "", ops);
		if(io_size > 0){
			uint64_t throughput = ops * io_size / 1024;
			printf("%s throughput: %ld KB\n", (msg) ? msg : "", throughput);
		}
	}
	printf("======================================================\n\n\n");
}

int main(int argc, char** argv){
	struct timeval start;
	struct timeval end;
	gettimeofday(&start, NULL);
	usleep(100 * 1000);
	gettimeofday(&end, NULL);
	show_elapsed_time(&start, &end, (char*)"WORK", 1, 0);

	return 0;
}
