//
//  wss_server.h
//  
//  Created by qiyun on 19-1-18.
//  Copyright (c) 2019 __jin10.com__. All rights reserved.
//

#ifndef __wss_server_h__
#define __wss_server_h__
#include "ws_server.h"
#include <ascs/ext/ssl.h>

typedef ascs::ssl::server_socket_base< DEFAULT_WS_PACKER, DEFAULT_WS_UNPACKER > wss_socket_connector;

class wss_socket : public wss_socket_connector
{
public:
	wss_socket(i_server& server_, asio::ssl::context& ctx)
	: wss_socket_connector(server_, ctx), m_bHandshaked(false)
	{
		m_packer = std::make_shared<DEFAULT_WS_PACKER>();
		m_packer->setIsServer(true);
		packer(m_packer);

		wsunpacker()->setIsServer(true);
	}
	~wss_socket() {  }


	virtual void reset() 
	{
		m_bHandshaked = false;
		m_packer->reset();
		wsunpacker()->reset();
		wss_socket_connector::reset();
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
		m_bHandshaked = false;
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
	std::shared_ptr<DEFAULT_WS_PACKER> m_packer;

	bool m_bHandshaked;//�Ƿ�������

};


typedef std::shared_ptr<wss_socket>			WssSocketPtr;

////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T_Socket, typename wss_server_base = ascs::ssl::server_base<T_Socket>  >
class wss_listen_server : public wss_server_base
{
public:
	typedef std::shared_ptr<T_Socket>	socket_type;

	wss_listen_server(const std::string& ip, unsigned short port, service_pump & sp, const string &strCert, const string &strKey, uint32_t maxobj = ST_ASIO_MAX_OBJECT_NUM )
		: wss_server_base(sp, asio::ssl::context::sslv23_server), m_service_pump(sp)
	{
		wss_server_base::context().set_options(asio::ssl::context::default_workarounds | asio::ssl::context::no_sslv2 | asio::ssl::context::single_dh_use);
		wss_server_base::context().set_verify_mode(asio::ssl::context::verify_none);

		//wss_server_base::context().load_verify_file(strCertPath + "/client.crt");
		wss_server_base::context().use_certificate_chain_file(strCert);
		wss_server_base::context().use_private_key_file(strKey, asio::ssl::context::pem);
		//wss_server_base::context().use_tmp_dh_file(strCertPath + "/server.pem");
		

		wss_server_base::max_size(maxobj);
		wss_server_base::set_server_addr(port, ip);
		m_service_pump.start_service(ASCS_SERVICE_THREAD_NUM);
		SLOG_INFO("wss server[%s:%d] started with thread:%d", ip.c_str(), port, ASCS_SERVICE_THREAD_NUM);

	}

	virtual ~wss_listen_server() {}

	socket_type  GetClient(uint_fast64_t clientID) { return wss_server_base::find(clientID); }
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

typedef wss_listen_server<wss_socket>		wss_server;



#endif//__wss_server_h__