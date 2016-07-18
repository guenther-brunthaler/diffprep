static char const version_info[]=
   "$APPLICATION_NAME Version 2016.200.2\n"
   "\n"
   "Copyright (c) 2016 Guenther Brunthaler. All rights reserved.\n"
   "\n"
   "This program is free software.\n"
   "Distribution is permitted under the terms of the GPLv3.\n"
;

/* User configuration option: Include "-D CONFIG_NO_LOCALE" in your CFLAGS in
 * order to build a version without locale or MBCS/UTF-8 support. */
#ifndef CONFIG_NO_LOCALE
   #define CONFIG_NO_LOCALE 0
#endif

static char const help[]=
   "Usage: $APPLICATION_NAME [ <options> ... [--] ] [ <input_file> ]\n"
   "\n"
   "$APPLICATION_NAME allows one to word-diff or character-diff text files,\n"
   "and to byte-diff or bit-diff binary files.\n"
   "\n"
   "It is a preprocessor for 'diff', losslessly transforming its input\n"
   "input into an intermediate line-based text format which can used with\n"
   "all usual line-based 'diff', 'patch' and merge tools.\n"
   "\n"
   "After one is done diffing, merging or patching the transformed files,\n"
   "$APPLICATION_NAME can be used again as a postprocessor for those files,\n"
   "transforming the intermediate line-based format back into its original\n"
   "form.\n"
   "\n"
   "By default, $APPLICATION_NAME reads from its standard input stream and\n"
   "writes the transformed input to its standard output stream. If\n"
   "<input_file> is specified, however, it reads that file rather than\n"
   "standard input.\n"
   "\n"
   "The options select the operation mode: Which kind of transformation is\n"
   "applied to the input file.\n"
   "\n"
   #if CONFIG_NO_LOCALE
   "Note that $APPLICATION_NAME has been built without locale support. Only\n"
   "ASCII and single-byte character sets which are supersets of ASCII are\n"
   "supported.\n"
   #else
   "For operation modes which read text files, the LC_CTYPE locale category\n"
   "determines the expected encoding of the input file. For instance, this\n"
   "allows $APPLICATION_NAME to process UTF-8 encoded input correctly, if a\n"
   "UTF-8 locale is currently in effect.\n"
   #endif
   "\n"
   "For every operation mode, there is a reverse operation mode which\n"
   "undoes the transformations made, converting a transformed file back\n"
   "into its original format.\n"
   "\n"
   "The default transformation mode is -w (which does word-diff\n"
   "preprocessing).\n"
   "\n"
   "The text-based intermediate output formats for the -w and -c modes have\n"
   "been designed to interact well with the \"diff -b\" and \"patch -l\"\n"
   "options, which allow to ignore whitespace differences in comparisons\n"
   "and merge operations.\n"
   "\n"
   "Supported options:\n"
   "\n"
   "-w: Convert normal text into an intermediate format containing only a\n"
   "    single word in every line. Use this for word-diffing.\n"
   "\n"
   "-W: Convert a file produced by -w back into a normal text file with an\n"
   "    unlimited number of words in its lines.\n"
   "\n"
   "-c: Convert normal text into an intermediate format containing only a\n"
   "    single character in every line. Use this for character-based\n"
   "    diffing.\n"
   "\n"
   "-C: Convert a file produced by -c back into a normal text file with an\n"
   "    unlimited number of characters in its lines.\n"
   "\n"
   "-x: Convert a binary file into an intermediate text format containing\n"
   "    only a single byte in every line, formatted as an uppercase 2-digit\n"
   "    hexadecimal ASCII number. Use this for binary-diffing and merging.\n"
   "\n"
   "-X: Convert a text file produced by -x back into a normal binary file\n"
   "    with an unstructured layout which is not line-oriented in any way.\n"
   "\n"
   "-b: Convert a binary file into an intermediate text format containing\n"
   "    only a single bit in every line, formatted as the character '0' or\n"
   "    '1'. Use this for bitstream-diffing and merging.\n"
   "\n"
   "-B: Convert a text file produced by -b back into a normal binary file\n"
   "    with an unstructured layout which is not line-oriented in any way.\n"
   "\n"
   "-n <count>: Specifies how many values per line options -x and -b will\n"
   "    emit. This count is 1 by default, because otherwise it would not be\n"
   "    possible to find changes with byte- or bit-granularity. However, it\n"
   "    makes sense, for example, to display 8 bits per line with -b if\n"
   "    byte granularity is sufficient, or to display 3 bytes per line with\n"
   "    -x as pixel values of an uncompressed 24-bit RGB raw image.\n"
   "\n"
   "-a: Add an ASCII dump of each byte after the end of the line normally\n"
   "    produced as the output of options -x and -b, provided it is not\n"
   "    invisible or a control character. This is helpful if part of the\n"
   "    binary data is actually ASCII text, and helps a human reader of the\n"
   "    output file correlate the hex values with the ASCII characters. The\n"
   "    ASCII dump will be ignored when the file is converted back into its\n"
   "    original format; only the values at the left side of the dump will\n"
   "    be processed.\n"
   "\n"
   "-h: Display this help.\n"
   "\n"
   "-V: Display only the copyright and version information.\n"
   "\n"
