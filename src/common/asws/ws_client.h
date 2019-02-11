//
//  ws_socket.h
//  
//  Created by qiyun on 19-1-18.
//  Copyright (c) 2019 __jin10.com__. All rights reserved.
//

#ifndef __ws_client_h__
#define __ws_client_h__

#include "ws_config.h"
#include <ascs/ext/tcp.h>
#include "ws_packer.h"

typedef ascs::tcp::client_socket_base< DEFAULT_WS_PACKER, DEFAULT_WS_UNPACKER > ws_nor_connector;

class ws_client_socket : public ws_nor_connector
{
public:
	ws_client_socket(asio::io_context& io_service_)
	: ws_nor_connector(io_service_), m_bHandshaked(false)
	{
		
		m_packer = std::make_shared<DEFAULT_WS_PACKER>();
		m_packer->setIsServer(false);
		packer(m_packer);

		wsunpacker()->setIsServer(false);

	}



	virtual void connect_addr(const string &strIP, uint32_t port, const string &path)
	{
		m_strPath = path;
		m_strIP = strIP;
		m_iPort = port;

		set_server_addr(port, strIP);

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
		if (msg.length() > 0)
		{
			send_msg((char*)msg.data(), msg.length(), false);
		}
	}
protected:
	virtual void sendPong()
	{
		auto msg = m_packer->pack_pong();
		direct_send_msg(std::move(msg));
	}
	virtual void on_unpack_error() { ws_nor_connector::on_unpack_error(); SLOG_ERROR("socket can not unpack msg."); }

	virtual void on_recv_error(const asio::error_code& ec)
	{
		SLOG_DEBUG("client socket[%s:%d] on recv error [%d]:%s", m_strIP.c_str(), m_iPort, ec.value(), ec.message().c_str());
		ws_nor_connector::on_recv_error(ec);
	}

	virtual void on_connect()
	{
		ws_nor_connector::on_connect();

		SLOG_DEBUG("client socket[%s:%d] on connect++++++++++++", m_strIP.c_str(), m_iPort);

		string strHandshake = Extensions::clientHandshakeString(m_strIP, m_iPort, m_strPath);//����������Ϣ
		send_msg((char*)strHandshake.data(), strHandshake.length(), false);

	}
	virtual void on_close()
	{

		SLOG_WARN("client socket[%s:%d] on close------------", m_strIP.c_str(), m_iPort);
		reset();
		
		ws_nor_connector::on_close();

	}
	virtual void reset() 
	{ 
		ws_nor_connector::reset();
		m_bHandshaked = false;
		m_packer->reset();
		wsunpacker()->reset();

	}

	std::shared_ptr<DEFAULT_WS_UNPACKER>  wsunpacker() { return  std::dynamic_pointer_cast< DEFAULT_WS_UNPACKER >(unpacker());	}

protected:


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
		case 10: {SLOG_DEBUG("+ on PONG"); return true; }//0xA��ʾpong

		default:
			break;
		}

		if (msg.size() == 0 || msg.data() == nullptr) return true;

		if (!m_bHandshaked)//���δ����,��һ���ǽ�������ɵ�������Ϣ,ֱ�ӷ���
		{
			m_bHandshaked = true;

			SLOG_DEBUG("%s:%d handshake success!!!", m_strIP.c_str(), m_iPort);
		}
		else
		{
			SLOG_DEBUG("recv[%d]:%s", msg.opcode, msg.data());
		}

		return true;
	}

protected:

	std::shared_ptr<DEFAULT_WS_PACKER> m_packer;

	string m_strPath;
	string m_strIP;
	uint16_t m_iPort;

	bool m_bHandshaked;
};

typedef ascs::tcp::single_client_base<ws_client_socket> ws_single_client;



//ws_client//////////////////////////////////////////////////////////////
template<class T_socket>
class ws_client_service 
{
public:
	ws_client_service(const string &strIPPort, service_pump &sp) :m_service_pump(sp), m_socket(sp)
	{
		changeUrl(strIPPort);

		m_service_pump.start_service(ASCS_CLIENT_SOCKET_THREAD);
		SLOG_INFO("ws client[%s] started with thread:%d", strIPPort.c_str(), ASCS_CLIENT_SOCKET_THREAD);
	}

	~ws_client_service() { }

	void disconnect()
	{
		m_service_pump.stop_service();
	}

	void connect(const std::string& strIPPort)
	{
		changeUrl(strIPPort);

		m_socket.force_shutdown(true);
		m_service_pump.start_service(ASCS_CLIENT_SOCKET_THREAD);
	}


	virtual void sendPacket(ws_message &msg) { m_socket.sendPacket(msg); }
	virtual void safeSendPacket(ws_message &msg) { m_socket.safeSendPacket(msg); }

	T_socket& GetSocket() { return m_socket; }

protected:
	bool changeUrl(const string &url)
	{
		bool bSSL = false;
		string strTempIP = url;
		string strIP;
		string strPath;
		int port = 0;

		if (Extensions::parseURI(strTempIP, bSSL, strIP, port, strPath))
		{
			m_socket.connect_addr(strIP, port, strPath);
			return true;
		}
		return false;
	}

protected:
	T_socket m_socket;
	service_pump & m_service_pump;
private:

};

typedef ws_client_service<ws_single_client>		ws_client;


#endif//__ws_client_h__