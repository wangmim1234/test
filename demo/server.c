#include<stdio.h>
#include<sys/stat.h>
#include<errno.h>
#include<netdb.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<string.h>
#include <sys/select.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<bits/select.h>
#include <sys/ioctl.h>
#include<linux/sockios.h>
#include <time.h>  
#include <sys/time.h>  
#include <stdlib.h>  
#include <signal.h>  
static struct itimerval oldtv;  
int  Time[1024];
int maxfdp=3;
fd_set rds;
//定时
void set_timer()  
{  
    struct itimerval itv;  
    itv.it_interval.tv_sec = 120;  
    itv.it_interval.tv_usec = 0;  
    itv.it_value.tv_sec = 80;  
    itv.it_value.tv_usec = 0;  
    setitimer(ITIMER_REAL, &itv, &oldtv);  
} 
//定时到所执行的函数
void signal_handler()
{

	int overtime=80;//10s
	time_t timep;
	time(&timep);
	for(int i=maxfdp;i>3;i--)
	{
		if(Time[i]!=-1)
		{
			
			if(timep-Time[i]>overtime)
			{
				close(i);
				FD_CLR(i,&rds);
				Time[i]=-1;
				printf("关闭客户业务员%d\n",i);
				//关闭客户连接套接字，改变maxfdp参数
				if(i==maxfdp)
				{
					maxfdp--;
					while(!FD_ISSET(maxfdp,&rds)&&maxfdp!=3)
						maxfdp--;
				}
				
                        	
                	}
        	}
 	}
}	
int main() 
{ 
	 memset(Time,-1,sizeof(Time));
   	 int serversock; 
   	 int sock=0,ret;
   	 fd_set fds; 
   	 fd_set eds;
   	 fd_set wds; 
   	 char sendbuffer[1024];
   	 char message[1024];
   	 char buffer[1024]={0}; //256字节的接收缓冲区
   	 memset(buffer,0,sizeof(buffer)); //缓冲区清0
   	 memset(sendbuffer,0,sizeof(sendbuffer)); //缓冲区清0
   	 memset(message,0,sizeof(message)); //缓冲区清0
   	 struct timeval timeout={0,0}; //select等待3秒，3秒轮询，要非阻塞就置0 
   	 struct sockaddr_in serveraddr;
   	 struct sockaddr_in clientaddr;
   	 int addr_len=sizeof(clientaddr);
   	 
   	 //获取服务器套接字
   	 if((serversock=socket(AF_INET,SOCK_STREAM,0))<0)
   	 {
   	     	perror("socket建立失败");
   	     	return 1;
   	 }
   	 //设置服务器的ip和端口
   	 serveraddr.sin_family=AF_INET;
   	 serveraddr.sin_port=htons(5088);
   	 serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
   	//服务器的ip和端口、申请的服务器套接字建立联系
  	if( bind(serversock, (struct sockaddr *) &serveraddr, sizeof(serveraddr))<0)
  	{
   		perror("bind失败");
        	return 1;
  	}
  	//开始监听，服务器套接字开始工作，最多可同时接受5个客户端的连接
    	if(listen(serversock, 5)<0)
    	{
    		perror("listen失败");
        	return 1;
    	} 
    	 maxfdp=serversock;
    	 FD_ZERO(&rds); 
    	 FD_SET(serversock,&rds); 
    	 FD_SET(0,&rds); 
    	 set_timer();
    	 signal(SIGALRM,signal_handler);  
    	 printf("按q+回车退出\n");
         printf("服务器运行中\n");
       //死循环检测几个数组列表里套接字的变化 
       while(1) 
    	{ 
       	FD_ZERO(&fds); 
       	
       	//FD_SET(0,&fds); 
       	
           	fds=rds;	
          	eds=rds;
          	wds=rds;
                ret=select(maxfdp+1,&fds,&wds,&eds,&timeout);
		if(-1==ret&&errno!=4)
		{
				printf("服务器停止运行\n");
				printf("select errno=%d\n",errno);
				close(serversock);
				break; //退出循环
			
		}
		
		if(ret>0)
   		{
   			//检测客户端的消息发送情况
    			for(int cslient=0;cslient<maxfdp+1;cslient++)
			{
				if(FD_ISSET(cslient,&fds)) 
				{
					int number;
					time_t timep;
					switch (cslient)
					{
						case 0:
							 read(0,message,100);
		       				if('q'==message[0]&&'\n'==message[1])
		       				{
		       					printf("服务器停止运行\n");
							close(serversock);
		       					goto exit;
		       					
		       				
		       				}
		       			case 1:
		       			case 2:
		       				break;
		       			case 3:
		       				sock=accept(serversock, (struct sockaddr *)&clientaddr, (socklen_t *)&addr_len);
							if(sock>0)  
							{
								maxfdp=maxfdp>sock?maxfdp:sock;//maxfdp++;
								//将接待客户的业务员套接字加入可读数组
								FD_SET(sock,&rds);
								printf("socket%d链接成功\n",sock);
							} 
							else perror("socket链接失败");
							goto LAB_updatetime;
						default:
							number=recv(cslient,buffer,1024,0);//接受网络数据
							buffer[number]=0;
							if(strcmp(buffer,"11")==0)
								goto LAB_updatetime;	 
							if (!number)
							{	//判断客户正常下线
								if(number==0)
								{
									close(cslient);
									printf("关闭客户业务员%d\n",cslient);
									if(cslient==maxfdp)
									 {
									 	maxfdp--;
										while(!FD_ISSET(maxfdp,&rds)&&maxfdp!=3)
											maxfdp--;
									 }
								}
								else printf("socket%d接收失败\n",cslient);//接收字节书小于0
							}
							else
							{
								if(maxfdp==5)
								{
									if(cslient==4&&FD_ISSET(cslient+1,&wds))
									{
										if(send(cslient+1,buffer,strlen(buffer),0)<0)
											perror("cslient4 send error");
									}
									if(cslient==5&&FD_ISSET(cslient-1,&wds))
									{
										if(send(cslient-1,buffer,strlen(buffer),0)<0)
											perror("cslient5 send error");
									}
								}
				LAB_updatetime:		time(&timep);//更新客户在线的时间
								Time[cslient]=timep;
							}
					}
				}
				/*if(FD_ISSET(cslient,&eds))
				{
					close(cslient);
				}*/
			}
		}
        }//end while 
    	exit:close(serversock);
}
//end main   

