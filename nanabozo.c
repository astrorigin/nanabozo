/*
    nanabozo - a tool for CHTML script-coding.
    Copyright (C) 2018-2020 Stanislas Marquis <stan@astrorigin.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef INPUTSIZE
#define INPUTSIZE 255
#endif

#ifdef _MSC_VER
#ifndef PAGESIZE
#define PAGESIZE 128
#endif
#endif

#ifndef VERSION_STR
#define VERSION_STR "20200315-anoxic-aphid"
#endif

const char _usage[] =
"\n"
"nanabozo - a tool for CHTML script-coding\n"
"\n"
"    Copyright (C) 2018-2020 Stanislas Marquis <stan@astrorigin.com>\n"
"\n"
"    This program is free software: you can redistribute it and/or modify\n"
"    it under the terms of the GNU General Public License as published by\n"
"    the Free Software Foundation, either version 3 of the License, or\n"
"    (at your option) any later version.\n"
"\n"
"    See the GNU General Public License for more details.\n"
"\n"
"Usage: nanabozo [OPTIONS] [(infile|-) [(outfile|-)]]\n"
"\n"
"Options:\n"
"  --main           Include boilerplate main function.\n"
"  --html           Send content-type header (text/html, charset utf-8).\n"
"  -b               Omit begin/end comments in output.\n"
"  -c <header>      Override default comment header.\n"
"                   Pass an empty string to disable comment header.\n"
"  -a <prefix>      String to prepend (default none).\n"
"                   Useful to pass some extra code at the beginning,\n"
"                   eg. (bash): -a $'void toto(const char *titi)\\n{'\n"
"                   or: -a \"$(cat myfile.h myfile.c)\"\n"
"  -z <suffix>      String to append (default none).\n"
"                   Useful to pass some extra code at the end,\n"
"                   eg. (bash): -z $'\\nreturn 0;\\n}'\n"
"  -p <func>        Override function 'print(x)'.\n"
"                   By default, 'print(x)' is a macro for 'fputs(x, stdout)'.\n"
"  -f <func>        Override function 'printf(x, ...)'.\n"
"  -v, --version    Print version information and exit.\n"
"  -h, --help       Print usage information and exit.\n"
"\n"
"Arguments:\n"
"  infile           Input file ('-' for stdin, default).\n"
"  outfile          Output file ('-' for stdout, default).\n"
"\n"
"For more information, see https://astrorigin.com/nanabozo\n"
"\n";

const char _version[] =
"\n"
"nanabozo - a tool for CHTML script-coding (" VERSION_STR ")\n"
"\n"
"    Copyright (C) 2018-2020 Stanislas Marquis <stan@astrorigin.com>.\n"
"\n"
"    This program is free software: you can redistribute it and/or modify\n"
"    it under the terms of the GNU General Public License as published by\n"
"    the Free Software Foundation, either version 3 of the License, or\n"
"    (at your option) any later version.\n"
"\n"
"    See the GNU General Public License for more details.\n"
"\n"
"Type 'nanabozo --help' for more information.\n"
"\n";

#define COMPILED_WITH \
    "Compiled " __DATE__ " " __TIME__ " with:\n" \
    "    INPUTSIZE=(%d)\n" \
    "\n"

struct match
{
    char *str;
    size_t len;
    void (*hook)( struct match *mt );
    char* p; /* search result */
};

void proceed( void );
size_t read_input( void );
struct match *context_match( void );
void reset_context( struct match *mt );
int check_identifier( const char* id );
int check_filepath( const char* fpath );

void bufwrite( const char *s, const size_t len );
void bufout( void );
void bufput( const int c );
void write( const char *s, const size_t len );
void put( const int c );
int cursor( void );

void c_fallback( const char *eol );
void html_fallback( const char *eol );

