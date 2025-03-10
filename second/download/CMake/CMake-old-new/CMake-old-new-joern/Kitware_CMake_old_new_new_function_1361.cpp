/* A Bison parser, made by GNU Bison 1.875a.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     tAGO = 258,
     tDAY = 259,
     tDAY_UNIT = 260,
     tDAYZONE = 261,
     tDST = 262,
     tHOUR_UNIT = 263,
     tID = 264,
     tMERIDIAN = 265,
     tMINUTE_UNIT = 266,
     tMONTH = 267,
     tMONTH_UNIT = 268,
     tSEC_UNIT = 269,
     tSNUMBER = 270,
     tUNUMBER = 271,
     tYEAR_UNIT = 272,
     tZONE = 273
   };
#endif
#define tAGO 258
#define tDAY 259
#define tDAY_UNIT 260
#define tDAYZONE 261
#define tDST 262
#define tHOUR_UNIT 263
#define tID 264
#define tMERIDIAN 265
#define tMINUTE_UNIT 266
#define tMONTH 267
#define tMONTH_UNIT 268
#define tSEC_UNIT 269
#define tSNUMBER 270
#define tUNUMBER 271
#define tYEAR_UNIT 272
#define tZONE 273




/* Copy the first part of user declarations.  */
#line 1 "getdate.y"

/*
**  Originally written by Steven M. Bellovin <smb@research.att.com> while
**  at the University of North Carolina at Chapel Hill.  Later tweaked by
**  a couple of people on Usenet.  Completely overhauled by Rich $alz
**  <rsalz@bbn.com> and Jim Berets <jberets@bbn.com> in August, 1990.
**
**  This code has been modified since it was included in curl, to make it
**  thread-safe and to make compilers complain less about it.
**
**  This code is in the public domain and has no copyright.
*/

#include "setup.h"

# ifdef HAVE_ALLOCA_H
#  include <alloca.h>
# endif

# ifdef HAVE_TIME_H
#  include <time.h>
# endif

#ifndef YYDEBUG
  /* to satisfy gcc -Wundef, we set this to 0 */
#define YYDEBUG 0
#endif

#ifndef YYSTACK_USE_ALLOCA
  /* to satisfy gcc -Wundef, we set this to 0 */
#define YYSTACK_USE_ALLOCA 0
#endif

/* Since the code of getdate.y is not included in the Emacs executable
   itself, there is no need to #define static in this file.  Even if
   the code were included in the Emacs executable, it probably
   wouldn't do any harm to #undef it here; this will only cause
   problems if we try to write to a static variable, which I don't
   think this code needs to do.  */
#ifdef emacs
# undef static
#endif

#ifdef __APPLE__
#include <sys/types.h>
#include <sys/malloc.h>
#else

#endif
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#if HAVE_STDLIB_H
# include <stdlib.h> /* for `free'; used by Bison 1.27 */
#else

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#endif

#if defined (STDC_HEADERS) || (!defined (isascii) && !defined (HAVE_ISASCII))
# define IN_CTYPE_DOMAIN(c) 1
#else
# define IN_CTYPE_DOMAIN(c) isascii(c)
#endif

#define ISSPACE(c) (IN_CTYPE_DOMAIN (c) && isspace (c))
#define ISALPHA(c) (IN_CTYPE_DOMAIN (c) && isalpha (c))
#define ISUPPER(c) (IN_CTYPE_DOMAIN (c) && isupper (c))
#define ISDIGIT_LOCALE(c) (IN_CTYPE_DOMAIN (c) && isdigit (c))

/* ISDIGIT differs from ISDIGIT_LOCALE, as follows:
   - Its arg may be any int or unsigned int; it need not be an unsigned char.
   - It's guaranteed to evaluate its argument exactly once.
   - It's typically faster.
   Posix 1003.2-1992 section 2.5.2.1 page 50 lines 1556-1558 says that
   only '0' through '9' are digits.  Prefer ISDIGIT to ISDIGIT_LOCALE unless
   it's important to use the locale's definition of `digit' even when the
   host does not conform to Posix.  */
#define ISDIGIT(c) ((unsigned) (c) - '0' <= 9)

#if defined (STDC_HEADERS) || defined (USG)
# include <string.h>
#endif

#include "memory.h"
/* The last #include file should be: */
#include "memdebug.h"

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 0
#endif

#if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 7)
# define __attribute__(x)
#endif

#ifndef ATTRIBUTE_UNUSED
# define ATTRIBUTE_UNUSED __attribute__ ((__unused__))
#endif

/* Some old versions of bison generate parsers that use bcopy.
   That loses on systems that don't provide the function, so we have
   to redefine it here.  */
#if !defined (HAVE_BCOPY) && defined (HAVE_MEMCPY) && !defined (bcopy)
# define bcopy(from, to, len) memcpy ((to), (from), (len))
#endif

/* Remap normal yacc parser interface names (yyparse, yylex, yyerror, etc),
   as well as gratuitiously global symbol names, so we can have multiple
   yacc generated parsers in the same program.  Note that these are only
   the variables produced by yacc.  If other parser generators (bison,
   byacc, etc) produce additional global names that conflict at link time,
   then those parser generators need to be fixed instead of adding those
   names to this list. */

#define yymaxdepth Curl_gd_maxdepth
#define yyparse Curl_gd_parse
#define yylex   Curl_gd_lex
#define yyerror Curl_gd_error
#define yylval  Curl_gd_lval
#define yychar  Curl_gd_char
#define yydebug Curl_gd_debug
#define yypact  Curl_gd_pact
#define yyr1    Curl_gd_r1
#define yyr2    Curl_gd_r2
#define yydef   Curl_gd_def
#define yychk   Curl_gd_chk
#define yypgo   Curl_gd_pgo
#define yyact   Curl_gd_act
#define yyexca  Curl_gd_exca
#define yyerrflag Curl_gd_errflag
#define yynerrs Curl_gd_nerrs
#define yyps    Curl_gd_ps
#define yypv    Curl_gd_pv
#define yys     Curl_gd_s
#define yy_yys  Curl_gd_yys
#define yystate Curl_gd_state
#define yytmp   Curl_gd_tmp
#define yyv     Curl_gd_v
#define yy_yyv  Curl_gd_yyv
#define yyval   Curl_gd_val
#define yylloc  Curl_gd_lloc
#define yyreds  Curl_gd_reds          /* With YYDEBUG defined */
#define yytoks  Curl_gd_toks          /* With YYDEBUG defined */
#define yylhs   Curl_gd_yylhs
#define yylen   Curl_gd_yylen
#define yydefred Curl_gd_yydefred
#define yydgoto Curl_gd_yydgoto
#define yysindex Curl_gd_yysindex
#define yyrindex Curl_gd_yyrindex
#define yygindex Curl_gd_yygindex
#define yytable  Curl_gd_yytable
#define yycheck  Curl_gd_yycheck

