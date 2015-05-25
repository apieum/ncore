#pragma once

/**

  @file ArduinoUnit.h

*/

#include <string>
#include <stdint.h>
#include <Print.h>

#define ARDUINO_UNIT_MAJOR_VERSION 2
#define ARDUINO_UNIT_MINOR_VERSION 0
#define  TEST_VERBOSITY_TESTS_SUMMARY     0x01
#define  TEST_VERBOSITY_TESTS_FAILED      0x02
#define  TEST_VERBOSITY_TESTS_PASSED      0x04
#define  TEST_VERBOSITY_TESTS_SKIPPED     0x08
#define  TEST_VERBOSITY_TESTS_ALL         0x0F
#define  TEST_VERBOSITY_ASSERTIONS_FAILED 0x10
#define  TEST_VERBOSITY_ASSERTIONS_PASSED 0x20
#define  TEST_VERBOSITY_ASSERTIONS_ALL    0x30
#define  TEST_VERBOSITY_ALL               0x3F
#define  TEST_VERBOSITY_NONE              0x00

#ifndef TEST_MAX_VERBOSITY
  #define TEST_MAX_VERBOSITY TEST_VERBOSITY_ALL
#endif

#define TEST_VERBOSITY_EXISTS(OF) ((TEST_MAX_VERBOSITY & TEST_VERBOSITY_ ## OF) != 0)
#define TEST_VERBOSITY_ALLOWED(OF) (TEST_VERBOSITY_EXISTS(OF) && ((Test::max_verbosity & TEST_VERBOSITY_ ## OF) != 0))
#define TEST_VERBOSITY_REQUIRED(OF) (TEST_VERBOSITY_ALLOWED(OF) && ((Test::min_verbosity & TEST_VERBOSITY_ ## OF) != 0))
#define TEST_VERBOSITY(OF) (TEST_VERBOSITY_ALLOWED(OF) && (((Test::min_verbosity & TEST_VERBOSITY_ ## OF ) != 0) || (((Test::current != 0) && ((Test::current->verbosity & TEST_VERBOSITY_ ## OF) != 0)))))

class Test
{
 private:
  static Test* root;
  Test *next;

  static uint16_t passed;
  static uint16_t failed;
  static uint16_t skipped;
  static uint16_t count;

  void resolve();
  void remove();
  void insert();

 public:
  Test(std::string _name, uint8_t _verbosity = TEST_VERBOSITY_TESTS_ALL|TEST_VERBOSITY_ASSERTIONS_FAILED);

  virtual ~Test();
  static uint8_t max_verbosity;
  static uint8_t min_verbosity;

  static inline uint16_t getCurrentPassed() { return passed; }
  static inline uint16_t getCurrentSkipped() { return skipped; }
  static inline uint16_t getCurrentFailed() { return failed; }
  static uint16_t getCurrentCount() { return count; }

  static const uint8_t UNSETUP;
  static const uint8_t LOOPING;
  static const uint8_t DONE_SKIP;
  static const uint8_t DONE_PASS;
  static const uint8_t DONE_FAIL;
  static Print *out;
  uint8_t state;
  static Test* current;
  std::string name;
  uint8_t verbosity;

  void pass();
  void fail();
  void skip();
  virtual void setup();
  virtual void loop() = 0;
  static void include(const char *pattern);
  static void exclude(const char *pattern);
  static void run();


  template <typename T>
    static bool assertion(const char *file, uint16_t line, const char *lhss, const T& lhs, const char *ops, bool (*op)(const T& lhs, const T& rhs), const char *rhss, const T& rhs) {
    bool ok = op(lhs,rhs);
    bool output = false;

    if ((!ok) && (current != 0)) current->fail();

#if TEST_VERBOSITY_EXISTS(ASSERTIONS_PASSED)
    if (ok && TEST_VERBOSITY(ASSERTIONS_PASSED)) {
      output = true;
    }
#endif

#if TEST_VERBOSITY_EXISTS(ASSERTIONS_FAILED)
    if ((!ok) && TEST_VERBOSITY(ASSERTIONS_FAILED)) {
      output = true;
    }
#endif

#if TEST_VERBOSITY_EXISTS(ASSERTIONS_FAILED) || TEST_VERBOSITY_EXISTS(ASSERTIONS_PASSED)
    if (output) {
      out->print("Assertion ");
      out->print(ok ? "passed" : "failed");
      out->print(": (");
      out->print(lhss);
      out->print("=");
      out->print(lhs);
      out->print(") ");
      out->print(ops);
      out->print(" (");
      out->print(rhss);
      out->print("=");
      out->print(rhs);
      out->print("), file ");
      out->print(file);
      out->print(", line ");
      out->print(line);
      out->println(".");
    }
#endif
    return ok;
  }
};



