//=====================================================================
//
// TML <Transmod Library>, by skywind 2004, itranspoc.h
//
// HISTORY:
// Dec. 25 2004   skywind  created and implement tcp operation
// Aug. 19 2005   skywind  implement udp operation
// Oct. 27 2005   skywind  interface add set nodelay in SYSCD
// Nov. 25 2005   skywind  extend connection close status
// Dec. 02 2005   skywind  implement channel timer event
// Dec. 17 2005   skywind  implement ioctl event
// Apr. 27 2010   skywind  fixed: when sys-timer changed, maybe error
// Mar. 15 2011   skywind  64bit support, header size configurable
// Jun. 25 2011   skywind  implement channel subscribe
// Sep. 09 2011   skywind  new: socket buf resize, congestion ctrl.
// Nov. 30 2011   skywind  new: channel broadcasting (v2.40)
//
// NOTES�� 
// ���紫��� TML<����ģ��>������ �ͻ�/Ƶ����ͨ��ģʽ���ṩ���ڶ�Ƶ��
// multi-channelͨ�ŵ� TCP/UDPͨ�Ż��ƣ�����/�ڴ��������ʱ���Ƶȷ���
//
//=====================================================================

#include "itransmod.h"


//=====================================================================
// Event Handlers
//=====================================================================


//---------------------------------------------------------------------
// �������������¼�
//---------------------------------------------------------------------
int itm_event_accept(int hmode) 
{
	struct sockaddr remote;
	struct sockaddr_in *addr;
	struct ITMD *itmd, *channel;
	int host = (hmode == ITMD_OUTER_HOST)? itm_outer_sock : itm_inner_sock;
	int sock = -1, node = 0, retval, r1, r2, result = 0;
	unsigned long noblock = 1;
	unsigned long revalue = 1;
	unsigned long bufsize = 0;
	long sockrcv, socksnd;
	
	sock = apr_accept(host, &remote);

	if (sock < 0) {
		itm_log(ITML_ERROR, 
			"[ERROR] can not accept new %s connection errno=%d", 
			(hmode == ITMD_OUTER_HOST)? "user" : "channel", apr_errno());
		return -1;
	}
	if (hmode == ITMD_OUTER_HOST) {
		if (itm_outer_cnt >= itm_outer_max) {
			apr_close(sock);
			itm_log(ITML_ERROR, 
				"[ERROR] connection refused: max connection riched limit %d", 
				itm_outer_max);
			return -2;
		}
	}	else {
		if (itm_inner_cnt >= itm_inner_max) {
			apr_close(sock);
			itm_log(ITML_ERROR, 
				"[ERROR] connection refused: max connection riched limit %d", 
				itm_inner_max);
			return -2;
		}
		if (itm_validate) {
			retval = itm_validate(&remote);
			if (retval != 0) {
				apr_close(sock);
				itm_log(ITML_ERROR, 
					"[ERROR] connection refused: invalid ip for %s code=%d",
					itm_epname(&remote), retval);
				return -2;
			}
		}
	}

	node = imp_newnode(&itm_fds);
	if (node < 0 || node >= 0x10000) {
		apr_close(sock);
		if (node >= 0) imp_delnode(&itm_fds, node);
		itm_log(ITML_ERROR, 
			"[ERROR] connection refused: error allocate connection descripter");
		return -3;
	}

	apr_ioctl(sock, FIONBIO, &noblock);
	apr_setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&revalue, sizeof(revalue));

	if (hmode == ITMD_OUTER_HOST) {
		socksnd = itm_socksndo;
		sockrcv = itm_sockrcvo;
	}	else {
		socksnd = itm_socksndi;
		sockrcv = itm_sockrcvi;
	}

	if (socksnd > 0) {
		bufsize = (unsigned long)socksnd;
		apr_setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&bufsize, sizeof(bufsize));
	}

	if (sockrcv > 0) {
		bufsize = (unsigned long)sockrcv;
		apr_setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&bufsize, sizeof(bufsize));
	}

	itmd = (struct ITMD*)IMP_DATA(&itm_fds, node);
	itmd->mode = (hmode == ITMD_OUTER_HOST)? ITMD_OUTER_CLIENT : ITMD_INNER_CLIENT;
	itmd->node = node;
	itmd->fd = sock;
	itmd->tag = -1;
	itmd->mask = 0;
	itmd->hid = (((++itm_counter) & 0x7fff) << 16) | (node & 0xffff);
	itmd->channel = (hmode == ITMD_OUTER_HOST)? 0 : -1;
	itmd->timeid = -1;
	itmd->initok = 0;
	itmd->ccnum = 0;
	itmd->inwlist = 0;

	if (hmode == ITMD_OUTER_HOST) itm_outer_cnt++;
	else itm_inner_cnt++;
	memcpy(&(itmd->remote), &remote, sizeof(remote));

	r1 = ims_init(&itmd->rstream, &itm_mem);
	r2 = ims_init(&itmd->wstream, &itm_mem);
	itmd->timeid = (hmode == ITMD_OUTER_HOST)? 
					idt_newtime(&itm_timeu, itmd->hid) : idt_newtime(&itm_timec, itmd->hid);

	if (itmd->timeid < 0 || r1 || r2) {
		itm_log(ITML_ERROR, "[ERROR] memory stream or collection set error for %s", 
			itm_epname(&remote));
		itm_event_close(itmd, 4001);
		return 0;
	}

	retval = apr_poll_add(itm_polld, itmd->fd, APOLL_IN, itmd);

	if (retval) {
		itm_log(ITML_ERROR, "[ERROR] poll add fd %d error for %s", 
			itmd->fd, itm_epname(&remote));
		itm_event_close(itmd, 4000);
		return 0;
	}

	iv_queue(&itmd->waitq);
	iv_qnode(&itmd->wnode, itmd);
	itm_mask(itmd, ITM_READ, 0);

	itmd->session = (unsigned long)((itm_wtime << 16) | itm_counter);
	itmd->touched = 0;
	itmd->dropped = 0;
	itmd->cnt_tcpr = 0;
	itmd->cnt_tcpw = 0;
	itmd->cnt_udpr = 0;
	itmd->cnt_udpw = 0;
	itmd->initok = 1;

	if (hmode == ITMD_OUTER_HOST) {
		channel = (struct ITMD*)itm_rchannel(0);
		if (channel == NULL) {
			itm_log(ITML_ERROR, "[ERROR] connection refused: channel 0 does not exist");
			itm_event_close(itmd, 2300);
		}	else {
			channel->ccnum++;
			itm_param_set(0, itm_headlen + 6, ITMT_NEW, itmd->hid, itmd->tag);
			addr = (struct sockaddr_in*)&remote;
			memcpy(itm_data + itm_headlen, &addr->sin_addr.s_addr, 4);
			itm_write_word(itm_data + itm_headlen + 4, (unsigned short)(addr->sin_port));
			itm_send(channel, itm_data, itm_headlen + 6);
			itm_log(ITML_INFO, "new user connected from %s hid=%XH", 
				itm_epname(&remote), itmd->hid);
			result = 1;
		}
		if (itm_headmsk && itm_booklen[0] > 0 && result == 1) {
			long len, i;
			len = itm_booklen[0];
			itm_param_set(0, itm_headlen + 6, ITMT_NEW, itmd->hid, itmd->tag);
			addr = (struct sockaddr_in*)&remote;
			memcpy(itm_data + itm_headlen, &addr->sin_addr.s_addr, 4);
			itm_write_word(itm_data + itm_headlen + 4, (unsigned short)(addr->sin_port));
			for (i = 0; i < len; i++) {
				int chid = itm_book[0][i];
				channel = (struct ITMD*)itm_rchannel(chid);
				if (channel != NULL && chid != 0) {
					itm_send(channel, itm_data, itm_headlen + 6);
				}
			}
		}
	}	else {
		result = 2;
		itm_log(ITML_INFO, "new channel connected from %s", itm_epname(&remote));
	}

	// ���UDP��������TOUCH�ź�
	if ((itm_udpmask & ITMU_MWORK) && result == 1) {
		*(apr_uint16*)(itm_data + 0) = ITMHTONS(16);
		*(apr_uint16*)(itm_data + 2) = ITMHTONS(ITMU_TOUCH);
		*(apr_uint32*)(itm_data + 4) = ITMHTONL((apr_uint32)(itmd->hid));
		*(apr_uint32*)(itm_data + 8) = ITMHTONL((apr_uint32)(itmd->session));
		*(apr_uint16*)(itm_data +12) = ITMHTONS((apr_uint16)itm_udpmask);
		*(apr_uint16*)(itm_data +14) = ITMHTONS((apr_uint16)itm_dgram_port);
		itm_send(itmd, itm_data, 16);
	}

	return 0;
}

