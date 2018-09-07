#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h>

#define UEVENT_BUFFER_SIZE	2048


static int init_hotplug_sock(void)
{
	struct sockaddr_nl snl;
	const int buffersize = 16 * 1024 * 1024;
	int retval;

	memset(&snl,0x00,sizeof(struct sockaddr_nl));
	snl.nl_family = AF_NETLINK;
	snl.nl_pid = getpid();
	snl.nl_groups = 1;

	int hotplug_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (hotplug_sock == -1)
	{
		printf("error getting socket: %s", strerror(errno));
		return -1;
	}
	
	setsockopt(hotplug_sock, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, sizeof(buffersize));
	retval = bind(hotplug_sock, (struct sockaddr *)&snl, sizeof(struct sockaddr_nl));
	if (retval < 0)
	{
		printf("bind failed: %s", strerror(errno));
		close(hotplug_sock);
		hotplug_sock == -1;
		return -1;
	}

	return hotplug_sock;
}

int get_len(char s[])
{
	int temp = 0;
	char x[200] = {0};
	strcpy(x, s);
	for (temp = 0; temp < 200; temp++)
	{
		if(x[temp] == 0) break;
	}
	return temp;
}

int find_sym1(char s[], int len)
{
	int i = 0;
	for (i = len; i >= 0; i--)
	{
		if(s[i] == '/') return i + 1;

	}
	return -1;
}

int find_sym2(char s[], int len)
{
	int i = 0;
	for( i = 0; i <= len; i++)
	{
		if(s[i] == '@') return i;
	}
	return -1;
}

int read_dir(char name[])
{
	int flag = 0;
	char str[200] = {0};
	strcpy(str, name);
	DIR *dir = opendir(str);
	if (dir == NULL)
	{
		perror("opendir");
		return -1;
	}
	struct dirent *ent = readdir(dir);
	while(ent != NULL)
	{

		char fname[100] = {0};
		char fname_cut[10] = {0};
		strcpy(str, name);
		int type = ent->d_type;
		strcpy(fname, ent->d_name);
		int flength = get_len(fname);
		//printf("name = %s: type = %d, length = %d\n", fname, type, get_len(fname));

		if (ent->d_type != 4 && flength >= 6)
		{
			int len = 6;
			strncpy(fname_cut, fname, len);
			//printf("%s\n", fname_cut);

			if (!strcmp(fname_cut, "ttyUSB"))
			{				
				printf("/dev/%s\t", fname);
				flag++; 	
			}

		}
	
		ent = readdir(dir);
	}
	printf("\n");
	printf("There are %d USB device have been found\n\n", flag);
	closedir(dir);
	return 0;
}

int analy_sentence(char s[])
{
	int len = get_len(s);
	int pos1, pos2;
	char sym1[10] = {0};
	char sym2[10] = {0};
	char sym3[10] = {0};
	pos2 = find_sym1(s, len);
	strncpy(sym2, s + pos2, 3);
	//printf("%s\n", sym2);
	if (strcmp(sym2, "tty")) return -1;
	int temp = len - pos2;
	strncpy(sym3, s + pos2, temp);
	pos1 = find_sym2(s, len);
	strncpy(sym1, s, pos1);
	//printf("%s\n", sym1);
	printf("Event------------------%s device: %s\n", sym1, sym3);
	return 0;
}

int Thread_one(void)
{
	int hotplug_sock = init_hotplug_sock();
	while (1)
	{
		char buf[UEVENT_BUFFER_SIZE * 2] = {0};
		recv(hotplug_sock, &buf, sizeof(buf), 0);
		//printf("%s\n", buf);
		analy_sentence(buf);
	}
	return 0;
}

int Thread_two(void)
{
	while (1)
	{	
		read_di ("/dev/");
		sleep(5);
	}
	return 0;
}

int main(int argc, char* argv[])
{
	int cmd;
	int i = 0;	
	pthread_t tid1, tid2;

	int ret1 = pthread_create(&tid1, NULL, (void*)Thread_one, NULL);
	if (ret1 != 0)
	{
		printf("Create pthread error!\n");
		exit(1);
	}

	int ret2 = pthread_create(&tid2, NULL, (void*)Thread_two, NULL);
	if (ret2 != 0)
	{
		printf("Create pthread error!\n");
		exit(1);
	}
	
	while(1)
	{
		scanf("%d", &cmd);
		
		if (cmd == 0)
		{
			printf("exit\n");
			exit(0);
		}
	
	}
	
 	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
	return 0;

}
