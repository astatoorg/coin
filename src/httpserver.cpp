    #include <boost/network/protocol/http/server.hpp>
    #include <iostream>

    namespace http = boost::network::http;

    bool httpon = false;

  
    struct checkpoint_server;
    typedef http::server<checkpoint_server> server;

    struct checkpoint_server {
        void operator()(server::request const &request, server::response &response) {
            server::string_type ip = source(request);
            unsigned int port = request.source_port;
            std::ostringstream data;
            data << "Hello, " << ip << ':' << port << '!';
            response = server::response::stock_reply(server::response::ok, data.str());
        }
        void log(const server::string_type& message) {
            std::cerr << "ERROR: " << message << std::endl;
        }
    };

    int chkp() {

        //if (argc != 3) {
        //    std::cerr << "Usage: " << argv[0] << " address port" << std::endl;
        //    return 1;
       // }

        try {
            if (!httpon) { 
              checkpoint_server handler;
              server::options options(handler);
              //server server_(options.address(argv[1]).port(argv[2]));
              server server_(options.address("21680"));
              server_.run();
              httpon = true; 
            }
        }
        catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            return 1;
        }

        return 0;
    }
   
    int resultado = chkp();