//---------------------------------------------------------------------
// �����Ͽ��¼�
//---------------------------------------------------------------------
int itm_event_close(struct ITMD *itmd, int code)
{
	struct ITMD *channel;
	long mode;

	if (itmd->mode == ITMD_OUTER_CLIENT) itm_outer_cnt--;
	else if (itmd->mode == ITMD_INNER_CLIENT) itm_inner_cnt--;
	else {
		itm_log(ITML_ERROR, "[ERROR] error to close a listen socket");
		return -1;
	}
	mode = itmd->mode;
	itmd->mode = -1;

	if (itmd->initok) apr_poll_del(itm_polld, itmd->fd);
	if (itmd->rstream.pool) ims_destroy(&itmd->rstream);
	if (itmd->wstream.pool) ims_destroy(&itmd->wstream);
	if (itmd->timeid >= 0) {
		idt_remove((mode == ITMD_OUTER_CLIENT)? &itm_timeu : &itm_timec, itmd->timeid);
		itmd->timeid = -1;
	}

	if (itmd->initok) {
		if (mode == ITMD_OUTER_CLIENT) {
			// �ӵȴ��������Ƴ�
			if (itmd->wnode.queue) 
				iv_remove((struct IVQUEUE*)itmd->wnode.queue, &itmd->wnode);
			// TODO ����֪ͨchannel�Ĵ���
			channel = itm_rchannel(itmd->channel);
			if (channel) {	// �����뿪����
				channel->ccnum--;
				itm_param_set(0, itm_headlen + 4, ITMT_LEAVE, itmd->hid, itmd->tag);
				itm_write_dword(itm_data + itm_headlen, (apr_uint32)code);
				itm_send(channel, itm_data, itm_headlen + 4);
			}
			if (itm_headmsk && itm_booklen[0] > 0) {
				long i;
				itm_param_set(0, itm_headlen + 4, ITMT_LEAVE, itmd->hid, itmd->tag);
				itm_write_dword(itm_data + itm_headlen, (apr_uint32)code);
				for (i = 0; i < itm_booklen[0]; i++) {
					long chid = itm_book[0][i];
					channel = itm_rchannel(chid);
					if (channel != NULL && chid != itmd->channel) {
						itm_send(channel, itm_data, itm_headlen + 4);
					}
				}
			}
			itm_log(ITML_INFO, "closed user %s hid=%XH channel=%d drop=%d code=%d", 
				itm_epname(&itmd->remote), itmd->hid, itmd->channel, itmd->dropped, code);
		}	else {
			// �������пͻ��Ķ��¼������ᵼ�¿ͻ��Ͽ�
			while (itmd->waitq.nodecnt) itm_permitr(itmd);
			// TODO ������Ƶ��
			if (itmd->channel >= 0) {
				itm_wchannel(itmd->channel, NULL);
				itm_book_reset(itmd->channel);
			}
			// TODO ֪ͨ0��channel��channel�Ͽ�
			channel = itm_rchannel(0);
			if (channel) {
				itm_param_set(0, itm_headlen + 4, ITMT_CHSTOP, itmd->channel, itmd->tag);
				itm_write_dword(itm_data + itm_headlen, (apr_uint32)code);
				itm_send(channel, itm_data, itm_headlen + 4);
			}
			itm_log(ITML_INFO, "closed channel %s hid=%XH channel=%d code=%d", 
				itm_epname(&itmd->remote), itmd->hid, itmd->channel, code);
		}
	}
	apr_shutdown(itmd->fd, 2);
	apr_close(itmd->fd);
	imp_delnode(&itm_fds, itmd->node);
	itmd->hid = -1;
	itmd->node = -1;
	itmd->mode = -1;

	return 0;
}