#define EPOCH           1970
#define HOUR(x)         ((x) * 60)

#define MAX_BUFF_LEN    128   /* size of buffer to read the date into */

/*
**  An entry in the lexical lookup table.
*/
typedef struct _TABLE {
    const char  *name;
    int         type;
    int         value;
} TABLE;


/*
**  Meridian:  am, pm, or 24-hour style.
*/
typedef enum _MERIDIAN {
    MERam, MERpm, MER24
} MERIDIAN;

/* parse results and input string */
typedef struct _CURL_CONTEXT {
    const char  *yyInput;
    int         yyDayOrdinal;
    int         yyDayNumber;
    int         yyHaveDate;
    int         yyHaveDay;
    int         yyHaveRel;
    int         yyHaveTime;
    int         yyHaveZone;
    int         yyTimezone;
    int         yyDay;
    int         yyHour;
    int         yyMinutes;
    int         yyMonth;
    int         yySeconds;
    int         yyYear;
    MERIDIAN    yyMeridian;
    int         yyRelDay;
    int         yyRelHour;
    int         yyRelMinutes;
    int         yyRelMonth;
    int         yyRelSeconds;
    int         yyRelYear;
} CURL_CONTEXT;

/* enable use of extra argument to yyparse and yylex which can be used to pass
**  in a user defined value (CURL_CONTEXT struct in our case)
*/
#define YYPARSE_PARAM cookie
#define YYLEX_PARAM cookie
#define context ((CURL_CONTEXT *) cookie)


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 223 "getdate.y"
typedef union YYSTYPE {
    int                 Number;
    enum _MERIDIAN      Meridian;
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 331 "y.tab.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */
#line 228 "getdate.y"

static int yylex (YYSTYPE *yylval, void *cookie);
static int yyerror (const char *s);


/* Line 214 of yacc.c.  */
#line 347 "y.tab.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
         || (YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))                         \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)              \
      do                                        \
        {                                       \
          register YYSIZE_T yyi;                \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (To)[yyi] = (From)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)                                        \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack, Stack, yysize);                          \
        Stack = &yyptr->Stack;                                          \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   50

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  22
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  11
/* YYNRULES -- Number of rules. */
#define YYNRULES  51
/* YYNRULES -- Number of states. */
#define YYNSTATES  61

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   273

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    20,     2,     2,    21,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    19,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned char yyprhs[] =
{
       0,     0,     3,     4,     7,     9,    11,    13,    15,    17,
      19,    22,    27,    32,    39,    46,    48,    50,    53,    55,
      58,    61,    65,    71,    75,    79,    82,    87,    90,    94,
      97,    99,   102,   105,   107,   110,   113,   115,   118,   121,
     123,   126,   129,   131,   134,   137,   139,   142,   145,   147,
     149,   150
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      23,     0,    -1,    -1,    23,    24,    -1,    25,    -1,    26,
      -1,    28,    -1,    27,    -1,    29,    -1,    31,    -1,    16,
      10,    -1,    16,    19,    16,    32,    -1,    16,    19,    16,
      15,    -1,    16,    19,    16,    19,    16,    32,    -1,    16,
      19,    16,    19,    16,    15,    -1,    18,    -1,     6,    -1,
      18,     7,    -1,     4,    -1,     4,    20,    -1,    16,     4,
      -1,    16,    21,    16,    -1,    16,    21,    16,    21,    16,
      -1,    16,    15,    15,    -1,    16,    12,    15,    -1,    12,
      16,    -1,    12,    16,    20,    16,    -1,    16,    12,    -1,
      16,    12,    16,    -1,    30,     3,    -1,    30,    -1,    16,
      17,    -1,    15,    17,    -1,    17,    -1,    16,    13,    -1,
      15,    13,    -1,    13,    -1,    16,     5,    -1,    15,     5,
      -1,     5,    -1,    16,     8,    -1,    15,     8,    -1,     8,
      -1,    16,    11,    -1,    15,    11,    -1,    11,    -1,    16,
      14,    -1,    15,    14,    -1,    14,    -1,    16,    -1,    -1,
      10,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   244,   244,   245,   248,   251,   254,   257,   260,   263,
     266,   272,   278,   287,   293,   305,   308,   312,   317,   321,
     325,   331,   335,   353,   359,   365,   369,   374,   378,   385,
     393,   396,   399,   402,   405,   408,   411,   414,   417,   420,
     423,   426,   429,   432,   435,   438,   441,   444,   447,   452,
     487,   490
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "tAGO", "tDAY", "tDAY_UNIT", "tDAYZONE", 
  "tDST", "tHOUR_UNIT", "tID", "tMERIDIAN", "tMINUTE_UNIT", "tMONTH", 
  "tMONTH_UNIT", "tSEC_UNIT", "tSNUMBER", "tUNUMBER", "tYEAR_UNIT", 
  "tZONE", "':'", "','", "'/'", "$accept", "spec", "item", "time", "zone", 
  "day", "date", "rel", "relunit", "number", "o_merid", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,    58,
      44,    47
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    22,    23,    23,    24,    24,    24,    24,    24,    24,
      25,    25,    25,    25,    25,    26,    26,    26,    27,    27,
      27,    28,    28,    28,    28,    28,    28,    28,    28,    29,
      29,    30,    30,    30,    30,    30,    30,    30,    30,    30,
      30,    30,    30,    30,    30,    30,    30,    30,    30,    31,
      32,    32
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       2,     4,     4,     6,     6,     1,     1,     2,     1,     2,
       2,     3,     5,     3,     3,     2,     4,     2,     3,     2,
       1,     2,     2,     1,     2,     2,     1,     2,     2,     1,
       2,     2,     1,     2,     2,     1,     2,     2,     1,     1,
       0,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       2,     0,     1,    18,    39,    16,    42,    45,     0,    36,
      48,     0,    49,    33,    15,     3,     4,     5,     7,     6,
       8,    30,     9,    19,    25,    38,    41,    44,    35,    47,
      32,    20,    37,    40,    10,    43,    27,    34,    46,     0,
      31,     0,     0,    17,    29,     0,    24,    28,    23,    50,
      21,    26,    51,    12,     0,    11,     0,    50,    22,    14,
      13
};

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,     1,    15,    16,    17,    18,    19,    20,    21,    22,
      55
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -20
static const yysigned_char yypact[] =
{
     -20,     0,   -20,   -19,   -20,   -20,   -20,   -20,   -13,   -20,
     -20,    30,    15,   -20,    14,   -20,   -20,   -20,   -20,   -20,
     -20,    19,   -20,   -20,     4,   -20,   -20,   -20,   -20,   -20,
     -20,   -20,   -20,   -20,   -20,   -20,    -6,   -20,   -20,    16,
     -20,    17,    23,   -20,   -20,    24,   -20,   -20,   -20,    27,
      28,   -20,   -20,   -20,    29,   -20,    32,    -8,   -20,   -20,
     -20
};

