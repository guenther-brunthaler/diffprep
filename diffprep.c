static char const version_info[]=
   "$APPLICATION_NAME Version 17.119\n"
   "\n"
   "Copyright (c) 2016-2017 Guenther Brunthaler. All rights reserved.\n"
   "\n"
   "This program is free software.\n"
   "Distribution is permitted under the terms of the GPLv3.\n"
;

/* User configuration option: Include "-D CONFIG_NO_LOCALE" in your CFLAGS in
 * order to build a version without locale or MBCS/UTF-8 support. */
#ifndef CONFIG_NO_LOCALE
   #define CONFIG_NO_LOCALE 0
#endif

static char const *const help[]= {
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
   /* Separated to avoid string literals longer than what an ANSI standard
    * compliant C compiler is required to support. */
   ,
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
   "-t: Append a '$'-terminator to all (and only those) lines produced by\n"
   "    -w and -c which contain whitespace at the end of the line. While\n"
   "    this makes the output larger and renders 'diff -b' and 'patch -l'\n"
   "    ineffective, it is helpful for a human reader. It also helps when\n"
   "    editing the transformed output in a text editor which automatically\n"
   "    strips whitespace from the end of a line. It may also be necessary\n"
   "    when sending the output as plaintext e-mail. '$'-terminators\n"
   "    generated by -t will be ignored by -W and -C.\n"
   "\n"
   "-s: Strips the '$'-terminators added by -t, undoing the effect of -t.\n"
   "    While this is not necessary for -W and -C, it makes the transformed\n"
   "    file shorter and allows to use 'diff -b' and 'patch -l' on it\n"
   "    again.\n"
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
};


#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
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

#if __STDC_VERSION__ >= 199901 && !CONFIG_NO_LOCALE
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

#define DIM(array) (sizeof (array) / sizeof *(array))

#define NL_RPLC ' '
#define ASCII_DUMP_SEP '|'
#define DUMP_UNIT_SEP ' '
#define GHOST_FACE '-'
#define WS_OPT_TERMINATOR '$'


static unsigned long read_pos;

static void die(char const *msg, ...) {
   va_list args;
   va_start(args, msg);
   (void)vfprintf(stderr, msg, args);
   va_end(args);
   (void)fputc('\n', stderr);
   if (read_pos) {
      (void)fprintf(stderr, "Last read position was %lu.\n", read_pos);
   }
   exit(EXIT_FAILURE);
}