//---------------------------------------------------------------------
// ���������¼�
//---------------------------------------------------------------------
int itm_event_send(struct ITMD *itmd)
{
	long size = 0;
	
	switch (itmd->mode)
	{
	case ITMD_OUTER_CLIENT:
		size = itm_trysend(itmd->fd, &itmd->wstream);
		if (size < 0) {
			itm_log(ITML_INFO, "closing connection for remote lost: %d", 
				(itm_error == IEAGAIN)? 0 : itm_error);
			itm_event_close(itmd, itm_error);
			break;
		}
		if (itmd->wstream.size == 0) {
			if ((itmd->mask & APOLL_OUT) != 0) 
				itm_mask(itmd, 0, ITM_WRITE);
		}	else {
			if ((itmd->mask & APOLL_OUT) == 0)
				itm_mask(itmd, ITM_WRITE, 0);
		}
		if (size > 0 && itm_utiming == 0) idt_active(&itm_timeu, itmd->timeid);
		break;

	case ITMD_INNER_CLIENT:
		size = itm_trysend(itmd->fd, &itmd->wstream);
		if (size < 0) {
			itm_log(ITML_INFO, "closing connection for remote lost: %d", 
				(itm_error == IEAGAIN)? 0 : itm_error);
			itm_event_close(itmd, itm_error);
			break;
		}
		if (itmd->wstream.size == 0) {
			if ((itmd->mask & APOLL_OUT) != 0) 
				itm_mask(itmd, 0, ITM_WRITE);
		}	else {
			if ((itmd->mask & APOLL_OUT) == 0)
				itm_mask(itmd, ITM_WRITE, 0);
		}
		if (size > 0) {
			idt_active(&itm_timec, itmd->timeid);
			if (itmd->wstream.size < itm_inner_blimit && itmd->waitq.nodecnt)
				itm_permitr(itmd);
		}
		break;

	case ITMD_DGRAM_HOST:
		itm_trysendto();
		if (itm_dgramdat.size == 0) itm_mask(NULL, 0, ITM_WRITE);
		break;
	}

	return 0;
}

//---------------------------------------------------------------------
// ���������¼�
//---------------------------------------------------------------------
int itm_event_recv(struct ITMD *itmd) 
{
	struct ITMD *channel;
	long retval = 0;
	long length = 0;
	
	if (itmd->mode == ITMD_OUTER_CLIENT) {
		// ����Ѿ��ٵȴ������о;�ֹ���¼����˳�
		if (itmd->wnode.queue) { 
			itm_mask(itmd, 0, ITM_READ);
			return 0;
		}
		channel = itm_rchannel(itmd->channel);
		if (channel == NULL) {
			itm_log(ITML_WARNING, 
				"[WARNING] data refused: channel %d does not exist", 
				itmd->channel);
			itm_event_close(itmd, 2302);
			return 0;
		}
		// Ƶ��д�����Ѿ�������ֹ������������ȴ�����
		if (channel->wstream.size > itm_inner_blimit) {
			itm_mask(itmd, 0, ITM_READ);
			if (itmd->wnode.queue == NULL) 
				iv_tailadd(&channel->waitq, &itmd->wnode);
			if (itm_logmask & ITML_DATA) {
				itm_log(ITML_DATA, 
					"channel buffer full disable read hid=%XH channel=%d", 
					itmd->hid, itmd->channel);
			}
			return 0;
		}
		// ����仹��ĳ��channel�ĵȴ���������Ļ���Ҳ��ͣ����Ϣ
		if (itmd->wnode.queue != NULL) {
			itm_mask(itmd, 0, ITM_READ);
			return 0;
		}
	}

	retval = itm_tryrecv(itmd->fd, &itmd->rstream);
	if (retval == 0) return 0;

	if (retval < 0) {
		itm_log(ITML_INFO, "remote socket disconnected: %d", 
			(itm_error == IEAGAIN)? 0 : itm_error);
		itm_event_close(itmd, itm_error);
		return 0;
	}

	// ���ǰ�ĳ�ʱ����
	idt_active((itmd->mode == ITMD_OUTER_CLIENT)? &itm_timeu : &itm_timec, itmd->timeid);

	// �����������������а�
	for (;;) {
		length = itm_dataok(&itmd->rstream);
		if (length == 0) break;
		if (length < 0) { 
			itm_log(ITML_INFO, "connection data error %s hid=%XH channel=%d", 
				itm_epname(&itmd->remote), itmd->hid, itmd->channel);
			itm_event_close(itmd, 2001); 
			break; 
		}
		if (length > itm_datamax) {
			itm_log(ITML_INFO, "data length is too long %s hid=%HX channel=%d",
				itm_epname(&itmd->remote), itmd->hid, itmd->channel);
			itm_event_close(itmd, 2002);
			break;
		}
		ims_read(&itmd->rstream, itm_data, length);
		if (itmd->mode == ITMD_OUTER_CLIENT) 
			retval = itm_data_outer(itmd);
		else 
			retval = itm_data_inner(itmd);
		if (retval < 0) { 
			itm_event_close(itmd, 2001); 
			break; 
		}
	}

	return 0;
}

//---------------------------------------------------------------------
// ��ȡ���ݱ�
//---------------------------------------------------------------------
int itm_event_dgram(void)
{
	struct sockaddr remote;
	struct ITMHUDP *head;
	long size;

	head = (struct ITMHUDP*)itm_zdata;
	for (size = 1; size > 0; ) {
		size = apr_recvfrom(itm_dgram_sock, itm_zdata, ITM_BUFSIZE, 0, &remote);
		if (size >= 16) {
			head->order = (unsigned long)ITMNTOHL(head->order);
			head->index = (unsigned long)ITMNTOHL(head->index);
			head->hid = (long)ITMNTOHL(head->hid);
			head->session = (long)ITMNTOHL(head->session);

			if (head->order < 0x80000000) {
				itm_dgram_data(&remote, head, itm_zdata + 16, size - 16);
			}	else {
				itm_dgram_cmd(&remote, head, itm_zdata + 16, size - 16);
			}
		}	else
		if (size > 0 && (itm_logmask & ITML_LOST)) {
			itm_log(ITML_LOST, "dgram data format error from ", itm_epname(&remote));
		}
	}
	return 0;
}


