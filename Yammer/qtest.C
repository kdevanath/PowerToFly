#include <setjmp.h>
#include <signal.h>
#include <strings.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <IPCQueue.H>

using namespace std;
using namespace Yammer;

struct ExampleMsg {
  enum { MAX_STRING = 32 };
  char portfolio_[MAX_STRING];
  char symbol_[MAX_STRING];
  long position_;
  unsigned short checksum_;
};

ExampleMsg m;

void set() {
  strcpy(m.portfolio_, "APS");
  strcpy(m.symbol_, "QQQ");
  m.position_ = 100;
  m.checksum_ = 1;
}

void clear() {
  m.portfolio_[0] = 0;
  m.symbol_[0] = 0;
  m.position_ = 0;
  m.checksum_ = 0;
}

bool pass() {
  return strcmp(m.portfolio_, "APS") == 0 && strcmp(m.symbol_, "QQQ") == 0 &&
    m.position_ == 100 && m.checksum_ == 1;
}

sigjmp_buf env;

extern "C" void alrm(int) { siglongjmp(env, 1); }

main()
{
  // ExampleMsg size = 72, plus 4 for size = 76
  size_t msgSize = sizeof(ExampleMsg), ret;

  struct sigaction sa;
  bzero(&sa, sizeof sa);
  sa.sa_handler = alrm;
  sigaction(SIGALRM, &sa, 0);

  unlink("test");
  set();

  IPCQueue *q;

  try {

    cout << "no wrap test" << endl;
    q = new IPCQueue("test", true, 300);
    q->push(&m, msgSize);
    q->push(&m, msgSize);
    q->push(&m, msgSize);
    clear();
    ret = q->pop(&m, msgSize);
    cout << (pass() ? "pass" : "fail") << endl;
    clear();
    ret = q->pop(&m, msgSize);
    cout << (pass() ? "pass" : "fail") << endl;
    clear();
    ret = q->pop(&m, msgSize);
    cout << (pass() ? "pass" : "fail") << endl;
    delete q;
    unlink("test");

    cout << "size wrap test 1" << endl;
    q = new IPCQueue("test", true, 77);
    q->push(&m, msgSize);
    clear();
    ret = q->pop(&m, msgSize);
    cout << (pass() ? "pass" : "fail") << endl;
    q->push(&m, msgSize);
    clear();
    ret = q->pop(&m, msgSize);
    cout << (pass() ? "pass" : "fail") << endl;
    delete q;
    unlink("test");

    cout << "size wrap test 2" << endl;
    q = new IPCQueue("test", true, 78);
    q->push(&m, msgSize);
    clear();
    ret = q->pop(&m, msgSize);
    cout << (pass() ? "pass" : "fail") << endl;
    q->push(&m, msgSize);
    clear();
    ret = q->pop(&m, msgSize);
    cout << (pass() ? "pass" : "fail") << endl;
    delete q;
    unlink("test");

    cout << "size wrap test 3" << endl;
    q = new IPCQueue("test", true, 79);
    q->push(&m, msgSize);
    clear();
    ret = q->pop(&m, msgSize);
    cout << (pass() ? "pass" : "fail") << endl;
    q->push(&m, msgSize);
    clear();
    ret = q->pop(&m, msgSize);
    cout << (pass() ? "pass" : "fail") << endl;
    delete q;
    unlink("test");

    cout << "size wrap test 4" << endl;
    q = new IPCQueue("test", true, 80);
    q->push(&m, msgSize);
    clear();
    ret = q->pop(&m, msgSize);
    cout << (pass() ? "pass" : "fail") << endl;
    q->push(&m, msgSize);
    clear();
    ret = q->pop(&m, msgSize);
    cout << (pass() ? "pass" : "fail") << endl;
    delete q;
    unlink("test");

    cout << "msg wrap test 1" << endl;
    q = new IPCQueue("test", true, 81);
    q->push(&m, msgSize);
    clear();
    ret = q->pop(&m, msgSize);
    cout << (pass() ? "pass" : "fail") << endl;
    q->push(&m, msgSize);
    clear();
    ret = q->pop(&m, msgSize);
    cout << (pass() ? "pass" : "fail") << endl;
    delete q;
    unlink("test");

    cout << "msg wrap test 2" << endl;
    q = new IPCQueue("test", true, 113);
    q->push(&m, msgSize);
    clear();
    ret = q->pop(&m, msgSize);
    cout << (pass() ? "pass" : "fail") << endl;
    q->push(&m, msgSize);
    clear();
    ret = q->pop(&m, msgSize);
    cout << (pass() ? "pass" : "fail") << endl;
    delete q;
    unlink("test");

    cout << "msg wrap test 3" << endl;
    q = new IPCQueue("test", true, 145);
    q->push(&m, msgSize);
    clear();
    ret = q->pop(&m, msgSize);
    cout << (pass() ? "pass" : "fail") << endl;
    q->push(&m, msgSize);
    clear();
    ret = q->pop(&m, msgSize);
    cout << (pass() ? "pass" : "fail") << endl;
    delete q;
    unlink("test");

    cout << "msg wrap test 4" << endl;
    q = new IPCQueue("test", true, 152);
    q->push(&m, msgSize);
    q->push(&m, msgSize);
    clear();
    ret = q->pop(&m, msgSize);
    cout << (pass() ? "pass" : "fail") << endl;
    clear();
    ret = q->pop(&m, msgSize);
    cout << (pass() ? "pass" : "fail") << endl;
    delete q;
    unlink("test");

    cout << "block test 1" << endl;
    q = new IPCQueue("test", true, 76);
    q->push(&m, msgSize);
    if (sigsetjmp(env, 1) == 0) {
      alarm(2);
      q->push(&m, msgSize);
    }
    clear();
    ret = q->pop(&m, msgSize);
    cout << (pass() ? "pass" : "fail") << endl;
    delete q;
    unlink("test");

    cout << "block test 2" << endl;
    q = new IPCQueue("test", true, 77);
    q->push(&m, msgSize);
    if (sigsetjmp(env, 1) == 0) {
      alarm(2);
      q->push(&m, msgSize);
    }
    clear();
    ret = q->pop(&m, msgSize);
    cout << (pass() ? "pass" : "fail") << endl;
    delete q;
    unlink("test");

    cout << "block test 3" << endl;
    q = new IPCQueue("test", true, 72);
    if (sigsetjmp(env, 1) == 0) {
      alarm(2);
      q->push(&m, msgSize);
    }
    cout << "pass" << endl;
    delete q;
    unlink("test");

    cout << "truncate test" << endl;
    q = new IPCQueue("test", true, 100);
    q->push(&m, msgSize);
    clear();
    ret = q->pop(&m, msgSize + 8);  // request more
    // only msgSize bytes should return
    cout << (ret == msgSize ? "pass" : "fail") << endl;
    q->push(&m, msgSize - 8);  // copy less
    clear();
    ret = q->pop(&m, msgSize);
    cout << (ret == msgSize - 8 ? "pass" : "fail") << endl;
    delete q;
    unlink("test");

  } catch (SystemException &e) {
    cout << e.what() << endl;
  }
}

