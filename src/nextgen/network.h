#ifndef NEXTGEN_NETWORK
#define NEXTGEN_NETWORK

#include "common.h"

namespace nextgen
{
    namespace network
    {
        namespace ip
        {
            namespace network
            {
                class layer_base
                {

                };

                namespace ipv4
                {
                    class layer : public layer_base
                    {
                        public: typedef string host_type;
                        public: typedef uint32_t port_type;

                        public: virtual host_type const& get_host()
                        {
                            auto self = *this;

                            return self->host_;
                        }

                        public: virtual void set_host(host_type const& host_)
                        {
                            auto self = *this;

                            self->host_ = host_;
                        }

                        public: virtual port_type get_port()
                        {
                            auto self = *this;

                            return self->port_;
                        }

                        public: virtual void set_port(port_type port_)
                        {
                            auto self = *this;

                            self->port_ = port_;
                        }

                        private: struct variables
                        {
                            variables() : host_("undefined"), port_(0)
                            {

                            }

                            variables(host_type const& host_, port_type port_) : host_(host_), port_(port_)
                            {

                            }

                            ~variables()
                            {

                            }

                            host_type host_;
                            port_type port_;
                        };

                        NEXTGEN_SHARED_DATA(layer, variables);
                    };
                }

                namespace ipv6
                {
                    class layer : public layer_base
                    {

                    };
                }
            }
        }

        typedef ip::network::ipv4::layer ipv4_address;
        typedef ip::network::ipv6::layer ipv6_address;

        namespace ip
        {
            namespace transport
            {
                class layer_base
                {

                };

                class accepter_base
                {

                };

                namespace tcp
                {

                    class accepter : public accepter_base
                    {
                        private: typedef asio::ip::tcp::acceptor accepter_type;
                        private: typedef asio::ip::tcp::socket socket_type;
                        private: typedef asio::ip::tcp::endpoint endpoint_type;
                        public: typedef service service_type;
                        public: typedef uint32_t port_type;

                        public: void open(port_type port)
                        {
                            auto self = *this;

                            asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);

                            try
                            {
                                self->accepter_.open(endpoint.protocol());
                                self->accepter_.set_option(asio::ip::tcp::acceptor::reuse_address(true));

                                self->accepter_.bind(endpoint);
                                self->accepter_.listen();

                                // successfully binded port
                                if(NEXTGEN_DEBUG_4)
                                    std::cout << "[nextgen:network:ip:transport:tcp:accepter] Successfully binded port " << port << "." << std::endl;

                                self->port = port;
                            }
                            catch(std::exception& e)
                            {
                                std::cout << "[nextgen:network:ip:transport:tcp:accepter] Failed to bind port " << port << "." << std::endl;
                            }
                        }

                        public: bool is_open()
                        {
                            auto self = *this;

                            return self->accepter_.is_open();
                        }

                        public: port_type get_port() const
                        {
                            auto self = *this;

                            return self->port;
                        }

                        public: template<typename handler_type> void accept(socket_type& socket_, handler_type handler_)
                        {
                            auto self = *this;

                            //if(self.is_open())
                            //{
                                self->accepter_.async_accept(socket_, handler_);
                            //}
                            //else
                            //{
                            //    asio::error_code ec = asio::error_code(asio::error::try_again);
                            //    handler_(ec);
                            //}
                        }

                        private: struct variables
                        {
                            variables(service_type service_) : port(0), accepter_(service_.get_service())
                            {

                            }

                            ~variables()
                            {

                            }

                            port_type port;
                            accepter_type accepter_;
                        };

                        NEXTGEN_SHARED_DATA(accepter, variables);
                    };

                    template<typename network_layer_type>
                    class layer : public layer_base
                    {
                        public: typedef layer<network_layer_type> this_type;
                        public: typedef service service_type;
                        public: typedef string host_type;
                        public: typedef uint32_t port_type;
                        public: typedef uint32_t timeout_type;

                        private: typedef asio::ip::tcp::socket socket_type;
                        private: typedef asio::ip::tcp::resolver resolver_type;
                        private: typedef asio::deadline_timer timer_type;
                        public: typedef accepter accepter_type;

                        public: typedef std::function<void(asio::error_code const&)> cancel_handler_type;

                        public: typedef std::function<void()> base_event_type;
                        public: typedef base_event_type connect_successful_event_type;
                        public: typedef base_event_type connect_failure_event_type;
                        public: typedef base_event_type receive_successful_event_type;
                        public: typedef base_event_type receive_failure_event_type;
                        public: typedef base_event_type send_successful_event_type;
                        public: typedef base_event_type send_failure_event_type;
                        public: typedef base_event_type quit_successful_event_type;
                        public: typedef base_event_type quit_failure_event_type;
                        public: typedef std::function<void(this_type)> accept_successful_event_type;
                        public: typedef base_event_type accept_failure_event_type;
                        public: typedef base_event_type close_event_type;

                        public: virtual void initialize()
                        {
                            auto self = *this;

                            self->cancel_handler_ = [=](asio::error_code const& error)
                            {
                                if(error == asio::error::operation_aborted)
                                {
                                    if(NEXTGEN_DEBUG_1)
                                        std::cout << "[nextgen:network:ip:transport:tcp:socket:cancel_handler] Timer cancelled (" << self->network_layer_.get_host() << ":" << self->network_layer_.get_port() << ")" << std::endl;
                                }
                                else
                                {
                                    if(NEXTGEN_DEBUG_1)
                                        std::cout << "[nextgen:network:ip:transport:tcp:socket:cancel_handler] Timer called back. Closing socket (" << self->network_layer_.get_host() << ":" << self->network_layer_.get_port() << ")" << std::endl;

                                    // bugfix(daemn): read timer doesn't actually cancel
                                    self.cancel();
                                }
                            };
                        }

                        public: virtual host_type const& get_host()
                        {
                            auto self = *this;

                            return self->network_layer_.get_host();
                        }

                        public: virtual void set_host(host_type const& host_)
                        {
                            auto self = *this;

                            self->network_layer_.set_host(host_);
                        }

                        public: virtual port_type get_port()
                        {
                            auto self = *this;

                            return self->network_layer_.get_port();
                        }

                        public: virtual void set_port(port_type port)
                        {
                            auto self = *this;

                            self->network_layer_.set_port(port);
                        }

                        public: virtual bool is_connected() const
                        {
                            auto self = *this;

                            return self->socket_.is_open();
                        }

                        public: virtual void cancel() const
                        {
                            auto self = *this;

                            if(NEXTGEN_DEBUG_1)
                                std::cout << "<socket::cancel> Cancelling socket (" << self->network_layer_.get_host() << ":" << self->network_layer_.get_port() << ")" << std::endl;

                            if(self->socket_.native() != asio::detail::invalid_socket)
                                self->socket_.cancel();
                            //else
                            //    std::cout << "<ClientSocket> Guarded an invalid socket." << std::endl;
                        }

                        public: virtual void close() const
                        {
                            auto self = *this;

                            if(NEXTGEN_DEBUG_1)
                                std::cout << "<socket::close> Closing socket normally. (" << self->network_layer_.get_host() << ":" << self->network_layer_.get_port() << ")" << std::endl;

                            if(self->socket_.native() != asio::detail::invalid_socket)
                                self->socket_.close();

                            self->close_event();
                        }

                        public: virtual size_t bytes_readable()
                        {
                            auto self = *this;

                            asio::socket_base::bytes_readable command(true);
                            self->socket_.io_control(command);

                            return command.get();
                        }

                        public: virtual void connect(host_type const& host_, port_type port_, connect_successful_event_type successful_handler2 = 0, connect_failure_event_type failure_handler2 = 0) const
                        {
                            auto self2 = *this;
                            auto self = self2;

                            auto successful_handler = successful_handler2; // bugfix(daemn) gah!
                            auto failure_handler = failure_handler2; // bugfix(daemn) gah!

                            if(successful_handler == 0)
                                successful_handler = self->connect_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->connect_failure_event;

                            self->network_layer_.set_host(host_);
                            self->network_layer_.set_port(port_);

                            if(NEXTGEN_DEBUG_1)
                                std::cout << "<socket::connect> (" << self->network_layer_.get_host() << ":" << self->network_layer_.get_port() << ")" << std::endl;

                            resolver_type::query query_(host_, port_ == 80 ? "http" : to_string(port_));

                            if(self->timeout_ > 0)
                            {
                                if(NEXTGEN_DEBUG_1)
                                    std::cout << "<socket::connect> create timer (" << self->network_layer_.get_host() << ":" << self->network_layer_.get_port() << ")" << std::endl;

                                self->timer_.expires_from_now(boost::posix_time::seconds(30));
                                self->timer_.async_wait(self->cancel_handler_);
                            }

                            self->resolver_.async_resolve(query_, [=](asio::error_code const& error, resolver_type::iterator endpoint_iterator)
                            {
                                if(NEXTGEN_DEBUG_1)
                                    std::cout << "<socket::connect handler> (" << self->network_layer_.get_host() << ":" << self->network_layer_.get_port() << ")" << std::endl;

                                if(self->timeout_ > 0)
                                    self.cancel_timer();

                                if(!error)
                                {
                                    if(NEXTGEN_DEBUG_1)
                                        std::cout << "<socket::connect handler> resolve success (" << self->network_layer_.get_host() << ":" << self->network_layer_.get_port() << ")" << std::endl;

                                    //todo(daemn) add additional endpoint connection tries
                                    asio::ip::tcp::endpoint endpoint = *endpoint_iterator;

                                    //++endpoint_iterator;

                                    if(NEXTGEN_DEBUG_1)
                                        std::cout << "<socket::connect handler> create timer (" << self->network_layer_.get_host() << ":" << self->network_layer_.get_port() << ")" << std::endl;

                                    if(self->timeout_ > 0)
                                    {
                                        self->timer_.expires_from_now(boost::posix_time::seconds(30));
                                        self->timer_.async_wait(self->cancel_handler_);
                                    }

                                    self->socket_.async_connect(endpoint, [=](asio::error_code const& error)
                                    {
                                        if(NEXTGEN_DEBUG_1)
                                            std::cout << "<socket::connect handler> (" << self->network_layer_.get_host() << ":" << self->network_layer_.get_port() << ")" << std::endl;

                                        if(self->timeout_ > 0)
                                            self.cancel_timer();

                                        if(!error)
                                        {
                                            successful_handler();
                                        }
                                        else
                                        {
                                            if(NEXTGEN_DEBUG_4)
                                                std::cout << "<socket::connect handler> Error: " << error.message() << std::endl;

                                            self.close();

                                            failure_handler();
                                        }
                                    });
                                }
                                else
                                {
                                    if(self->timeout_ > 0)
                                        self.cancel_timer();

                                    if(NEXTGEN_DEBUG_4)
                                        std::cout << "<socket::connect handler> Error: " << error.message() << std::endl;

                                    self.close();

                                    failure_handler();
                                }
                            });
                        }