//---------------------------------------------------------------------
// ���ݼ��ܺ�IP��֤ģ�麯��ָ��
//---------------------------------------------------------------------
int (*itm_encrypt)(void *output, const void *input, int length, int fd, int mode);	
int (*itm_validate)(const void *sockaddr);

//---------------------------------------------------------------------
// �յ�һ���������ⲿ��
//---------------------------------------------------------------------
int itm_data_outer(struct ITMD *itmd)
{
	long length;
	struct ITMD *channel;
	int category;

	channel = itm_rchannel(itmd->channel);
	if (channel == NULL) {
		itm_log(ITML_WARNING, 
			"[WARNING] data refused: channel %d does not exist", itmd->channel);
		return -1;
	}

	length = itm_size_get(itm_data);
	category = -1;

	if (itm_headmsk) {
		category = itm_cate_get(itm_data);
	}

	// ���û�м�����ֱ��ת��
	itm_param_set(length, (unsigned short)(length + itm_headlen - itm_hdrsize), 
		ITMT_DATA, itmd->hid, itmd->tag);

	// û�����ù��������Ϣ�ŷ��͵�ԭƵ��
	if (category <= 0) {
		itm_send(channel, itm_data + length, itm_headlen);
		itm_send(channel, itm_data + itm_hdrsize, length - itm_hdrsize);
		if (itm_logmask & ITML_DATA) {
			itm_log(ITML_DATA, "recv %d bytes data from %s hid=%XH channel=%d", 
				length, itm_epname(&itmd->remote), itmd->hid, itmd->channel);
		}
	}

	// ���Ƶ������������������������������Ķ��¼�
	if (channel->wstream.size < itm_inner_blimit) itm_permitr(channel);

	// �����հ�������
	ITMDINCD(itmd->cnt_tcpr);

	// ����ȫ��ͳ��
	itm_stat_recv++;

	// ���͵������˸���Ϣ��Ƶ��
	if (itm_headmsk && category > 0 && category < 255) {
		short *book = itm_book[category];
		int booklen = itm_booklen[category];
		int chid, i;
		for (i = 0; i < booklen; i++) {
			chid = book[i];
			channel = itm_rchannel(chid);
			if (channel) {
				// Ƶ��д�����Ѿ�������ֹ������������ȴ�����
				if (channel->wstream.size > itm_inner_blimit) {
					itm_mask(itmd, 0, ITM_READ);
					if (itmd->wnode.queue == NULL) 
						iv_tailadd(&channel->waitq, &itmd->wnode);
					if (itm_logmask & ITML_DATA) {
						itm_log(ITML_DATA, 
							"channel buffer full disable read hid=%XH channel=%d dstch=%d", 
							itmd->hid, itmd->channel, channel->channel);
					}
					return 0;
				}
				itm_send(channel, itm_data + length, itm_headlen);
				itm_send(channel, itm_data + itm_hdrsize, length - itm_hdrsize);
				// ���Ƶ������������������������������Ķ��¼�
				if (channel->wstream.size < itm_inner_blimit) itm_permitr(channel);
			}
		}
	}

	return 0;
}

//---------------------------------------------------------------------
// �յ�һ���������ڲ���
//---------------------------------------------------------------------
int itm_data_inner(struct ITMD *itmd)
{
	long event, length = 0, retval;
	long wparam = 0, lparam = 0;
	short cmd = 0;

	// ����Ƶ����½
	if (itmd->channel < 0) {
		retval = itm_on_logon(itmd);
		return retval;
	}

	// ��ȡ������Ϣ
	itm_param_get(itm_data, &length, &cmd, &wparam, &lparam);

	event = cmd;
	retval = 0;

	// ����Ƶ�����͵ĸ�������
	switch (event)
	{
	case ITMC_DATA:		// �����������ݲ���
		retval = itm_on_data(itmd, wparam, lparam, length);
		break;

	case ITMC_CLOSE:	// �����Ͽ����Ӳ���
		retval = itm_on_close(itmd, wparam, lparam, length);
		break;

	case ITMC_TAG:		// ��������TAG
		retval = itm_on_tag(itmd, wparam, lparam, length);
		break;

	case ITMC_CHANNEL:	// �������ͨ��
		retval = itm_on_channel(itmd, wparam, lparam, length);
		break;

	case ITMC_MOVEC:	// �����ƶ�ITMD
		retval = itm_on_movec(itmd, wparam, lparam, length);
		break;
	
	case ITMC_UNRDAT:	// �������ݱ�
		retval = itm_on_dgram(itmd, wparam, lparam, length);
		break;
	
	case ITMC_IOCTL:	// ���ӿ���ָ��
		retval = itm_on_ioctl(itmd, wparam, lparam, length);
		break;

	case ITMC_SYSCD:	// ����ϵͳ��Ϣ
		retval = itm_on_syscd(itmd, wparam, lparam, length);
		break;
	
	case ITMC_BROADCAST:	// �������ݹ㲥
		retval = itm_on_broadcast(itmd, wparam, lparam, length);
		break;

	case ITMC_NOOP:		// ��������Ϣ
		itm_param_set(0, length, ITMT_NOOP, wparam, lparam);
		itm_send(itmd, itm_data, length);
		break;

	default:			// ����Ĭ����Ϣ
		itm_log(ITML_WARNING, "[WARNING] channel %d send unknow command [%d]", 
			itmd->channel, event);
		break;
	}

	return retval;
}

