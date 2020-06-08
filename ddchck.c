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

#define max 10

int elements_count = 0;
int edges[max][max] = {
	0,
};

typedef struct element
{
	int id;
	int owner;
	char resource[256];
	int index;
} element_node;

element_node elements[max];

int dfs(int v, int visited[])
{
	if (visited[v] == 1)
		return 1;
	else
		visited[v] = 1;

	for (int i = 0; i < max; ++i)
		if (edges[v][i] == 1)
			if (dfs(i, visited))
				return 1;

	return 0;
}

int check_cycle()
{
	for (int i = 0; i < max; ++i)
	{
		int visited[max] = {
			0,
		}; 
		if (dfs(i, visited))
			return 1;
	}
	return 0;
}

int add_element(element_node input)
{
	elements[elements_count].id = input.id;
	elements[elements_count].owner = input.owner;
	strcpy(elements[elements_count].resource, input.resource);
	elements[elements_count].index = elements_count;
	elements_count++;
	return elements[elements_count-1].index;
}

int return_index(element_node input)
{
	int i = 0;
	while (i < elements_count)
	{
		if (strcmp(input.resource, elements[i].resource) == 0)
		{
			return elements[i].index;
		}
		i++;
	}
	return add_element(input);
}

int indexby_id(int id)
{
	int i = 0;
	while (i < elements_count)
	{
		if (id == elements[i].id)
		{
			return elements[i].index;
		}
		i++;
	}
	return -1;
}

int indexby_resource(char resource[]){
	int i = 0;
	while (i < elements_count)
	{
		if (strcmp(resource, elements[i].resource) == 0)
		{
			return elements[i].index;
		}
		i++;
	}
	return -1;
}

int main(int argc, char *argv[])
{
	//if (argc == 0){
	int fd = open(".ddtrace", O_RDONLY | O_SYNC);
	element_node input;
	element_node wait[max][max];
	int wait_count[max] = {0,};
	int index, lock;

	while (1)
	{
		char s[256];
		int len = read(fd, s, 256);
		if (len == -1)
			break;
		if (len > 0)
		{
			char *ptr = strtok(s, " ");
			input.id = atoi(ptr);

			ptr = strtok(NULL, " ");
			input.owner = atoi(ptr);

			char *str = strtok(NULL, " ");
			strcpy(input.resource, str);

			ptr = strtok(NULL, " ");
			lock = atoi(ptr);

			//lock
			if (lock == 1)
			{
				//self_deadlock
				int i = 0;
				while(i < elements_count){
					if(elements[i].id == input.id){
						if(strcmp(elements[i].resource, input.resource) == 0){
							//check the node is already exist
							if(index <= elements_count){
								if(input.owner == 0 || input.owner == input.id){
									edges[index][index] = 1;
									printf("add edge: %d --> %d\n", index, index);
								}
							}

						}
					}
					i++;
				}

				//check mutex is exist or not
				index = return_index(input);
				printf("id %d, owner %d, address %s, index %d\n", input.id, input.owner, input.resource, index);

				//already someone holds it
				if(input.owner != input.id && input.owner != 0){
					int owner_index = indexby_id(input.owner);
					//owner is exist
					if(owner_index > -1){
						int i = 0;
						while(i < elements_count){
						if(elements[i].id == input.id){
							edges[elements[i].index][owner_index] = 1;
							printf("add edge: %d --> %d\n", elements[i].index, owner_index);
						}
						i++;
					}


					wait[owner_index][wait_count[owner_index]++] = input;
					printf("wait id %d, owner %d, address %s in %d\n", input.id, input.owner, input.resource, wait_count[owner_index]);
					}
					printf("\n");
				}

				if (check_cycle() == 1)
				{
					printf("***Deadlock is founded***\n");
					printf("This program is finished. If you want to restart the program, start with new program name\n");
					exit(1);
				}
			}

			//unlcok
			else if (lock == 0)
			{
				index = return_index(input);
				printf("release index %d\n", index);
				for (int i = 0; i < elements_count; i++)
				{
					edges[i][index] = 0;
				}
			}
		}
	}
	/*
	}

	else if (argc == 1)
	{
		char file_name[256];
		strcpy(file_name, argv[1]);
		printf("Execute ddchck with %s", file_name);
	}
	else
	{
		printf("wrong arguments. You only can put filename or nothing\n");
	}*/

	return 0;
}