                        public: virtual void cancel_timer() const
                        {
                            auto self = *this;

                            self->timer_.cancel();
                        }

                        public: template<typename stream_type> void send(stream_type stream, send_successful_event_type successful_handler = 0, send_failure_event_type failure_handler = 0) const
                        {
                            auto self = *this;

                            if(successful_handler == 0)
                                successful_handler = self->send_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->send_failure_event;

                            if(NEXTGEN_DEBUG_1)
                                std::cout << "<socket::write> create timer (" << self->network_layer_.get_host() << ":" << self->network_layer_.get_port() << ")" << std::endl;

                            if(self->timeout_ > 0)
                            {
                                self->timer_.expires_from_now(boost::posix_time::seconds(self->timeout_));
                                self->timer_.async_wait(self->cancel_handler_);
                            }

                            asio::async_write(self->socket_, stream.get_buffer(),
                            [=](asio::error_code const& error, size_t& total)
                            {
                                stream.get_buffer(); // bugfix(daemn)

                                if(self->timeout_ > 0)
                                    self.cancel_timer();

                                if(NEXTGEN_DEBUG_1)
                                    std::cout << "<socket::write handler> (" << self->network_layer_.get_host() << ":" << self->network_layer_.get_port() << ")" << std::endl;

                                if(!error)
                                {
                                    successful_handler();
                                }
                                else
                                {
                                    if(NEXTGEN_DEBUG_4)
                                        std::cout << "<socket::write handler> Error: " << error.message() << std::endl;

                                    self.close();

                                    failure_handler();
                                }
                            });
                        }

                        public: template<typename delimiter_type, typename stream_type> void receive_until(delimiter_type const& delimiter, stream_type stream, receive_successful_event_type successful_handler2 = 0, receive_failure_event_type failure_handler2 = 0) const
                        {
                            auto self2 = *this;
                            auto self = self2; // bugfix(daemn) weird lambda stack bug, would only accept PBR

                            auto successful_handler = successful_handler2; // bugfix(daemn) gah!
                            auto failure_handler = failure_handler2; // bugfix(daemn) gah!

                            if(successful_handler == 0)
                                successful_handler = self->receive_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->receive_failure_event;

                            if(NEXTGEN_DEBUG_1)
                                std::cout << "<socket::receive> (" << self->network_layer_.get_host() << ":" << self->network_layer_.get_port() << ")" << std::endl;

                            auto on_read = [=](asio::error_code const& error, uint32_t total)
                            {
                                stream.get_buffer(); // bugfix(daemn)

                                if(NEXTGEN_DEBUG_1)
                                    std::cout << "<socket::receive handler> (" << self->network_layer_.get_host() << ":" << self->network_layer_.get_port() << ")" << std::endl;

                                if(self->timeout_ > 0)
                                    self.cancel_timer();

                                if(!error)
                                {
                                    successful_handler();
                                }
                                else
                                {
                                    if(NEXTGEN_DEBUG_4)
                                        std::cout << "<socket::receive handler> Error: " << error.message() << std::endl;

                                    self.close();

                                    failure_handler();
                                }
                            };

                            if(self->timeout_ > 0)
                            {
                                self->timer_.expires_from_now(boost::posix_time::seconds(self->timeout_));
                                self->timer_.async_wait(self->cancel_handler_);
                            }

                            asio::async_read_until(self.get_socket(), stream.get_buffer(), delimiter, on_read);
                        }

                        public: template<typename delimiter_type, typename stream_type> void receive(delimiter_type delimiter, stream_type stream, receive_successful_event_type successful_handler2 = 0, receive_failure_event_type failure_handler2 = 0) const
                        {
                            auto self2 = *this;
                            auto self = self2; // bugfix(daemn) weird lambda stack bug, would only accept PBR

                            auto successful_handler = successful_handler2; // bugfix(daemn) gah!
                            auto failure_handler = failure_handler2; // bugfix(daemn) gah!

                            if(successful_handler == 0)
                                successful_handler = self->receive_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->receive_failure_event;

                            if(NEXTGEN_DEBUG_1)
                                std::cout << "<socket::receive> (" << self->network_layer_.get_host() << ":" << self->network_layer_.get_port() << ")" << std::endl;

                            auto on_read = [=](asio::error_code const& error, uint32_t total)
                            {
                                stream.get_buffer(); // bugfix(daemn)

                                if(NEXTGEN_DEBUG_1)
                                    std::cout << "<socket::receive handler> (" << self->network_layer_.get_host() << ":" << self->network_layer_.get_port() << ")" << std::endl;

                                if(self->timeout_ > 0)
                                    self.cancel_timer();

                                if(!error)
                                {
                                    successful_handler();
                                }
                                else
                                {
                                    if(NEXTGEN_DEBUG_4)
                                        std::cout << "<socket::receive handler> Error: " << error.message() << std::endl;

                                    self.close();

                                    failure_handler();
                                }
                            };

                            if(self->timeout_ > 0)
                            {
                                self->timer_.expires_from_now(boost::posix_time::seconds(self->timeout_));
                                self->timer_.async_wait(self->cancel_handler_);
                            }

                            asio::async_read(self.get_socket(), stream.get_buffer(), delimiter, on_read);
                        }

                        public: virtual void accept(port_type port_, accept_successful_event_type successful_handler2 = 0, accept_failure_event_type failure_handler2 = 0) const
                        {
                            auto self = *this;

                            auto successful_handler = successful_handler2; // bugfix(daemn) gah!!
                            auto failure_handler = failure_handler2; // bugfix(daemn) gah!!

                            if(successful_handler == 0)
                                successful_handler = self->accept_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->accept_failure_event;

                            if(NEXTGEN_DEBUG_1)
                                std::cout << "[nextgen:network:ip:transport:tcp:socket:accept] " << std::endl;

                            this_type client(self.get_service());

                            if(self->accepter_->port != port_)
                                self->accepter_.open(port_);

                            self->accepter_.accept(client.get_socket(), [=](asio::error_code const& error)
                            {
                                if(NEXTGEN_DEBUG_1)
                                    std::cout << "[nextgen:network:ip:transport:tcp:socket:accept] Trying to accept client..." << std::endl;

                                if(!error)
                                {
                                    successful_handler(client);

                                    self.accept(port_, successful_handler, failure_handler);
                                }
                                else
                                {
                                    if(NEXTGEN_DEBUG_4)
                                        std::cout << "[nextgen:network:ip:transport:tcp:socket:accept] Error: " << error.message() << std::endl;

                                    failure_handler();
                                }
                            });
                        }

                        public: virtual socket_type& get_socket() const
                        {
                            auto self = *this;

                            return self->socket_;
                        }

                        public: virtual service_type get_service() const
                        {
                            auto self = *this;

                            return self->service_;
                        }

                        public: virtual timer_type& get_timer() const
                        {
                            auto self = *this;

                            return self->timer_;
                        }

                        private: struct variables
                        {
                            variables(service_type service_) : service_(service_), socket_(service_.get_service()), accepter_(service_), resolver_(service_.get_service()), timer_(service_.get_service()), timeout_(180)
                            {

                            }

                            ~variables()
                            {

                            }

                            event<send_successful_event_type> send_successful_event;
                            event<send_failure_event_type> send_failure_event;
                            event<connect_successful_event_type> connect_successful_event;
                            event<connect_failure_event_type> connect_failure_event;
                            event<receive_successful_event_type> receive_successful_event;
                            event<receive_failure_event_type> receive_failure_event;
                            event<accept_failure_event_type> accept_failure_event;
                            event<accept_successful_event_type> accept_successful_event;
                            event<close_event_type> close_event;

                            service_type service_;
                            socket_type socket_;
                            accepter_type accepter_;
                            resolver_type resolver_;
                            timer_type timer_;
                            network_layer_type network_layer_;
                            timeout_type timeout_;
                            cancel_handler_type cancel_handler_;

                        };

