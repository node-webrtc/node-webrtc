#if !defined( COMMON_H )
#define COMMON_H

#define WARN(msg) fprintf(stdout, "\033[01;33m native:%s \033[00m\n", msg)
#define ERROR(msg) fprintf(stdout, "\033[01;32m native:%s \033[00m\n", msg)
#define INFO(msg) fprintf(stdout, "\033[01;34m native:%s \033[00m\n", msg)

#if defined(TRACING)

#include <stdio.h>

#define TRACE(msg) printf("   TRACE: %s\n", msg)
#define TRACE_S(msg, s) printf("   TRACE: %s : %s\n", msg, s)
#define TRACE_I(msg, i) printf("   TRACE: %s : %d\n", msg, i)
#define TRACE_U(msg, u) printf("   TRACE: %s : %u\n", msg, u)
#define TRACE_X(msg, x) printf("   TRACE: %s : 0x%x\n", msg, x)
#define TRACE_PTR(msg, p) printf("   TRACE: %s : %p\n", msg, p)
#define TRACE_CALL printf("-> TRACE: Call::%s\n", __PRETTY_FUNCTION__)
#define TRACE_CALL_I(p1) printf("-> TRACE: Call::%s(%d)\n", __PRETTY_FUNCTION__, p1)
#define TRACE_END printf("<- Call::%s\n", __PRETTY_FUNCTION__)

#else

#define TRACE(msg)
#define TRACE_S(msg, s)
#define TRACE_I(msg, i)
#define TRACE_U(msg, u)
#define TRACE_X(msg, x)
#define TRACE_PTR(msg, x)
#define TRACE_CALL
#define TRACE_CALL_I(p1)
#define TRACE_END

#endif

#define THROW_TYPE_ERROR( MSG ) \
  return ThrowException(Exception::TypeError( \
    String::New( MSG )));

#define CHECK_ARG(I, CHECK, DO_TRUE, DO_FALSE) \
  if ( args.Length() <= (I) || !args[I]->CHECK ) { DO_FALSE; } else { DO_TRUE; }

#define REQUIRE_ARG(I, CHECK) \
  CHECK_ARG( I, CHECK, , THROW_TYPE_ERROR("Argument " #I " must be an object") )

#define REQ_OBJ_ARG(I, VAR) \
  REQUIRE_ARG( I, IsObject() ) \
  Local<Object> VAR = Local<Object>::Cast( args[I] )

#define OPT_INT_ARG(I, VAR, DEFAULT) \
  int VAR; \
  CHECK_ARG( I, IsNumber(), VAR = args[I]->Int32Value(), VAR = DEFAULT )

#define REQ_INT_ARG(I, VAR) \
  REQUIRE_ARG( I, IsNumber() ) \
  int VAR = args[I]->Int32Value();

#define REQ_FUN_ARG(I, VAR)                                      \
  if (args.Length() <= (I) || !args[I]->IsFunction())            \
    return ThrowException(Exception::TypeError(              \
      String::New("Argument " #I " must be a function")));   \
  Local<Function> VAR = Local<Function>::Cast(args[I]);

#define CREATE_BUFFER( name, data, length ) \
  Buffer* name ## _slow = Buffer::New( length ); \
  memcpy(Buffer::Data( name ## _slow ), data, length ); \
  Local<Object> name; \
  Handle<Value> ctorArgs[3] = { name ## _slow->handle_, Integer::New( length ), Integer::New(0) }; \
  name = Local<Function>::Cast( \
        Context::GetCurrent() \
          ->Global() \
          ->Get( String::New("Buffer")) \
      )->NewInstance( 3, ctorArgs );

#endif