;


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>

#ifdef MALLOC_TRACE
   #ifdef NDEBUG
      #undef MALLOC_TRACE
   #else
      #include <mcheck.h>
      #include <malloc.h>
   #endif
#endif

#if !CONFIG_NO_LOCALE
   #include <locale.h>
#endif

#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901 \
    && !CONFIG_NO_LOCALE
   #include <wctype.h>
#else
   #include <ctype.h>
   #ifdef iswspace
      #undef iswspace
   #endif
   #define iswspace(wc) ( \
      (wc) <= (wchar_t)UCHAR_MAX && isspace((int)(unsigned char)(wc)) \
   )
#endif

#define CMD_WORD ':'
#define CMD_NEWLINE 'n'
#define NL_RPLC ' '


static void die(char const *msg, ...) {
   va_list args;
   va_start(args, msg);
   (void)vfprintf(stderr, msg, args);
   va_end(args);
   (void)fputc('\n', stderr);
   exit(EXIT_FAILURE);
}

static void chk_stdin(void) {
   if (ferror(stdin)) die("Error reading from standard input stream!");
}

static void stdout_error(void) {
   die("Error writing to standard output stream!");
}

static void ck_printf(char const *format, ...) {
   va_list args;
   va_start(args, format);
   if (vprintf(format, args) < 0) stdout_error();
   va_end(args);
}

static void ck_putc(int c) {
   if (putchar(c) != c) stdout_error();
}

static void ck_write(char const *buf, size_t bytes) {
   if (fwrite(buf, sizeof(char), bytes, stdout) != bytes) stdout_error();
}

static void ck_puts(char const *s) {
   if (fputs(s, stdout) < 0) stdout_error();
}

static void perror_exit(int e, char const *msg) {
   errno= e;
   perror(msg);
   exit(EXIT_FAILURE);
}

#if CONFIG_NO_LOCALE
   #ifdef wctomb
      #undef wctomb
   #endif
   #define wctomb fake_wctomb
   #ifdef mbtowc
      #undef mbtowc
   #endif
   #define mbtowc fake_mbtowc
   #ifdef MB_LEN_MAX
      #undef MB_LEN_MAX
   #endif
   #define MB_LEN_MAX 1
   #ifdef MB_CUR_MAX
      #undef MB_CUR_MAX
   #endif
   #define MB_CUR_MAX MB_LEN_MAX

   static int wctomb(char *s, wchar_t wchar) {
      if (!s) return 0;
      s[0]= (char)(unsigned)wchar;
      return 1;
   }

   static int mbtowc(wchar_t *pwc, const char *s, size_t n) {
      if (!s) return 0;
      if (n < 1) return -1;
      if (pwc) *pwc= (wchar_t)(int)s[0];
      return 1;
   }
#endif

static void die_with_wchar(char const *msg, wchar_t wc) {
   char chr[MB_LEN_MAX + 1];
   int r;
   if ((r= wctomb(chr, wc)) < 1) {
      die(
            "Cannot display error message \"%s\" for wide character"
            " with code point %#lx!"
         ,  msg, (unsigned long)wc
      );
   }
   assert((size_t)r < sizeof chr);
   chr[r]= '\0';
   die(msg, chr);
}

static void appinfo(const char *text, const char *app) {
   static char const marker[]= "$APPLICATION_NAME";
   int const mlen= (int)(sizeof marker - sizeof(char));
   char const *end;
   while (end= strstr(text, marker)) {
      ck_write(text, (size_t)(end - text));
      ck_puts(app);
      text= end + mlen;
   }
   if (text) ck_puts(text);
}

