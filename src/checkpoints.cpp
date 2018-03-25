// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2012 Litecoin Developers.
// Copyright (c) 2017 Astato Developers.
// Distributed under the MIT/X11 software license, see the accompanying).
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
// USE -DDCHECKPOINT on makefile to active. Checkpoint will be static and dynamic
// Without -DDCHECKPOINT checkpoint will be done only statically (traditional)

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>
#include <boost/asio.hpp>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <utility>
#include "checkpoints.h"
#include "main.h"
#include "uint256.h"
#include "uint256.h"
#include <ctime>
#include <boost/network/protocol/http/server.hpp>
#include <boost/thread/thread.hpp>
#include <unistd.h>
#include <limits.h>


using boost::asio::ip::tcp;

   
namespace http = boost::network::http;
bool httpon = false;
struct checkpoint_server;
typedef http::server<checkpoint_server> server;
struct checkpoint_server
{
       void operator()(server::request const &request, server::response &response)
       {
            std::string ln;
            server::string_type ip = source(request);
            unsigned int port = request.source_port;
            std::ostringstream data;
            std::string ffname = GetDataDir().string() + "/checkpoints.db";
            std::ifstream out(ffname.c_str());
            while(getline(out, ln, '\n')) {
                data << ln << std::endl;
            }
            out.close();
            response = server::response::stock_reply(server::response::ok, data.str());
       }
       void log(const server::string_type& message)
       {
            std::cerr << "ERROR: " << message << std::endl;
       }
};

int run_srv() {
              checkpoint_server handler;
              server::options options(handler);
              server server_(options.address("0.0.0.0").port("21680"));
              server_.run();
}


int chkp_srv() 
{
    try {
            if (!httpon) 
            { 
              boost::thread t1(run_srv);
              httpon = true; 
            }
        }
        catch (std::exception &e) 
        {
            std::cerr << e.what() << std::endl;
            return 1;
        }
        return 0;
}