void bad_tag_end( struct match *mt );
void bad_tag_start( struct match *mt );
void c_dquote_start( struct match *mt );
void c_end( struct match *mt );
void c_macro_start( struct match *mt );
void c_ml_comment_start( struct match *mt );
void c_print_format_start( struct match *mt );
void c_print_start( struct match *mt );
void c_sl_comment_start( struct match *mt );
void c_squote_start( struct match *mt );
void c_start( struct match *mt );
void eat_c_dquote( void );
void eat_c_macro( void );
void eat_c_ml_comment( void );
void eat_c_print_format( void );
void eat_c_print_string( void );
void eat_c_sl_comment( void );
void eat_c_squote( void );
void eat_html_comment( void );
void eat_script_dquote( void );
void eat_script_ml_comment( void );
void eat_script_sl_comment( void );
void eat_script_squote( void );
void html_comment_start( struct match *mt );
void script_dquote_start( struct match *mt );
void script_end( struct match *mt );
void script_ml_comment_start( struct match *mt );
void script_sl_comment_start( struct match *mt );
void script_squote_start( struct match *mt );
void script_start( struct match *mt );
void style_end( struct match *mt );
void style_ml_comment_start( struct match *mt );
void style_start( struct match *mt );
void tag_dquote_start( struct match *mt );
void tag_end( struct match *mt );
void tag_squote_start( struct match *mt );
void tag_start( struct match *mt );

void stop( const char *msg );
void stop2( const char *fmt, ... );

/* misc parameters */
size_t _lineno = 0;
char *_m_print = "print";
char *_m_printf = "printf";
int _no_comments = 0;
int _do_mainfunc = 0;
int _do_send_headers = 0;
int _reached_eof = 0;

#define _M_PRINT_DEFINE \
  "#include <stdio.h>\n#define print(x) fputs(x, stdout)\n\n"

#define _M_PRINTF_DEFINE \
  "#include <stdio.h>\n\n"

#define MAINFUNC_START \
    "int main(void) {\n"

#define MAINFUNC_STOP \
    "\nreturn 0; } /* end main function */\n"

#define CONTENTTYPE_HTML \
    "Content-Type: text/html; charset=utf-8"

/*
 *  Altogether, NONDIGIT DIGIT SPECIALCHAR correspond to the
 *  POSIX portable filename character set, plus the delimiters '/' and '\'.
 *  That should make it for decent file names or paths given as arguments.
 */

#define NONDIGIT \
  "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"

#define DIGIT \
  "0123456789"

#define SPECIALCHAR \
  ".-/\\" /* underscore is in NONDIGIT */

/* additional chars for c++ */
#define CPPSPECIAL ":.-<>"

/* buffer for html output */
#ifndef _MSC_VER
FILE *_f = NULL; /* file for open_memstream */
#else
char *_b;
size_t _b_len = 0; /* length of buffer */
#endif
char *_buf = NULL;
size_t _bufsz = 0;

/* input buffer (line) */
char _input[INPUTSIZE+1];
/* always points to the end of input line */
char *_eol = _input;

/* cursor */
char *_q = _input;
size_t _q_len = 0;

/*
 *  Context tables
 *  Longer matches must be searched first.
 */

struct match c_context[] =
{
    { "?>\r\n", 4, &c_end, NULL },
    { "?>\n",   3, &c_end, NULL },
    { "?>",     2, &c_end, NULL },
    { "/*",     2, &c_ml_comment_start, NULL },
    { "//",     2, &c_sl_comment_start, NULL },
    { "\"",     1, &c_dquote_start, NULL },
    { "'",      1, &c_squote_start, NULL },
    { "#",      1, &c_macro_start, NULL },
    { NULL, 0, NULL, NULL }
};

struct match html_context[] =
{
    { "<script",    8, &script_start, NULL },
    { "<SCRIPT",    8, &script_start, NULL },
    { "<style",     7, &style_start, NULL },
    { "<STYLE",     7, &style_start, NULL },
    { "<!--",       4, &html_comment_start, NULL },
    { "<?\r\n",     4, &c_start, NULL },
    { "<?\n",       3, &c_start, NULL },
    { "<?=",        3, &c_print_start, NULL },
    { "<?%",        3, &c_print_format_start, NULL },
    { "<?",         2, &c_start, NULL },
    { "< ",         2, &bad_tag_start, NULL },
    { "<\n",        2, &bad_tag_start, NULL },
    { "<",          1, &tag_start, NULL },
    { ">",          1, &bad_tag_end, NULL },
    { NULL, 0, NULL, NULL }
};

