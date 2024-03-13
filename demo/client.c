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
static int clientsocket;
static struct itimerval oldtv;  
//定时函数
void signal_hearthop()
{	
	//初始化变量
	char heartdata[]="11";
	send(clientsocket,heartdata,strlen(heartdata),0);
	//printf("timer\n");
}
int main()
{
     
	struct timeval timeout={0,0}; 
	struct itimerval itv;  
    itv.it_interval.tv_sec = 30;  
    itv.it_interval.tv_usec = 0;  
    itv.it_value.tv_sec = 30;  
    itv.it_value.tv_usec = 0;  
	struct sockaddr_in serveraddr;
	int flag;
	int communication(int clientsocket);
    if((clientsocket=socket(AF_INET,SOCK_STREAM,0))<0)
    {  
       	perror("socket");
    	return 1;
    }

	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(5088);
	serveraddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	
        if((connect(clientsocket,(struct sockaddr *)&serveraddr,sizeof(serveraddr)))<0)
        {
        	//printf("%d\n",errno);
    		perror("errno");
        	return 1;
    	} 
    	setitimer(ITIMER_REAL, &itv, &oldtv); 
    	signal(SIGALRM,signal_hearthop); 
    	printf("*****************1.communication*****************2.quit*******************\n");
    	scanf("%d",&flag);
    	switch(flag)
    	{
    		case 1:
    			communication(clientsocket);
    			break;
    		case 2:
    			close(clientsocket);
    			break;
    	        default:
    	        	break;
        }
        return 0;
        
}


int communication(int clientsocket)
{	
		
	
	   	char message[100];
		char info[1024];
        	fd_set fds;
		int maxfds;
		int ret;
		int messageBt;
		struct timeval timeout={1,0}; 
		memset(info,0,sizeof(info));
		memset(message,0,sizeof(message));
		while(1)
        	{
     			
         		FD_ZERO(&fds); 
         		FD_SET(clientsocket,&fds); 
          		FD_SET(0,&fds); 
         		maxfds=clientsocket;
         		ret=select(maxfds+1,&fds,NULL,NULL,&timeout);
           		switch(ret) //select使用 
       		 { 
            			case 0:break; //再次轮询
            			case -1: 
            				if(errno==4)
            		         	{
            		         		break;
            		         	}
            		         	else  return -1;; //select错误，再次轮询 
            			default: 
          				if(FD_ISSET(clientsocket,&fds)) //测试sock是否可读，即是否网络上有数据 
                			{ 
                				messageBt=recv(clientsocket,info,1024,0);
                	  			if(messageBt)
                	     			{
                	     				info[messageBt]=0;
                   					printf("%80s\n",info);
                     				}  
                    				else
                    				{
                    					printf("服务器异常\n");
                	         			return 1;
                	         		}
                     				  
        	  			}        
          				if(FD_ISSET(0,&fds))
        	  			{
        	      			
                       			 messageBt=read(0,message,100);
                       			 if(strcmp(message,"2\n")==0)
                       			 {
                       		 		close(clientsocket);
                       		 		return 0;
                       		 	 }	
                       		 	 message[messageBt]=0;
          					 send(clientsocket,message,strlen(message),0);
                   			}
                   			break;
                   	}
        
      
         }
 } 