//---------------------------------------------------------------------
// Ƶ���������Ƶ����½
//---------------------------------------------------------------------
int itm_on_logon(struct ITMD *itmd)
{
	struct ITMD *channel;
	long length, c, i;

	length = itm_size_get(itm_data);

	if (length != itm_hdrsize + 2) {
		itm_log(ITML_WARNING, 
			"[WARNING] channel cannot start, channel id is unknow");
		return -1;
	}

	c = itm_read_word(itm_data + itm_hdrsize);

	if (c == 0xffff) {
		if (itm_dhcp_index > itm_dhcp_high) itm_dhcp_index = itm_dhcp_base;
		if (itm_dhcp_index < itm_dhcp_base) itm_dhcp_index = itm_dhcp_base;
		for (i = itm_dhcp_base; i < itm_dhcp_high; i++) {
			int index = itm_dhcp_index++;
			if (itm_dhcp_index >= itm_dhcp_high) 
				itm_dhcp_index = itm_dhcp_base;
			if (itm_rchannel(index) == NULL) {
				c = index;
				break;
			}
		}
	}

	if (itm_rchannel(c)) {
		itm_log(ITML_WARNING, 
			"[WARNING] closing connection because channel %d is already used: ", c);
		return -2;
	}

	if (itm_wchannel(c, itmd)) {
		itm_log(ITML_WARNING, "[WARNING] can not set channel to channel %d", c);
		return -3;
	}
	itmd->channel = c;

	itm_log(ITML_INFO, "channel %d started from connection %s hid=%XH", 
		c, itm_epname(&itmd->remote), itmd->hid);

	channel = itm_rchannel(0);
	if (channel && c != 0) {	// �������Ƶ����֪ͨͨ�� 0
		itm_param_set(0, itm_headlen, ITMT_CHNEW, c, itmd->hid);
		itm_send(channel, itm_data, itm_headlen);
	}
	if (c == 0) {				// �����Ƶ�� 0��λƵ����Ϣ
		itm_interval = 0;
		itm_notice_slap = 0;
		itm_notice_cycle = -1;
		itm_notice_count = 0;
	}

	return 0;
}

//---------------------------------------------------------------------
// Ƶ������������ݵ��û�
//---------------------------------------------------------------------
int itm_on_data(struct ITMD *itmd, long wparam, long lparam, long length)
{
	struct ITMD *to = itm_hid_itmd(wparam);
	long dlength = length - itm_headlen;
	int cansend = 1;
	char *ptr;

	if (to == NULL) { 
		if (itm_logmask & ITML_WARNING) {
			itm_log(ITML_WARNING, 
				"[WARNING] cannot send data to hid %XH from channel %d for hid error", 
				wparam, itmd->channel);
		}
		return 0;
	}
	if (to->channel != itmd->channel && itmd->channel != 0 && itm_headmsk == 0) { 
		if (itm_logmask & ITML_WARNING) {
			itm_log(ITML_WARNING, 
				"[WARNING] cannot send data to hid %XH from channel %d", 
				wparam, itmd->channel);
		}
		return 0;
	}
	if (to->mode != ITMD_OUTER_CLIENT) {
		if (itm_logmask & ITML_WARNING) {
			itm_log(ITML_WARNING,
				"[WARNING] cannot send data to hid %XH from channel %d for mode error",
				wparam, itmd->channel);
		}
		return 0;
	}

	// ���������ӷ�������
	ptr = itm_data + itm_headlen - itm_hdrsize;
	itm_size_set(ptr, dlength + itm_hdrsize);

	// ���Ҫ�����жϴ�С�پ������͵Ļ�
	if (lparam & 0x40000000) {
		long limit = lparam & 0x3fffffff;
		if (to->wstream.size >= limit) cansend = 0;		// ���ý�ֹ����
	}

	// ������Է��ͣ�����Ҫ�жϻ����С�����жϳɹ�
	if (cansend) {
		itm_send(to, ptr, dlength + itm_hdrsize);
		if (itm_logmask & ITML_DATA) {
			itm_log(ITML_DATA, "channel %d send %ld bytes data to %s hid=%XH", 
				itmd->channel, dlength + itm_hdrsize, itm_epname(&to->remote), to->hid);
		}
		itm_bcheck(to);
		itm_stat_send++;
	}	else {
		if (itm_logmask & ITML_DATA) {
			itm_log(ITML_DATA, "channel %d discard sending data to %s hid=%XH",
				itmd->channel, itm_epname(&to->remote), to->hid);
		}
		itm_stat_discard++;
	}

	return 0;
}


//---------------------------------------------------------------------
// Ƶ������ر��û�����
//---------------------------------------------------------------------
int itm_on_close(struct ITMD *itmd, long wparam, long lparam, long length)
{
	struct ITMD *to = itm_hid_itmd(wparam);

	if (to == NULL) { 
		itm_log(ITML_WARNING, 
			"[WARNING] cannot close hid %XH from channel %d for hid error", 
			wparam, itmd->channel);
		return 0;
	}
	if (to->channel != itmd->channel && itmd->channel != 0 && itm_headmsk == 0) {
		itm_log(ITML_WARNING, 
			"[WARNING] cannot close hid %XH from channel %d for hid not in channel", 
			wparam, itmd->channel);
		return 0;
	}
	itm_log(ITML_INFO, "channel %d closing user %s hid=%XH: %d", 
		itmd->channel, itm_epname(&to->remote), to->hid, lparam);

	itm_event_close(to, lparam);

	wparam = wparam;
	lparam = lparam;
	length = length;

	return 0;
}

//---------------------------------------------------------------------
// Ƶ��������ñ�ǩ
//---------------------------------------------------------------------
int itm_on_tag(struct ITMD *itmd, long wparam, long lparam, long length)
{
	struct ITMD *to = itm_hid_itmd(wparam);
	if (to == NULL) { 
		itm_log(ITML_WARNING, 
			"[WARNING] cannot set tag to hid %XH from channel %d for hid error", 
			wparam, itmd->channel);
		return 0; 
	}
	if (to->channel != itmd->channel && itmd->channel != 0) { 
		itm_log(ITML_WARNING, 
			"[WARNING] cannot set tag to hid %XH from channel %d form hid not in channel", 
			wparam, itmd->channel);
		return 0; 
	}
	to->tag = lparam;
	itm_log(ITML_DATA, "channel %d set tag to %d for %s hid=%XH: ", 
		itmd->channel, lparam, itm_epname(&to->remote), wparam);

	wparam = wparam;
	lparam = lparam;
	length = length;

	return 0;
}