                        NEXTGEN_SHARED_DATA(layer, variables,
                        {
                            this->initialize();
                        });
                    };
                }
            }
        }

        typedef ip::transport::tcp::layer<ipv4_address> tcp_socket;

        namespace ip
        {
            namespace application
            {
                template<typename message>
                class layer_base
                {
                    public: typedef service service_type;
                    public: typedef message message_type;
                    public: typedef timer timer_type;
                    public: typedef string host_type;
                    public: typedef uint32_t port_type;

                    public: typedef std::function<void()> base_event_type;
                    public: typedef base_event_type send_successful_event_type;
                    public: typedef base_event_type send_failure_event_type;
                    public: typedef std::function<void(message_type)> receive_successful_event_type;
                    public: typedef base_event_type receive_failure_event_type;
                    public: typedef std::function<void(message_type)> request_successful_event_type;
                    public: typedef base_event_type request_failure_event_type;
                    public: typedef base_event_type connect_successful_event_type;
                    public: typedef base_event_type connect_failure_event_type;
                    public: typedef base_event_type disconnect_event_type;
                };

                class message_base
                {
                    public: typedef string raw_header_list_type;
                    public: typedef hash_map<string, string> header_list_type;
                    public: typedef uint32_t status_code_type;
                    public: typedef string referer_type;
                    public: typedef hash_map<string, string> post_list_type;
                    public: typedef string content_type;
                    public: typedef string path_type;
                    public: typedef uint32_t id_type;
                    public: typedef string version_type;
                    public: typedef string network_layer_type;
                    public: typedef string host_type;
                    public: typedef uint32_t port_type;
                    public: typedef byte_array stream_type;
                    public: typedef string url_type;
                    public: typedef string status_description_type;
                    public: typedef string method_type;
                };


                namespace smtp
                {
                    class message : public message_base
                    {
                        public: struct state_type
                        {
                            static const uint32_t none = 0;
                        };

                        public: stream_type get_stream() const
                        {
                            auto self = *this;

                            return self->stream;
                        }

                        public: void pack() const
                        {
                            auto self = *this;

                            std::ostream data_stream(&self->stream.get_buffer());

                        }

                        public: void unpack_content() const
                        {
                            auto self = *this;

                            std::istream data_stream(&self->stream.get_buffer());

                            self->content += string((std::istreambuf_iterator<char>(data_stream)), std::istreambuf_iterator<char>());

                        }

                        private: struct variables
                        {
                            variables() : state(state_type::none)
                            {

                            }

                            ~variables()
                            {

                            }

                            content_type content;
                            stream_type stream;
                            uint32_t state;
                        };

                        NEXTGEN_SHARED_DATA(message, variables);
                    };

                    template<typename transport_layer_type>
                    class layer : public layer_base<message>
                    {
                        public: typedef layer<transport_layer_type> this_type;
                        public: typedef std::function<void(this_type)> accept_successful_event_type;
                        public: typedef base_event_type accept_failure_event_type;
                        public: typedef float keep_alive_threshold_type;

                        public: virtual void reconnect(connect_successful_event_type successful_handler2 = 0, connect_failure_event_type failure_handler2 = 0) const
                        {
                            auto self = *this;

                            if(NEXTGEN_DEBUG_5)
                                std::cout << "[nextgen::network::smtp_client] reconnecting" << std::endl;

                            self.disconnect();
                            self.connect(self->host, self->port, successful_handler2, failure_handler2);
                        }

                        public: virtual void connect(host_type const& host_, port_type port_, connect_successful_event_type successful_handler2 = 0, connect_failure_event_type failure_handler2 = 0) const
                        {
                            auto self = *this;

                            auto successful_handler = successful_handler2; // bugfix(daemn) gah!!
                            auto failure_handler = failure_handler2; // bugfix(daemn) gah!!

                            if(successful_handler == 0)
                                successful_handler = self->connect_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->connect_failure_event;

                            self->host = host_;
                            self->port = port_;

                            std::string host;
                            uint32_t port;

                            self->transport_layer_ = transport_layer_type(self->transport_layer_->service_);

                            self->transport_layer_.connect(host, port,
                            [=]
                            {
                                if(NEXTGEN_DEBUG_4)
                                    std::cout << "[nextgen::network::smtp_client] Connected" << std::endl;

                            },
                            [=]
                            {
                                failure_handler();
                            });
                        }

                        public: virtual void disconnect() const
                        {
                            auto self = *this;

                            self->transport_layer_.close();
                        }

                        public: virtual void send(message_type request_, send_successful_event_type successful_handler = 0, send_failure_event_type failure_handler = 0) const
                        {
                            auto self = *this;

                            request_.pack();

                            self.send(request_->stream, successful_handler, failure_handler);
                        }

                        public: virtual void send_and_receive(message_type request_, receive_successful_event_type successful_handler = 0, receive_failure_event_type failure_handler = 0) const
                        {
                            auto self = *this;

                            if(successful_handler == 0)
                                successful_handler = self->receive_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->receive_failure_event;

                            request_.pack();

                            self.send(request_->stream,
                            [=]()
                            {
                                request_->stream.get_buffer(); // bugfix(daemn)

                                if(NEXTGEN_DEBUG_4)
                                    std::cout << "smtp message sent, now receiving" << std::endl;

                                self.receive(successful_handler, failure_handler);
                            },
                            [=]()
                            {
                                failure_handler();
                            });
                        }

                        public: template<typename stream_type> void send(stream_type stream, send_successful_event_type successful_handler = 0, send_failure_event_type failure_handler = 0) const
                        {
                            auto self = *this;

                            if(successful_handler == 0)
                                successful_handler = self->send_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->send_failure_event;

                            self->transport_layer_.send(stream,
                            [=]()
                            {
                                successful_handler();
                            },
                            [=]()
                            {
                                failure_handler();
                            });
                        }

                        public: void send_helo(send_successful_event_type successful_handler = 0, send_failure_event_type failure_handler = 0) const
                        {
                            auto self = *this;

                            message_type m1;

                            m1->content = "220 localhost\r\n";

                            self.send(m1, successful_handler, failure_handler);
                        }

                        public: virtual void receive(receive_successful_event_type successful_handler = 0, receive_failure_event_type failure_handler = 0) const
                        {
                            auto self = *this;

                            if(successful_handler == 0)
                                successful_handler = self->receive_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->receive_failure_event;

                            message_type response2;
                            auto response = response2; // bugfix(daemn)

                            self->transport_layer_.receive_until("\r\n", response->stream,
                            [=]()
                            {
                                response.unpack_content();

                                if(NEXTGEN_DEBUG_3)
                                    std::cout << "Y: " << response->content << std::endl;

                                if(response->content.find("EHLO") != std::string::npos)
                                {
                                    std::string ehlo = nextgen::regex_single_match("EHLO (.+)\r\n", response->content);

                                    std::cout << "S: " << "250-localhost\r\n" << std::endl;
                                    std::cout << "S: " << "250 HELP\r\n" << std::endl;

                                    message_type m1;

                                    m1->content = "250-localhost\r\n"
                                    "250 HELP\r\n"
                                    "\r\n";

                                    self.send(m1,
                                    [=]()
                                    {
                                        self.receive(successful_handler, failure_handler);
                                    },
                                    failure_handler);
                                }
                                else if(response->content.find("HELO") != std::string::npos)
                                {
                                    self.receive(successful_handler, failure_handler);
                                }
                                else if(response->content.find("MAIL FROM") != std::string::npos
                                || response->content.find("VRFY") != std::string::npos
                                || response->content.find("RCPT TO") != std::string::npos)
                                {
                                    message_type m2;
                                    m2->content = "250 OK\r\n";

                                    self.send(m2,
                                    [=]
                                    {
                                        self.receive(successful_handler, failure_handler);
                                    },
                                    failure_handler);
                                }
                                else if(response->content.find("DATA") != std::string::npos)
                                {
                                    message_type m2;
                                    m2->content = "354 GO\r\n";

                                    self->transport_layer_.send(m2->stream,
                                    [=]
                                    {
                                        m2->stream.get_buffer(); // bugfix(daemn)

                                        self->transport_layer_.receive_until("\r\n.\r\n", response->stream,
                                        [=]()
                                        {
                                            successful_handler(response);
                                        },
                                        failure_handler);
                                    },
                                    failure_handler);
                                }
                                else if(response->content.find("QUIT") != std::string::npos)
                                {
                                    message_type m2;
                                    m2->content = "221 BYE\r\n";

                                    self.send(m2,
                                    [=]
                                    {
                                        self.disconnect();

                                        //successful_handler(response);
                                    },
                                    [=]
                                    {
                                        //successful_handler(response);
                                    });
                                }
                                else
                                {
                                    std::cout << "<Mail Server> Disconnecting incompatible client." << std::endl;

                                    self.disconnect();

                                    failure_handler();
                                }
                            },
                            failure_handler);
                        }

                        public: virtual void accept(port_type port_, accept_successful_event_type successful_handler2 = 0, accept_failure_event_type failure_handler2 = 0)
                        {
                            auto self = *this;

                            auto successful_handler = successful_handler2; // bugfix(daemn) gah!!
                            auto failure_handler = failure_handler2; // bugfix(daemn) gah!!

                            if(successful_handler == null)
                                successful_handler = self->accept_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->accept_failure_event;

                            self->transport_layer_.accept(port_,
                            [=](transport_layer_type client)
                            {
                                successful_handler(this_type(client));
                            },
                            [=]()
                            {
                                failure_handler();
                            });
                        }

