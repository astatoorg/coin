// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2012 Litecoin Developers.
// Copyright (c) 2017 Astato Developers.
// Distributed under the MIT/X11 software license, see the accompanying).
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
// USE -DDCHECKPOINT on makefile to active Linux CURL support and Linux hostnames, checkpoint will be static and dynamic
// Without -DDCHECKPOINT checkpoint will be done only statically (traditional)
// For seednodes is recomended -DDCHECKPOINT

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>
#ifdef DCHECKPOINT
  #include <curlpp/cURLpp.hpp>
  #include <curlpp/Options.hpp>
#endif
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
    const int checkpointinterval = 4;
    const int checkpointloadinterval = 40; //bom numero 40
    const std::time_t updatetimeinterval = 600; //bom numero 600
    bool loadedchecks = false;
    bool verbose = false;
    int checkpointnosavecount = 5;
    int checkpointloadintervalcount = 0;
    std::string chkaddr = "http://node001.astato.org:21680"; // descentralizar, em desenvolvimento
    std::string lindice[sizeofscan];
    std::string lhash[sizeofscan];
    std::time_t lastupdate = 0;
    std::stringstream readBuffer;
   
    bool is_astato_node() {
     #ifdef DCHECKPOINT
      char hostname[HOST_NAME_MAX];
      gethostname(hostname, HOST_NAME_MAX);
      std::string sname(hostname);
      sname = '.'+sname;
      int instr = sname.find("astato.org", 0);
      if ( instr > 0) {
          if (verbose) {std::cout << sname << " " << instr << std::endl;}
          return true;           
      } 
     #endif
      return false;
    }                   

    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of // Yo dawg, this is the secret. Checkpoint 0 hash == Genesis block hash.
        (           126744, uint256("0x6170444f83c4d0a078660d34a2f7fd76af28f74cb559d56d53e118c6575b5e62"))
        (           114330, uint256("0x99bf6c8b449368074bc4831151b05307e469a93a825c14c02cd79cd789998aca"))
        (            37030, uint256("0x38b86db28b2844c26135a805e972622807a8dad0f482920ad2a12818d184357a"))
        (            13302, uint256("0x00c8624caed4946880e3c77906d08feba8cea04f239653e0a7afdeb9d6e150c6"))
        (             7699, uint256("0x61371251d4849847ecb89f205c72f0256e846dd6633ae42919fc96e54043073f"))
        (             7700, uint256("0xd773120542281ff15cbd37ba10e37215a012eab57933933788493699de39330e"))
        (             7729, uint256("0xaf7ccbf3137b2267be129c7602eec11eb4da2a2a7890d4f3afa65f6cffd9e7a2"))
        (             7730, uint256("0x8f8df82e524441867f9ba35b8f4d57dffed1b3159abab9c563b79885799e6122"))
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
       if (is_astato_node()) {
           chkaddr = "file://"+GetDataDir().string() + "/checkpoints.db"; 
       }
       CURL *curl;
       CURLcode res;
       curl_global_init(CURL_GLOBAL_ALL);
       curl = curl_easy_init();
       if(curl) {
          curl_easy_setopt(curl, CURLOPT_URL, chkaddr.c_str());
          curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
          curl_easy_setopt(curl, CURLOPT_WRITEDATA, &strBuffer);
          res = curl_easy_perform(curl);
          readBuffer.clear();
          readBuffer << strBuffer;
          if (verbose) {std::cout << "Conteudo da String: " << strBuffer;} 
          if(res != CURLE_OK)
             {
                fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
             }
          curl_easy_cleanup(curl);  
      }
      curl_global_cleanup();
      if (verbose) {std::cout << "Carregado " << 1 << std::endl;}
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
         }
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
            MapCheckpoints::const_iterator iv = mapCheckpoints.find(114330);
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
       std::string StrnHeight; 
       LoadCheckPoints();
       for (int i=0;i<sizeofscan;i++) 
       {
            StrnHeight = ToString(nHeight);
            if (StrnHeight == lindice[i])
            {
               if (uint256(hash) != uint256(lhash[i]))
               {
                  std::cout << "Opss.. its wrong. "<< StrnHeight << std::endl << hash <<std::endl << lhash[i] << std::endl;
                  return false;
               } 
            }            
       }        
       return true;   
    }

    bool CheckBlock(int nHeight, const uint256& hash)
    {
        if (fTestNet) return true; // Testnet has no checkpoints
        MapCheckpoints::const_iterator i = mapCheckpoints.find(nHeight);
        if (i == mapCheckpoints.end()) return true;
        return hash == i->second;
    }

    bool SavePoint(int nHeight, std::string hash)
    {
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
       return true;
    }

    bool PushCheckpoint(int nHeight, const uint256& hash)
    {
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