#include"locker.hpp"
#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<unistd.h>
#include<pthread.h>

typedef struct LinkNode
{
	int data;
	struct LinkNode *next;
}Node;

Mutex mutex;
pthread_cond_t mycond = PTHREAD_COND_INITIALIZER;

Node *CreatNode(int data)
{
	Node *NewNode=(Node *)malloc(sizeof(Node));
	if(NULL == NewNode)
	{
		perror("malloc");
		return NULL;
	}
	NewNode->data=data;
	NewNode->next=NULL;
	return NewNode;
}

void InitLink(Node **head)
{
	*head=CreatNode(0);
}

int IsEmpty(Node *head)
{
	assert(head);
	if(head->next)
		return 0;    //not empty
	else
		return 1;    //empty
}

void PushFront(Node *head,int data)
{
	assert(head);
	Node *NewNode=CreatNode(data);
	NewNode->next=head->next;
	head->next=NewNode;
}

void PopFront(Node *head,int *data)
{
	assert(data);
	assert(head);
	if(IsEmpty(head))
	{
		printf("empty link\n");
		return ;
	}
	Node *del=head->next;
	*data=del->data;
	head->next=del->next;
	free(del);
	del=NULL;
}

void DisplayLink(Node *head)
{
	assert(head);
	Node *cur=head->next;
	while(cur)
	{
		printf("%d ",cur->data);
		cur=cur->next;
	}
	printf("\n");
}

void DestroyLink(Node *head)
{
	int data=0;
	assert(head);
	while(!IsEmpty(head))
	{
		PopFront(head,&data);
	}
	free(head);
}

void *product_run(void *arg)
{
	int data=0;
	Node *head=(Node *)arg;
	while(1)
	{
		usleep(100000);
		data=rand()%1000;
		mutex.Lock();
		PushFront(head,data);
		mutex.Unlock();
		pthread_cond_signal(&mycond);
		printf("product is done,data=%d\n",data);
	}
}

void *consumer_run(void *arg)
{
	int data=0;
	Node *head=(Node *)arg;
	while(1)
	{
		mutex.Lock();
		while(IsEmpty(head))
		{
			pthread_cond_wait(&mycond, &mutex.m_mutex);
		}
		PopFront(head,&data);
		mutex.Unlock();
		printf("consumer is done,data=%d\n",data);
	}
}

void testprocon()
{
	
	Node *head=NULL;
	InitLink(&head);
	pthread_t tid1;
	pthread_t tid2;
	pthread_create(&tid1,NULL,product_run,(void *)head);
	pthread_create(&tid2,NULL,consumer_run,(void *)head);

	pthread_join(tid1,NULL);
	pthread_join(tid2,NULL);
	DestroyLink(head);
	//pthread_cond_destroy(&mycond);

}

int main()
{
	testprocon();
	//Sem sem;
	//sem.Wait();
	//sem.Post();
	return 0;
}
