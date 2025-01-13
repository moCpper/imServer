#include "CServer.h"
#include"UserMgr.h"
CServer::CServer(boost::asio::io_context& io_context, short port):_io_context(io_context)
	,_port(port)
	,_acceptor(io_context, tcp::endpoint(tcp::v4(), port)){
	std::cout << "Server start success, listen on port : " << _port << std::endl;
	StartAccept();
}

CServer::~CServer(){

}

void CServer::ClearSession(std::string session_id){
	if (_sessions.find(session_id) != _sessions.end()) {
		//移除用户和Session的关联
		UserMgr::GetInstance()->RmvUserSession(_sessions[session_id]->GetUserId());
	}
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_sessions.erase(session_id);
	}
}

void CServer::HandleAccept(std::shared_ptr<CSession> new_session, const boost::system::error_code& error){
	if (!error) {
		new_session->Start();
		std::lock_guard<std::mutex> lock(_mutex);
		_sessions.insert(std::make_pair(new_session->GetSessionId(), new_session));
	}else {
		std::cout << "session accept failed, error is " << error.what() << std::endl;
	}

	StartAccept();
}

void CServer::StartAccept(){
	auto& io_context = AsioIOServicePool::GetInstance()->GetIOService();
	std::shared_ptr<CSession> new_session = std::make_shared<CSession>(io_context,this);
	_acceptor.async_accept(new_session->GetSocket(), std::bind(&CServer::HandleAccept, this, new_session, placeholders::_1));
}