static void ck_stdin(void) {
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

static int ck_getc(void) {
   int r;
   if ((r= getchar()) == EOF) {
      ck_stdin();
   } else {
      assert(r >= 0);
      ++read_pos;
   }
   return r;
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

/* Errors which should never happen under normal circumstances. */
static void internal_error(void) {
   die("Internal error!");
}

static void ck_ungetc(int c) {
   if (ungetc(c, stdin) != c) internal_error();
   --read_pos;
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

static char *dump_buf= 0;

static void cleanup() {
   if (dump_buf) free(dump_buf);
}

static int ignore_line_suffix(void) {
   int c, ignore_mode= 0;
   while ((c= ck_getc()) != '\n') {
      switch (c) {
         case EOF: return 0;
         case ASCII_DUMP_SEP: ignore_mode= 1; /* Fall through. */
         case GHOST_FACE: case DUMP_UNIT_SEP: break; /* Ignore it. */
         default: if (!ignore_mode) die("Input format syntax error!");
      }
   }
   return 1;
}

static int actual_main(int argc, char **argv) {
   int mode= 'w';
   unsigned units_per_line= 1;
   int ascii_dump, terminate_ws;
   ascii_dump= terminate_ws= 0;
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
            case 'W': case 'C': case 'X': case 'B':
            case 'w': case 'c': case 'x': case 'b':
            case 's':
               mode= c;
               break;
            case 'a': ascii_dump= 1; break;
            case 't': terminate_ws= 1; break;
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
                  if (units_per_line != optval || units_per_line < 1) {
                     goto invalid_argument;
                  }
               }
               goto next_arg;
            case 'h': {
                  unsigned i;
                  for (i= 0; i < DIM(help); ++i) appinfo(help[i], argv[0]);
               }
               /* Fall through. */
            case 'V': appinfo(version_info, argv[0]); goto done;
            default: bad_option: die("Unsupported option -%c!", c);
         }
         ++argpos;
      }
      end_of_options:
      if (optind < argc) {
         char const *fname= argv[optind++];
         char const *fmode= strchr("xb", mode) ? "rb" : "r";
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
      case 'x': case 'b':
         if (ascii_dump) {
            assert(units_per_line >= 1);
            if (
               !(
                  dump_buf= malloc(
                        mode == 'b'
                     ?  (
                           /* The maximum number of whole characters which
                            * could be produced by the bits left of the text
                            * dump area plus another byte for pre-caching the
                            * next byte to be processed, even if no more than
                            * the first bit of it has already been included
                            * in <dump_bits> so far. In other words,
                            * <dump_buf> needs to provide CHAR_BIT - 1 bits
                            * more space than <dump_bits> will ever tell. */
                           (units_per_line + CHAR_BIT - 1) / CHAR_BIT
                        )
                           /* Plus one character more, because it is possible
                            * that a partial byte from the line before became
                            * a whole byte which will also be dumped here. */
                        +  1
                     :  units_per_line
                  )
               )
            ) {
               die("Memory allocation error!");
            }
         }
         {
            int ghost;
            unsigned c_bits, dump_bits, unit;
            c_bits= dump_bits= unit= 0;
            for (ghost= 0;; ) {
               unsigned c;
               #ifndef NDEBUG
                  if (c_bits == 0) {
                     /* Ensure that any masking omission will not go
                      * undetected. This will also keep valgrind happy. */
                     c= ~0u;
                  }
               #endif
               if (!ghost) {
                  /* Not at EOF yet. */
                  if (c_bits < CHAR_BIT) {
                     int byte;
                     if ((byte= ck_getc()) == EOF) {
                        ghost= 1; /* Daddy, I can see DEAD BYTES! */
                        continue;
                     }
                     assert(CHAR_BIT * sizeof c >= CHAR_BIT + (CHAR_BIT - 1));
                     c= c << CHAR_BIT | byte;
                     c_bits+= CHAR_BIT;
                  }
               } else {
                  /* EOF has already been reached. Should we linger around? */
                  if (unit == 0 && c_bits == 0 && dump_bits < CHAR_BIT) {
                     /* We are done! Less than CHAR_BIT trailing bits after
                      * the last full byte will be ignored for mode "-b",
                      * because we cannot display a partial character. */
                     goto done;
                  }
               }
               if (unit) ck_putc(DUMP_UNIT_SEP);
               if (c_bits) {
                  switch (mode) {
                     case 'b':
                        ck_putc(c & 1 << c_bits - 1 ? '1' : '0');
                        if (ascii_dump) {
                           if (dump_bits % CHAR_BIT == 0) {
                              assert(c_bits >= CHAR_BIT);
                              dump_buf[dump_bits / CHAR_BIT]=
                                 (char)(c >> c_bits - CHAR_BIT)
                              ;
                           }
                           ++dump_bits;
                        }
                        --c_bits;
                        break;
                     default: assert(mode == 'x'); {
                        assert(c_bits == CHAR_BIT);
                        c&= (1 << CHAR_BIT) - 1;
                        ck_printf("%02X", c);
                        if (ascii_dump) {
                           dump_buf[unit]= (char)c;
                           dump_bits+= CHAR_BIT;
                        }
                        c_bits= 0;
                     }
                  }
               } else {
                  switch (mode) {
                     case 'x': ck_putc(GHOST_FACE); /* Fall through. */
                     default: assert(strchr("xb", mode)); ck_putc(GHOST_FACE);
                  }
               }
               assert(unit < units_per_line);
               if (++unit == units_per_line) {
                  if (ascii_dump) {
                     unsigned ndump;
                     if (ndump= dump_bits >> 3) {
                        unsigned i;
                        ck_putc(ASCII_DUMP_SEP);
                        for (i= 0; i < ndump; ++i) {
                           int c;
                           ck_putc(
                              (c= dump_buf[i]) >= 0x20 && c < 0x7f ? c : '.'
                           );
                        }
                        if (dump_bits & 8 - 1) {
                           /* There are left-over unprocessed bits. Move them
                            * to the beginning of the buffer. */
                           dump_buf[0]= dump_buf[ndump];
                        }
                        assert(dump_bits >= ndump << 3);
                        dump_bits-= ndump << 3;
                     }
                     assert(dump_bits < 8);
                  }
                  unit= 0;
                  ck_putc('\n');
               }
            }
         }
         break;
      case 'X':
         {
            auto unsigned int b;
            int n;
            errno= 0;
            for (;;) {
               if ((n= scanf("%2x", &b)) == 1) {
                  if (putchar((int)b) == EOF) stdout_error();
               } else {
                  if (n == EOF) {
                     ck_stdin();
                     if (feof(stdin)) break;
                  }
                  if (!ignore_line_suffix()) break;
               }
            }
         }
         goto done;
      case 'B': {
         do {
            int mask, byte, c;
            for (byte= 0, mask= 0x80; mask; mask>>= 1) {
               read_next:
               if ((c= ck_getc()) == EOF) {
                  stop:
                  if (mask == 0x80) goto done;
                  die("Incomplete binary octet (8 bit byte) at end of input!");
               }
               switch (c) {
                  case '0': c= 0; break;
                  case '1': c= mask; break;
                  default:
                     ck_ungetc(c); if (!ignore_line_suffix()) goto stop;
                     /* Fall through. */
                  case DUMP_UNIT_SEP: goto read_next;
               }
               byte|= c;
            }
            if (putchar(byte) == EOF) stdout_error();
         } while (!feof(stdin));
         goto done;
      }
   }
   {
      /* In -c and -w encodings, the following whitespace characters are
       * transformed into a sequence of 1 to 6 consecutive SPACE characters,
       * terminated by HT. The length of the sequence corresponds to the
       * 1-based character position in the string below. As a special
       * abbreviation rule, SPACE and HT in the transformed output which do
       * not match the above pattern represent themselves literally. */
      char const wse[]= {"\012\040\015\011\014\013"};
      int const lit_SPACE= '\040'; /* SPACE of explanation above. */
      int const lit_HT= '\011'; /* HT of explanation above. */
      unsigned const SPACE_enc= (int)(strchr(wse, lit_SPACE) + 1 - wse);
      #ifndef NDEBUG
      unsigned const HT_enc= (int)(strchr(wse, lit_HT) + 1 - wse);
      #endif
      size_t const mb_cur_max= MB_CUR_MAX;
      enum {
         st_initial, st_word, st_space, st_otherws, st_skip
      } state= st_initial;
      char c[MB_LEN_MAX];
      size_t nc0, nc= 0;
      int nnul, b, eof= 0;
      unsigned nsp;
      wchar_t wc;
      assert(SPACE_enc >= 1 && SPACE_enc <= sizeof wse - 1);
      assert(HT_enc >= 1 && HT_enc <= sizeof wse - 1);
      {
         /* Determine the length of the MBCS-encoding of L'\0'. */
         char nul[MB_LEN_MAX];
         if ((nnul= wctomb(nul, L'\0')) < 1) die("Unsupported locale!");
      }
      for (;;) {
         /* Read as much bytes into c[] as possible, but not more than the
          * longest possible MBCS-sequence. */
         while (!eof && nc < mb_cur_max) {
            if ((b= ck_getc()) == EOF) {
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
                * st_initial: Initial state. No byte read yet.
                * st_word: Not after a whitespace sequence.
                * st_space: After <nsp> lit_SPACE characters yet to be output.
                * st_otherws: After any other kind of whitespace character. */
               if (wc == (wchar_t)lit_SPACE) {
                  switch (state) {
                     case st_space:
                        assert(nsp + 1 > nsp); /* No overflow. */
                        ++nsp;
                        break;
                     default: nsp= 1; state= st_space;
                  }
               } else if (wc == (wchar_t)lit_HT) {
                  switch (state) {
                     default: assert(state == st_otherws); break;
                     case st_space:
                        /* HT following SPACEs. That's unfortunate. We have to
                         * encode all the SPACEs. But at least we can output
                         * the HT literally following them. */
                        assert(nsp >= 1);
                        do {
                           unsigned i;
                           for (i= SPACE_enc; i--; ) ck_putc(lit_SPACE);
                           ck_putc(lit_HT);
                        } while (--nsp);
                        /* Fall through. */
                     case st_initial: case st_word: state= st_otherws;
                  }
                  /* Output the literal HT which is not preceded by a SPACE
                   * (in the encoded output) and therefore needs no
                   * encoding. */
                  ck_putc(lit_HT);
                  assert(state == st_otherws);
               } else if (iswspace(wc)) {
                  /* Whitespace which cannot use an abbreviated literal form
                   * if it needs encoding. */
                  union {
                     unsigned enc;
                     char const *found;
                     ptrdiff_t offset;
                  } u;
                  if (
                     wc < (wchar_t)128 /* ASCII? */
                     && (u.found= strchr(wse, (char)(unsigned char)wc))
                  ) {
                     /* A whitespace character which needs to be encoded. */
                     u.offset= u.found - wse;
                     assert(u.offset >= 0 && (size_t)u.offset < sizeof wse);
                     u.enc= (unsigned)u.offset + 1;
                     assert(u.enc >= 1 && u.enc <= sizeof wse - 1);
                     switch (state) {
                        case st_space:
                           /* Whitespace which needs encoding following
                            * SPACEs. That's unfortunate. We have to encode
                            * all the SPACEs. */
                           assert(nsp >= 1);
                           do {
                              unsigned i;
                              for (i= SPACE_enc; i--; ) ck_putc(lit_SPACE);
                              ck_putc(lit_HT);
                           } while (--nsp);
                           /* Fall through. */
                        case st_initial: case st_word:
                           state= st_otherws;
                           /* Fall through. */
                        default: {
                           assert(state == st_otherws);
                           /* Encode <wc> itself. */
                           do ck_putc(lit_SPACE); while (--u.enc);
                           ck_putc(lit_HT);
                        }
                     }
                  } else {
                     /* Some other whitespace character which needs no
                      * encoding. */
                     switch (state) {
                        case st_space:
                           /* Whitespace which does not need encoding
                            * following SPACEs. That's fine. We can output the
                            * SPACEs literally. */
                           assert(nsp >= 1);
                           do ck_putc(lit_SPACE); while (--nsp);
                        case st_initial: case st_word:
                           state= st_otherws;
                           /* Fall through. */
                        default: {
                           assert(state == st_otherws);
                           ck_write(c, nc0); /* Output <wc> literally. */
                        }
                     }
                  }
                  assert(state == st_otherws);
               } else {
                  /* <wc> is a 'word' character. */
                  switch (state) {
                     case st_word:
                        if (mode == 'c') goto terminate;
                        break;
                     case st_space:
                        /* 'word'-character following SPACEs. That's fine. We
                         * can output the SPACEs literally. */
                        assert(nsp >= 1);
                        do ck_putc(lit_SPACE); while (--nsp);
                     case st_otherws: terminate:
                        if (terminate_ws) {
                           switch (state) {
                              default:
                                 assert( \
                                    state == st_word || state == st_initial \
                                 );
                                 break;
                              case st_space: case st_otherws: {
                                 ck_putc(WS_OPT_TERMINATOR);
                              }
                           }
                        }
                        ck_putc('\n');
                        /* Fall through. */
                     default: state= st_word;
                  }
                  ck_write(c, nc0); /* Output <wc> literally. */
               }
               break;
            case 's':
               /* States:
                * st_initial: At the beginning of a line or within a word.
                * st_space: After whitespace (other than newline).
                * st_skip: Ignore rest of input line.  */
               if (wc == L'\n') state= st_initial;
               if (state != st_skip) {
                  if (wc == (wchar_t)WS_OPT_TERMINATOR && state == st_space) {
                     state= st_skip;
                     break;
                  }
                  ck_write(c, nc0);
                  state= iswspace(wc) && wc != L'\n' ? st_space : st_initial;
               }
               break;
            default: {
               assert(mode == 'W' || mode == 'C');
               /* States:
                * st_initial: At the beginning of input or within a word.
                * st_space: After <nsp> <lit_SPACE>s read but unprocessed.
                * st_otherws: After whitespace but not in mode st_space.
                * st_skip: Ignore rest of input line.  */
               if (state == st_skip) {
                  if (wc == L'\n') state= st_initial;
                  break;
               }
               if (wc == (wchar_t)lit_SPACE) {
                  if (state != st_space) {
                     assert(state == st_initial || state == st_otherws);
                     nsp= 1;
                     state= st_space;
                  } else {
                     assert(state == st_space);
                     if (nsp == sizeof wse - 1) {
                        /* Our SPACE-counted encoding sequence cannot be
                         * longer than this. Therefore we emit the first of
                         * the spaces, because it cannot be part of an
                         * encoding sequence any more. */
                        ck_putc(lit_SPACE);
                     } else {
                        assert(nsp < sizeof wse - 1);
                        ++nsp;
                     }
                  }
                  assert(state == st_space);
               } else {
                  /* Some other character than a SPACE. */
                  if (state == st_space) {
                     if (wc == (wchar_t)lit_HT) {
                        /* It is an encoded whitespace character. Decode it. */
                        assert(nsp >= 1 && nsp <= sizeof wse - 1);
                        ck_putc(wse[nsp - 1]);
                        state= st_otherws;
                        break;
                     }
                     /* It is some literal character. Emit the delayed spaces
                      * before checking the character any further. */
                     assert(nsp >= 1 && nsp <= sizeof wse - 1);
                     do ck_putc(lit_SPACE); while (--nsp);
                     state= st_otherws;
                  }
                  assert(state == st_initial || state == st_otherws);
                  if (wc == L'\n') {
                     state= st_initial;
                     break;
                  }
                  if (
                     wc == (wchar_t)WS_OPT_TERMINATOR && state != st_initial
                  ) {
                     assert(state == st_otherws);
                     state= st_skip;
                     break;
                  }
                  ck_write(c, nc0);
                  state= iswspace(wc) ? st_otherws : st_initial;
               }
            }
         }
         assert(nc0 <= nc);
         if (nc0 < nc) (void)memmove(c, c + nc0, nc - nc0);
         nc-= nc0;
      }
      switch (mode) {
         case 'w': case 'c': {
            if (state == st_space) {
               /* EOF following SPACEs. That's fine. We can output the SPACEs
                * literally. */
               assert(nsp >= 1);
               do ck_putc(lit_SPACE); while (--nsp);
            }
            if (terminate_ws && (state == st_otherws || state == st_space)) {
               ck_putc(WS_OPT_TERMINATOR);
            }
            ck_putc('\n'); /* Terminate the last output line. */
         }
      }
   }
   done:
   if (fflush(0)) die("Error writing to output stream!");
   return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
   /* This function is separate from actual_main() because the mcheck...() and
    * mallopt() functions need to be called before any other library
    * functions, including functions called in initializer expressions. */
   #ifdef MALLOC_TRACE
       assert(mcheck_pedantic(0) == 0);
       (void)mallopt(M_PERTURB, (int)0xaaaaaaaa);
       mcheck_check_all();
       mtrace();
   #endif
   #if !CONFIG_NO_LOCALE
      (void)setlocale(LC_ALL, "");
   #endif
   atexit(cleanup);
   return actual_main(argc, argv);
}