class TestOnce : public Test {
 public:
  TestOnce(std::string name);
  void loop();
  virtual void once() = 0;
};

template <typename T>
bool isEqual(const T& a, const T& b) { return a==b; }

template <typename T>
bool isNotEqual(const T& a, const T& b) { return !(a==b); }

template <typename T>
bool isLess(const T& a, const T& b) { return a < b; }

template <typename T>
bool isMore(const T& a, const T& b) { return b < a; }

template <typename T>
bool isLessOrEqual(const T& a, const T& b) { return !(b<a); }

template <typename T>
bool isMoreOrEqual(const T& a, const T& b) { return !(a<b); }

template <> bool isLess<const char*>(const char* const &a, const char* const &b);

template <> bool isLessOrEqual<const char*>(const char* const &a, const char* const &b);

template <> bool isEqual<const char*>(const char* const &a, const char* const &b);

template <> bool isNotEqual<const char*>(const char* const &a, const char* const &b);

template <> bool isMore<const char*>(const char* const &a, const char* const &b);

template <> bool isMoreOrEqual<const char*>(const char* const &a, const char* const &b);


#define test(name) struct test_ ## name : TestOnce { test_ ## name() : TestOnce(#name) {}; void once(); } test_ ## name ## _instance; void test_ ## name :: once()
#define externTest(name) struct test_ ## name : TestOnce { test_ ## name(); void once(); }; extern test_##name test_##name##_instance
#define testing(name) struct test_ ## name : Test { test_ ## name() : Test(#name) {}; void loop(); } test_ ## name ## _instance; void test_ ## name :: loop()
#define externTesting(name) struct test_ ## name : Test { test_ ## name(); void loop(); }; extern test_##name test_##name##_instance

#ifdef __GNUC__
  #define typeof __typeof__
#endif
#define assertOp(arg1,op,op_name,arg2) if (!Test::assertion<typeof(arg2)>(__FILE__,__LINE__,#arg1,(arg1),op_name,op,#arg2,(arg2))) return;
#define assertEqual(arg1,arg2)       assertOp(arg1,isEqual,"==",arg2)
#define assertNotEqual(arg1,arg2)    assertOp(arg1,isNotEqual,"!=",arg2)
#define assertLess(arg1,arg2)        assertOp(arg1,isLess,"<",arg2)
#define assertMore(arg1,arg2)        assertOp(arg1,isMore,">",arg2)
#define assertLessOrEqual(arg1,arg2) assertOp(arg1,isLessOrEqual,"<=",arg2)
#define assertMoreOrEqual(arg1,arg2) assertOp(arg1,isMoreOrEqual,">=",arg2)
#define assertTrue(arg) assertEqual(arg,true)
#define assertFalse(arg) assertEqual(arg,false)

#define checkTestDone(name) (test_##name##_instance.state >= Test::DONE_SKIP)
#define checkTestNotDone(name) (test_##name##_instance.state < Test::DONE_SKIP)
#define checkTestPass(name) (test_##name##_instance.state == Test::DONE_PASS)
#define checkTestNotPass(name) (test_##name##_instance.state != Test::DONE_PASS)
#define checkTestFail(name) (test_##name##_instance.state == Test::DONE_FAIL)
#define checkTestNotFail(name) (test_##name##_instance.state != Test::DONE_FAIL)
#define checkTestSkip(name) (test_##name##_instance.state == Test::DONE_SKIP)
#define checkTestNotSkip(name) (test_##name##_instance.state != Test::DONE_SKIP)

#define assertTestDone(name) assertMoreOrEqual(test_##name##_instance.state,Test::DONE_SKIP)
#define assertTestNotDone(name) assertLess(test_##name##_instance.state,Test::DONE_SKIP)
#define assertTestPass(name) assertEqual(test_##name##_instance.state,Test::DONE_PASS)
#define assertTestNotPass(name) assertNotEqual(test_##name##_instance.state,Test::DONE_PASS)
#define assertTestFail(name) assertEqual(test_##name##_instance.state,Test::DONE_FAIL)
#define assertTestNotFail(name) assertNotEqual(test_##name##_instance.state,Test::DONE_FAIL)
#define assertTestSkip(name) assertEqual(test_##name##_instance.state,Test::DONE_SKIP)
#define assertTestNotSkip(name) assertNotEqual(test_##name##_instance.state,Test::DONE_SKIP)