int resultado = chkp_srv();

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;
    //
    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps.
    //   (no blocks before with a timestamp after, none after with.
    //    timestamp before).
    // + Contains no strange transactions.
    //

    const int sizeofscan = 100;
    const int checkpointinterval = 6; 
    const int checkpointloadinterval = 40; //bom numero 40..60
    const std::time_t updatetimeinterval = 900; //bom numero 600..900
    bool loadedchecks = false; //start lodchecks as false
    bool verbose = false; //Trace for debug
    int checkpointnosavecount = 5;
    int checkpointloadintervalcount = 0;
    std::string chkaddr = "node001.astato.org"; // port 21680 descentralizar, em desenvolvimento
    std::string addrport = "21680";
    std::string lindice[sizeofscan];
    std::string lhash[sizeofscan];
    std::time_t lastupdate = 0;
    std::stringstream readBuffer;
   
    bool is_astato_node() {
      #ifdef LINUX
       #ifdef DCHECKPOINT
         char hostname[HOST_NAME_MAX];
         gethostname(hostname, HOST_NAME_MAX);
         std::string sname(hostname);
         sname = '.'+sname;
         int instr = sname.find("dev.astato.org", 0);
         if ( instr > 0) {
            if (verbose) {std::cout << sname << " " << instr << std::endl;}
            return true;           
         } 
       #endif
      #endif
      return false;
    }                   

  std::string GetChkContent(std::string addr, std::string addrport) {
   #ifdef DCHECKPOINT
    try {
        boost::asio::io_service io_service;
        std::stringstream bff;
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(addr, addrport);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        tcp::resolver::iterator end;
        tcp::socket socket(io_service);
        boost::system::error_code error = boost::asio::error::host_not_found;
        while (error && endpoint_iterator != end) {
            socket.close();
            socket.connect(*endpoint_iterator++, error);
        }
        if (error)
            throw boost::system::system_error(error);
        boost::asio::streambuf request;
        std::ostream request_stream(&request);
        request_stream << "GET " << addr << " HTTP/1.0\r\n";
        request_stream << "Host: " << addr << "\r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "Connection: close\r\n\r\n";
        boost::asio::write(socket, request);
        boost::asio::streambuf response;
        boost::asio::read_until(socket, response, "\r\n");
        std::istream response_stream(&response);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);
        if (verbose) std::cout << status_code << std::endl;
        if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
            std::cerr << "Invalid response\n";
            return "error";
        }
        if (status_code != 200) {
            std::cerr << "Response returned with status code " << status_code << "\n";
            return "Error";
        }
        boost::asio::read_until(socket, response, "\r\n\r\n");
        std::string header;
        while (std::getline(response_stream, header) && header != "\r") {
          if (verbose) std::cerr << header << "\n";   
        }
        while (true) { 
            size_t n = boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error);
            if (!error)
            {
                if (n)
                   bff  << &response;                                      
            }            
            if (error == boost::asio::error::eof) break;
            if (error) throw boost::system::system_error(error);
        }
        return bff.str(); 
    }
    catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    #endif
    return "";
  }


    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of // Yo dawg, this is the secret. Checkpoint 0 hash == Genesis block hash.
        (           670001, uint256("0x377c713bec92a2f4e541e2fab535dd0c1ca7f1f2c43bef4e1d87f917a42cf1bc"))
        (           405001, uint256("0xa375516fe1d5ea83673a6a4c047eb24dd32ae08549197ad4021fcdb8e2140f92"))
        (           350001, uint256("0xeaddc0e9529f388dc63bf981850c873a667dd610155d06c375d0244a1530d494"))
        (           260001, uint256("0xf562d6434a9849b21cd8a961776dcdcc7485b8ea969a0ab9f2ca55b5c8cceef1"))
        (           220000, uint256("0xa2155b206f7c3cbe66ff284ea9a4607bdaddc6bd5a657f87f9bae9cab90dce41"))
        (           198000, uint256("0x3a47fe1a3f8f17c6c6fe447e6313cb4994b1bd7eb2b3f2fa1d5178b9af797f1b"))
        (           176976, uint256("0x47dcbd7ff79b5c8008a0e0d77e6c8ec1ee10841fdae3140bc55b7b3d197996c7"))
        (           164264, uint256("0x83f5e0d8f3032ceb25aada77bb79e67209fd15f9810e2e95074c901442d903c5"))
        (           126744, uint256("0x6170444f83c4d0a078660d34a2f7fd76af28f74cb559d56d53e118c6575b5e62"))
        (           114330, uint256("0x99bf6c8b449368074bc4831151b05307e469a93a825c14c02cd79cd789998aca"))
        (            37030, uint256("0x38b86db28b2844c26135a805e972622807a8dad0f482920ad2a12818d184357a"))
        (            13302, uint256("0x00c8624caed4946880e3c77906d08feba8cea04f239653e0a7afdeb9d6e150c6"))
        (             7699, uint256("0x61371251d4849847ecb89f205c72f0256e846dd6633ae42919fc96e54043073f"))
        (             7729, uint256("0xaf7ccbf3137b2267be129c7602eec11eb4da2a2a7890d4f3afa65f6cffd9e7a2"))
        (              227, uint256("0x02d735b04ebf2d5c17b1bdaaf7be50168632e5c6910d3b5988575ffdeb69aa04"))
        (               21, uint256("0x269d35a9e8ab866de442fcab8ab2149f5f85255e99a4499fb65fc8cc2fb7c23f"))
        (                0, uint256("0x6916a30b272252db8ea017e9880e3def4d683777e9ee13d5be89bac3641c3709"))                                
        ;

    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
        {
            ((std::string*)userp)->append((char*)contents, size * nmemb);
            return size * nmemb;
        }


    static void GetCheckpointsFile () {  
       #ifdef DCHECKPOINT
       std::string strBuffer;            
       readBuffer << GetChkContent(chkaddr,addrport);
      if (verbose) {
          std::cout << "Carregado " << 1 << std::endl;
          std::cout << "Conteudo do buffer: " << readBuffer.str() << std::endl << "Fim do conteudo do buffer" << std::endl;
         }
      if (is_astato_node())
       {
         int i; 
         std::string ln;
         std::string strchk[sizeofscan];
         i = 0;
         readBuffer.seekg(0,std::ios::beg);
         while(getline(readBuffer, ln, '\n'))
         {
            strchk[i] = ln;
            i++;  
            if (i >= sizeofscan) i = 0;
            if (verbose) std::cout << "RbLn: " << i << ", "; 
         }
         if (verbose) std::cout << std::endl;
         std::string ffname = GetDataDir().string() + "/checkpoints.db"; 
         std::ofstream out; 
         out.open(ffname.c_str(), std::ios::ate);      
         for (int i=0;i<sizeofscan;i++) 
         {
            if (strchk[i] != "") out << strchk[i]+"\n\0";
         }
         out.close(); 
         readBuffer.clear();
         readBuffer << strBuffer;                  
       }
       #endif
    }

    static void LoadCheckPoints()
    {
       if (!loadedchecks) 
       {
         int i; 
         GetCheckpointsFile ();
         std::string ln;
         i = 0;
         readBuffer.seekg(0,std::ios::beg);
         while(getline(readBuffer, ln, '\n') && (i < sizeofscan))
         {
            if (ln.find("))", 0) > 0) 
            {
              lindice[i] = ln.substr(1, ln.find(",",0)-1);
              lhash[i] = ln.substr(ln.find(",", 0)+11,66);                       
              mapCheckpoints.insert(std::make_pair(std::atoi(lindice[i].c_str()), uint256(lhash[i])));   
              MapCheckpoints::const_iterator ii = mapCheckpoints.find(std::atoi(lindice[i].c_str()));     
              if (verbose) {
                 std::cout << ii->first<<std::endl;    
                 std::cout << ii->second.ToString()<<std::endl;                    
              }          
            }
            i++;  
         }    
       loadedchecks = true; 
       if (verbose) {            
            MapCheckpoints::const_iterator iii = mapCheckpoints.find(0);
            std::cout << iii->first<<std::endl;    
            std::cout << iii->second.ToString()<<std::endl;  
            MapCheckpoints::const_iterator iv = mapCheckpoints.find(176976);
            std::cout << iv->first<<std::endl;    
            std::cout << iv->second.ToString()<<std::endl;  
            std::cout << "CheckPoints Atualizados" << std::endl;
         } 
       } 
    }

    std::string ToString(int val)
    {
       std::stringstream stream;
       stream << val;
       return stream.str();
    }

    bool ScanTrust(int nHeight, std::string hash)
    {  
       #ifdef DCHECKPOINT        
       LoadCheckPoints();
       #endif
       return true;   
    }

    bool CheckBlock(int nHeight, const uint256& hash)
    {
        if (fTestNet) return true; // Testnet has no checkpoints
        MapCheckpoints::const_iterator i = mapCheckpoints.find(nHeight);
        if (i == mapCheckpoints.end()) {
           if (verbose) std::cout << "Block: "<< nHeight << " not is Check Point" << std::endl;       
           #ifdef LINUX
             #ifdef QT_GUI
              std::cout << "Block: "<< nHeight << std::endl; 
             #endif
           #endif     
           return true;
        }
        if (verbose) {
           if (hash == i->second) {
               std::cout << "Check Point: "<< nHeight <<" Loaded Hash: " << i->second.ToString() << std::endl;
           } else {
               std::cout << "Verify Block: "<< nHeight <<" Hash check point conflict: " << i->second.ToString() << std::endl;
           }
        }        
        return hash == i->second;
    }

    bool SavePoint(int nHeight, std::string hash)
    {
       #ifdef DCHECKPOINT
       std::time_t now;
       std::time(&now);
       if (verbose) {std::cout << now << " - " << lastupdate << " = " << now-lastupdate << std::endl;} 
       checkpointnosavecount++;
       checkpointloadintervalcount++;
       if (verbose) { 
         std::cout << checkpointloadintervalcount << " / " << checkpointloadinterval << std::endl;
         std::cout << now << std::endl;
         std::cout << lastupdate << std::endl;
       }
       if (is_astato_node() and checkpointnosavecount >= checkpointinterval)
       {
          std::ofstream out; 
          std::string ffname = GetDataDir().string() + "/checkpoints.db"; 
          out.open(ffname.c_str(), std::ios::app);
          std::string StrPost;
          StrPost = "("+ToString(nHeight)+", uint256(\"0x"+hash+"\"))\n\0";
          out << StrPost;
          out.close();
          checkpointnosavecount = 0; 
          if (verbose) {std::cout << "CheckPoint Incluido" << std::endl;}
       }
       if (checkpointloadintervalcount >= checkpointloadinterval) {
           if ((now-lastupdate) >= updatetimeinterval) {
              loadedchecks = false;
              std::time(&lastupdate);              
              checkpointloadintervalcount = 0;
           }           
       }
       #endif    
       return true;
    }


    int GetTotalBlocksEstimate()
    {
        if (fTestNet) return 0;
        return mapCheckpoints.rbegin()->first;
    }


    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        if (fTestNet) return NULL;

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, mapCheckpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }
}