/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
     -20,   -20,   -20,   -20,   -20,   -20,   -20,   -20,   -20,   -20,
      -7
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
{
       2,    23,    52,    24,     3,     4,     5,    59,     6,    46,
      47,     7,     8,     9,    10,    11,    12,    13,    14,    31,
      32,    43,    44,    33,    45,    34,    35,    36,    37,    38,
      39,    48,    40,    49,    41,    25,    42,    52,    26,    50,
      51,    27,    53,    28,    29,    57,    54,    30,    58,    56,
      60
};

static const unsigned char yycheck[] =
{
       0,    20,    10,    16,     4,     5,     6,    15,     8,    15,
      16,    11,    12,    13,    14,    15,    16,    17,    18,     4,
       5,     7,     3,     8,    20,    10,    11,    12,    13,    14,
      15,    15,    17,    16,    19,     5,    21,    10,     8,    16,
      16,    11,    15,    13,    14,    16,    19,    17,    16,    21,
      57
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    23,     0,     4,     5,     6,     8,    11,    12,    13,
      14,    15,    16,    17,    18,    24,    25,    26,    27,    28,
      29,    30,    31,    20,    16,     5,     8,    11,    13,    14,
      17,     4,     5,     8,    10,    11,    12,    13,    14,    15,
      17,    19,    21,     7,     3,    20,    15,    16,    15,    16,
      16,    16,    10,    15,    19,    32,    21,    16,    16,    15,
      32
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrlab1


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL          goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY && yylen == 1)                          \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      yytoken = YYTRANSLATE (yychar);                           \
      YYPOPSTACK;                                               \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror ("syntax error: cannot back up");\
      YYERROR;                                                  \
    }                                                           \
while (0)

#define YYTERROR        1
#define YYERRCODE       256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

# define YYDSYMPRINT(Args)                      \
do {                                            \
  if (yydebug)                                  \
    yysymprint Args;                            \
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)            \
do {                                                            \
  if (yydebug)                                                  \
    {                                                           \
      YYFPRINTF (stderr, "%s ", Title);                         \
      yysymprint (stderr,                                       \
                  Token, Value);        \
      YYFPRINTF (stderr, "\n");                                 \
    }                                                           \
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (cinluded).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylineno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylineno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (Rule);             \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  /* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;             /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack. Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        short *yyss1 = yyss;


        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow ("parser stack overflow",
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),

                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        short *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyoverflowlab;
        YYSTACK_RELOCATE (yyss);
        YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 4:
#line 248 "getdate.y"
    {
            context->yyHaveTime++;
        }
    break;

  case 5:
#line 251 "getdate.y"
    {
            context->yyHaveZone++;
        }
    break;

  case 6:
#line 254 "getdate.y"
    {
            context->yyHaveDate++;
        }
    break;

  case 7:
#line 257 "getdate.y"
    {
            context->yyHaveDay++;
        }
    break;

  case 8:
#line 260 "getdate.y"
    {
            context->yyHaveRel++;
        }
    break;

  case 10:
#line 266 "getdate.y"
    {
            context->yyHour = yyvsp[-1].Number;
            context->yyMinutes = 0;
            context->yySeconds = 0;
            context->yyMeridian = yyvsp[0].Meridian;
        }
    break;

  case 11:
#line 272 "getdate.y"
    {
            context->yyHour = yyvsp[-3].Number;
            context->yyMinutes = yyvsp[-1].Number;
            context->yySeconds = 0;
            context->yyMeridian = yyvsp[0].Meridian;
        }
    break;

  case 12:
#line 278 "getdate.y"
    {
            context->yyHour = yyvsp[-3].Number;
            context->yyMinutes = yyvsp[-1].Number;
            context->yyMeridian = MER24;
            context->yyHaveZone++;
            context->yyTimezone = (yyvsp[0].Number < 0
                                   ? -yyvsp[0].Number % 100 + (-yyvsp[0].Number / 100) * 60
                                   : - (yyvsp[0].Number % 100 + (yyvsp[0].Number / 100) * 60));
        }
    break;

  case 13:
#line 287 "getdate.y"
    {
            context->yyHour = yyvsp[-5].Number;
            context->yyMinutes = yyvsp[-3].Number;
            context->yySeconds = yyvsp[-1].Number;
            context->yyMeridian = yyvsp[0].Meridian;
        }
    break;

  case 14:
#line 293 "getdate.y"
    {
            context->yyHour = yyvsp[-5].Number;
            context->yyMinutes = yyvsp[-3].Number;
            context->yySeconds = yyvsp[-1].Number;
            context->yyMeridian = MER24;
            context->yyHaveZone++;
            context->yyTimezone = (yyvsp[0].Number < 0
                                   ? -yyvsp[0].Number % 100 + (-yyvsp[0].Number / 100) * 60
                                   : - (yyvsp[0].Number % 100 + (yyvsp[0].Number / 100) * 60));
        }
    break;

  case 15:
#line 305 "getdate.y"
    {
            context->yyTimezone = yyvsp[0].Number;
        }
    break;

  case 16:
#line 308 "getdate.y"
    {
            context->yyTimezone = yyvsp[0].Number - 60;
        }
    break;

  case 17:
#line 312 "getdate.y"
    {
            context->yyTimezone = yyvsp[-1].Number - 60;
        }
    break;

  case 18:
#line 317 "getdate.y"
    {
            context->yyDayOrdinal = 1;
            context->yyDayNumber = yyvsp[0].Number;
        }
    break;

  case 19:
#line 321 "getdate.y"
    {
            context->yyDayOrdinal = 1;
            context->yyDayNumber = yyvsp[-1].Number;
        }
    break;

  case 20:
#line 325 "getdate.y"
    {
            context->yyDayOrdinal = yyvsp[-1].Number;
            context->yyDayNumber = yyvsp[0].Number;
        }
    break;

  case 21:
#line 331 "getdate.y"
    {
            context->yyMonth = yyvsp[-2].Number;
            context->yyDay = yyvsp[0].Number;
        }
    break;

  case 22:
#line 335 "getdate.y"
    {
          /* Interpret as YYYY/MM/DD if $1 >= 1000, otherwise as MM/DD/YY.
             The goal in recognizing YYYY/MM/DD is solely to support legacy
             machine-generated dates like those in an RCS log listing.  If
             you want portability, use the ISO 8601 format.  */
          if (yyvsp[-4].Number >= 1000)
            {
              context->yyYear = yyvsp[-4].Number;
              context->yyMonth = yyvsp[-2].Number;
              context->yyDay = yyvsp[0].Number;
            }
          else
            {
              context->yyMonth = yyvsp[-4].Number;
              context->yyDay = yyvsp[-2].Number;
              context->yyYear = yyvsp[0].Number;
            }
        }
    break;

  case 23:
#line 353 "getdate.y"
    {
            /* ISO 8601 format.  yyyy-mm-dd.  */
            context->yyYear = yyvsp[-2].Number;
            context->yyMonth = -yyvsp[-1].Number;
            context->yyDay = -yyvsp[0].Number;
        }
    break;

  case 24:
#line 359 "getdate.y"
    {
            /* e.g. 17-JUN-1992.  */
            context->yyDay = yyvsp[-2].Number;
            context->yyMonth = yyvsp[-1].Number;
            context->yyYear = -yyvsp[0].Number;
        }
    break;

  case 25:
#line 365 "getdate.y"
    {
            context->yyMonth = yyvsp[-1].Number;
            context->yyDay = yyvsp[0].Number;
        }
    break;

  case 26:
#line 369 "getdate.y"
    {
            context->yyMonth = yyvsp[-3].Number;
            context->yyDay = yyvsp[-2].Number;
            context->yyYear = yyvsp[0].Number;
        }
    break;

  case 27:
#line 374 "getdate.y"
    {
            context->yyMonth = yyvsp[0].Number;
            context->yyDay = yyvsp[-1].Number;
        }
    break;

  case 28:
#line 378 "getdate.y"
    {
            context->yyMonth = yyvsp[-1].Number;
            context->yyDay = yyvsp[-2].Number;
            context->yyYear = yyvsp[0].Number;
        }
    break;

  case 29:
#line 385 "getdate.y"
    {
            context->yyRelSeconds = -context->yyRelSeconds;
            context->yyRelMinutes = -context->yyRelMinutes;
            context->yyRelHour = -context->yyRelHour;
            context->yyRelDay = -context->yyRelDay;
            context->yyRelMonth = -context->yyRelMonth;
            context->yyRelYear = -context->yyRelYear;
        }
    break;

  case 31:
#line 396 "getdate.y"
    {
            context->yyRelYear += yyvsp[-1].Number * yyvsp[0].Number;
        }
    break;

  case 32:
#line 399 "getdate.y"
    {
            context->yyRelYear += yyvsp[-1].Number * yyvsp[0].Number;
        }
    break;

  case 33:
#line 402 "getdate.y"
    {
            context->yyRelYear += yyvsp[0].Number;
        }
    break;

  case 34:
#line 405 "getdate.y"
    {
            context->yyRelMonth += yyvsp[-1].Number * yyvsp[0].Number;
        }
    break;

  case 35:
#line 408 "getdate.y"
    {
            context->yyRelMonth += yyvsp[-1].Number * yyvsp[0].Number;
        }
    break;

  case 36:
#line 411 "getdate.y"
    {
            context->yyRelMonth += yyvsp[0].Number;
        }
    break;

  case 37:
#line 414 "getdate.y"
    {
            context->yyRelDay += yyvsp[-1].Number * yyvsp[0].Number;
        }
    break;

  case 38:
#line 417 "getdate.y"
    {
            context->yyRelDay += yyvsp[-1].Number * yyvsp[0].Number;
        }
    break;

  case 39:
#line 420 "getdate.y"
    {
            context->yyRelDay += yyvsp[0].Number;
        }
    break;

  case 40:
#line 423 "getdate.y"
    {
            context->yyRelHour += yyvsp[-1].Number * yyvsp[0].Number;
        }
    break;

  case 41:
#line 426 "getdate.y"
    {
            context->yyRelHour += yyvsp[-1].Number * yyvsp[0].Number;
        }
    break;

  case 42:
#line 429 "getdate.y"
    {
            context->yyRelHour += yyvsp[0].Number;
        }
    break;

  case 43:
#line 432 "getdate.y"
    {
            context->yyRelMinutes += yyvsp[-1].Number * yyvsp[0].Number;
        }
    break;

  case 44:
#line 435 "getdate.y"
    {
            context->yyRelMinutes += yyvsp[-1].Number * yyvsp[0].Number;
        }
    break;

  case 45:
#line 438 "getdate.y"
    {
            context->yyRelMinutes += yyvsp[0].Number;
        }
    break;

  case 46:
#line 441 "getdate.y"
    {
            context->yyRelSeconds += yyvsp[-1].Number * yyvsp[0].Number;
        }
    break;

  case 47:
#line 444 "getdate.y"
    {
            context->yyRelSeconds += yyvsp[-1].Number * yyvsp[0].Number;
        }
    break;

  case 48:
#line 447 "getdate.y"
    {
            context->yyRelSeconds += yyvsp[0].Number;
        }
    break;

  case 49:
