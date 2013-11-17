#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "rnotify.h"

#define MAX_MEMORY_SIZE 1000*1024

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("Usage: a.out <dir>\n");
		exit(1);
	}
	long page_size = sysconf(_SC_PAGESIZE);
	size_t path_to_statm_size = 64;
	char path_to_statm[path_to_statm_size];
	snprintf(path_to_statm, path_to_statm_size, "/proc/%d/statm", getpid());
	const long max_memory_pages = MAX_MEMORY_SIZE / page_size;

	const char* dir = argv[1];
	printf("Start to watch %s page_size=%ld max_memory_pages=%ld\n", dir, page_size, max_memory_pages);

	uint32_t mask = IN_MODIFY | IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MOVE_SELF | IN_MOVED_FROM | IN_MOVED_TO;

	Notify* ntf = initNotify(dir, mask, "^\\.");
	if(ntf == NULL)
	{
		exit(EXIT_FAILURE);
	}

	for(;;)
	{
		FILE* f = fopen("/var/readydropms/rd.fifo", "w");
		if(!f)
		{
			printf("==%s\n", strerror(errno));
		}

		char* path = NULL;
		uint32_t mask = 0;
		waitNotify(ntf, &path, &mask);
		if(path == NULL)
		{
			exit(EXIT_FAILURE);
		}
		if(mask & IN_MODIFY)
		{
			//printf("modify \t%s\n", path);
			fprintf(f, "~%s*data/readydrop*readydrop\n", path + 15);
		}
		if(mask & IN_CREATE)
		{
			//printf("create \t%s\n", path);
			fprintf(f, "+%s*data/readydrop*readydrop\n", path + 15);
		}
		if(mask & IN_DELETE)
		{
			//printf("delete \t%s\n", path);
			fprintf(f, "-%s*data/readydrop*readydrop\n", path + 15);
		}
		if(mask & IN_DELETE_SELF)
		{
			//printf("delete self %s\n", path);
		}
		if(mask & IN_MOVE_SELF)
		{
			//printf("move self %s\n", path);
		}
		if(mask & IN_MOVED_FROM)
		{
			//printf("moved from \t%s\n", path);
			fprintf(f, "-%s*data/readydrop*readydrop\n", path + 15);
		}
		if(mask & IN_MOVED_TO)
		{
			//printf("moved to \t%s\n", path);
			fprintf(f, "+%s*data/readydrop*readydrop\n", path + 15);
		}
		if(mask & IN_Q_OVERFLOW)
		{
			printf("overflow\n");
		}
		if(!strlen(path))
		{
			exit(-1);
		}
		free(path);
		fclose(f);

		/*
		unsigned long pages = 0;
		FILE* f = fopen(path_to_statm, "r");
		if(f == NULL)
		{
			freeNotify(ntf);
			return -1;
		}
		if(1 != fscanf(f, "%*u %lu", &pages))
		{
			freeNotify(ntf);
			fclose(f);
			return -1;
		}
		fclose(f);
		*/
		//printf("memsize=%lu\n", pages);
	}
	freeNotify(ntf);
	return 0;
}