//---------------------------------------------------------------------
// Ƶ�����Ƶ����ͨ��
//---------------------------------------------------------------------
int itm_on_channel(struct ITMD *itmd, long wparam, long lparam, long length)
{
	struct ITMD *channel = itm_rchannel(wparam);
	struct IDTIMEN *tmnode;
	long c;

	if (channel == NULL && wparam >= 0) { 
		itm_log(ITML_WARNING, 
			"[WARNING] channel %d cannot send data to channel %d for channel error", 
			itmd->channel, wparam);
		return 0; 
	}

	itm_write_dword(itm_data + itm_hdrsize + 2, (apr_uint32)itmd->channel);
	itm_write_dword(itm_data + itm_hdrsize + 6, (apr_uint32)itmd->tag);

	// �ж��ǵ�������Ⱥ��
	if (wparam >= 0) {	// ����ǵ���
		if (itm_logmask & ITML_CHANNEL) {
			itm_log(ITML_CHANNEL, 
				"channel %d send %d bytes data to channel %d", 
				itmd->channel, length, wparam);
		}
		itm_send(channel, itm_data, length);
	}	else {			// �����Ⱥ��
		for (tmnode = itm_timec.tail, c = 0; tmnode; tmnode = tmnode->next) {
			channel = itm_hid_itmd(tmnode->data);
			if (channel != NULL) {
				if (channel->channel != itmd->channel) {
					itm_send(channel, itm_data, length);
					c++;
				}
			}	else {
				itm_log(ITML_ERROR, 
					"[ERROR] list channel error in time queue hid=%XH", 
					tmnode->data);
			}
		}
		if (itm_logmask & ITML_CHANNEL) {
			itm_log(ITML_CHANNEL, "channel %d send %d bytes data to %d channels", 
				itmd->channel, length, c);
		}
	}

	lparam = lparam;

	return 0;
}

//---------------------------------------------------------------------
// Ƶ������ƶ��û����ӵ�ĳƵ��
//---------------------------------------------------------------------
int itm_on_movec(struct ITMD *itmd, long wparam, long lparam, long length)
{
	struct ITMD *channel = itm_rchannel(wparam);
	struct ITMD *to = itm_hid_itmd(lparam);
	struct ITMD *ochannel;

	if (to == NULL) { 
		itm_log(ITML_WARNING, 
			"[WARNING] channel %d can not move user connection to channel %d for hid %XH error", 
			itmd->channel, wparam, lparam);
		return 0;
	}
	if (channel == NULL) { 
		itm_log(ITML_WARNING, 
			"[WARNING] channel %d can not move user connection to channel %d for channel %d does not exist", 
			itmd->channel, wparam, wparam);
		itm_event_close(to, 0);
		return 0;
	}
	if (to->channel != itmd->channel && itmd->channel != 0) {
		itm_log(ITML_WARNING, 
			"[WARNING] channel %d can not move user connection to channel %d for hid %XH does not belong to this channel", 
			itmd->channel, wparam, lparam);
		return 0; 
	}
	if (to->mode != ITMD_OUTER_CLIENT) {
		itm_log(ITML_WARNING, 
			"[WARNING] can not move user connection for hid=%XH is not a user connection", to->hid);
		return 0;
	}

	// ���������ĳ�ȴ�������
	if (to->wnode.queue) {	
		iv_remove((struct IVQUEUE*)to->wnode.queue, &to->wnode);
		itm_mask(to, ITM_READ, 0);
	}

	ochannel = itm_rchannel(to->channel);
	if (ochannel) ochannel->ccnum--;
	to->channel = wparam;
	channel->ccnum++;

	if (itm_logmask & ITML_DATA) {
		itm_log(ITML_DATA, 
			"channel %d move user connection hid=%XH to channel %d", 
			itmd->channel, lparam, wparam);
	}
	itm_param_set(0, length, ITMT_NEW, to->hid, to->tag);
	itm_send(channel, itm_data, length);

	return 0;
}


//---------------------------------------------------------------------
// Ƶ������������ݱ�
//---------------------------------------------------------------------
int itm_on_dgram(struct ITMD *itmd, long wparam, long lparam, long length)
{
	struct ITMD *to = itm_hid_itmd(wparam);
	long dlength = length - itm_headlen;

	if (to == NULL) { 
		if (itm_logmask & ITML_WARNING) {
			itm_log(ITML_WARNING, 
				"[WARNING] cannot send dgram to hid %XH from channel %d for hid error", 
				wparam, itmd->channel);
		}
		return 0;
	}
	if (to->channel != itmd->channel && itmd->channel != 0) { 
		if (itm_logmask & ITML_WARNING) {
			itm_log(ITML_WARNING, 
				"[WARNING] cannot send dgram to hid %XH from channel %d", 
				wparam, itmd->channel);
		}
		return 0;
	}
	if (to->touched == 0) {
		if (itm_logmask & ITML_WARNING) {
			itm_log(ITML_WARNING, 
				"[WARNING] cannot send dgram to hid %XH from channel %d for not touched", 
				wparam, itmd->channel);
		}
		return 0;
	}
	if ((itm_udpmask & ITMU_MWORK) == 0) {
		if (itm_logmask & ITML_WARNING) {
			itm_log(ITML_WARNING,
				"[WARNING] cannot send dgram to hid %XH from channel %d for udp disable",
				wparam, itmd->channel);
		}
		return 0;
	}

	// ���������ӷ�������
	if (itm_logmask & ITML_DATA) {
		itm_log(ITML_DATA, "channel %d send %d bytes dgram to %s hid=%XH", 
			itmd->channel, dlength + 2, itm_epname(&to->remote), to->hid);
	}
	itm_sendto(to, itm_data + itm_headlen, dlength);
	lparam = lparam;

	return 0;
}