#line 453 "getdate.y"
    {
            if (context->yyHaveTime && context->yyHaveDate &&
                !context->yyHaveRel)
              context->yyYear = yyvsp[0].Number;
            else
              {
                if (yyvsp[0].Number>10000)
                  {
                    context->yyHaveDate++;
                    context->yyDay= (yyvsp[0].Number)%100;
                    context->yyMonth= (yyvsp[0].Number/100)%100;
                    context->yyYear = yyvsp[0].Number/10000;
                  }
                else
                  {
                    context->yyHaveTime++;
                    if (yyvsp[0].Number < 100)
                      {
                        context->yyHour = yyvsp[0].Number;
                        context->yyMinutes = 0;
                      }
                    else
                      {
                        context->yyHour = yyvsp[0].Number / 100;
                        context->yyMinutes = yyvsp[0].Number % 100;
                      }
                    context->yySeconds = 0;
                    context->yyMeridian = MER24;
                  }
              }
          }
    break;

  case 50:
#line 487 "getdate.y"
    {
            yyval.Meridian = MER24;
          }
    break;

  case 51:
#line 491 "getdate.y"
    {
            yyval.Meridian = yyvsp[0].Meridian;
          }
    break;


    }

/* Line 999 of yacc.c.  */
#line 1688 "y.tab.c"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
        {
          YYSIZE_T yysize = 0;
          int yytype = YYTRANSLATE (yychar);
          char *yymsg;
          int yyx, yycount;

          yycount = 0;
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  */
          for (yyx = yyn < 0 ? -yyn : 0;
               yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
              yysize += yystrlen (yytname[yyx]) + 15, yycount++;
          yysize += yystrlen ("syntax error, unexpected ") + 1;
          yysize += yystrlen (yytname[yytype]);
          yymsg = (char *) YYSTACK_ALLOC (yysize);
          if (yymsg != 0)
            {
              char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
              yyp = yystpcpy (yyp, yytname[yytype]);

              if (yycount < 5)
                {
                  yycount = 0;
                  for (yyx = yyn < 0 ? -yyn : 0;
                       yyx < (int) (sizeof (yytname) / sizeof (char *));
                       yyx++)
                    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
                      {
                        const char *yyq = ! yycount ? ", expecting " : " or ";
                        yyp = yystpcpy (yyp, yyq);
                        yyp = yystpcpy (yyp, yytname[yyx]);
                        yycount++;
                      }
                }
              yyerror (yymsg);
              YYSTACK_FREE (yymsg);
            }
          else
            yyerror ("syntax error; also virtual memory exhausted");
        }
      else
#endif /* YYERROR_VERBOSE */
        yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
          /* Pop the error token.  */
          YYPOPSTACK;
          /* Pop the rest of the stack.  */
          while (yyss < yyssp)
            {
              YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
              yydestruct (yystos[*yyssp], yyvsp);
              YYPOPSTACK;
            }
          YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      yyvsp--;
      yystate = *--yyssp;

      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 496 "getdate.y"


/* Include this file down here because bison inserts code above which
   may define-away `const'.  We want the prototype for get_date to have
   the same signature as the function definition does. */
#include "getdate.h"

#ifndef WIN32 /* the windows dudes don't need these, does anyone really? */
extern struct tm        *gmtime (const time_t *);
extern struct tm        *localtime (const time_t *);
extern time_t           mktime (struct tm *);
#endif

/* Month and day table. */
static TABLE const MonthDayTable[] = {
    { "january",        tMONTH,  1 },
    { "february",       tMONTH,  2 },
    { "march",          tMONTH,  3 },
    { "april",          tMONTH,  4 },
    { "may",            tMONTH,  5 },
    { "june",           tMONTH,  6 },
    { "july",           tMONTH,  7 },
    { "august",         tMONTH,  8 },
    { "september",      tMONTH,  9 },
    { "sept",           tMONTH,  9 },
    { "october",        tMONTH, 10 },
    { "november",       tMONTH, 11 },
    { "december",       tMONTH, 12 },
    { "sunday",         tDAY, 0 },
    { "monday",         tDAY, 1 },
    { "tuesday",        tDAY, 2 },
    { "tues",           tDAY, 2 },
    { "wednesday",      tDAY, 3 },
    { "wednes",         tDAY, 3 },
    { "thursday",       tDAY, 4 },
    { "thur",           tDAY, 4 },
    { "thurs",          tDAY, 4 },
    { "friday",         tDAY, 5 },
    { "saturday",       tDAY, 6 },
    { NULL, 0, 0 }
};

/* Time units table. */
static TABLE const UnitsTable[] = {
    { "year",           tYEAR_UNIT,     1 },
    { "month",          tMONTH_UNIT,    1 },
    { "fortnight",      tDAY_UNIT,      14 },
    { "week",           tDAY_UNIT,      7 },
    { "day",            tDAY_UNIT,      1 },
    { "hour",           tHOUR_UNIT,     1 },
    { "minute",         tMINUTE_UNIT,   1 },
    { "min",            tMINUTE_UNIT,   1 },
    { "second",         tSEC_UNIT,      1 },
    { "sec",            tSEC_UNIT,      1 },
    { NULL, 0, 0 }
};

/* Assorted relative-time words. */
static TABLE const OtherTable[] = {
    { "tomorrow",       tMINUTE_UNIT,   1 * 24 * 60 },
    { "yesterday",      tMINUTE_UNIT,   -1 * 24 * 60 },
    { "today",          tMINUTE_UNIT,   0 },
    { "now",            tMINUTE_UNIT,   0 },
    { "last",           tUNUMBER,       -1 },
    { "this",           tMINUTE_UNIT,   0 },
    { "next",           tUNUMBER,       1 },
    { "first",          tUNUMBER,       1 },
/*  { "second",         tUNUMBER,       2 }, */
    { "third",          tUNUMBER,       3 },
    { "fourth",         tUNUMBER,       4 },
    { "fifth",          tUNUMBER,       5 },
    { "sixth",          tUNUMBER,       6 },
    { "seventh",        tUNUMBER,       7 },
    { "eighth",         tUNUMBER,       8 },
    { "ninth",          tUNUMBER,       9 },
    { "tenth",          tUNUMBER,       10 },
    { "eleventh",       tUNUMBER,       11 },
    { "twelfth",        tUNUMBER,       12 },
    { "ago",            tAGO,   1 },
    { NULL, 0, 0 }
};

/* The timezone table. */
static TABLE const TimezoneTable[] = {
    { "gmt",    tZONE,     HOUR ( 0) }, /* Greenwich Mean */
    { "ut",     tZONE,     HOUR ( 0) }, /* Universal (Coordinated) */
    { "utc",    tZONE,     HOUR ( 0) },
    { "wet",    tZONE,     HOUR ( 0) }, /* Western European */
    { "bst",    tDAYZONE,  HOUR ( 0) }, /* British Summer */
    { "wat",    tZONE,     HOUR ( 1) }, /* West Africa */
    { "at",     tZONE,     HOUR ( 2) }, /* Azores */
#if     0
    /* For completeness.  BST is also British Summer, and GST is
     * also Guam Standard. */
    { "bst",    tZONE,     HOUR ( 3) }, /* Brazil Standard */
    { "gst",    tZONE,     HOUR ( 3) }, /* Greenland Standard */
#endif
#if 0
    { "nft",    tZONE,     HOUR (3.5) },        /* Newfoundland */
    { "nst",    tZONE,     HOUR (3.5) },        /* Newfoundland Standard */
    { "ndt",    tDAYZONE,  HOUR (3.5) },        /* Newfoundland Daylight */
#endif
    { "ast",    tZONE,     HOUR ( 4) }, /* Atlantic Standard */
    { "adt",    tDAYZONE,  HOUR ( 4) }, /* Atlantic Daylight */
    { "est",    tZONE,     HOUR ( 5) }, /* Eastern Standard */
    { "edt",    tDAYZONE,  HOUR ( 5) }, /* Eastern Daylight */
    { "cst",    tZONE,     HOUR ( 6) }, /* Central Standard */
    { "cdt",    tDAYZONE,  HOUR ( 6) }, /* Central Daylight */
    { "mst",    tZONE,     HOUR ( 7) }, /* Mountain Standard */
    { "mdt",    tDAYZONE,  HOUR ( 7) }, /* Mountain Daylight */
    { "pst",    tZONE,     HOUR ( 8) }, /* Pacific Standard */
    { "pdt",    tDAYZONE,  HOUR ( 8) }, /* Pacific Daylight */
    { "yst",    tZONE,     HOUR ( 9) }, /* Yukon Standard */
    { "ydt",    tDAYZONE,  HOUR ( 9) }, /* Yukon Daylight */
    { "hst",    tZONE,     HOUR (10) }, /* Hawaii Standard */
    { "hdt",    tDAYZONE,  HOUR (10) }, /* Hawaii Daylight */
    { "cat",    tZONE,     HOUR (10) }, /* Central Alaska */
    { "ahst",   tZONE,     HOUR (10) }, /* Alaska-Hawaii Standard */
    { "nt",     tZONE,     HOUR (11) }, /* Nome */
    { "idlw",   tZONE,     HOUR (12) }, /* International Date Line West */
    { "cet",    tZONE,     -HOUR (1) }, /* Central European */
    { "met",    tZONE,     -HOUR (1) }, /* Middle European */
    { "mewt",   tZONE,     -HOUR (1) }, /* Middle European Winter */
    { "mest",   tDAYZONE,  -HOUR (1) }, /* Middle European Summer */
    { "mesz",   tDAYZONE,  -HOUR (1) }, /* Middle European Summer */
    { "swt",    tZONE,     -HOUR (1) }, /* Swedish Winter */
    { "sst",    tDAYZONE,  -HOUR (1) }, /* Swedish Summer */
    { "fwt",    tZONE,     -HOUR (1) }, /* French Winter */
    { "fst",    tDAYZONE,  -HOUR (1) }, /* French Summer */
    { "eet",    tZONE,     -HOUR (2) }, /* Eastern Europe, USSR Zone 1 */
    { "bt",     tZONE,     -HOUR (3) }, /* Baghdad, USSR Zone 2 */
#if 0
    { "it",     tZONE,     -HOUR (3.5) },/* Iran */
#endif
    { "zp4",    tZONE,     -HOUR (4) }, /* USSR Zone 3 */
    { "zp5",    tZONE,     -HOUR (5) }, /* USSR Zone 4 */
#if 0
    { "ist",    tZONE,     -HOUR (5.5) },/* Indian Standard */
#endif
    { "zp6",    tZONE,     -HOUR (6) }, /* USSR Zone 5 */
#if     0
    /* For completeness.  NST is also Newfoundland Standard, and SST is
     * also Swedish Summer. */
    { "nst",    tZONE,     -HOUR (6.5) },/* North Sumatra */
    { "sst",    tZONE,     -HOUR (7) }, /* South Sumatra, USSR Zone 6 */
#endif  /* 0 */
    { "wast",   tZONE,     -HOUR (7) }, /* West Australian Standard */
    { "wadt",   tDAYZONE,  -HOUR (7) }, /* West Australian Daylight */
#if 0
    { "jt",     tZONE,     -HOUR (7.5) },/* Java (3pm in Cronusland!) */
#endif
    { "cct",    tZONE,     -HOUR (8) }, /* China Coast, USSR Zone 7 */
    { "jst",    tZONE,     -HOUR (9) }, /* Japan Standard, USSR Zone 8 */
#if 0
    { "cast",   tZONE,     -HOUR (9.5) },/* Central Australian Standard */
    { "cadt",   tDAYZONE,  -HOUR (9.5) },/* Central Australian Daylight */
#endif
    { "east",   tZONE,     -HOUR (10) },        /* Eastern Australian Standard */
    { "eadt",   tDAYZONE,  -HOUR (10) },        /* Eastern Australian Daylight */
    { "gst",    tZONE,     -HOUR (10) },        /* Guam Standard, USSR Zone 9 */
    { "nzt",    tZONE,     -HOUR (12) },        /* New Zealand */
    { "nzst",   tZONE,     -HOUR (12) },        /* New Zealand Standard */
    { "nzdt",   tDAYZONE,  -HOUR (12) },        /* New Zealand Daylight */
    { "idle",   tZONE,     -HOUR (12) },        /* International Date Line East */
    {  NULL, 0, 0  }
};

/* Military timezone table. */
static TABLE const MilitaryTable[] = {
    { "a",      tZONE,  HOUR (  1) },
    { "b",      tZONE,  HOUR (  2) },
    { "c",      tZONE,  HOUR (  3) },
    { "d",      tZONE,  HOUR (  4) },
    { "e",      tZONE,  HOUR (  5) },
    { "f",      tZONE,  HOUR (  6) },
    { "g",      tZONE,  HOUR (  7) },
    { "h",      tZONE,  HOUR (  8) },
    { "i",      tZONE,  HOUR (  9) },
    { "k",      tZONE,  HOUR ( 10) },
    { "l",      tZONE,  HOUR ( 11) },
    { "m",      tZONE,  HOUR ( 12) },
    { "n",      tZONE,  HOUR (- 1) },
    { "o",      tZONE,  HOUR (- 2) },
    { "p",      tZONE,  HOUR (- 3) },
    { "q",      tZONE,  HOUR (- 4) },
    { "r",      tZONE,  HOUR (- 5) },
    { "s",      tZONE,  HOUR (- 6) },
    { "t",      tZONE,  HOUR (- 7) },
    { "u",      tZONE,  HOUR (- 8) },
    { "v",      tZONE,  HOUR (- 9) },
    { "w",      tZONE,  HOUR (-10) },
    { "x",      tZONE,  HOUR (-11) },
    { "y",      tZONE,  HOUR (-12) },
    { "z",      tZONE,  HOUR (  0) },
    { NULL, 0, 0 }
};




/* ARGSUSED */
static int
yyerror (const char *s ATTRIBUTE_UNUSED)
{
  return 0;
}

static int
ToHour (int Hours, MERIDIAN Meridian)
{
  switch (Meridian)
    {
    case MER24:
      if (Hours < 0 || Hours > 23)
        return -1;
      return Hours;
    case MERam:
      if (Hours < 1 || Hours > 12)
        return -1;
      if (Hours == 12)
        Hours = 0;
      return Hours;
    case MERpm:
      if (Hours < 1 || Hours > 12)
        return -1;
      if (Hours == 12)
        Hours = 0;
      return Hours + 12;
    default:
      break; /* used to do abort() here */
    }
  /* NOTREACHED - but make gcc happy! */
  return -1;
}

static int
ToYear (int Year)
{
  if (Year < 0)
    Year = -Year;

  /* XPG4 suggests that years 00-68 map to 2000-2068, and
     years 69-99 map to 1969-1999.  */
  if (Year < 69)
    Year += 2000;
  else if (Year < 100)
    Year += 1900;

  return Year;
}

static int
LookupWord (YYSTYPE *yylval, char *buff)
{
  char *p;
  char *q;
  const TABLE *tp;
  size_t i;
  int abbrev;

  /* Make it lowercase. */
  for (p = buff; *p; p++)
    if (ISUPPER ((unsigned char) *p))
      *p = tolower ((int)*p);

  if (strcmp (buff, "am") == 0 || strcmp (buff, "a.m.") == 0)
    {
      yylval->Meridian = MERam;
      return tMERIDIAN;
    }
  if (strcmp (buff, "pm") == 0 || strcmp (buff, "p.m.") == 0)
    {
      yylval->Meridian = MERpm;
      return tMERIDIAN;
    }

  /* See if we have an abbreviation for a month. */
  if (strlen (buff) == 3)
    abbrev = 1;
  else if (strlen (buff) == 4 && buff[3] == '.')
    {
      abbrev = 1;
      buff[3] = '\0';
    }
  else
    abbrev = 0;

  for (tp = MonthDayTable; tp->name; tp++)
    {
      if (abbrev)
        {
          if (strncmp (buff, tp->name, 3) == 0)
            {
              yylval->Number = tp->value;
              return tp->type;
            }
        }
      else if (strcmp (buff, tp->name) == 0)
        {
          yylval->Number = tp->value;
          return tp->type;
        }
    }

  for (tp = TimezoneTable; tp->name; tp++)
    if (strcmp (buff, tp->name) == 0)
      {
        yylval->Number = tp->value;
        return tp->type;
      }

  if (strcmp (buff, "dst") == 0)
    return tDST;

  for (tp = UnitsTable; tp->name; tp++)
    if (strcmp (buff, tp->name) == 0)
      {
        yylval->Number = tp->value;
        return tp->type;
      }

  /* Strip off any plural and try the units table again. */
  i = strlen (buff) - 1;
  if (buff[i] == 's')
    {
      buff[i] = '\0';
      for (tp = UnitsTable; tp->name; tp++)
        if (strcmp (buff, tp->name) == 0)
          {
            yylval->Number = tp->value;
            return tp->type;
          }
      buff[i] = 's';            /* Put back for "this" in OtherTable. */
    }

  for (tp = OtherTable; tp->name; tp++)
    if (strcmp (buff, tp->name) == 0)
      {
        yylval->Number = tp->value;
        return tp->type;
      }

  /* Military timezones. */
  if (buff[1] == '\0' && ISALPHA ((unsigned char) *buff))
    {
      for (tp = MilitaryTable; tp->name; tp++)
        if (strcmp (buff, tp->name) == 0)
          {
            yylval->Number = tp->value;
            return tp->type;
          }
    }

  /* Drop out any periods and try the timezone table again. */
  for (i = 0, p = q = buff; *q; q++)
    if (*q != '.')
      *p++ = *q;
    else
      i++;
  *p = '\0';
  if (i)
    for (tp = TimezoneTable; tp->name; tp++)
      if (strcmp (buff, tp->name) == 0)
        {
          yylval->Number = tp->value;
          return tp->type;
        }

  return tID;
}

static int
yylex (YYSTYPE *yylval, void *cookie)
{
  register unsigned char c;
  register char *p;
  char buff[20];
  int Count;
  int sign;

  for (;;)
    {
      while (ISSPACE ((unsigned char) *context->yyInput))
        context->yyInput++;

      if (ISDIGIT (c = *context->yyInput) || c == '-' || c == '+')
        {
          if (c == '-' || c == '+')
            {
              sign = c == '-' ? -1 : 1;
              if (!ISDIGIT (*++context->yyInput))
                /* skip the '-' sign */
                continue;
            }
          else
            sign = 0;
          for (yylval->Number = 0; ISDIGIT (c = *context->yyInput++);)
            yylval->Number = 10 * yylval->Number + c - '0';
          context->yyInput--;
          if (sign < 0)
            yylval->Number = -yylval->Number;
          return sign ? tSNUMBER : tUNUMBER;
        }
      if (ISALPHA (c))
        {
          for (p = buff; (c = *context->yyInput++, ISALPHA (c)) || c == '.';)
            if (p < &buff[sizeof buff - 1])
              *p++ = c;
          *p = '\0';
          context->yyInput--;
          return LookupWord (yylval, buff);
        }
      if (c != '(')
        return *context->yyInput++;
      Count = 0;
      do
        {
          c = *context->yyInput++;
          if (c == '\0')
            return c;
          if (c == '(')
            Count++;
          else if (c == ')')
            Count--;
        }
      while (Count > 0);
    }
}

#define TM_YEAR_ORIGIN 1900

/* Yield A - B, measured in seconds.  */
static long
difftm (struct tm *a, struct tm *b)
{
  int ay = a->tm_year + (TM_YEAR_ORIGIN - 1);
  int by = b->tm_year + (TM_YEAR_ORIGIN - 1);
  long days = (
  /* difference in day of year */
                a->tm_yday - b->tm_yday
  /* + intervening leap days */
                + ((ay >> 2) - (by >> 2))
                - (ay / 100 - by / 100)
                + ((ay / 100 >> 2) - (by / 100 >> 2))
  /* + difference in years * 365 */
                + (long) (ay - by) * 365
  );
  return (60 * (60 * (24 * days + (a->tm_hour - b->tm_hour))
                + (a->tm_min - b->tm_min))
          + (a->tm_sec - b->tm_sec));
}

time_t
curl_getdate (const char *p, const time_t *now)
{
  struct tm tm, tm0, *tmp;
  time_t Start;
  CURL_CONTEXT cookie;
#ifdef HAVE_LOCALTIME_R
  struct tm keeptime;
#endif
  cookie.yyInput = p;
  Start = now ? *now : time ((time_t *) NULL);
#ifdef HAVE_LOCALTIME_R
  tmp = (struct tm *)localtime_r(&Start, &keeptime);
#else
  tmp = localtime (&Start);
#endif
  if (!tmp)
    return -1;
  cookie.yyYear = tmp->tm_year + TM_YEAR_ORIGIN;
  cookie.yyMonth = tmp->tm_mon + 1;
  cookie.yyDay = tmp->tm_mday;
  cookie.yyHour = tmp->tm_hour;
  cookie.yyMinutes = tmp->tm_min;
  cookie.yySeconds = tmp->tm_sec;
  tm.tm_isdst = tmp->tm_isdst;
  cookie.yyMeridian = MER24;
  cookie.yyRelSeconds = 0;
  cookie.yyRelMinutes = 0;
  cookie.yyRelHour = 0;
  cookie.yyRelDay = 0;
  cookie.yyRelMonth = 0;
  cookie.yyRelYear = 0;
  cookie.yyHaveDate = 0;
  cookie.yyHaveDay = 0;
  cookie.yyHaveRel = 0;
  cookie.yyHaveTime = 0;
  cookie.yyHaveZone = 0;

  if (yyparse (&cookie)
      || cookie.yyHaveTime > 1 || cookie.yyHaveZone > 1 ||
      cookie.yyHaveDate > 1 || cookie.yyHaveDay > 1)
    return -1;

  tm.tm_year = ToYear (cookie.yyYear) - TM_YEAR_ORIGIN + cookie.yyRelYear;
  tm.tm_mon = cookie.yyMonth - 1 + cookie.yyRelMonth;
  tm.tm_mday = cookie.yyDay + cookie.yyRelDay;
  if (cookie.yyHaveTime ||
      (cookie.yyHaveRel && !cookie.yyHaveDate && !cookie.yyHaveDay))
    {
      tm.tm_hour = ToHour (cookie.yyHour, cookie.yyMeridian);
      if (tm.tm_hour < 0)
        return -1;
      tm.tm_min = cookie.yyMinutes;
      tm.tm_sec = cookie.yySeconds;
    }
  else
    {
      tm.tm_hour = tm.tm_min = tm.tm_sec = 0;
    }
  tm.tm_hour += cookie.yyRelHour;
  tm.tm_min += cookie.yyRelMinutes;
  tm.tm_sec += cookie.yyRelSeconds;

  /* Let mktime deduce tm_isdst if we have an absolute timestamp,
     or if the relative timestamp mentions days, months, or years.  */
  if (cookie.yyHaveDate | cookie.yyHaveDay | cookie.yyHaveTime |
      cookie.yyRelDay | cookie.yyRelMonth | cookie.yyRelYear)
    tm.tm_isdst = -1;

  tm0 = tm;

  Start = mktime (&tm);

  if (Start == (time_t) -1)
    {

      /* Guard against falsely reporting errors near the time_t boundaries
         when parsing times in other time zones.  For example, if the min
         time_t value is 1970-01-01 00:00:00 UTC and we are 8 hours ahead
         of UTC, then the min localtime value is 1970-01-01 08:00:00; if
         we apply mktime to 1970-01-01 00:00:00 we will get an error, so
         we apply mktime to 1970-01-02 08:00:00 instead and adjust the time
         zone by 24 hours to compensate.  This algorithm assumes that
         there is no DST transition within a day of the time_t boundaries.  */
      if (cookie.yyHaveZone)
        {
          tm = tm0;
          if (tm.tm_year <= EPOCH - TM_YEAR_ORIGIN)
            {
              tm.tm_mday++;
              cookie.yyTimezone -= 24 * 60;
            }
          else
            {
              tm.tm_mday--;
              cookie.yyTimezone += 24 * 60;
            }
          Start = mktime (&tm);
        }

      if (Start == (time_t) -1)
        return Start;
    }

  if (cookie.yyHaveDay && !cookie.yyHaveDate)
    {
      tm.tm_mday += ((cookie.yyDayNumber - tm.tm_wday + 7) % 7
                     + 7 * (cookie.yyDayOrdinal - (0 < cookie.yyDayOrdinal)));
      Start = mktime (&tm);
      if (Start == (time_t) -1)
        return Start;
    }

  if (cookie.yyHaveZone)
    {
      long delta;
      struct tm *gmt;
#ifdef HAVE_GMTIME_R
      /* thread-safe version */
      struct tm keeptime2;
      gmt = (struct tm *)gmtime_r(&Start, &keeptime2);
#else
      gmt = gmtime(&Start);
#endif
      if (!gmt)
        return -1;
      delta = cookie.yyTimezone * 60L + difftm (&tm, gmt);
      if ((Start + delta < Start) != (delta < 0))
        return -1;              /* time_t overflow */
      Start += delta;
    }

  return Start;
}


