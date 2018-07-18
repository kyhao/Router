// TCP服务器 采用select方式
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "errwrap.h"
 
#define MAXLINE 80
#define SERVERPORT 8000
 
int main(int argc, char *argv[])
{
		int i, maxi, maxfd, listenfd, confd, sockfd;
		int nret, client[FD_SETSIZE]; // FD_SETSIZE 默认为1024 
		ssize_t n;
		fd_set rset, allset;
		char buf[MAXLINE];
		char str[INET_ADDRSTRLEN]; //#define INET_ADDRSTRLEN 16 
		socklen_t clientaddr_len;
		struct sockaddr_in clientaddr, serveraddr;
		listenfd = Socket(AF_INET, SOCK_STREAM, 0);//获得一个监听fd
		
		bzero(&serveraddr, sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
		serveraddr.sin_port = htons(SERVERPORT);
		Bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
		Listen(listenfd, 128); 
 
		maxfd = listenfd; //maxfd为现在监听的文件描述符的最大值
		maxi = -1; /* client[]的下标*/
		for (i = 0; i < FD_SETSIZE; i++)
				client[i] = -1; /* 用-1初始化client[] */
 
		FD_ZERO(&allset);
		FD_SET(listenfd, &allset); // 设置allset（所有监听文件描述符集合最新状态）
		for ( ; ; ) 
		{
				rset = allset; //当前文件描述符集合
				nret = select(maxfd+1, &rset, NULL, NULL, NULL);//检测“读”
				if (nret < 0)
						perr_exit("select error");
						if (FD_ISSET(listenfd, &rset)) //如果是新的连接到来
						{ 
								clientaddr_len = sizeof(clientaddr);
								confd = Accept(listenfd, (struct sockaddr *)&clientaddr, &clientaddr_len);
								printf("received from %s at PORT %d\n",
												inet_ntop(AF_INET, &clientaddr.sin_addr, str, sizeof(str)),
												ntohs(clientaddr.sin_port));
								for (i = 0; i < FD_SETSIZE; i++)
								{
										if (client[i] < 0) 
										{
												client[i] = confd; //将新的文件描述符添加到client里
												break;
										}
								}
								// 达到select能监控的文件个数上限1024 
								if (i == FD_SETSIZE) 
								{
										fputs("too many clients\n", stderr);
										exit(1);
								}
								FD_SET(confd, &allset); //将新的新的文件描述符添加到监控信号集里
								if (confd > maxfd)
										maxfd = confd; //更新最大的文件描述符
								if (i > maxi)
										maxi = i; // 更新client最大下标
								if (--nret == 0)
										continue; /* 如果没有更多的就绪文件描述符继续回到上面select阻塞监听,负责处理未
													 处理完的就绪文件描述符*/
						}
				for (i = 0; i <= maxi; i++) 
				{ // 检测哪个clients 有数据就绪
						if ( (sockfd = client[i]) < 0)
								continue;
						if (FD_ISSET(sockfd, &rset)) //对连接到服务器的每个客户机（文件描述符），检测是否有数据到达
						{
								if ( (n = Read(sockfd, buf, MAXLINE)) == 0) 
								{
										// 当client关闭链接时，服务器端也关闭对应链接
										Close(sockfd);
										FD_CLR(sockfd, &allset); // 解除select监控此文件描述符
										client[i] = -1;
								} else 
								{//处理每个有数据到达的fd
										int j;
										for (j = 0; j < n; j++)
												buf[j] = toupper(buf[j]);
												Write(sockfd, buf, n);
								}
								if (--nret == 0)//处理的那个fd不是最后一个
										break;
						}
				}
		}
		close(listenfd);
		return 0;
}
