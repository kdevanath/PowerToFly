#include <ExampleWebServer.H> 
#include <string> 
#include <memory> 
#include <Task.H> 
#include <TaskSync.H> 
#include <TaskDispatcher.H> 
#include <BufferedStream.H> 
#include <TCPStreamAdapter.H> 
#include <Logger.H> 
#include <bitmaps.h> 

namespace ReferenceImplementation {

  using std::stringstream;
  using std::auto_ptr;
  using std::string;
  using Yammer::BufferedStream;
  using Yammer::TCPStreamAdapter;

  const string CRLF = "\r\n",

               HomePage = "<HTML>"
                          "<HEAD><TITLE>Web Admin Port</TITLE></HEAD>"
                          "<IMG SRC=/cnlogo.gif>"
                          "<BODY><H3>Admin Functions</H3>"
                          "<FORM ACTION=/>"
                          "<TABLE>"
                          "<TR><TD>Monitor Usage</TD>"
                          "<TD><INPUT TYPE=SUBMIT></TD></TR>"
                          "<TR><TD>Modify Configuration</TD>"
                          "<TD><INPUT TYPE=SUBMIT></TD></TR>"
                          "</TABLE>"
                          "</FORM></BODY>"
                          "</HTML>\r\n",

               NotImplError = "HTTP/1.0 501 Not Implemented\r\n"
                              "Server: internal-httpd 1.0\r\n"
                              "Content-type: text/html\r\n\r\n"
                              "<HTML><HEAD>"
                              "<TITLE>Not Implemented</TITLE>"
                              "</HEAD><BODY>"
                              "<H1>HTTP Error 501: Not Implemented</H1>"
                              "</BODY></HTML>\r\n",

               FileNotFoundError = "HTTP/1.0 404 File Not Found\r\n"
                                   "Server: internal-httpd 1.0\r\n"
                                   "Content-type: text/html\r\n\r\n"
                                   "<HTML><HEAD>"
                                   "<TITLE>File Not Found</TITLE>"
                                   "</HEAD><BODY>"
                                   "<H1>HTTP Error 404: File Not Found</H1>"
                                   "</BODY></HTML>\r\n";


  ExampleWebServer::ExampleWebServer(int port)
    throw (ExampleWebServer::BindFailed)
  {
    // enable socket address reuse, then bind
    if (acceptor_.open(ACE_INET_Addr(port), 1) == -1)  
      throw BindFailed();

    ACE_Thread::spawn(run, this, THR_BOUND|THR_DETACHED);
  }

  void *ExampleWebServer::run(void *This)
  {
    ExampleWebServer *that = reinterpret_cast<ExampleWebServer*>(This);
    auto_ptr<ExampleWebServer> deleter(that);
    ACE_SOCK_Acceptor acceptor = that->socket();
    ACE_SOCK_Stream peer;

    for (;;) {
      peer.close();

      if (acceptor.accept(peer) == -1) {  // blocks
        LOG("accept failed");
        break;
      }

      // HTTP protocol: <method> <URL> <version> <CRLF>
      // example: "GET /index.html HTTP/1.1 \r\n"
      // the URL will be our command

      BufferedStream stream(new TCPStreamAdapter(peer));
      string buffer;
      if (stream.readline(buffer, '\r') <= 0)  // peer close or error
        continue;

      // tokenize the query string
      const char space = ' ';
      size_t pos = buffer.find(space);
      if (pos > buffer.length())
        continue;

      string method = buffer.substr(0, pos);
      if (method != "GET") {
        stream.write(NotImplError.data(), NotImplError.size());
        continue;
      }

      pos++;  // position after space
      size_t pos2 = buffer.find(space, pos);
      if (pos2 > buffer.length())
        continue;

      // dispatch to appropriate web services (there are none)
      string url = buffer.substr(pos, pos2 - pos);
      if (url == "/")
        stream.write(HomePage.data(), HomePage.size());
      else if (url == "/cnlogo.gif")
        stream.write(cnlogo_gif, sizeof cnlogo_gif);
      else
        stream.write(FileNotFoundError.data(), FileNotFoundError.size());

    }

    return 0;
  }

}