//---------------------------------------------------------------------
// Ƶ�����ϵͳ������Ϣ
//---------------------------------------------------------------------
int itm_on_syscd(struct ITMD *itmd, long wparam, long lparam, long length)
{
	struct ITMD *channel, *ochannel;
	struct IDTIMEN *tmnode;
	int retval = 0, c;
	char *lptr;

	switch (wparam)
	{
	case ITMS_CONNC:
		itm_param_set(0, itm_headlen + 8, ITMT_SYSCD, ITMS_CONNC, 0);
		itm_write_dword(itm_data + itm_headlen + 0, (apr_uint32)itm_outer_cnt);
		itm_write_dword(itm_data + itm_headlen + 4, (apr_uint32)itm_inner_cnt);
		itm_send(itmd, itm_data, itm_headlen + 8);
		itm_log(ITML_CHANNEL, 
			"system info: %d user connection(s) and %d channel connection(s)", 
			itm_outer_cnt, itm_inner_cnt);
		break;

	case ITMS_LOGLV:
		itm_logmask = lparam;
		itm_log(ITML_INFO, "system info: set log mask to %XH", itm_logmask);
		break;

	case ITMS_LISTC:
		lparam = itm_timec.wtime + itm_timec.timeout;
		lparam = (lparam < 0)? 0x7fffffff : lparam;
		lptr = itm_data + itm_headlen;
		for (tmnode = itm_timec.tail, c = 0; tmnode; tmnode = tmnode->next) {
			channel = itm_hid_itmd(tmnode->data);
			if (channel) {
				wparam = lparam - tmnode->time;
				wparam = (wparam < 0)? 0 : ((wparam >= 0x10000)? 0xffff : wparam);
				itm_write_dword(lptr, (apr_uint32)channel->channel); lptr += 4;
				itm_write_dword(lptr, (apr_uint32)channel->hid); lptr += 4;
				itm_write_dword(lptr, (apr_uint32)channel->tag); lptr += 4;
				itm_write_dword(lptr, (apr_uint32)((wparam & 0xffff) | (channel->ccnum << 16))); lptr += 4;
				c++;
			}	else {
				itm_log(ITML_ERROR, "[ERROR] list channel error in time queue hid=%XH", 
					tmnode->data);
			}
		}
		itm_param_set(0, (lptr - itm_data), ITMT_SYSCD, ITMS_LISTC, c);
		itm_send(itmd, itm_data, lptr - itm_data);
		itm_log(ITML_CHANNEL, "system info: list info for %d channel(s)", c);
		break;

	case ITMS_RTIME:
		itm_param_set(0, itm_headlen, ITMT_SYSCD, ITMS_RTIME, itm_wtime);
		itm_send(itmd, itm_data, itm_headlen);
		itm_log(ITML_INFO, "system info: service is running for %d second(s)", itm_wtime);
		break;

	case ITMS_TMVER:
		itm_param_set(0, itm_headlen, ITMT_SYSCD, ITMS_TMVER, ITMV_VERSION);
		itm_send(itmd, itm_data, itm_headlen);
		itm_log(ITML_INFO, "system info: transmod version is %x", ITMV_VERSION);
		break;

	case ITMS_REHID:
		ochannel = itm_rchannel(lparam);
		wparam = (ochannel)? ochannel->hid : -1;
		itm_param_set(0, itm_headlen, ITMT_SYSCD, ITMS_REHID, wparam);
		itm_send(itmd, itm_data, itm_headlen);
		itm_log(ITML_INFO, "system info: hid of channel %d is %XH", lparam, wparam);
		break;

	case ITMS_TIMER:
		if (itmd->channel != 0) {
			itm_log(ITML_WARNING, "system info: channel %d cannot access timer", itmd->channel);
			break;
		}
		if (lparam > 0) {
			itm_notice_count = 0;
			itm_notice_cycle = lparam;
			itm_notice_slap = itm_time_current;
			itm_notice_saved = itm_time_current;
			itm_log(ITML_INFO, "system info: channel 0 set timer on %dms", lparam);
		}	else {
			itm_notice_slap = 0;
			itm_notice_cycle = -1;
			itm_notice_count = 0;
			itm_log(ITML_INFO, "system info: channel 0 set timer off");
		}
		break;
	
	case ITMS_INTERVAL:
		if (itmd->channel != 0) {
			itm_log(ITML_WARNING, "system info: channel %d cannot change interval mode", itmd->channel);
			break;
		}
		itm_interval = lparam;
		itm_log(ITML_INFO, "system info: channel 0 set interval to %d", lparam);
		break;
	
	case ITMS_FASTMODE:
		if (itmd->channel != 0) {
			itm_log(ITML_WARNING, "system info: channel %d cannot set fast mode", itmd->channel);
			break;
		}
		itm_fastmode = lparam;
		itm_log(ITML_INFO, "system info: channel 0 set fast mode to %d", lparam);
		break;

	case ITMS_CHID:
		itm_param_set(0, itm_headlen, ITMT_SYSCD, ITMS_CHID, itmd->channel);
		itm_send(itmd, itm_data, itm_headlen);
		itm_log(ITML_INFO, "system info: channel %d get id", itmd->channel);
		break;
	
	case ITMS_BOOKADD:
		itm_book_add(lparam, itmd->channel);
		itm_log(ITML_INFO, "system info: channel %d subscribe add %d", itmd->channel, lparam);
		break;

	case ITMS_BOOKDEL:
		itm_book_del(lparam, itmd->channel);
		itm_log(ITML_INFO, "system info: channel %d subscribe del %d", itmd->channel, lparam);
		break;

	case ITMS_BOOKRST:
		itm_book_reset(itmd->channel);
		itm_log(ITML_INFO, "system info: channel %d subscribe reset", itmd->channel);
		break;

	case ITMS_QUITD:
		itm_log(ITML_INFO, "system info: channel %d asks to quit", itmd->channel);
		retval = -10;
		break;

	case ITMS_STATISTIC:
		{
			char text1[32];
			char text2[32];
			char text3[32];
			itm_lltoa(text1, itm_stat_send);
			itm_lltoa(text2, itm_stat_recv);
			itm_lltoa(text3, itm_stat_discard);
			itm_log(ITML_INFO, "system info: statistic send=%s recv=%s discard=%s",
				text1, text2, text3);
		}
		break;
	}

	return retval;
}