                        public: virtual bool is_alive() const
                        {
                            auto self = *this;

                            return (self->keep_alive_threshold == 0) ? true : (self->keep_alive_threshold > self->keep_alive_timer.stop());
                        }

                        public: virtual void set_keep_alive(keep_alive_threshold_type keep_alive_threshold) const
                        {
                            auto self = *this;

                            self->keep_alive_threshold = keep_alive_threshold;
                        }

                        private: struct variables
                        {
                            variables(service_type service_) : transport_layer_(service_), keep_alive_threshold(0), host(""), port(0)
                            {

                            }

                            variables(transport_layer_type transport_layer_) : transport_layer_(transport_layer_), keep_alive_threshold(0), host(""), port(0)
                            {

                            }

                            ~variables()
                            {

                            }

                            event<send_successful_event_type> send_successful_event;
                            event<send_failure_event_type> send_failure_event;
                            event<receive_successful_event_type> receive_successful_event;
                            event<receive_failure_event_type> receive_failure_event;
                            event<connect_successful_event_type> connect_successful_event;
                            event<connect_failure_event_type> connect_failure_event;
                            event<accept_failure_event_type> accept_failure_event;
                            event<accept_successful_event_type> accept_successful_event;
                            event<disconnect_event_type> disconnect_event;

                            timer keep_alive_timer;
                            transport_layer_type transport_layer_;
                            keep_alive_threshold_type keep_alive_threshold;
                            string host;
                            uint32_t port;
                        };

                        NEXTGEN_SHARED_DATA(layer, variables);
                    };
                }

                namespace http
                {
                    class message : public message_base
                    {
                        public: struct state_type
                        {
                            static const uint32_t none = 0;
                            static const uint32_t remove_data_crlf = 1;
                        };

                        public: stream_type get_stream() const
                        {
                            auto self = *this;

                            return self->stream;
                        }

                        public: void pack() const
                        {
                            auto self = *this;

                            std::ostream data_stream(&self->stream.get_buffer());

                            if(self->status_code)
                            {
                                switch(self->status_code)
                                {
                                    case 200: self->status_description = "OK"; break;
                                    default: self->status_description = "UNDEFINED"; break;
                                }

                                std::string response_header = "HTTP/" + self->version + " " + to_string(self->status_code) + " " + self->status_description;

                                if(!self->header_list.empty())
                                // header list already exists
                                {
                                    if(self->username.length() && self->password.length())
                                    // add authentication into header list
                                    {
                                        if(self->header_list.find("Proxy-Authorization") != self->header_list.end())
                                        // authentication header doesn't already exist
                                        {
                                            //self->header_list["Proxy-Authorization"] = "Basic " << base64encode(ps->username + ":" + ps->password);
                                        }
                                    }

                                    // turn header list into raw header string
                                    for(header_list_type::iterator i = self->header_list.begin(), l = self->header_list.end(); i != l; ++i)
                                    {
                                        self->raw_header_list += (*i).first + ": " + (*i).second + "\r\n";
                                    }
                                }

                                self->raw_header_list += "Content-Length: " + to_string(self->content.length()) + "\r\n";

                                if(NEXTGEN_DEBUG_4)
                                {
                                    std::cout << response_header << "\r\n" << std::endl;
                                    std::cout << self->raw_header_list << "\r\n" << std::endl;
                                    std::cout << self->content << std::endl;
                                }

                                data_stream << response_header + "\r\n";
                                data_stream << self->raw_header_list + "\r\n";
                                data_stream << self->content;
                            }
                            else if(self->method.length())
                            {
                                if(!self->post_list.empty())
                                // parse post list
                                {

                                }

                                auto header_list = self->header_list;
                                string raw_header_list = "";
                                string content = self->content;

                                header_list_type::iterator i, l;

                                std::string request_header = self->method + " " + self->url + " " + "HTTP" + "/" + self->version;

                                if(header_list.empty())
                                // turn raw header string into a header list
                                {

                                }
                                else
                                // header list already exists
                                {

                                    bool content_length_exists;

                                    if(header_list.find("Content-Length") != header_list.end())
                                        content_length_exists = true;
                                    else
                                        content_length_exists = false;


                                    if(self->username.length() && self->password.length())
                                    // add authentication into header list
                                    {
                                        if(header_list.find("Proxy-Authorization") != header_list.end())
                                        // authentication header doesn't already exist
                                        {
                                            //self->header_list["Proxy-Authorization"] = "Basic " << base64encode(ps->username + ":" + ps->password);
                                        }
                                    }

                                    if((i = header_list.find("Host")) != header_list.end())
                                    {
                                        raw_header_list = (*i).first + ": " + (*i).second  + "\r\n" + raw_header_list;

                                        header_list.erase(i);
                                    }

                                    if((i = header_list.find("User-Agent")) != header_list.end())
                                    {
                                        raw_header_list += (*i).first + ": " + (*i).second  + "\r\n";

                                        // we're at the user agent header, so let's tell them what encoding our agent accepts
                                        raw_header_list += "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" "\r\n";
                                        raw_header_list += "Accept-Language: en-us,en;q=0.5" "\r\n";
                                        raw_header_list += "Accept-Encoding: gzip,deflate" "\r\n";
                                        raw_header_list += "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7" "\r\n";

                                        header_list.erase(i);
                                    }

                                    if((i = header_list.find("Keep-Alive")) != header_list.end())
                                    {
                                        raw_header_list += (*i).first + ": " + (*i).second  + "\r\n";

                                        header_list.erase(i);
                                    }

                                    if((i = header_list.find("Connection")) != header_list.end())
                                    {
                                        raw_header_list += (*i).first + ": " + (*i).second  + "\r\n";

                                        header_list.erase(i);
                                    }

                                    // turn header list into raw header string
                                    for(i = header_list.begin(), l = header_list.end(); i != l; ++i)
                                    {
                                        auto header_title = (*i).first;
                                        auto header_value = (*i).second;

                                        raw_header_list += header_title + ": " + header_value + "\r\n";
                                    }

                                    if(self->method == "POST" && !content_length_exists)
                                    // user didn't specify a content length, so we'll count it for them
                                        raw_header_list += "Content-Length: " + to_string(content.length()) + "\r\n";
                                }

                                if(NEXTGEN_DEBUG_4)
                                {
                                    std::cout << request_header << std::endl;
                                    std::cout << raw_header_list << std::endl;
                                    std::cout << content << std::endl;
                                }

                                data_stream << request_header + "\r\n";
                                data_stream << raw_header_list + "\r\n";
                                data_stream << content;
                            }
                        }

                        public: void unpack_headers() const
                        {
                            auto self = *this;

                            if(NEXTGEN_DEBUG_4)
                                std::cout << "XXXXXX" << self->stream.get_buffer().in_avail() << std::endl;

                            if(self->stream.get_buffer().in_avail())
                            {
                                std::istream data_stream(&self->stream.get_buffer());

                                string line;
                                std::getline(data_stream, line);

                                if(NEXTGEN_DEBUG_3)
                                    std::cout << "UNPACKING: " << line << std::endl;

                                string data((std::istreambuf_iterator<char>(data_stream)), std::istreambuf_iterator<char>());

                                size_t header_end = data.find("\r\n\r\n");

                                if(header_end != string::npos)
                                {
                                    self->raw_header_list = data.substr(0, header_end + 4);

                                    boost::erase_head(data, header_end + 4);
                                }

                                if(NEXTGEN_DEBUG_3)
                                    std::cout << "GARRRRR" << self->raw_header_list << std::endl;

                                boost::regex_error paren(boost::regex_constants::error_paren);

                                try
                                {
                                    boost::match_results<std::string::const_iterator> what;
                                    boost::match_flag_type flags = boost::regex_constants::match_perl | boost::regex_constants::format_perl;

                                    string::const_iterator start = line.begin();
                                    string::const_iterator end = line.end();

                                    // todo(daemn) fix line bug
                                    if(boost::regex_search(start, end, what, boost::regex("^HTTP\\/(.+?) ([0-9]+) (.+?)\r"), flags))
                                    {
                                        if(NEXTGEN_DEBUG_2)
                                            std::cout << "ZZZZZZZZZZ" << what[1] << what[2] << what[3] << std::endl;

                                        self->version = what[1];
                                        self->status_code = to_int(what[2]);
                                        self->status_description = what[3];
                                    }
                                    else
                                    {
                                        if(boost::regex_search(start, end, what, boost::regex("^(.+?) (.+?) HTTP\\/(.+?)\r"), flags))
                                        {
                                            if(NEXTGEN_DEBUG_2)
                                                std::cout << "YYYYYYYY " << what[1] << what[2] << what[3] << std::endl;

                                            self->method = what[1];
                                            self->path = what[2];
                                            self->version = what[3];
                                        }
                                        else
                                        {
                                            if(NEXTGEN_DEBUG_5)
                                                std::cout << "Error unpacking HTTP header." << std::endl;
                                        }
                                    }
                                }
                                catch(boost::regex_error const& e)
                                {
                                    std::cout << "regex error: " << (e.code() == paren.code() ? "unbalanced parentheses" : "?") << std::endl;
                                }

                                self->content = data;

                                self->header_list.clear();

                                try
                                {
                                    boost::match_results<std::string::const_iterator> what;
                                    boost::match_flag_type flags = boost::regex_constants::match_perl | boost::regex_constants::format_perl;

                                    string::const_iterator start = self->raw_header_list.begin();
                                    string::const_iterator end = self->raw_header_list.end();

                                    std::cout << "raw: " << self->raw_header_list << std::endl;

                                    self->header_list["set-cookie"] = "";

                                    while(boost::regex_search(start, end, what, boost::regex("(.+?)\\: (.+?)\r\n"), flags))
                                    {
                                        if(what[1].length() > 0)
                                        {


                                            std::string key = what[1];

                                            boost::to_lower(key);

                                            if(NEXTGEN_DEBUG_2)
                                                std::cout << "K: " << key << ": " << what[2] << std::endl;

                                            if(key == "set-cookie")
                                            {
                                                self->header_list[key] += what[2] + " ";
                                            }
                                            else
                                                self->header_list[key] = what[2];
                                        }

                                        // update search position:
                                        start = what[0].second;

                                        // update flags:
                                        flags |= boost::match_prev_avail;
                                        flags |= boost::match_not_bob;
                                    }
                                }
                                catch(boost::regex_error const& e)
                                {
                                    std::cout << "regex error: " << (e.code() == paren.code() ? "unbalanced parentheses" : "?") << std::endl;
                                }

                                if(self->status_code != 0)
                                // we're unpacking response headers
                                {


                                    // todo(daemn) look for content-length and set-cookie and content-type
                                }
                                else
                                // we're unpacking request headers
                                {

                                }
                            }
                        }