static void bitdump(int byte) {
   int mask;
   for (mask= 0x80; mask; mask>>= 1) {
      ck_putc(byte & mask ? '1' : '0');
      ck_putc('\n');
   }
}

int main(int argc, char **argv) {
   int mode= 'w';
   int ascii_dump= 0;
   unsigned units_per_line= 1;
   #ifdef MALLOC_TRACE
       assert(mcheck_pedantic(0) == 0);
       (void)mallopt(M_PERTURB, (int)0xaaaaaaaa);
       mcheck_check_all();
       mtrace();
   #endif
   #if !CONFIG_NO_LOCALE
      (void)setlocale(LC_ALL, "");
   #endif
   if (argc > 1) {
      int optind= 1, argpos;
      char *arg;
      process_arg:
      arg= argv[optind];
      for (argpos= 0;; ) {
         int c;
         switch (c= arg[argpos]) {
            case '-':
               if (argpos) goto bad_option;
               switch (arg[++argpos]) {
                  case '-':
                     if (c= arg[argpos + 1]) {
                        die("Unsupported long option %s!", arg);
                     }
                     ++optind; /* "--". */
                     goto end_of_options;
                  case '\0': goto end_of_options; /* "-". */
               }
               /* At first option switch character now. */
               continue;
            case '\0': {
               next_arg:
               if (++optind == argc) goto end_of_options;
               goto process_arg;
            }
         }
         if (!argpos) goto end_of_options;
         switch (c) {
            case 'W': case 'C': case 'J': case 'X': case 'B':
            case 'w': case 'c': case 'j': case 'x': case 'b':
               mode= c;
               break;
            case 'a': ascii_dump= 1; break;
            case 'n':
               if (!arg[++argpos]) {
                  if (++optind == argc) {
                     die("Missing argument for option -%c!", c);
                  }
                  arg= argv[optind];
                  argpos= 0;
               }
               {
                  unsigned long optval;
                  {
                     char *end;
                     optval= strtoul(arg + argpos, &end, 0);
                     if (!*arg || *end) {
                        invalid_argument:
                        die(
                              "Invalid argument '%s' for option -%c!"
                           ,  arg + argpos, c
                        );
                     }
                  }
                  units_per_line= (unsigned)optval;
                  if (units_per_line != optval) goto invalid_argument;
               }
               goto next_arg;
            case 'h': appinfo(help, argv[0]);
               /* Fall through. */
            case 'V': appinfo(version_info, argv[0]); goto done;
            default: bad_option: die("Unsupported option -%c!", c);
         }
         ++argpos;
      }
      end_of_options:
      if (optind < argc) {
         char const *fname= argv[optind++];
         char const *fmode= strchr("xbm", mode) ? "rb" : "r";
         if (!freopen(fname, fmode, stdin)) {
            die("Could not open file \"%s\" in mode \"%s\"!", fname, fmode);
         }
      }
      if (optind != argc) die("Too many arguments!");
   }
   /* Reset initial multibyte character conversion shift state - just to be
    * sure. */
   (void)mbtowc(0, 0, 0);
   switch (mode) {
      case 'j': case 'x': case 'b':
         {
            register int c;
            while ((c= getchar()) != EOF) {
               if (mode == 'b') bitdump(c);
               else if (mode == 'x' || c == 0x20) ck_printf("%02X\n", c);
               else ck_printf("%02X %c\n", c, c > 0x20 && c < 0x7f ? c : '.');
            }
         }
         chk_stdin();
         break;
      case 'J': case 'X':
         {
            auto unsigned int b;
            int n, c;
            errno= 0;
            while ((n= scanf("%2x", &b)) == 1) {
               if (putchar((int)b) == EOF) stdout_error();
               while ((c= getchar()) != '\n') {
                  if (c == EOF) {
                     chk_stdin();
                     goto done;
                  }
               }
            }
            if (n == EOF) {
               int e= errno;
               chk_stdin();
               if (e) perror_exit(e, "Parsing error");
            }
            else bad_syntax: die("Input format syntax error!");
         }
         goto done;
      case 'B': {
         do {
            int mask, byte, c;
            for (byte= 0, mask= 0x80; mask; mask>>= 1) {
               if ((c= getchar()) == EOF) {
                  chk_stdin();
                  if (mask == 0x80) goto done;
                  die("Incomplete binary octet (8 bit byte) at end of input!");
               }
               switch (c) {
                  case '0': c= 0; break;
                  case '1': c= mask; break;
                  default: goto bad_syntax;
               }
               byte|= c;
               while ((c= getchar()) != '\n') {
                  if (c == EOF) {
                     chk_stdin();
                     goto bad_syntax;
                  }
               }
            }
            if (putchar(byte) == EOF) stdout_error();
         } while (!feof(stdin));
         goto done;
      }
   }
   {
      size_t const mb_cur_max= MB_CUR_MAX;
      int state= 0;
      char c[MB_LEN_MAX];
      size_t nc0, nc= 0;
      int nnul, b, eof= 0;
      wchar_t wc;
      {
         /* Determine the length of the MBCS-encoding of L'\0'. */
         char nul[MB_LEN_MAX];
         if ((nnul= wctomb(nul, L'\0')) < 1) die("Unsupported locale!");
      }
      for (;;) {
         /* Read as much bytes into c[] as possible, but not more than the
          * longest possible MBCS-sequence. */
         while (!eof && nc < mb_cur_max) {
            if ((b= getchar()) == EOF) {
               chk_stdin();
               eof= 1;
               break;
            }
            c[nc++]= (char)b;
         }
         if (nc == 0) {
            assert(eof);
            break;
         }
         {
            int r;
            auto wchar_t wcbuf; /* Address will be taken. */
            if ((r= mbtowc(&wcbuf, c, nc)) == -1) {
               die(
                     eof
                  ?  "Incomplete multibyte character at end of input!"
                  :  "Illegal character encoding encountered!"
               );
            }
            if (r == 0) r= nnul;
            assert(r > 0);
            assert((size_t)r <= nc);
            /* In contrary to <wcbuf>, <wc> might be a register variable. */
            wc= wcbuf;
            nc0= (size_t)r;
         }
         switch (mode) {
            case 'w': case 'c':
               /* States:
                * 0: Start of file.
                * 1: Convert <NL_RPLC>s into newline charaters.
                * 2: Outputting whitespace at the end of a cmd_word.
                * 3: Outputing non-whitespace characters of a cmd_word. */
               if (wc == L'\n') {
                  switch (state) {
                     case 2: case 3: ck_putc('\n'); /* Fall through. */
                     case 0: ck_putc(CMD_NEWLINE); state= 1; break;
                     default: assert(state == 1); ck_putc(NL_RPLC);
                  }
               } else if (iswspace(wc)) {
                  switch (state) {
                     case 1: ck_putc('\n'); /* Fall through. */
                     case 0: ck_putc(CMD_WORD); /* Fall through. */
                     case 3: state= 2; /* Fall through. */
                     default: assert(state == 2); ck_write(c, nc0);
                  }
               } else {
                  switch (state) {
                     case 1: case 2: ck_putc('\n'); /* Fall through. */
                     case 0: ck_putc(CMD_WORD); state= 3; /* Fall through. */
                     default: {
                        assert(state == 3); ck_write(c, nc0);
                        if (mode == 'c') {
                           ck_putc('\n');
                           state= 0;
                        }
                     }
                  }
               }
               break;
            default: {
               assert(mode == 'W' || mode == 'C');
               /* States:
                * 0: Initial state - parse command letter.
                * 1: Convert <NL_RPLC>s into newline charaters.
                * 2: Output characters before next '\n'. */
               switch (state) {
                  case 0:
                     switch (wc) {
                        case (wchar_t)CMD_WORD: state= 2; break;
                        case (wchar_t)CMD_NEWLINE: state= 1; goto nlgen;
                        default: {
                           die_with_wchar("Invalid line command \"%s\"!", wc);
                        }
                     }
                     break;
                  case 1:
                     switch (wc) {
                        case (wchar_t)NL_RPLC: nlgen: ck_putc('\n'); break;
                        case L'\n': state= 0; break;
                        default: {
                           die_with_wchar(
                              "Invalid newline substitute \"%s\"!", wc
                           );
                        }
                     }
                     break;
                  case 2: {
                     if (wc == L'\n') state= 0;
                     else ck_write(c, nc0);
                  }
               }
            }
         }
         assert(nc0 <= nc);
         if (nc0 < nc) (void)memmove(c, c + nc0, nc - nc0);
         nc-= nc0;
      }
      switch (mode) {
         case 'w': case 'c': {
            switch (state) {
               case 1: case 2: case 3: ck_putc('\n');
            }
         }
      }
   }
   done:
   if (fflush(0)) die("Error writing to output stream!");
   return EXIT_SUCCESS;
}