struct match script_context[] =
{
    { "</script>",  9, &script_end, NULL },
    { "</SCRIPT>",  9, &script_end, NULL },
    { "/*",         2, &script_ml_comment_start, NULL },
    { "//",         2, &script_sl_comment_start, NULL },
    { "\"",         1, &script_dquote_start, NULL },
    { "'",          1, &script_squote_start, NULL },
    { NULL, 0, NULL, NULL }
};

struct match style_context[] =
{
    { "</style>",   8, &style_end, NULL },
    { "</STYLE>",   8, &style_end, NULL },
    { "/*",         2, &style_ml_comment_start, NULL },
    { NULL, 0, NULL, NULL }
};

struct match tag_context[] =
{
    { "\"",   1, &tag_dquote_start, NULL },
    { "'",    1, &tag_squote_start, NULL },
    { ">",    1, &tag_end, NULL },
    { NULL, 0, NULL, NULL }
};

/* current context */
struct match *_context = html_context;
/* current context fallback */
void (*_context_fallback)( const char* eol ) = &html_fallback;

int main(int argc, char *argv[])
{
    int i;
    char *comment = NULL;
    char *prefix = NULL;
    char *suffix = NULL;
    int print_arg = 0;
    int printf_arg = 0;
    int input_file_arg = 0;
    char *input_file = NULL;
    int output_file_arg = 0;
    char *output_file = NULL;
    int endof_options = 0;

    for (i = 1; i < argc; ++i) {
        char* p = argv[i];
        if (endof_options == 0) {
            /* Options */
            if (!strcmp(p, "-b")) {
                _no_comments = 1;
                continue;
            }
            else if (!strcmp(p, "-c")) {
                if (++i >= argc) {
                    stop("missing argument after option '-c'");
                }
                comment = argv[i];
                continue;
            }
            else if (!strcmp(p, "-a")) {
                if (++i >= argc) {
                    stop("missing argument after option '-a'");
                }
                prefix = argv[i];
                continue;
            }
            else if (!strcmp(p, "-z")) {
                if (++i >= argc) {
                    stop("missing argument after option '-z'");
                }
                suffix = argv[i];
                continue;
            }
            else if (!strcmp(p, "-p")) {
                if (++i >= argc) {
                    stop("missing argument after option '-p'");
                }
                if (check_identifier(argv[i]) == 0) {
                    stop2("invalid identifier for option '-p': '%s'", argv[i]);
                }
                _m_print = argv[i];
                print_arg = 1;
                continue;
            }
            else if (!strcmp(p, "-f")) {
                if (++i >= argc) {
                    stop("missing argument after option '-f'");
                }
                if (check_identifier(argv[i]) == 0) {
                    stop2("invalid identifier for option '-f': '%s'", argv[i]);
                }
                _m_printf = argv[i];
                printf_arg = 1;
                continue;
            }
            else if (!strcmp(p, "--main")) {
                _do_mainfunc = 1;
                continue;
            }
            else if (!strcmp(p, "--html")) {
                _do_send_headers = 1;
                continue;
            }
            else if (!strcmp(p, "-v") || !strcmp(p, "--version")) {
                if (fputs(_version, stdout) == EOF) {
                    stop("lost stdout");
                }
                if (fprintf(stdout, COMPILED_WITH, INPUTSIZE) < 0) {
                    stop("lost stdout");
                }
                exit(EXIT_SUCCESS);
            }
            else if (!strcmp(p, "-h") || !strcmp(p, "--help")) {
                if (fputs(_usage, stdout) == EOF) {
                    stop("lost stdout");
                }
                exit(EXIT_SUCCESS);
            }
        }
        /* Arguments */
        if (strcmp(p, "-") != 0) {
            if (check_filepath(p) == 0) {
                stop2("invalid argument '%s'", p);
            }
            if (!input_file_arg) {
                input_file = p;
                input_file_arg = 1;
            }
            else if (!output_file_arg) {
                output_file = p;
                output_file_arg = 1;
            }
            else {
                stop("too many arguments");
            }
            if (endof_options == 0) {
                endof_options = 1;
            }
            continue;
        }
        else /* arg == '-' */
        {
            if (!input_file_arg) {
                /* reading from stdin */
                input_file_arg = 1;
            }
            else if (!output_file_arg) {
                /* writing to stdout */
                output_file_arg = 1;
            }
            else {
                stop("too many arguments");
            }
            if (endof_options == 0) {
                endof_options = 1;
            }
            continue;
        }
    }
    if (input_file) {
        /* open input file */
        if (freopen(input_file, "r", stdin) == NULL) {
            stop2("unable to open '%s' for reading", input_file);
        }
    }
    if (output_file) {
        /* open output file */
        if (freopen(output_file, "w", stdout) == NULL) {
            stop2("unable to open '%s' for writing", output_file);
        }
    }
    if (comment == NULL) {
        /* print default comment */
        char tmp[90];
#ifndef _MSC_VER
        char* fmt =
            "/*\n"
            " *\tGenerated by nanabozo (do not edit)\n"
            " *\t%a %b %d %H:%M:%S %Z %Y\n"
            " */\n\n";
#else /* no %Z on windows */
        char* fmt =
            "/*\n"
            " *\tGenerated by nanabozo (do not edit)\n"
            " *\t%a %b %d %H:%M:%S %Y\n"
            " */\n\n";
#endif
        time_t t = time(NULL);
        strftime(tmp, 90, fmt, localtime(&t));
        if (fputs(tmp, stdout) == EOF) {
            stop("lost stdout");
        }
    }
    else if (*comment) {
        /* print user comment */
        fprintf(stdout, "/*\n%s\n*/\n", comment);
    }
    if (print_arg == 0) {
        /* define print(x) */
        if (fputs(_M_PRINT_DEFINE, stdout) == EOF) {
            stop("lost stdout");
        }
    }
    else if (printf_arg == 0) {
        /* need stdio.h */
        if (fputs(_M_PRINTF_DEFINE, stdout) == EOF) {
            stop("lost stdout");
        }
    }
    if (prefix && *prefix) {
        /* print prefix string */
        fprintf(stdout, "%s\n", prefix);
    }
    if (_do_mainfunc) {
        if (fputs(MAINFUNC_START, stdout) == EOF) {
            stop("lost stdout");
        }
    }
    if (_do_send_headers) {
        if (fprintf(stdout, "%s(\"%s\\n\\n\");\n",
                    _m_print, CONTENTTYPE_HTML) < 0) {
            stop("lost stdout");
        }
    }
#ifdef _MSC_VER
    /* prepare buffer */
    if ((_buf = malloc(PAGESIZE)) == NULL) {
        stop("no memory");
    }
    _bufsz = PAGESIZE;
    _buf[0] = '\0';
    _b = _buf;
#endif
    /* start scanning */
    proceed();
    /* send the last bits */
    _reached_eof = 1;
    bufout();
    if (_do_mainfunc) {
        if (fputs(MAINFUNC_STOP, stdout) == EOF) {
            stop("lost stdout");
        }
    }
    if (suffix && *suffix) {
        /* print suffix string */
        fprintf(stdout, "%s\n", suffix);
    }
    return EXIT_SUCCESS;
}
void proceed( void )
{
    struct match *mt = NULL;

    while (read_input() != 0) {
        while ((mt = context_match()) != NULL) {
            if (mt->p > _q) {
                /* eat preceding string */
                (*_context_fallback)(mt->p);
            }
            assert(_q == mt->p);
            (*mt->hook)(mt);
            if (_q == _eol) {
                break;
            }
        }
        /* no more matches in current line */
        if (_q != _eol) {
            (*_context_fallback)(NULL);
        }
    }
}
size_t read_input( void )
{
    assert(_q == _eol && _q_len == 0);
    /* reset line */
    memset(_input, 0, INPUTSIZE+1);
    _q_len = 0;
    _eol = _q = _input;
    /* read line */
    if (fgets(_input, INPUTSIZE+1, stdin) == NULL) {
        return 0;
    }
    _q_len = strlen(_input);
    if (_q_len >= INPUTSIZE) {
        stop2("reached maximum input size (%lu)", INPUTSIZE);
    }
    _eol += _q_len;
    /* reset contexts */
    reset_context(c_context);
    reset_context(html_context);
    reset_context(script_context);
    reset_context(style_context);
    reset_context(tag_context);
    /* increment line count */
    _lineno++;
    return _q_len;
}
struct match *context_match( void )
{
    struct match *next = NULL;
    struct match *mt = _context;
    char *p;