                        public: void unpack_content() const
                        {
                            auto self = *this;

                            if(NEXTGEN_DEBUG_4)
                            {
                                std::cout << "LEN!! " << self->stream.get_buffer().in_avail() << std::endl;
                                //std::cout << "LEN2!! " << self->raw_header_list.length() << std::endl;
                                //std::cout << "LEN3!! " << self->raw_header_list << std::endl;
                            }

                            std::istream data_stream(&self->stream.get_buffer());

                            self->content += string((std::istreambuf_iterator<char>(data_stream)), std::istreambuf_iterator<char>());
std::cout << "encoding: " << self->header_list["content-encoding"] << std::endl;
std::cout << "size: " << self->content.size() << std::endl;
                            if(self->header_list["content-encoding"] == "gzip"
                            && self->content.size() > 0)
                            {
                                std::vector<char> buffer;
                                std::string error;

                                if(NEXTGEN_DEBUG_4)
                                    std::cout << "unpacking compressed l: " << self->content.length() << std::endl;

                                if(inflate_gzip(self->content.data(), self->content.length(), buffer, 1024 * 1024, error))
                                {
                                    std::cout << "Error uncompressing: " << error << std::endl;
                                }

                                if(buffer.size() > 0)
                                    self->content = &buffer[0];
                                else
                                    std::cout << "No content after uncompression" << std::endl;
                            }
                        }

                        private: struct variables
                        {
                            variables() : status_code(0), version("1.1"), state(state_type::none)
                            {

                            }

                            ~variables()
                            {

                            }

                            referer_type referer;
                            url_type url;
                            content_type content;
                            header_list_type header_list;
                            raw_header_list_type raw_header_list;
                            status_code_type status_code;
                            //raw_post_list_type raw_post_list;
                            post_list_type post_list;
                            status_description_type status_description;
                            id_type id;
                            method_type method;
                            string path;
                            version_type version;
                            network_layer_type address;
                            port_type port;
                            host_type host;
                            stream_type stream;
                            string username;
                            string password;
                            string scheme;
                            uint32_t state;
                        };

                        NEXTGEN_SHARED_DATA(message, variables);
                    };

                    template<typename transport_layer_type>
                    class layer : public layer_base<message>
                    {
                        public: typedef layer<transport_layer_type> this_type;
                        public: typedef std::function<void(this_type)> accept_successful_event_type;
                        public: typedef base_event_type accept_failure_event_type;
                        public: typedef float keep_alive_threshold_type;

                        public: virtual void reconnect(connect_successful_event_type successful_handler2 = 0, connect_failure_event_type failure_handler2 = 0) const
                        {
                            auto self = *this;

                            if(NEXTGEN_DEBUG_5)
                                std::cout << "[nextgen::network::http_client] reconnecting" << std::endl;

                            self.disconnect();
                            self.connect(self->host, self->port, self->proxy_address, successful_handler2, failure_handler2);
                        }

                        public: virtual void connect(host_type const& host_, port_type port_, ipv4_address proxy = 0, connect_successful_event_type successful_handler2 = 0, connect_failure_event_type failure_handler2 = 0) const
                        {
                            auto self = *this;

                            auto successful_handler = successful_handler2; // bugfix(daemn) gah!!
                            auto failure_handler = failure_handler2; // bugfix(daemn) gah!!

                            if(successful_handler == 0)
                                successful_handler = self->connect_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->connect_failure_event;

                            self->host = host_;
                            self->port = port_;

                            std::string host;
                            uint32_t port;

                            if(proxy != 0)
                            {
                                host = proxy.get_host();
                                port = proxy.get_port();

                                self->proxy_address = proxy;
                            }
                            else
                            {
                                host = self->host;
                                port = self->port;
                            }

                            self->transport_layer_ = transport_layer_type(self->transport_layer_->service_);

                            self->transport_layer_.connect(host, port,
                            [=]
                            {
                                if(NEXTGEN_DEBUG_4)
                                    std::cout << "[nextgen::network::http_client] Connected" << std::endl;

                                std::cout << "proxy type: " << self->proxy << std::endl;

                                if(self->proxy == "socks4")
                                {
                                    hostent* host_entry = gethostbyname(self->host.c_str());

                                    if(host_entry == NULL)
                                    {
                                        failure_handler();

                                        return;
                                    }

                                    std::string addr = inet_ntoa(*(in_addr*)*host_entry->h_addr_list);

                                    byte_array r1;

                                    r1 << (byte)4;
                                    r1 << (byte)1;
                                    r1 << htons(self->port);
                                    r1 << inet_addr(addr.c_str());
                                    r1 << "PRO";

                                    if(NEXTGEN_DEBUG_4)
                                        std::cout << r1.to_string() << std::endl;

                                    self->transport_layer_.send(r1,
                                    [=]()
                                    {
                                        if(NEXTGEN_DEBUG_4)
                                            std::cout << "sent socks4 request" << std::endl;

                                        byte_array r2;

                                        self->transport_layer_.receive(asio::transfer_at_least(8), r2,
                                        [=]()
                                        {
                                            if(NEXTGEN_DEBUG_4)
                                                std::cout << "received socks4 response" << std::endl;

                                            byte none;
                                            byte status;

                                            r2 >> none;
                                            r2 >> status;

                                            if(NEXTGEN_DEBUG_4)
                                                std::cout << r2.to_string() << std::endl;

                                            switch(status)
                                            {
                                                case 0x5a:
                                                {
                                                    if(NEXTGEN_DEBUG_4)
                                                        std::cout << "is valid socks4 response" << std::endl;

                                                    successful_handler();
                                                }
                                                break;

                                                case 0x5b:
                                                {
                                                    if(NEXTGEN_DEBUG_4)
                                                        std::cout << "rejected socks4 response" << std::endl;

                                                    failure_handler();
                                                }
                                                break;

                                                case 0x5c:
                                                {
                                                    if(NEXTGEN_DEBUG_4)
                                                        std::cout << "failed1 socks4 response" << std::endl;

                                                    failure_handler();
                                                }
                                                break;

                                                case 0x52:
                                                {
                                                    if(NEXTGEN_DEBUG_4)
                                                        std::cout << "failed2 socks4 response" << std::endl;

                                                    failure_handler();
                                                }
                                                break;

                                                default:
                                                {
                                                    failure_handler();
                                                }
                                            }
                                        },
                                        [=]()
                                        {
                                            failure_handler();
                                        });
                                    },
                                    [=]()
                                    {
                                        failure_handler();
                                    });
                                }
                                else if(self->proxy == "socks5")
                                {
                                    byte_array r1;

                                    r1 << (byte)5;
                                    r1 << (byte)1;
                                    r1 << (byte)0;

                                    if(NEXTGEN_DEBUG_4)
                                        std::cout << r1.to_string() << std::endl;

                                    self->transport_layer_.send(r1,
                                    [=]()
                                    {
                                        if(NEXTGEN_DEBUG_4)
                                            std::cout << "sent socks5 request" << std::endl;

                                        byte_array r2;

                                        self->transport_layer_.receive(asio::transfer_at_least(2), r2,
                                        [=]()
                                        {
                                            if(NEXTGEN_DEBUG_4)
                                                std::cout << "received socks5 response" << std::endl;

                                            byte status;
                                            byte status2;

                                            r2 >> status;
                                            r2 >> status2;

                                            if(NEXTGEN_DEBUG_4)
                                                std::cout << r2.to_string() << std::endl;

                                            if(status == 0x05 && status2 == 0x00)
                                            {
                                                hostent* host_entry = gethostbyname(self->host.c_str());
                                                std::string addr = inet_ntoa(*(in_addr*)*host_entry->h_addr_list);

                                                if(host_entry == NULL)
                                                {
                                                    failure_handler();

                                                    return;
                                                }

                                                byte_array r3;

                                                r3 << (byte)5;
                                                r3 << (byte)1;
                                                r3 << (byte)0;
                                                r3 << (byte)1;
                                                r3 << inet_addr(addr.c_str());
                                                r3 << htons(self->port);

                                                if(NEXTGEN_DEBUG_4)
                                                    std::cout << r3.to_string() << std::endl;

                                                self->transport_layer_.send(r3,
                                                [=]()
                                                {
                                                    byte_array r4;

                                                    self->transport_layer_.receive(asio::transfer_at_least(1), r4,
                                                    [=]()
                                                    {
                                                        byte nothing;
                                                        byte address_type;

                                                        r4 << nothing;
                                                        r4 << address_type;

                                                        // todo(daemn) we've gotten this far, assume we're good.
                                                        successful_handler();
                                                    },
                                                    [=]()
                                                    {
                                                        failure_handler();
                                                    });
                                                },
                                                [=]()
                                                {
                                                    failure_handler();
                                                });
                                            }
                                            else
                                            {
                                                failure_handler();
                                            }
                                        },
                                        [=]()
                                        {
                                            failure_handler();
                                        });
                                    },
                                    [=]()
                                    {
                                        failure_handler();
                                    });
                                }
                                else
                                {
                                    successful_handler();
                                }
                            },
                            [=]
                            {
                                failure_handler();
                            });
                        }

