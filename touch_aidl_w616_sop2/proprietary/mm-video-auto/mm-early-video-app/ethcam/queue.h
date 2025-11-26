/*
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

  Copyright (c) 2010-2011, 2016, The Linux Foundation. All rights reserved.*/
#ifndef QUEUE_H
#define QUEUE_H

typedef struct Node
{
  void *element;
  struct Node *next;
} Node;

struct Queue
{
  Node *head;
  Node *tail;
  int  current_size;
};

typedef struct Queue Queue;

Queue *alloc_queue(void);
void free_queue(Queue *q);
void free_queue_and_qelement(Queue *q);
int push(Queue *q, void * element);
void *pop(Queue *q);

#endif
