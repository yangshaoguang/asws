//
//  ascs_web_socket.h
//  
//  Created by qiyun on 19-1-18.
//  Copyright (c) 2019 __jin10.com__. All rights reserved.
//

#ifndef __ws_server_h__
#define __ws_server_h__

#include "ws_config.h"
#include <ascs/ext/tcp.h>
#include "ws_packer.h"


using namespace ascs;
using namespace ascs::tcp;
using namespace ascs::ext;
using namespace ascs::ext::tcp;

class ws_socket : public server_socket_base < DEFAULT_WS_PACKER, DEFAULT_WS_UNPACKER, i_server >
{
public:
	ws_socket(i_server& server_)
	: server_socket_base(server_), m_bHandshaked(false)
	{
		m_packer = std::make_shared<DEFAULT_WS_PACKER>();
		m_packer->setIsServer(true);
		packer(m_packer);

		wsunpacker()->setIsServer(true);
	}

	~ws_socket() {  }


	virtual void reset() 
	{
		m_bHandshaked = false;
		m_packer->reset();
		wsunpacker()->reset();
		server_socket_base::reset();
	}


	virtual void on_connect()
	{

	}
	virtual void safeSendPacket(out_msg_type &msg)
	{
		if (msg.length() > 0)
		{
			safe_send_msg((char*)msg.data(), msg.length(), false);
		}
	}
	virtual void sendPacket(out_msg_type &msg)
	{
		if (msg.length() > 0 )
		{
			send_msg((char*)msg.data(), msg.length(), false);
		}
	}
	string GetSocketIP()
	{
		if (m_strIP.empty())
		{
			try { auto ep = lowest_layer().remote_endpoint(); m_strIP = ep.address().to_string(); }
			catch (std::exception& e) { SLOG_ERROR("get socket ip error:%s", e.what()); }
		}

		return m_strIP;
	}

	std::shared_ptr<DEFAULT_WS_UNPACKER>  wsunpacker() { return  std::dynamic_pointer_cast<DEFAULT_WS_UNPACKER>(unpacker()); }

protected:

	virtual void on_close()
	{
		reset();
		server_socket_base::on_close();
		
	}

	virtual void sendPong()
	{
		auto msg = m_packer->pack_pong();
		direct_send_msg(std::move(msg));
	}
	virtual bool on_msg_handle(out_msg_type& msg)
	{
		/*֡���� type �Ķ���data[0] & 0xF
		0x3-7��ʱ�޶��壬Ϊ�Ժ�ķǿ���֡����
		0xB-F��ʱ�޶��壬Ϊ�Ժ�Ŀ���֡����
		*/
		switch (msg.opcode)
		{
// 		case 0: { printf("* recv added\n"); break; }//0x0��ʾ��������֡
// 		case 1: { printf("* recv text\n"); break; }//0x1��ʾ�ı�����֡
// 		case 2: { printf("* recv binary\n"); break; }//0x2��ʾ����������֡
		case 8: { SLOG_DEBUG("* recv shutdown"); force_shutdown(); return true; }//0x8��ʾ���ӹر�
		case 9: { SLOG_DEBUG("- on PING"); sendPong(); return true; }//0x9��ʾping
		case 10:{ SLOG_DEBUG("+ on PONG"); return true; }//0xA��ʾpong

		default:
			break;
		}

		if (msg.size() == 0 || msg.data() == nullptr) return true;

		if (!m_bHandshaked)//���δ����,��һ���ǽ�������ɵ�������Ϣ,ֱ�ӷ���
		{
			m_bHandshaked = true;
			send_msg((char*)msg.data(), msg.size(), false);

			SLOG_DEBUG("%s", msg.data());
		}
		else
		{
			SLOG_DEBUG("recv[%d]:%s", msg.opcode, msg.data());
		}
		return true;
	}

protected:

	string m_strIP;
	bool m_bHandshaked;//�Ƿ�������
	std::shared_ptr<DEFAULT_WS_PACKER> m_packer;


};


typedef std::shared_ptr<ws_socket>			WsSocketPtr;

////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T_Socket, typename listen_server_base = server_base<T_Socket, object_pool<T_Socket>, i_server> >
class ws_listen_server : public listen_server_base
{
public:
	typedef std::shared_ptr<T_Socket>	socket_type;

	ws_listen_server(const std::string& ip, unsigned short port, service_pump & sp, uint32_t maxobj = ST_ASIO_MAX_OBJECT_NUM)
		: listen_server_base(sp), m_service_pump(sp)
	{
		listen_server_base::max_size(maxobj);
		listen_server_base::set_server_addr(port, ip);

		m_service_pump.start_service(ASCS_SERVICE_THREAD_NUM);

		SLOG_INFO("ws server[%s:%d] started with thread:%d, max size:%d", ip.c_str(), port, ASCS_SERVICE_THREAD_NUM, maxobj);

	}

	virtual ~ws_listen_server() {}
	socket_type  GetClient(uint_fast64_t clientID) { return listen_server_base::find(clientID); }

	virtual bool sendPacketToClient(uint_fast64_t clientID, ws_message &msg)
	{
		socket_type obj = GetClient(clientID);
		if (obj != nullptr)
		{
			obj->sendPacket(msg);
			return true;
		}
		return false;
	}
protected:
	service_pump & m_service_pump;

private:


};

typedef ws_listen_server<ws_socket>		ws_server;


#endif//__ws_server_h__