                        public: virtual void disconnect() const
                        {
                            auto self = *this;

                            self->transport_layer_.close();
                        }

                        public: virtual void send(message_type request_, send_successful_event_type successful_handler = 0, send_failure_event_type failure_handler = 0) const
                        {
                            auto self = *this;

                            request_.pack();

                            self.send(request_->stream, successful_handler, failure_handler);
                        }

                        public: virtual void send_and_receive(message_type request_, receive_successful_event_type successful_handler = 0, receive_failure_event_type failure_handler = 0) const
                        {
                            auto self = *this;

                            if(successful_handler == 0)
                                successful_handler = self->receive_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->receive_failure_event;

                            request_.pack();

                            self.send(request_->stream,
                            [=]()
                            {
                                request_->stream.get_buffer(); // bugfix(daemn)

                                if(NEXTGEN_DEBUG_4)
                                    std::cout << "http message sent, now receiving" << std::endl;

                                self.receive(successful_handler, failure_handler);
                            },
                            [=]()
                            {
                                failure_handler();
                            });
                        }

                        public: template<typename stream_type> void send(stream_type stream, send_successful_event_type successful_handler = 0, send_failure_event_type failure_handler = 0) const
                        {
                            auto self = *this;

                            if(successful_handler == 0)
                                successful_handler = self->send_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->send_failure_event;

                            self->transport_layer_.send(stream,
                            [=]()
                            {
                                successful_handler();
                            },
                            [=]()
                            {
                                failure_handler();
                            });
                        }

                        private: virtual void receive_chunked_data(message_type response, size_t length = 1, base_event_type successful_handler = 0, base_event_type failure_handler = 0) const
                        {
                            auto self = *this;

                            std::cout << "receive_chunked_data" << std::endl;

                            self->transport_layer_.receive(asio::transfer_at_least(length), response->stream,
                            [=]()
                            {
                                std::istream data_stream(&response->stream.get_buffer());

                                response->content += string((std::istreambuf_iterator<char>(data_stream)), std::istreambuf_iterator<char>());

                                        std::cout << "CONTENT L BEFORE: " << response->content.size() << std::endl;

                                        std::cout << "CONTENT BEFORE: " <<  to_hex(response->content) << std::endl;

                                //int l = static_cast<int>(strtol(length, NULL, 16));


                                uint32_t length;
                                size_t pos;
                                std::string data;

                                while(true)
                                {
                                    if(to_int(response->header_list["content-length"]) >= response->content.size())
                                    {
                                        size_t recv_amount = to_int(response->header_list["content-length"]) - response->content.size();

                                        if(recv_amount == 0) recv_amount = 1;

                                        self.receive_chunked_data(response, recv_amount, successful_handler, failure_handler);
                                    }
                                    else if(to_int(response->header_list["content-length"]) < response->content.size())
                                    {
                                        if(response->state == message_type::state_type::remove_data_crlf)
                                        {
                                            // erase newline
                                            response->content.erase(to_int(response->header_list["content-length"]), 2);

                                            response->state = message_type::state_type::none;
                                        }

                                        pos = response->content.find("\r\n", to_int(response->header_list["content-length"]));

                                        if(pos == std::string::npos)
                                        {
                                            failure_handler();

                                            return;
                                        }

                                        std::cout << "getline_intern " << pos << std::endl;

                                        data = response->content.substr(to_int(response->header_list["content-length"]), pos - to_int(response->header_list["content-length"]));

                                        response->content.erase(to_int(response->header_list["content-length"]), pos - to_int(response->header_list["content-length"]) + 2);

                                        length = readHex(data.c_str());

                                        std::cout << "chunked length hex: " << data.c_str() << std::endl;
                                        std::cout << "chunked length: " << length << std::endl;

                                        response->header_list["content-length"] = to_string(to_int(response->header_list["content-length"]) + length);

                                        if(response->content.size() >= (to_int(response->header_list["content-length"]) + 2))
                                        {
                                            // erase newline
                                            response->content.erase(to_int(response->header_list["content-length"]), 2);
                                        }
                                        else
                                        {
                                            response->state = message_type::state_type::remove_data_crlf;
                                        }

                                        std::cout << "CHUNK: " << to_int(response->header_list["content-length"]) << " / " << response->content.size() << std::endl;


                                        if(length == 0)
                                        {
                                            //response->content.erase(response->content.size() - 2, 2); //blank line

                                            std::cout << "after: " << to_hex(response->content) << std::endl;

                                            successful_handler();
                                        }
                                        else
                                            continue;
                                    }

                                    break;
                                }
                            },
                            [=]()
                            {
                                failure_handler();
                            });
                        }

                        public: virtual void receive(receive_successful_event_type successful_handler = 0, receive_failure_event_type failure_handler = 0) const
                        {
                            auto self = *this;

                            if(successful_handler == 0)
                                successful_handler = self->receive_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->receive_failure_event;

                            message_type response2;
                            auto response = response2; // bugfix(daemn)

                            self->transport_layer_.receive_until("\r\n\r\n", response->stream,
                            [=]()
                            {
                                response.unpack_headers();

                                if(NEXTGEN_DEBUG_3)
                                    std::cout << "Z: " << response->raw_header_list << std::endl;

                                if(response->stream.get_buffer().in_avail())
                                {
                                    response.unpack_content();

                                    if(NEXTGEN_DEBUG_3)
                                        std::cout << "Y: " << response->content << std::endl;
                                }

                                if(response->status_code != 0 || response->method == "POST")
                                // this http message has content, and we need to know the length
                                {
                                    if(response->header_list.find("transfer-encoding") != response->header_list.end()
                                    &&response->header_list["transfer-encoding"] == "chunked")
                                    {
                                        std::cout << "unchunking encoding" << std::endl;

                                        std::istream data_stream(&response->stream.get_buffer());

                                        response->header_list["content-length"] = "0";

                                        response->content += string((std::istreambuf_iterator<char>(data_stream)), std::istreambuf_iterator<char>());

                                        std::cout << "CONTENT L BEFORE: " << response->content.size() << std::endl;

                                        std::cout << "CONTENT BEFORE: " <<  to_hex(response->content) << std::endl;

                                        uint32_t length;
                                        size_t pos;
                                        std::string data;

                                        while(true)
                                        {
                                            if(to_int(response->header_list["content-length"]) >= response->content.size())
                                            {
                                                size_t recv_amount = to_int(response->header_list["content-length"]) - response->content.size();

                                                if(recv_amount == 0) recv_amount = 1;

                                                self.receive_chunked_data(response, recv_amount, [=]
                                                {
                                                    response.unpack_content();

                                                    successful_handler(response);
                                                },
                                                [=]()
                                                {
                                                    if(NEXTGEN_DEBUG_5)
                                                        std::cout << "failed to receive rest of chunked encoding" << std::endl;

                                                    failure_handler();
                                                });
                                            }
                                            else if(to_int(response->header_list["content-length"]) < response->content.size())
                                            {
                                        if(response->state == message_type::state_type::remove_data_crlf)
                                        {
                                            // erase newline
                                            response->content.erase(to_int(response->header_list["content-length"]), 2);

                                            response->state = message_type::state_type::none;
                                        }

                                                pos = response->content.find("\r\n", to_int(response->header_list["content-length"]));

                                                if(pos == std::string::npos)
                                                {
                                                    failure_handler();

                                                    return;
                                                }


                                                std::cout << "getline_intern " << pos << std::endl;

                                                data = response->content.substr(to_int(response->header_list["content-length"]), pos - to_int(response->header_list["content-length"]));

                                                response->content.erase(to_int(response->header_list["content-length"]), pos - to_int(response->header_list["content-length"]) + 2);

                                                length = readHex(data.c_str());

                                                std::cout << "chunked length hex: " << data.c_str() << std::endl;
                                                std::cout << "chunked length: " << length << std::endl;

                                                response->header_list["content-length"] = to_string(to_int(response->header_list["content-length"]) + length);

                                        if(response->content.size() >= (to_int(response->header_list["content-length"]) + 2))
                                        {
                                            // erase newline
                                            response->content.erase(to_int(response->header_list["content-length"]), 2);
                                        }
                                        else
                                        {
                                            response->state = message_type::state_type::remove_data_crlf;
                                        }

                                                std::cout << "CHUNK: " << to_int(response->header_list["content-length"]) << " / " << response->content.size() << std::endl;


                                                if(length == 0)
                                                {
                                                    //response->content.erase(response->content.size() - 2, 2); //blank line


                                                std::cout << "after: " << to_hex(response->content) << std::endl;

                                                    successful_handler(response);
                                                }
                                                else
                                                    continue;
                                            }

                                            break;
                                        }
                                    }
                                    else if(response->header_list.find("content-length") != response->header_list.end())
                                    {
                                        if(NEXTGEN_DEBUG_3)
                                            std::cout << "VVVVVVVVVVVVV " << response->header_list["content-length"] << " VVVVVV " << response->header_list["Content-Length"].length() << std::endl;

                                        auto content_length = to_int(response->header_list["content-length"]) - response->content.length();

                                        if(content_length == 0)
                                        {
                                            successful_handler(response);
                                        }
                                        else
                                        {
                                            if(NEXTGEN_DEBUG_4)
                                                std::cout << "trying to receive length = " << content_length << std::endl;

                                            self->transport_layer_.receive(asio::transfer_at_least(content_length), response->stream,
                                            [=]()
                                            {
                                                response.unpack_content();

                                                successful_handler(response);
                                            },
                                            [=]()
                                            {
                                                if(NEXTGEN_DEBUG_5)
                                                    std::cout << "failed to receive rest of length but still gonna success" << std::endl;

                                                response.unpack_content();

                                                successful_handler(response);
                                            });
                                        }
                                    }
                                    else
                                    {
                                        if(response->status_code == 204)
                                        {
                                            if(NEXTGEN_DEBUG_5)
                                                std::cout << "No http content-length specified due to 204" << std::endl;

                                            response.unpack_content();

                                            successful_handler(response);
                                        }
                                        else
                                        {
                                            if(NEXTGEN_DEBUG_5)
                                                std::cout << "No http content-length specified so assuming its auto EOF" << std::endl;

                                            self->transport_layer_.receive(asio::transfer_at_least(1), response->stream,
                                            [=]()
                                            {
                                                response.unpack_content();

                                                successful_handler(response);
                                            },
                                            [=]()
                                            {
                                                if(NEXTGEN_DEBUG_5)
                                                    std::cout << "failed to receive after no content-length" << std::endl;

                                                failure_handler();
                                            });
                                        }
                                    }
                                }
                                else
                                // this http message just contains raw headers
                                {
                                    if(response->raw_header_list.length() > 0)
                                        successful_handler(response);
                                    else
                                    {
                                        self->transport_layer_.close();

                                        failure_handler();
                                    }
                                }
                            },
                            [=]()
                            {
                                failure_handler();
                            });
                        }

