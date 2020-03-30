
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>

using namespace std;

class MemStream
{

  FILE*   m_file;
  char*   m_buf;
  size_t  m_bufsz;

public:

  MemStream():
    m_file(NULL),
    m_buf(NULL),
    m_bufsz(0)
  {}

  ~MemStream()
  {
    close();
    if (m_buf != NULL) {
      free(m_buf);
    }
  }

  void input( const char* txt )
  {
    if (txt == NULL || txt[0] == '\0') {
      return;
    }
    if (m_file == NULL) {
      init();
    }
    if (fputs(txt, m_file) == EOF) {
      fputs("Unable to write memstream\n", stderr);
      exit(EXIT_FAILURE);
    }
  }

  void inputf( const char* fmt, ... )
  {
    if (fmt == NULL || fmt[0] == '\0') {
      return;
    }
    va_list ap;
    va_start(ap, fmt);
    if (m_file == NULL) {
      init();
    }
    if (vfprintf(m_file, fmt, ap) < 0) {
      fputs("Unable to write memstream\n", stderr);
      exit(EXIT_FAILURE);
    }
    va_end(ap);
  }

  void reset()
  {
    close();
    if (m_buf != NULL) {
      free(m_buf);
      m_buf = NULL;
    }
    m_bufsz = 0;
  }

  string toString()
  {
    close();
    if (m_bufsz != 0) {
      return m_buf;
    }
    return "";
  }

private:

  void init()
  {
    if (m_file != NULL) {
      return;
    }
    if (m_buf != NULL) {
      free(m_buf);
      m_buf = NULL;
    }
    m_bufsz = 0;
    m_file = open_memstream(&m_buf, &m_bufsz);
    if (m_file == NULL) {
      fprintf(stderr, "Unable to open memstream (errno=%d)\n", errno);
      exit(EXIT_FAILURE);
    }
  }

  void close()
  {
    if (m_file == NULL) {
      return;
    }
    if (fclose(m_file) == EOF) {
      fprintf(stderr, "Unable to close memstream (errno=%d)\n", errno);
      exit(EXIT_FAILURE);
    }
    m_file = NULL;
  }

};

// vi: set ai et sw=2 sts=2 ts=2 :
