asws (a websocket base on ascs)
QQ:421264988
����QQȺ:953980601
===========================================================================
I am chinese, so chinese takes precedence!

���ȸ�л youngwolf,�� websocket ���� ascs/asio ��װ���� .

- ����:
1.C++���Կ���,֧�ֿ�ƽ̨,windows/linux/mac/ios/android
3.֧��ascs��ȫ������(https://github.com/youngwolf-project/ascs.git)
2.֧��ssl


- ���ٿ�ʼ
1.server:
#include "common/asws/ws_common.h"


service_pump sp;
#if WEB_SOCKET_SSL //with ssl
	wss_server server(DEFAULT_WEB_IP, DEFAULT_WEB_PORT, sp, "./certs/server.crt", "./certs/server.key");
#else //normal
  	ws_server server(DEFAULT_WEB_IP, DEFAULT_WEB_PORT, sp);
#endif


2.client:
#include "common/asws/ws_common.h"
	service_pump sp;

#if WEB_SOCKET_SSL //with ssl
	wss_client client("wss://127.0.0.1:3001", sp, "./certs/client.crt","./certs/client.key");
#else //normal
	ws_client client("ws://127.0.0.1:3001", sp);
#endif


- ����:
1.wss��Ŀǰֻ֧��˫����֤
2.�ݲ�֧����Ϣѹ��


websocket ���߲��Թ���(��Ϊ��Ҫ˫����֤����wss���޷�����):
https://easyswoole.com/wstool.html


-������֧��
Visual C++ 11.0, GCC 4.6 or Clang 3.1 at least, with c++11 features;</br>


==enlgish=========================================================================

First of all, thank you for youngwolf, this development based on ascs/asio package.


- Overview:
1.C++ language development, support cross-platform, windows/linux/mac/ios/android
3. Support all features of ASCs (https://github.com/youngwolf-project/ascs.git)
2. support SSL


- Quick Start
1.server:
#include "common/asws/ws_common.h"

service_pump sp;
#if WEB_SOCKET_SSL //with ssl
	wss_server server(DEFAULT_WEB_IP, DEFAULT_WEB_PORT, sp, "./certs/server.crt", "./certs/server.key");
#else //normal
  	ws_server server(DEFAULT_WEB_IP, DEFAULT_WEB_PORT, sp);
#endif

2.client:
#include "common/asws/ws_common.h"
	service_pump sp;

#if WEB_SOCKET_SSL //with ssl
	wss_client client("wss://127.0.0.1:3001", sp, "./certs/client.crt","./certs/client.key");
#else //normal
	ws_client client("ws://127.0.0.1:3001", sp);
#endif


- question:
1. Currently, only two-way authentication is supported under WSS
2. Message compression is not supported for the time being


Websocket online testing tool (cannot be tested under WSS because of the need for bidirectional validation):
https://easyswoole.com/wstool.html


- Compiler requirement:
Visual C++ 11.0, GCC 4.6 or Clang 3.1 at least, with c++11 features;</br>