                        public: virtual void accept(port_type port_, accept_successful_event_type successful_handler2 = 0, accept_failure_event_type failure_handler2 = 0)
                        {
                            auto self = *this;

                            auto successful_handler = successful_handler2; // bugfix(daemn) gah!!
                            auto failure_handler = failure_handler2; // bugfix(daemn) gah!!

                            if(successful_handler == null)
                                successful_handler = self->accept_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->accept_failure_event;

                            self->transport_layer_.accept(port_,
                            [=](transport_layer_type client)
                            {
                                successful_handler(this_type(client));
                            },
                            [=]()
                            {
                                failure_handler();
                            });
                        }

                        public: virtual bool is_alive() const
                        {
                            auto self = *this;

                            return (self->keep_alive_threshold == 0) ? true : (self->keep_alive_threshold > self->keep_alive_timer.stop());
                        }

                        public: virtual void set_keep_alive(keep_alive_threshold_type keep_alive_threshold) const
                        {
                            auto self = *this;

                            self->keep_alive_threshold = keep_alive_threshold;
                        }

                        private: struct variables
                        {
                            variables(service_type service_) : transport_layer_(service_), keep_alive_threshold(0), proxy(""), host(""), port(0)
                            {

                            }

                            variables(transport_layer_type transport_layer_) : transport_layer_(transport_layer_), keep_alive_threshold(0), proxy(""), host(""), port(0)
                            {

                            }

                            ~variables()
                            {

                            }

                            event<send_successful_event_type> send_successful_event;
                            event<send_failure_event_type> send_failure_event;
                            event<receive_successful_event_type> receive_successful_event;
                            event<receive_failure_event_type> receive_failure_event;
                            event<connect_successful_event_type> connect_successful_event;
                            event<connect_failure_event_type> connect_failure_event;
                            event<accept_failure_event_type> accept_failure_event;
                            event<accept_successful_event_type> accept_successful_event;
                            event<disconnect_event_type> disconnect_event;

                            timer keep_alive_timer;
                            transport_layer_type transport_layer_;
                            keep_alive_threshold_type keep_alive_threshold;
                            string proxy;
                            ipv4_address proxy_address;
                            string host;
                            uint32_t port;
                        };

                        NEXTGEN_SHARED_DATA(layer, variables);
                    };
                }

                namespace xml
                {
                    class message : public message_base
                    {
                        typedef string data_type;

                        public: stream_type get_stream() const
                        {
                            auto self = *this;

                            return self->stream;
                        }

                        public: void pack() const
                        {
                            auto self = *this;

                            std::ostream data_stream(&self->stream.get_buffer());

                            data_stream << self->data;
                        }

                        public: void unpack() const
                        {
                            auto self = *this;

                            if(self->stream.get_buffer().in_avail())
                            {
                                std::istream data_stream(&self->stream.get_buffer());

                                self->data = string((std::istreambuf_iterator<char>(data_stream)), std::istreambuf_iterator<char>());
                            }

                        }

                        private: struct variables
                        {
                            variables()
                            {

                            }

                            ~variables()
                            {

                            }

                            data_type data;
                            stream_type stream;
                        };

                        NEXTGEN_SHARED_DATA(message, variables);
                    };

                    template<typename transport_layer_type>
                    class layer : public layer_base<message>
                    {
                        public: typedef layer<transport_layer_type> this_type;
                        public: typedef std::function<void(this_type)> accept_successful_event_type;
                        public: typedef base_event_type accept_failure_event_type;

                        public: virtual void connect(host_type const& host_, port_type port_, connect_successful_event_type successful_handler = 0, connect_failure_event_type failure_handler = 0) const
                        {
                            auto self = *this;

                            if(successful_handler == 0)
                                successful_handler = self->connect_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->connect_failure_event;

                            self->transport_layer_.connect(host_, port_,
                            [=]
                            {
                                successful_handler();
                            },
                            [=]
                            {
                                failure_handler();
                            });
                        }

                        public: virtual void disconnect() const
                        {
                            auto self = *this;

                            self->transport_layer_.close();
                        }

                        public: virtual void send(message_type request_, send_successful_event_type successful_handler = 0, send_failure_event_type failure_handler = 0) const
                        {
                            auto self = *this;

                            request_.pack();

                            self.send(request_.get_stream().get_buffer(), successful_handler, failure_handler);
                        }

                        public: template<typename stream_type> void send(stream_type& stream, send_successful_event_type successful_handler = 0, send_failure_event_type failure_handler = 0) const
                        {
                            auto self = *this;

                            if(successful_handler == 0)
                                successful_handler = self->send_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->send_failure_event;

                            self->transport_layer_.send(stream,
                            [=]()
                            {
                                successful_handler();
                            },
                            [=]()
                            {
                                failure_handler();
                            });
                        }

                        public: virtual void receive(receive_successful_event_type successful_handler = 0, receive_failure_event_type failure_handler = 0) const
                        {
                            auto self = *this;

                            if(successful_handler == 0)
                                successful_handler = self->receive_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->receive_failure_event;

                            message_type response;

                            self->transport_layer_.receive("#all#", response.get_stream().get_buffer(),
                            [=]()
                            {
                                response.unpack();

                                successful_handler(response);
                            },
                            [=]()
                            {
                                failure_handler();
                            });
                        }

                        public: virtual void accept(port_type port_, accept_successful_event_type successful_handler = 0, accept_failure_event_type failure_handler = 0)
                        {
                            auto self = *this;

                            self->transport_layer_.accept(port_,
                            [=](transport_layer_type client)
                            {
                                successful_handler(this_type(client));
                            },
                            [=]()
                            {
                                failure_handler();
                            });
                        }

                        private: struct variables
                        {
                            variables(service_type service_) : transport_layer_(service_)
                            {

                            }

                            variables(transport_layer_type transport_layer_) : transport_layer_(transport_layer_)
                            {

                            }

                            ~variables()
                            {

                            }

                            event<send_successful_event_type> send_successful_event;
                            event<send_failure_event_type> send_failure_event;
                            event<receive_successful_event_type> receive_successful_event;
                            event<receive_failure_event_type> receive_failure_event;
                            event<connect_successful_event_type> connect_successful_event;
                            event<connect_failure_event_type> connect_failure_event;
                            event<accept_failure_event_type> accept_failure_event;
                            event<accept_successful_event_type> accept_successful_event;

                            transport_layer_type transport_layer_;
                        };

                        NEXTGEN_SHARED_DATA(layer, variables);
                    };
                }

                namespace ngp
                {
                    class message : public message_base
                    {
                        public: void pack() const
                        {
                            auto self = *this;

                            self->stream << self->id << self->data.length() << self->data;
                        }

                        public: void unpack() const
                        {
                            auto self = *this;

                            self->stream->little_endian = true;

                            if(self->stream.available() >= 8)
                            {
                                self->stream >> self->id;
                                self->stream >> self->length;

                                if(self->stream.available() >= self->length)
                                {
                                    self->data = byte_array(self->stream, self->length);
                                }
                            }
                        }

