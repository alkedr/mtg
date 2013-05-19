#include "magic.hpp"

#include <cstdint>
#include <boost/asio.hpp>
#include <boost/bind.hpp>



class Session {
	boost::asio::ip::tcp::socket socket_;
	enum { max_length = 1024 };
	char data_[max_length];


	void handle_read(const boost::system::error_code & error, size_t bytes_transferred)
	{
		if (!error) {
			std::cout << __FUNCTION__ << std::endl;
  			boost::asio::async_write(
				socket_,
      			boost::asio::buffer(data_, bytes_transferred),
      			boost::bind(
					&Session::handle_write,
					this,
					boost::asio::placeholders::error
				)
			);
		} else {
  			delete this;
		}
	}

	void handle_write(const boost::system::error_code & error)
	{
		if (!error) {
			std::cout << __FUNCTION__ << std::endl;
  			socket_.async_read_some(
				boost::asio::buffer(data_, max_length),
				boost::bind(
					&Session::handle_read,
					this,
        			boost::asio::placeholders::error,
        			boost::asio::placeholders::bytes_transferred
				)
			);
		} else {
  			delete this;
		}
	}
public:
	Session(boost::asio::io_service & io_service)
	 : socket_(io_service)
	{
	}

	boost::asio::ip::tcp::socket & socket()
	{
		return socket_;
	}

	void start()
	{
		socket_.async_read_some(
			boost::asio::buffer(data_, max_length),
    		boost::bind(
				&Session::handle_read,
				this,
      			boost::asio::placeholders::error,
      			boost::asio::placeholders::bytes_transferred
			)
		);
	}

};




class Server {
	boost::asio::io_service & io_service;
	boost::asio::ip::tcp::acceptor acceptor;


	void start_accept()
	{
		Session * new_session = new Session(io_service);
		acceptor.async_accept(
			new_session->socket(),
			boost::bind(
				&Server::handle_accept,
				this,
				new_session,
				boost::asio::placeholders::error
			)
		);
	}

	void handle_accept(Session * new_session, const boost::system::error_code & error)
	{
		if (!error)
			new_session->start();
		else 
			delete new_session;
    	start_accept();
	}

public:
	Server(boost::asio::io_service & _io_service, uint16_t _port)
	 : io_service(_io_service)
	 , acceptor(_io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), _port))
	{
		start_accept();
	}

	void run()
	{

	}
};


const char server_name[] = "mtg";
const char server_version[] = "1.0";


int main(/*int argc, const char ** argv*/)
{
	Game g;
	auto c = Game::newCard(1, g, 0);

	std::cout << server_name << " v" << server_version << std::endl;
	//std::cout << "cards: " << cardAllocators.size() << std::endl;

	boost::asio::io_service io_service;
	Server server(io_service, 5643);
	io_service.run();
	return 0;
}
