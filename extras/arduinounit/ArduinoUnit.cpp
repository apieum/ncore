#include <Arduino.h>
#include <ArduinoUnit.h>
#include <regex.h>


const uint8_t Test::UNSETUP = 0;
const uint8_t Test::LOOPING = 1;
const uint8_t Test::DONE_SKIP = 2;
const uint8_t Test::DONE_PASS = 3;
const uint8_t Test::DONE_FAIL = 4;

Test* Test::root = 0;
Test* Test::current = 0;

uint16_t Test::count = 0;
uint16_t Test::passed = 0;
uint16_t Test::failed = 0;
uint16_t Test::skipped = 0;
uint8_t Test::max_verbosity = TEST_VERBOSITY_ALL;
uint8_t Test::min_verbosity = TEST_VERBOSITY_TESTS_SUMMARY;

Print* Test::out = &Serial;

void Test::resolve()
{
  bool pass = current->state==DONE_PASS;
  bool fail = current->state==DONE_FAIL;
  bool skip = current->state==DONE_SKIP;
  bool done = (pass || fail || skip);

  if (done) {
    if (pass) ++Test::passed;
    if (fail) ++Test::failed;
    if (skip) ++Test::skipped;

    #if TEST_VERBOSITY_EXISTS(TESTS_SKIPPED) || TEST_VERBOSITY_EXISTS(TESTS_PASSED) || TEST_VERBOSITY_EXISTS(TESTS_FAILED)

    bool output = false;

    output = output || (skip && TEST_VERBOSITY(TESTS_SKIPPED));
    output = output || (pass && TEST_VERBOSITY(TESTS_PASSED));
    output = output || (fail && TEST_VERBOSITY(TESTS_FAILED));

    if (output) {
      out->print("Test ");
      out->print(name.c_str());
      #if TEST_VERBOSITY_EXISTS(TESTS_SKIPPED)
      if (skip) out->println(" skipped.");
      #endif

      #if TEST_VERBOSITY_EXISTS(TESTS_PASSED)
      if (pass) out->println(" passed.");
      #endif

      #if TEST_VERBOSITY_EXISTS(TESTS_FAILED)
      if (fail) out->println(" failed.");
      #endif
    }
    #endif
  }
#if TEST_VERBOSITY_EXISTS(TESTS_SUMMARY)
  if (root == 0 && TEST_VERBOSITY(TESTS_SUMMARY)) {
    out->print("Test summary: ");
    out->print(passed);
    out->print(" passed, ");
    out->print(failed);
    out->print(" failed, and ");
    out->print(skipped);
    out->print(" skipped, out of ");
    out->print(count);
    out->println(" test(s).");
  }
#endif
}

void Test::remove()
{
  for (Test **p = &root; *p != 0; p=&((*p)->next)) {
    if (*p == this) {
      *p = (*p)->next;
      break;
    }
  }
}


Test::Test(std::string _name, uint8_t _verbosity)
  : name(_name), verbosity(_verbosity)
{
  insert();
}

Test::~Test()
{
  remove();
}

void Test::insert()
{
  state = UNSETUP;
  {
    Test **p = &Test::root;
    while ((*p) != 0) {
      if (name.compare((*p)->name) <= 0) break;
      p=&((*p)->next);
    }
    next=(*p);
    (*p)=this;
  }
  ++Test::count;
}

void Test::pass() { state = DONE_PASS; }
void Test::fail() { state = DONE_FAIL; }
void Test::skip() { state = DONE_SKIP; }

void Test::setup() {};

void Test::run()
{
  for (Test **p = &root; (*p) != 0; )
  {
    current = *p;
    if (current->state == LOOPING) current->loop();
    else if (current->state == UNSETUP)
    {
      current->setup();
      if (current->state == UNSETUP) current->state = LOOPING;
    }

    if (current->state != LOOPING)
    {
      (*p)=((*p)->next);
      current->resolve();
    }
    else p=&((*p)->next);
  }
}


void Test::include(const char *pattern)
{
  regex_t preg;
  int err;
  int match;
  err = regcomp (&preg, pattern, REG_NOSUB);
  if (err != 0)
  {
    out->println("[Error] pattern not valid in Test::include");
    return;
  }
  for (Test *p = root; p != 0; p=p->next) {
    match = regexec(&preg, p->name.c_str(), 0, NULL, 0);
    if (p->state == DONE_SKIP && match == 0) {
      p->state = UNSETUP;
    }
  }
  regfree (&preg);
}

void Test::exclude(const char *pattern)
{
  regex_t preg;
  int err;
  int match;
  err = regcomp(&preg, pattern, REG_NOSUB);
  if (err != 0)
  {
    out->println("[Error] pattern not valid in Test::Exclude");
    return;
  }
  for (Test *p = root; p != 0; p=p->next) {
    match = regexec(&preg, p->name.c_str(), 0, NULL, 0);
    if (p->state == UNSETUP && match == 0) {
      p->state = DONE_SKIP;
    }
  }
  regfree(&preg);
}


TestOnce::TestOnce(std::string name) : Test(name) {}
void TestOnce::once() {}

void TestOnce::loop()
{
  once();
  if (state == LOOPING) state = DONE_PASS;
}

template <>
bool isLess<const char*>(const char* const &a, const char* const &b)
{
  return (strcmp(a,b) < 0);
}

template <>
bool isLessOrEqual<const char*>(const char* const &a, const char* const &b)
{
  return (strcmp(a,b) <= 0);
}

template <>
bool isEqual<const char*>(const char* const &a, const char* const &b)
{
  return (strcmp(a,b) == 0);
}

template <>
bool isNotEqual<const char*>(const char* const &a, const char* const &b)
{
  return (strcmp(a,b) != 0);
}

template <>
bool isMoreOrEqual<const char*>(const char* const &a, const char* const &b)
{
  return (strcmp(a,b) >= 0);
}

template <>
bool isMore<const char*>(const char* const &a, const char* const &b)
{
  return (strcmp(a,b) > 0);
}
