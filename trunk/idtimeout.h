//======================================================================
//
// idtimeout.h
//
// NOTE: This is a timeout controller implement with ANSI C
// By Linwei 12/4/2004
//
//======================================================================

#ifndef __I_DTIMEOUT_H__
#define __I_DTIMEOUT_H__

#include "impoold.h"


struct IDTIMEN				// ��ʱ���ڵ�
{
	struct IDTIMEN *next;	// ��һ���ڵ�
	struct IDTIMEN *prev;	// ��һ���ڵ�
	long time;				// �趨��ʱ��
	long data;				// �û�����
	long node;				// �ڵ��¼
};

struct IDTIMEV				// ʱ�������
{
	struct IMPOOL pnodes;	// �ڵ������
	struct IDTIMEN *head;	// ����ͷ���ڵ�
	struct IDTIMEN *tail;	// ����β���ڵ�
	long timeout;			// ʱ������
	long wtime;				// ��ǰʱ��
};

// ��ʼ����ʱ������
void idt_init(struct IDTIMEV *idtime, long timeout, struct IALLOCATOR *allocator);

// ���ٳ�ʱ������
void idt_destroy(struct IDTIMEV *idtime);

// ���õ�ǰʱ��
int idt_settime(struct IDTIMEV *idtime, long time);

// �½���ʱ��
int idt_newtime(struct IDTIMEV *idtime, long data);

// �Ƴ���ǰ��ʱ��
int idt_remove(struct IDTIMEV *idtime, int n);

// ���ǰ��ʱ��
int idt_active(struct IDTIMEV *idtime, int n);

// ��õ�ǰ��ɵļ�ʱ��
int idt_timeout(struct IDTIMEV *idtime, int *nodev, long *datav);


#endif