//---------------------------------------------------------------------
// ������������
//---------------------------------------------------------------------
int itm_dgram_data(struct sockaddr *remote, struct ITMHUDP *head, void *data, long size)
{
	struct ITMD *itmd;
	struct ITMD *channel;
	itmd = itm_hid_itmd(head->hid);
	if (itmd == NULL) {
		itm_log(ITML_LOST, "[DROP] dgram session error from %s", itm_epname(remote));
		return 0;
	}
	if (itmd->session != head->session) {
		itm_log(ITML_LOST, "[DROP] dgram session error for hid=%XH channel=%d",
			head->hid, itmd->channel);
		return 0;
	}
	channel = itm_rchannel(itmd->channel);
	if (channel == NULL) {
		itm_log(ITML_WARNING, 
			"[WARNING] dgram refused: channel %d does not exist", itmd->channel);
		return -1;
	}

	if (head->order < itmd->cnt_udpr && (itm_udpmask & ITMU_MDUDP)) {
		if (itm_logmask & ITML_LOST) {
			itm_log(ITML_LOST, "[DROP] dgram udp order error hid=%x channel=%x", 
				itmd->hid, itmd->channel);
		}
		itmd->dropped++;
		itm_dropped++;
		return -2;
	}

	itmd->cnt_udpr = (head->order > itmd->cnt_udpr)? head->order : itmd->cnt_udpr;
	
	if (head->index != itmd->cnt_tcpr && (itm_udpmask & ITMU_MDTCP)) {
		if (itm_logmask & ITML_LOST) {
			itm_log(ITML_LOST, "[DROP] dgram tcp index error hid=%x channel=%x", 
				itmd->hid, itmd->channel);
		}
		itmd->dropped++;
		itm_dropped++;
		return -3;
	}

	itm_param_set(0, (size + itm_headlen), ITMT_UNRDAT, itmd->hid, itmd->tag);
	itm_send(channel, itm_data, itm_headlen);
	itm_send(channel, data, size);

	if (itm_logmask & ITML_DATA) {
		itm_log(ITML_DATA, "recv %d bytes dgram from %s hid=%XH channel=%d", 
			size, itm_epname(&itmd->remote), itmd->hid, itmd->channel);
	}

	return 0;
}

//---------------------------------------------------------------------
// ������������
//---------------------------------------------------------------------
int itm_dgram_cmd(struct sockaddr *remote, struct ITMHUDP *head, void *data, long size)
{
	unsigned long sender, port;
	int cmd = head->order & 0x7fffffff;
	struct sockaddr_in *ep;
	struct ITMD *itmd;

	switch(cmd) 
	{
	case ITMU_TOUCH:
		itmd = itm_hid_itmd(head->hid);
		if (itmd == NULL) {
			itm_log(ITML_LOST, "[DROP] dgram touching failed to hid=%XH from %s",
				head->hid, itm_epname(remote));
			break;
		}
		if (itmd->session != head->session) {
			itm_log(ITML_LOST, "[DROP] dgram touching error to hid=%XH from %s",
			head->hid, itm_epname(remote));
			break;
		}
		memcpy(&(itmd->dgramp), remote, sizeof(struct sockaddr));
		itmd->touched = 1;
		itmd->dgramp = *remote;
		itm_sendudp(remote, head, data, size);
		itm_log(ITML_INFO, "dgram touched to hid=%XH from %s", head->hid, itm_epname(remote));
		break;
	case ITMU_ECHO:
		itm_sendudp(remote, head, data, size);
		break;
	case ITMU_MIRROR:
		itm_sendudp(remote, head, remote, sizeof(struct sockaddr));
		break;
	case ITMU_DELIVER:
		ep = (struct sockaddr_in*)remote;
		ep->sin_addr.s_addr = head->index;
		ep->sin_port = htons((unsigned short)(head->session));
		itm_sendudp(remote, NULL, data, size);
		break;
	case ITMU_FORWARD:
		ep = (struct sockaddr_in*)remote;
		sender = ep->sin_addr.s_addr;
		port = ntohs(ep->sin_port);
		ep->sin_addr.s_addr = head->index;
		ep->sin_port = htons((unsigned short)(head->session));
		head->index = sender;
		head->session = htons((unsigned short)(port));
		itm_sendudp(remote, head, data, size);
		break;
	default:
		break;
	}

	return 0;
}

//---------------------------------------------------------------------
// ���ӿ���ָ��
//---------------------------------------------------------------------
int itm_on_ioctl(struct ITMD *itmd, long wparam, long lparam, long length)
{
	struct ITMD *to = itm_hid_itmd(wparam);
	long valued = (lparam >> 4);
	long retval;

	switch (lparam & 0x0f)
	{
	case ITMS_NODELAY:
		if (to == NULL) {
			itm_log(ITML_WARNING, "[WARNING] channel %d cannot set nodelay to hid=%XH",
				itmd->channel, wparam);
			break;
		}
		valued = (valued)? 1 : 0;
		retval = apr_setsockopt(to->fd, IPPROTO_TCP, TCP_NODELAY, (char*)&valued, sizeof(valued));
		if (itm_logmask & ITML_DATA) {
			itm_log(ITML_DATA, "ioctl: channel %d set nodelay %d to hid=%XH result=%d",
					itmd->channel, valued, wparam, retval);
		}
		break;

	case ITMS_NOPUSH:
		if (to == NULL) {
			itm_log(ITML_WARNING, "[WARNING] channel %d cannot set nopush to hid=%XH",
				itmd->channel, wparam);
			break;
		}
		if (valued) retval = apr_enable(to->fd, APR_NOPUSH);
		else retval = apr_disable(to->fd, APR_NOPUSH);
		if (itm_logmask & ITML_DATA) {
			itm_log(ITML_DATA, "ioctl: channel %d set nopush %d to hid=%XH result=%d",
					itmd->channel, valued, wparam, retval);
		}
		break;
	}

	return 0;
}


//---------------------------------------------------------------------
// �������ݹ㲥
//---------------------------------------------------------------------
int itm_on_broadcast(struct ITMD *itmd, long wparam, long lparam, long length)
{
	apr_int64 stat_send = itm_stat_send;
	apr_int64 stat_discard = itm_stat_discard;
	long dlength = length - itm_headlen;
	long newsize = 0;
	long count = wparam;
	long success = 0;
	long discard = 0;
	char *ptr;
	long i;

	if (dlength < count * 4) {
		if (itm_logmask & ITML_WARNING) {
			itm_log(ITML_WARNING, 
				"[WARNING] broadcast data format error for channel %d",
				itmd->channel);
		}
		return 0;
	}

	newsize = length - count * 4;
	ptr = itm_data + newsize;

	for (i = 0; i < count; i++) {
		apr_uint32 hid;
		idecode32u_lsb(ptr, &hid);
		ptr += 4;
		itm_on_data(itmd, (long)hid, lparam, newsize);
	}

	success = (long)(itm_stat_send - stat_send);
	discard = (long)(itm_stat_discard - stat_discard);

	if (itm_logmask & ITML_CHANNEL) {
		itm_log(ITML_CHANNEL,
			"broadcast from channel %d: count=%ld success=%ld discard=%ld",
			itmd->channel, count, success, discard);
	}

	return 0;
}