    assert(_q != _eol);
    for (; mt->str; mt++) {
        if (!mt->p || mt->p < _q) {
            /* mt was not searched in this context */
            p = strstr(_q, mt->str);
            mt->p = p ? p : _eol;
        }
        if (mt->p != _eol) {
            /* something was found in this context */
            if (!next || mt->p < next->p) {
                /* mt is closer match */
                next = mt;
            }
        }
    }
    return next;
}
void reset_context( struct match *mt )
{
    for (; mt->str; mt++) {
        mt->p = NULL;
    }
}
int check_identifier( const char* id )
{
    if (!id || !*id || strchr(NONDIGIT CPPSPECIAL, *id++) == NULL) {
        return 0;
    }
    for (; *id; id++) {
        if (strchr(NONDIGIT DIGIT CPPSPECIAL, *id) == NULL) {
            return 0;
        }
    }
    return 1;
}
int check_filepath( const char* fpath )
{
    if (!fpath || !*fpath || *fpath == '-') {
        return 0;
    }
    for (; *fpath; fpath++) {
        if (strchr(NONDIGIT DIGIT SPECIALCHAR, *fpath) == NULL) {
            return 0;
        }
    }
    return 1;
}
#ifndef _MSC_VER
void bufwrite( const char *s, const size_t len )
{
    if ((!_f && (_f = open_memstream(&_buf, &_bufsz)) == NULL)
        || fwrite(s, sizeof(char), len, _f) != len)
    {
        stop("no memory");
    }
}
#else
void bufwrite( const char *s, const size_t len )
{
    if (_b_len + len >= _bufsz) {
        const size_t sz = (((_bufsz + len) / PAGESIZE) + 1) * PAGESIZE;
        if ((_buf = realloc(_buf, sz)) == NULL) {
            stop("no memory");
        }
        _bufsz = sz;
        _b = _buf + _b_len;
    }
    _b_len += len;
    strncat(_b, s, len);
    _b += len;
}
#endif
void bufout( void )
{
#ifndef _MSC_VER
    if (!_f) {
        return;
    }
    fclose(_f);
    _f = NULL;
    if (_bufsz) {
#else
    if (_b_len) {
#endif
        char *p = _buf;
        /* dont send trailing spaces */
        if (_reached_eof) {
            while (*p && isspace(*p)) {
                ++p;
            }
            if (!*p) {
                goto bufout_done;
            }
            else {
                p = _buf;
            }
        }
        /* transfer buffer to stdout */
        fprintf(stdout, "\n%s(\"", _m_print);
        for (; *p; p++) {
            switch (*p) {
            case '\\':
                write("\\\\", 2);
                break;
            case '"':
                write("\\\"", 2);
                break;
            case '\n':
                if (*(p+1)) {
                    write("\\n\"\n\"", 5);
                }
                else {
                    write("\\n\"", 3);
                }
                break;
            case '\r':
                write("\\r", 2);
                break;
            case '\t':
                write("\\t", 2);
                break;
            case '\a':
            case '\b':
            case '\f':
            case '\v':
                break;
            default:
                put(*p);
            }
        }
        if (*(p-1) != '\n') {
            write("\");\n", 4);
        }
        else {
            write(");\n", 3);
        }
    bufout_done:
#ifndef _MSC_VER
        _bufsz = 0;
#else
        /* reset buffer */
        free(_buf);
        _buf = NULL;
        if ((_buf = malloc(PAGESIZE)) == NULL) {
            stop("no memory");
        }
        _bufsz = PAGESIZE;
        _buf[0] = '\0';
        _b_len = 0;
        _b = _buf;
#endif
    }
#ifndef _MSC_VER
    /* reset buffer */
    if (_buf) {
        free(_buf);
        _buf = NULL;
    }
#endif
}
#ifndef _MSC_VER
void bufput( const int c )
{
    if ((!_f && (_f = open_memstream(&_buf, &_bufsz)) == NULL)
        || fputc(c, _f) != c)
    {
        stop("no memory");
    }
}
#else
void bufput( const int c )
{
    char tmp[2] = {'\0', '\0'};
    if (_b_len + 1 >= _bufsz) {
        const size_t sz = (((_bufsz + 1) / PAGESIZE) + 1) * PAGESIZE;
        if ((_buf = realloc(_buf, sz)) == NULL) {
            stop("no memory");
        }
        _bufsz = sz;
        _b = _buf + _b_len;
    }
    _b_len++;
    tmp[0] = c;
    strcat(_b, tmp);
    _b++;
}
#endif
void write( const char *s, const size_t len )
{
    if (fwrite(s, sizeof(char), len, stdout) != len) {
        stop("lost stdout");
    }
}
void put( const int c )
{
    if (fputc(c, stdout) != c) {
        stop("lost stdout");
    }
}
int cursor( void )
{
  if (_q != _eol || read_input() != 0) {
      _q_len--;
      return *_q++;
  }
  return EOF;
}
void c_fallback( const char *eol )
{
    const size_t sz = eol ? eol - _q : _q_len;
    assert(*_q && sz);
    write(_q, sz);
    _q += sz;
    _q_len -= sz;
}
void html_fallback( const char *eol )
{
    const size_t sz = eol ? eol - _q : _q_len;
    assert(*_q && sz);
    bufwrite(_q, sz);
    _q += sz;
    _q_len -= sz;
}
void bad_tag_end( struct match *mt )
{
    stop("special char '>' => '&gt;' ?");
}
void bad_tag_start( struct match *mt )
{
    stop("special char '<' => '&lt;' ?");
}
void c_dquote_start( struct match *mt )
{
    write(_q, mt->len);
    _q += mt->len;
    _q_len -= mt->len;
    eat_c_dquote();
}
void c_end( struct match *mt )
{
    if (!_no_comments) {
        fprintf(stdout, "/* END C (line %lu) */", _lineno);
    }
    _q += mt->len;
    _q_len -= mt->len;
    _context = html_context;
    _context_fallback = &html_fallback;
}
void c_macro_start( struct match *mt )
{
    write(_q, mt->len);
    _q += mt->len;
    _q_len -= mt->len;
    eat_c_macro();
}
void c_ml_comment_start( struct match *mt )
{
    write(_q, mt->len);
    _q += mt->len;
    _q_len -= mt->len;
    eat_c_ml_comment();
}
void c_sl_comment_start( struct match *mt )
{
    write(_q, mt->len);
    _q += mt->len;
    _q_len -= mt->len;
    eat_c_sl_comment();
}
void c_squote_start( struct match *mt )
{
    write(_q, mt->len);
    _q += mt->len;
    _q_len -= mt->len;
    eat_c_squote();
}
void c_start( struct match *mt )
{
    bufout();
    if (!_no_comments) {
        fprintf(stdout, "/* BEGIN C (line %lu) */\n", _lineno);
    }
    _q += mt->len;
    _q_len -= mt->len;
    _context = c_context;
    _context_fallback = &c_fallback;
}
void c_print_format_start( struct match *mt )
{
    bufout();
    if (!_no_comments) {
        fprintf(stdout, "/* BEGIN C%% (line %lu) */\n", _lineno);
    }
    _q += mt->len;
    _q_len -= mt->len;
    eat_c_print_format();
    if (!_no_comments) {
        fprintf(stdout, "\n/* END C%% (line %lu) */", _lineno);
    }
}
void c_print_start( struct match *mt )
{
    bufout();
    if (!_no_comments) {
        fprintf(stdout, "/* BEGIN C= (line %lu) */\n", _lineno);
    }
    _q += mt->len;
    _q_len -= mt->len;
    eat_c_print_string();
    if (!_no_comments) {
        fprintf(stdout, "\n/* END C= (line %lu) */", _lineno);
    }
}
void html_comment_start( struct match *mt )
{
    bufwrite(_q, mt->len);
    _q += mt->len;
    _q_len -= mt->len;
    eat_html_comment();
}
void script_dquote_start( struct match* mt )
{
    bufwrite(_q, mt->len);
    _q += mt->len;
    _q_len -= mt->len;
    eat_script_dquote();
}
void script_ml_comment_start( struct match* mt )
{
    bufwrite(_q, mt->len);
    _q += mt->len;
    _q_len -= mt->len;
    eat_script_ml_comment();
}
void script_sl_comment_start( struct match* mt )
{
    bufwrite(_q, mt->len);
    _q += mt->len;
    _q_len -= mt->len;
    eat_script_sl_comment();
}
void script_squote_start( struct match* mt )
{
    bufwrite(_q, mt->len);
    _q += mt->len;
    _q_len -= mt->len;
    eat_script_squote();
}
void script_end( struct match *mt )
{
    bufwrite(_q, mt->len);
    _q += mt->len;
    _q_len -= mt->len;
    _context = html_context;
    _context_fallback = &html_fallback;
}
void script_start( struct match *mt )
{
    bufwrite(_q, mt->len);
    _q += mt->len;
    _q_len -= mt->len;
    _context = script_context;
    _context_fallback = &html_fallback;
}
void style_end( struct match *mt )
{
    bufwrite(_q, mt->len);
    _q += mt->len;
    _q_len -= mt->len;
    _context = html_context;
    _context_fallback = &html_fallback;
}
void style_ml_comment_start( struct match* mt )
{
    bufwrite(_q, mt->len);
    _q += mt->len;
    _q_len -= mt->len;
    eat_script_ml_comment();
}
void style_start( struct match *mt )
{
    bufwrite(_q, mt->len);
    _q += mt->len;
    _q_len -= mt->len;
    _context = style_context;
    _context_fallback = &html_fallback;
}
void tag_dquote_start( struct match *mt )
{
    bufwrite(_q, mt->len);
    _q += mt->len;
    _q_len -= mt->len;
    eat_script_dquote();
}
void tag_end( struct match *mt )
{
    bufwrite(_q, mt->len);
    _q += mt->len;
    _q_len -= mt->len;
    _context = html_context;
    _context_fallback = &html_fallback;
}
void tag_squote_start( struct match *mt )
{
    bufwrite(_q, mt->len);
    _q += mt->len;
    _q_len -= mt->len;
    eat_script_squote();
}
void tag_start( struct match *mt )
{
    bufwrite(_q, mt->len);
    _q += mt->len;
    _q_len -= mt->len;
    _context = tag_context;
    _context_fallback = &html_fallback;
}
void eat_c_dquote( void )
{
    int i, prev = -1;
    while ((i = cursor()) != EOF) {
        if (i == '\n') {
            stop("unexpected newline in C double-quoted string");
        }
        put(i);
        if (i == '"' && prev != '\\') {
            return;
        }
        prev = i;
    }
    stop("eof while scanning C double-quoted string");
}
void eat_c_macro( void )
{
    int i, j = 0, prev = -1;
    while (j || (i = cursor()) != EOF) {
        if (j) {
            i = j;
            j = 0;
        }
        switch (i) {
        case '"':
            /* dquote string begins */
            put(i);
            eat_c_dquote();
            prev = -1;
            continue;
        case '\'':
            /* squote char begins */
            put(i);
            eat_c_squote();
            prev = -1;
            continue;
        case '/':
            switch ((j = cursor())) {
            case EOF:
                stop("eof while scanning C macro");
            case '*':
                /* ml comment begins */
                write("/*", 2);
                eat_c_ml_comment();
                return;
            case '/':
                /* sl comment begins */
                write("//", 2);
                eat_c_sl_comment();
                return;
            }
        }
        put(i);
        if (i == '\n' && prev != '\\') {
            /* end of macro */
            return;
        }
        prev = i;
    }
    stop("eof while scanning C macro");
}
void eat_c_print_format( void )
{
    int i, j = 0;
    fprintf(stdout, "%s(", _m_printf);
    while (j || (i = cursor()) != EOF) {
        if (j) {
            i = j;
            j = 0;
        }
        switch (i)
        {
        case '"':
            /* dquote string begins */
            put(i);
            eat_c_dquote();
            continue;
        case '\'':
            /* squote char begins */
            put(i);
            eat_c_squote();
            continue;
        case '?':
            switch ((j = cursor())) {
            case EOF:
                stop("eof while scanning C print-formatted arguments");
            case '>':
                /* end tag */
                write(");", 2);
                return;
            default:
                put('?');
                continue;
            }
        }
        put(i);
    }
    stop("eof while scanning C print-formatted arguments");
}
void eat_c_print_string( void )
{
    int i, j = 0;
    fprintf(stdout, "%s(", _m_print);
    while (j || (i = cursor()) != EOF) {
        if (j) {
            i = j;
            j = 0;
        }
        switch (i) {
        case '"':
            /* dquote string begins */
            put(i);
            eat_c_dquote();
            continue;
        case '\'':
            /* squote char begins */
            put(i);
            eat_c_squote();
            continue;
        case '?':
            switch ((j = cursor())) {
            case EOF:
                stop("eof while scanning C print-string arguments");
            case '>':
                /* end tag */
                write(");", 2);
                return;
            }
        }
        put(i);
    }
    stop("eof while scanning C print-string arguments");
}
void eat_c_ml_comment( void )
{
    int i, prev = -1;
    while ((i = cursor()) != EOF) {
        put(i);
        if (i == '/' && prev == '*') {
            /* end of comment */
            return;
        }
        prev = i;
    }
    stop("eof while scanning C multi-line comment");
}
void eat_c_sl_comment( void )
{
    int i;
    while ((i = cursor()) != EOF) {
        put(i);
        if (i == '\n') {
            /* end of comment */
            return;
        }
    }
    stop("eof while scanning C single-line comment");
}
void eat_c_squote( void )
{
    int i, j = 0;
    while ((i = cursor()) != EOF) {
        switch (j) {
        case 0:
            switch (i) {
            case '\'':
            case '\n':
                /* invalid chars, etc todo */
                stop("invalid C single-quoted char");
            }
            put(i);
            if (i == '\\') {
                j++;
                continue;
            }
            else {
                j += 2;
                continue;
            }
        case 1:
            put(i);
            j++;
            continue;
        case 2:
            /* expecting single quote */
            if (i != '\'') {
                stop("invalid C single-quoted char");
            }
            put(i);
            return;
        }
    }
    stop("eof while scanning C single-quoted char");
}
void eat_html_comment( void )
{
    int i, prev = -1, prev1 = -1;
    while ((i = cursor()) != EOF) {
        bufput(i);
        if (i == '>' && prev == '-' && prev1 == '-') {
            /* end of comment */
            return;
        }
        prev1 = prev;
        prev = i;
    }
    stop("eof while scanning html comment");
}
void eat_script_dquote( void )
{
    int i, prev = -1;
    while ((i = cursor()) != EOF) {
        if (i == '\n') {
            stop("unexpected newline in script double-quoted string");
        }
        bufput(i);
        if (i == '"' && prev != '\\') {
            /* end of string */
            return;
        }
        prev = i;
    }
    stop("eof while scanning script double-quoted string");
}
void eat_script_ml_comment( void )
{
    int i, prev = -1;
    while ((i = cursor()) != EOF) {
        bufput(i);
        if (i == '/' && prev == '*') {
            /* end of comment */
            return;
        }
        prev = i;
    }
    stop("eof while scanning script multi-line comment");
}
void eat_script_sl_comment( void )
{
    int i;
    while ((i = cursor()) != EOF) {
        bufput(i);
        if (i == '\n') {
            /* end of comment */
            return;
        }
    }
    stop("eof while scanning script single-line comment");
}
void eat_script_squote( void )
{
    int i, prev = -1;
    while ((i = cursor()) != EOF) {
        if (i == '\n') {
            stop("unexpected newline in script single-quoted string");
        }
        bufput(i);
        if (i == '\'' && prev != '\\') {
            /* end of string */
            return;
        }
        prev = i;
    }
    stop("eof while scanning script single-quoted string");
}
void stop( const char* msg )
{
    bufout();
    fputs("\nnanabozo error: ", stderr);
    fputs(msg, stderr);
    if (_lineno > 0) {
        fprintf(stderr, "\n(line: %lu)\n", _lineno);
    }
    else {
        fputc('\n', stderr);
    }
    exit(EXIT_FAILURE);
}
void stop2( const char* fmt, ... )
{
    va_list ap;
    va_start(ap, fmt);
    bufout();
    fputs("\nnanabozo error: ", stderr);
    vfprintf(stderr, fmt, ap);
    if (_lineno > 0) {
        fprintf(stderr, "\n(line: %lu)\n", _lineno);
    }
    else {
        fputc('\n', stderr);
    }
    va_end(ap);
    exit(EXIT_FAILURE);
}

/* vi: set fenc=utf-8 ff=unix et ai sw=4 ts=4 sts=4 : */