                        private: struct variables
                        {
                            variables()
                            {

                            }

                            ~variables()
                            {

                            }

                            uint32_t id;
                            uint32_t length;
                            byte_array data;
                            byte_array stream;
                        };

                        NEXTGEN_SHARED_DATA(message, variables);
                    };

                    template<typename transport_layer_type>
                    class layer : public layer_base<message>
                    {
                        public: typedef layer<transport_layer_type> this_type;
                        public: typedef std::function<void(this_type)> accept_successful_event_type;
                        public: typedef base_event_type accept_failure_event_type;

                        public: virtual void connect(host_type const& host_, port_type port_, connect_successful_event_type successful_handler = 0, connect_failure_event_type failure_handler = 0) const
                        {
                            auto self = *this;
                        }

                        public: virtual void disconnect() const
                        {
                            auto self = *this;

                            self->transport_layer_.close();
                        }

                        public: virtual void send(message_type request_, send_successful_event_type successful_handler = 0, send_failure_event_type failure_handler = 0) const
                        {
                            auto self = *this;

                            request_.pack();

                            self.send(request_->stream.get_buffer(), successful_handler, failure_handler);
                        }

                        public: template<typename stream_type> void send(stream_type& stream, send_successful_event_type successful_handler = 0, send_failure_event_type failure_handler = 0) const
                        {
                            auto self = *this;

                            if(successful_handler == 0)
                                successful_handler = self->send_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->send_failure_event;

                            self->transport_layer_.send(stream,
                            [=]()
                            {
                                successful_handler();
                            },
                            [=]()
                            {
                                failure_handler();
                            });
                        }

                        public: virtual void receive(receive_successful_event_type successful_handler2 = 0, receive_failure_event_type failure_handler2 = 0) const
                        {
                            auto self2 = *this;
                            auto self = self2; // bugfix(daemn) gah!!

                            auto successful_handler = successful_handler2; // bugfix(daemn) gah!!
                            auto failure_handler = failure_handler2; // bugfix(daemn) gah!!

                            if(successful_handler == 0)
                                successful_handler = self->receive_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->receive_failure_event;

                            message_type response2;
                            auto response = response2; // bugfix(daemn)

                            self->transport_layer_.receive("#all#", response->stream.get_buffer(),
                            [=]()
                            {
                                response.unpack();

                                successful_handler(response);

                                self.receive(successful_handler, failure_handler);
                            },
                            [=]()
                            {
                                failure_handler();
                            });
                        }

                        public: virtual void accept(port_type port, accept_successful_event_type successful_handler2 = 0, accept_failure_event_type failure_handler2 = 0)
                        {
                            auto self = *this;

                            auto successful_handler = successful_handler2; // bugfix(daemn) gah!!
                            auto failure_handler = failure_handler2; // bugfix(daemn) gah!!

                            if(successful_handler == 0)
                                successful_handler = self->accept_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->accept_failure_event;

                            self->transport_layer_.accept(port,
                            [=](transport_layer_type client)
                            {
                                successful_handler(this_type(client));
                            },
                            [=]()
                            {
                                failure_handler();
                            });
                        }

                        private: struct variables
                        {
                            variables(service_type service_) : transport_layer_(service_)
                            {

                            }

                            variables(transport_layer_type transport_layer_) : transport_layer_(transport_layer_)
                            {

                            }

                            ~variables()
                            {

                            }

                            event<send_successful_event_type> send_successful_event;
                            event<send_failure_event_type> send_failure_event;
                            event<receive_successful_event_type> receive_successful_event;
                            event<receive_failure_event_type> receive_failure_event;
                            event<connect_successful_event_type> connect_successful_event;
                            event<connect_failure_event_type> connect_failure_event;
                            event<accept_failure_event_type> accept_failure_event;
                            event<accept_successful_event_type> accept_successful_event;

                            transport_layer_type transport_layer_;
                        };

                        NEXTGEN_SHARED_DATA(layer, variables);
                    };
                }
            }
        }

        typedef ip::application::smtp::layer<tcp_socket> smtp_client;
        typedef ip::application::smtp::message smtp_message;

        typedef ip::application::http::layer<tcp_socket> http_client;
        typedef ip::application::http::message http_message;

        typedef ip::application::xml::layer<tcp_socket> xml_client;
        typedef ip::application::xml::message xml_message;

        typedef ip::application::ngp::layer<tcp_socket> ngp_client;
        typedef ip::application::ngp::message ngp_message;

        template<typename layer>
        class server_base
        {
            public: typedef layer client_type;
            public: typedef uint32_t port_type;

            public: typedef std::function<void()> base_event_type;
            public: typedef std::function<void(client_type)> accept_successful_event_type;
            public: typedef base_event_type accept_failure_event_type;
        };

        template<typename layer_type>
        class server : public server_base<layer_type> // todo(daemn) inheriting doesnt seem to be working
        {
            public: typedef service service_type;
            public: typedef layer_type server_type;
            public: typedef layer_type client_type;
            public: typedef std::list<client_type> client_list_type;
            public: typedef uint32_t port_type;
            public: typedef std::function<void()> base_event_type;
            public: typedef std::function<void(client_type)> accept_successful_event_type;
            public: typedef base_event_type accept_failure_event_type;

            public: void accept(accept_successful_event_type successful_handler2 = 0, accept_failure_event_type failure_handler2 = 0)
            {
                auto self2 = *this;
                auto self = self2;

                if(NEXTGEN_DEBUG_2)
                    std::cout << "[nextgen:network:server:accept] Waiting for client..." << std::endl;

                auto successful_handler = successful_handler2; // bugfix(daemn) gah!!
                auto failure_handler = failure_handler2; // bugfix(daemn) gah!!

                if(successful_handler == 0)
                    successful_handler = self->accept_successful_event;

                if(failure_handler == 0)
                    failure_handler = self->accept_failure_event;

                self->server_.accept(self->port,
                [=](client_type client)
                {
                    auto self3 = self; // bugfix(daemn) wow..... no stack

                    if(NEXTGEN_DEBUG_2)
                        std::cout << "[nextgen::network::server::accept] Successfully accepted client." << std::endl;

                    self->client_list.push_back(client);

                    auto i = self->client_list.end();

                    client.set_keep_alive(240);

                    client->disconnect_event += [=]()
                    {
                        if(NEXTGEN_DEBUG_4)
                            std::cout << "ERASE SERVER CLIENT" << std::endl;

                        self3.remove_client(client);
                    };

                    client->transport_layer_->close_event += client->disconnect_event; // temp(daemn) event not saving callback refs or something

                    successful_handler(client);
                },
                [=]()
                {
                    if(NEXTGEN_DEBUG_2)
                        std::cout << "[nextgen::network::server::accept] Failed to accept client." << std::endl;

                    failure_handler();
                });
            }

            public: void remove_client(client_type client) const
            {
                auto self = *this;

                auto i = std::find(self->client_list.begin(), self->client_list.end(), client);

                if(i != self->client_list.end())
                    self->client_list.erase(i);
            }

            public: void disconnect()
            {
                auto self = *this;

                std::for_each(self->client_list.begin(), self->client_list.end(), [=](client_type& client)
                {
                    client.disconnect();

                    if(NEXTGEN_DEBUG_4)
                        std::cout << "DISCONNECT SERVER CLIENT" << std::endl;
                });

                self->client_list.reset();
            }

            public: void clean()
            {
                auto self = *this;

                if(NEXTGEN_DEBUG_4)
                    std::cout << "[nextgen:server] Cleaning out expired clients.";

                std::remove_if(self->client_list.begin(), self->client_list.end(), [=](client_type& client) -> bool
                {
                    if(client.is_alive())
                    {
                        return false;
                    }
                    else
                    {
                        if(NEXTGEN_DEBUG_4)
                            std::cout << ".";

                        client.disconnect();

                        return true;
                    }
                });

                if(NEXTGEN_DEBUG_4)
                    std::cout << std::endl;
            }

            private: struct variables
            {
                variables(service_type service_, port_type port) : service_(service_), server_(service_), port(port)
                {

                }

                ~variables()
                {

                }

                event<accept_failure_event_type> accept_failure_event;
                event<accept_successful_event_type> accept_successful_event;

                service_type service_;
                server_type server_;
                port_type port;
                client_list_type client_list;
            };

            NEXTGEN_SHARED_DATA(server, variables);
        };

        typedef server<http_client> http_server;
        typedef server<smtp_client> smtp_server;
        typedef server<xml_client> xml_server;
        typedef server<ngp_client> ngp_server;

        template<typename layer_type>
        void create_server(service service_, uint32_t port, std::function<void(layer_type)> successful_handler = 0, std::function<void()> failure_handler = 0)
        {
            if(NEXTGEN_DEBUG_2)
                std::cout << "[nextgen:network:server:accept] Waiting for client..." << std::endl;

            layer_type server(service_);

            server.accept(port,
            [=](layer_type client)
            {
                if(NEXTGEN_DEBUG_2)
                    std::cout << "[nextgen::network::server::accept] Successfully accepted client." << std::endl;

                if(successful_handler != 0)
                    successful_handler(client);
            },
            [=]()
            {
                if(NEXTGEN_DEBUG_2)
                    std::cout << "[nextgen::network::server::accept] Failed to accept client." << std::endl;

                if(failure_handler != 0)
                    failure_handler();
            });
        }
    }
}

#endif
