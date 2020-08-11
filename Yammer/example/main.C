#include <ExampleServer.H>

int main(int argc, char **argv)
{
  using ReferenceImplementation::ExampleServer;
  using ReferenceImplementation::ExampleServerSingleton;

  ExampleServer *server = ExampleServerSingleton::instance();  
  server->configure(argc, argv);
  server->start();
}

