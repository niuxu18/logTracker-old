@@ -1,7 +1,6 @@
-#include "cmStandardIncludes.h"
-
-
+#line 2 "cmCommandArgumentLexer.cxx"
 
+#line 4 "cmCommandArgumentLexer.cxx"
 
 #define  YY_INT_ALIGNED short int
 
@@ -10,7 +9,7 @@
 #define FLEX_SCANNER
 #define YY_FLEX_MAJOR_VERSION 2
 #define YY_FLEX_MINOR_VERSION 5
-#define YY_FLEX_SUBMINOR_VERSION 31
+#define YY_FLEX_SUBMINOR_VERSION 33
 #if YY_FLEX_SUBMINOR_VERSION > 0
 #define FLEX_BETA
 #endif
@@ -32,7 +31,15 @@
 
 /* C99 systems have <inttypes.h>. Non-C99 systems may or may not. */
 
-#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
+#if __STDC_VERSION__ >= 199901L
+
+/* C99 says to define __STDC_LIMIT_MACROS before including stdint.h,
+ * if you want the limit (max/min) macros for int types. 
+ */
+#ifndef __STDC_LIMIT_MACROS
+#define __STDC_LIMIT_MACROS 1
+#endif
+
 #include <inttypes.h>
 typedef int8_t flex_int8_t;
 typedef uint8_t flex_uint8_t;
@@ -85,7 +92,7 @@ typedef unsigned int flex_uint32_t;
 /* The "const" storage-class-modifier is valid. */
 #define YY_USE_CONST
 
-#else  /* ! __cplusplus */
+#else   /* ! __cplusplus */
 
 #if __STDC__
 
@@ -155,6 +162,10 @@ int cmCommandArgument_yylex_init (yyscan_t* scanner);
 #define YY_BUF_SIZE 16384
 #endif
 
+/* The state buf must be large enough to hold one state per character in the main buffer.
+ */
+#define YY_STATE_BUF_SIZE   ((YY_BUF_SIZE + 2) * sizeof(yy_state_type))
+
 #ifndef YY_TYPEDEF_YY_BUFFER_STATE
 #define YY_TYPEDEF_YY_BUFFER_STATE
 typedef struct yy_buffer_state *YY_BUFFER_STATE;
@@ -166,20 +177,21 @@ typedef struct yy_buffer_state *YY_BUFFER_STATE;
 
     #define YY_LESS_LINENO(n)
     
-/* Return all but the first "n" matched characters back to the input
-   stream. */
+/* Return all but the first "n" matched characters back to the input stream. */
 #define yyless(n) \
-  do \
-    { \
-    /* Undo effects of setting up yytext. */ \
+        do \
+                { \
+                /* Undo effects of setting up yytext. */ \
         int yyless_macro_arg = (n); \
         YY_LESS_LINENO(yyless_macro_arg);\
-    *yy_cp = yyg->yy_hold_char; \
-    YY_RESTORE_YY_MORE_OFFSET \
-    yyg->yy_c_buf_p = yy_cp = yy_bp + yyless_macro_arg - YY_MORE_ADJ; \
-    YY_DO_BEFORE_ACTION; /* set up yytext again */ \
-    } \
-  while ( 0 )
+                *yy_cp = yyg->yy_hold_char; \
+                YY_RESTORE_YY_MORE_OFFSET \
+                yyg->yy_c_buf_p = yy_cp = yy_bp + yyless_macro_arg - YY_MORE_ADJ; \
+                YY_DO_BEFORE_ACTION; /* set up yytext again */ \
+                } \
+        while ( 0 )
+
+#define unput(c) yyunput( c, yyg->yytext_ptr , yyscanner )
 
 /* The following is because we cannot portably get our hands on size_t
  * (without autoconf's help, which isn't available because we want
@@ -194,66 +206,66 @@ typedef unsigned int yy_size_t;
 #ifndef YY_STRUCT_YY_BUFFER_STATE
 #define YY_STRUCT_YY_BUFFER_STATE
 struct yy_buffer_state
-  {
-  FILE *yy_input_file;
-
-  char *yy_ch_buf;    /* input buffer */
-  char *yy_buf_pos;    /* current position in input buffer */
-
-  /* Size of input buffer in bytes, not including room for EOB
-   * characters.
-   */
-  yy_size_t yy_buf_size;
-
-  /* Number of characters read into yy_ch_buf, not including EOB
-   * characters.
-   */
-  int yy_n_chars;
-
-  /* Whether we "own" the buffer - i.e., we know we created it,
-   * and can realloc() it to grow it, and should free() it to
-   * delete it.
-   */
-  int yy_is_our_buffer;
-
-  /* Whether this is an "interactive" input source; if so, and
-   * if we're using stdio for input, then we want to use getc()
-   * instead of fread(), to make sure we stop fetching input after
-   * each newline.
-   */
-  int yy_is_interactive;
-
-  /* Whether we're considered to be at the beginning of a line.
-   * If so, '^' rules will be active on the next match, otherwise
-   * not.
-   */
-  int yy_at_bol;
+        {
+        FILE *yy_input_file;
+
+        char *yy_ch_buf;                /* input buffer */
+        char *yy_buf_pos;               /* current position in input buffer */
+
+        /* Size of input buffer in bytes, not including room for EOB
+         * characters.
+         */
+        yy_size_t yy_buf_size;
+
+        /* Number of characters read into yy_ch_buf, not including EOB
+         * characters.
+         */
+        int yy_n_chars;
+
+        /* Whether we "own" the buffer - i.e., we know we created it,
+         * and can realloc() it to grow it, and should free() it to
+         * delete it.
+         */
+        int yy_is_our_buffer;
+
+        /* Whether this is an "interactive" input source; if so, and
+         * if we're using stdio for input, then we want to use getc()
+         * instead of fread(), to make sure we stop fetching input after
+         * each newline.
+         */
+        int yy_is_interactive;
+
+        /* Whether we're considered to be at the beginning of a line.
+         * If so, '^' rules will be active on the next match, otherwise
+         * not.
+         */
+        int yy_at_bol;
 
     int yy_bs_lineno; /**< The line count. */
     int yy_bs_column; /**< The column count. */
     
-  /* Whether to try to fill the input buffer when we reach the
-   * end of it.
-   */
-  int yy_fill_buffer;
+        /* Whether to try to fill the input buffer when we reach the
+         * end of it.
+         */
+        int yy_fill_buffer;
 
-  int yy_buffer_status;
+        int yy_buffer_status;
 
 #define YY_BUFFER_NEW 0
 #define YY_BUFFER_NORMAL 1
-  /* When an EOF's been seen but there's still some text to process
-   * then we mark the buffer as YY_EOF_PENDING, to indicate that we
-   * shouldn't try reading from the input source any more.  We might
-   * still have a bunch of tokens to match, though, because of
-   * possible backing-up.
-   *
-   * When we actually see the EOF, we change the status to "new" (via
-   * cmCommandArgument_yyrestart()), so that the user can continue scanning
-   * by just pointing yyin at a new input file.
-   */
+        /* When an EOF's been seen but there's still some text to process
+         * then we mark the buffer as YY_EOF_PENDING, to indicate that we
+         * shouldn't try reading from the input source any more.  We might
+         * still have a bunch of tokens to match, though, because of
+         * possible backing-up.
+         *
+         * When we actually see the EOF, we change the status to "new"
+         * (via cmCommandArgument_yyrestart()), so that the user can continue scanning by
+         * just pointing yyin at a new input file.
+         */
 #define YY_BUFFER_EOF_PENDING 2
 
-  };
+        };
 #endif /* !YY_STRUCT_YY_BUFFER_STATE */
 
 /* We provide macros for accessing buffer states in case in the
@@ -266,29 +278,22 @@ struct yy_buffer_state
                           ? yyg->yy_buffer_stack[yyg->yy_buffer_stack_top] \
                           : NULL)
 
-/* Same as previous macro, but useful when we know that the buffer stack is
- * not NULL or when we need an lvalue. For internal use only.
+/* Same as previous macro, but useful when we know that the buffer stack is not
+ * NULL or when we need an lvalue. For internal use only.
  */
 #define YY_CURRENT_BUFFER_LVALUE yyg->yy_buffer_stack[yyg->yy_buffer_stack_top]
 
 void cmCommandArgument_yyrestart (FILE *input_file ,yyscan_t yyscanner );
-void cmCommandArgument_yy_switch_to_buffer (YY_BUFFER_STATE new_buffer ,
-                                            yyscan_t yyscanner );
-YY_BUFFER_STATE cmCommandArgument_yy_create_buffer (FILE *file,int size ,
-                                                    yyscan_t yyscanner );
-void cmCommandArgument_yy_delete_buffer (YY_BUFFER_STATE b ,
-                                         yyscan_t yyscanner );
-void cmCommandArgument_yy_flush_buffer (YY_BUFFER_STATE b ,
-                                        yyscan_t yyscanner );
-void cmCommandArgument_yypush_buffer_state (YY_BUFFER_STATE new_buffer ,
-                                            yyscan_t yyscanner );
+void cmCommandArgument_yy_switch_to_buffer (YY_BUFFER_STATE new_buffer ,yyscan_t yyscanner );
+YY_BUFFER_STATE cmCommandArgument_yy_create_buffer (FILE *file,int size ,yyscan_t yyscanner );
+void cmCommandArgument_yy_delete_buffer (YY_BUFFER_STATE b ,yyscan_t yyscanner );
+void cmCommandArgument_yy_flush_buffer (YY_BUFFER_STATE b ,yyscan_t yyscanner );
+void cmCommandArgument_yypush_buffer_state (YY_BUFFER_STATE new_buffer ,yyscan_t yyscanner );
 void cmCommandArgument_yypop_buffer_state (yyscan_t yyscanner );
 
 static void cmCommandArgument_yyensure_buffer_stack (yyscan_t yyscanner );
 static void cmCommandArgument_yy_load_buffer_state (yyscan_t yyscanner );
-static void cmCommandArgument_yy_init_buffer (YY_BUFFER_STATE b,
-                                              FILE *file ,
-                                              yyscan_t yyscanner );
+static void cmCommandArgument_yy_init_buffer (YY_BUFFER_STATE b,FILE *file ,yyscan_t yyscanner );
 
 #define YY_FLUSH_BUFFER cmCommandArgument_yy_flush_buffer(YY_CURRENT_BUFFER ,yyscanner)
 
@@ -303,24 +308,24 @@ void cmCommandArgument_yyfree (void * ,yyscan_t yyscanner );
 #define yy_new_buffer cmCommandArgument_yy_create_buffer
 
 #define yy_set_interactive(is_interactive) \
-  { \
-  if ( ! YY_CURRENT_BUFFER ){ \
+        { \
+        if ( ! YY_CURRENT_BUFFER ){ \
         cmCommandArgument_yyensure_buffer_stack (yyscanner); \
-    YY_CURRENT_BUFFER_LVALUE =    \
+                YY_CURRENT_BUFFER_LVALUE =    \
             cmCommandArgument_yy_create_buffer(yyin,YY_BUF_SIZE ,yyscanner); \
-  } \
-  YY_CURRENT_BUFFER_LVALUE->yy_is_interactive = is_interactive; \
-  }
+        } \
+        YY_CURRENT_BUFFER_LVALUE->yy_is_interactive = is_interactive; \
+        }
 
 #define yy_set_bol(at_bol) \
-  { \
-  if ( ! YY_CURRENT_BUFFER ){\
+        { \
+        if ( ! YY_CURRENT_BUFFER ){\
         cmCommandArgument_yyensure_buffer_stack (yyscanner); \
-    YY_CURRENT_BUFFER_LVALUE =    \
+                YY_CURRENT_BUFFER_LVALUE =    \
             cmCommandArgument_yy_create_buffer(yyin,YY_BUF_SIZE ,yyscanner); \
-  } \
-  YY_CURRENT_BUFFER_LVALUE->yy_at_bol = at_bol; \
-  }
+        } \
+        YY_CURRENT_BUFFER_LVALUE->yy_at_bol = at_bol; \
+        }
 
 #define YY_AT_BOL() (YY_CURRENT_BUFFER_LVALUE->yy_at_bol)
 
@@ -344,21 +349,21 @@ static void yy_fatal_error (yyconst char msg[] ,yyscan_t yyscanner );
  * corresponding action - sets up yytext.
  */
 #define YY_DO_BEFORE_ACTION \
-  yyg->yytext_ptr = yy_bp; \
-  yyleng = (size_t) (yy_cp - yy_bp); \
-  yyg->yy_hold_char = *yy_cp; \
-  *yy_cp = '\0'; \
-  yyg->yy_c_buf_p = yy_cp;
+        yyg->yytext_ptr = yy_bp; \
+        yyleng = (size_t) (yy_cp - yy_bp); \
+        yyg->yy_hold_char = *yy_cp; \
+        *yy_cp = '\0'; \
+        yyg->yy_c_buf_p = yy_cp;
 
 #define YY_NUM_RULES 12
 #define YY_END_OF_BUFFER 13
 /* This struct is not used in this scanner,
    but its presence is necessary. */
 struct yy_trans_info
-  {
-  flex_int32_t yy_verify;
-  flex_int32_t yy_nxt;
-  };
+        {
+        flex_int32_t yy_verify;
+        flex_int32_t yy_nxt;
+        };
 static yyconst flex_int16_t yy_accept[20] =
     {   0,
         0,    0,   13,    8,    9,    6,    5,   11,   10,    4,
@@ -371,7 +376,7 @@ static yyconst flex_int32_t yy_ec[256] =
         1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
         1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
         1,    1,    1,    1,    1,    3,    1,    1,    1,    1,
-        1,    1,    1,    1,    4,    4,    4,    4,    4,    4,
+        1,    1,    4,    1,    4,    4,    4,    4,    4,    4,
         4,    4,    4,    4,    4,    4,    4,    1,    1,    1,
         1,    1,    1,    5,    4,    4,    4,    4,    4,    4,
         4,    4,    4,    4,    4,    4,    4,    4,    4,    4,
@@ -437,8 +442,8 @@ static yyconst flex_int16_t yy_chk[30] =
 #define yymore() yymore_used_but_not_detected
 #define YY_MORE_ADJ 0
 #define YY_RESTORE_YY_MORE_OFFSET
-
-
+#line 1 "cmCommandArgumentLexer.in.l"
+#line 2 "cmCommandArgumentLexer.in.l"
 /*=========================================================================
 
   Program:   CMake - Cross-Platform Makefile Generator
@@ -491,7 +496,7 @@ Modify cmCommandArgumentLexer.h:
 #include "cmCommandArgumentParserTokens.h"
 
 /*--------------------------------------------------------------------------*/
-
+#line 500 "cmCommandArgumentLexer.cxx"
 
 #define INITIAL 0
 
@@ -541,6 +546,8 @@ struct yyguts_t
 
     }; /* end struct yyguts_t */
 
+static int yy_init_globals (yyscan_t yyscanner );
+
 /* Accessor methods to globals.
    These are made visible to non-reentrant scanners for convenience. */
 
@@ -582,6 +589,8 @@ extern int cmCommandArgument_yywrap (yyscan_t yyscanner );
 #endif
 #endif
 
+    static void yyunput (int c,char *buf_ptr  ,yyscan_t yyscanner);
+    
 #ifndef yytext_ptr
 static void yy_flex_strncpy (char *,yyconst char *,int ,yyscan_t yyscanner);
 #endif
@@ -613,38 +622,38 @@ static int input (yyscan_t yyscanner );
 #define ECHO (void) fwrite( yytext, yyleng, 1, yyout )
 #endif
 
-/* Gets input and stuffs it into "buf".  number of characters read, or
- * YY_NULL, is returned in "result".
+/* Gets input and stuffs it into "buf".  number of characters read, or YY_NULL,
+ * is returned in "result".
  */
 #ifndef YY_INPUT
 #define YY_INPUT(buf,result,max_size) \
-  if ( YY_CURRENT_BUFFER_LVALUE->yy_is_interactive ) \
-    { \
-    int c = '*'; \
-    size_t n; \
-    for ( n = 0; n < max_size && \
-           (c = getc( yyin )) != EOF && c != '\n'; ++n ) \
-      buf[n] = (char) c; \
-    if ( c == '\n' ) \
-      buf[n++] = (char) c; \
-    if ( c == EOF && ferror( yyin ) ) \
-      YY_FATAL_ERROR( "input in flex scanner failed" ); \
-    result = n; \
-    } \
-  else \
-    { \
-    errno=0; \
-    while ( (result = fread(buf, 1, max_size, yyin))==0 && ferror(yyin)) \
-      { \
-      if( errno != EINTR) \
-        { \
-        YY_FATAL_ERROR( "input in flex scanner failed" ); \
-        break; \
-        } \
-      errno=0; \
-      clearerr(yyin); \
-      } \
-    }\
+        if ( YY_CURRENT_BUFFER_LVALUE->yy_is_interactive ) \
+                { \
+                int c = '*'; \
+                size_t n; \
+                for ( n = 0; n < max_size && \
+                             (c = getc( yyin )) != EOF && c != '\n'; ++n ) \
+                        buf[n] = (char) c; \
+                if ( c == '\n' ) \
+                        buf[n++] = (char) c; \
+                if ( c == EOF && ferror( yyin ) ) \
+                        YY_FATAL_ERROR( "input in flex scanner failed" ); \
+                result = n; \
+                } \
+        else \
+                { \
+                errno=0; \
+                while ( (result = fread(buf, 1, max_size, yyin))==0 && ferror(yyin)) \
+                        { \
+                        if( errno != EINTR) \
+                                { \
+                                YY_FATAL_ERROR( "input in flex scanner failed" ); \
+                                break; \
+                                } \
+                        errno=0; \
+                        clearerr(yyin); \
+                        } \
+                }\
 \
 
 #endif
@@ -693,122 +702,122 @@ extern int cmCommandArgument_yylex (yyscan_t yyscanner);
 #endif
 
 #define YY_RULE_SETUP \
-  YY_USER_ACTION
+        YY_USER_ACTION
 
 /** The main scanner function which does all the work.
  */
 YY_DECL
 {
-  register yy_state_type yy_current_state;
-  register char *yy_cp, *yy_bp;
-  register int yy_act;
+        register yy_state_type yy_current_state;
+        register char *yy_cp, *yy_bp;
+        register int yy_act;
     struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
 
+#line 60 "cmCommandArgumentLexer.in.l"
 
 
+#line 720 "cmCommandArgumentLexer.cxx"
 
-
-
-  if ( yyg->yy_init )
-    {
-    yyg->yy_init = 0;
+        if ( !yyg->yy_init )
+                {
+                yyg->yy_init = 1;
 
 #ifdef YY_USER_INIT
-    YY_USER_INIT;
+                YY_USER_INIT;
 #endif
 
-    if ( ! yyg->yy_start )
-      yyg->yy_start = 1;  /* first start state */
+                if ( ! yyg->yy_start )
+                        yyg->yy_start = 1;      /* first start state */
 
-    if ( ! yyin )
-      yyin = stdin;
+                if ( ! yyin )
+                        yyin = stdin;
 
-    if ( ! yyout )
-      yyout = stdout;
+                if ( ! yyout )
+                        yyout = stdout;
 
-    if ( ! YY_CURRENT_BUFFER ) {
-      cmCommandArgument_yyensure_buffer_stack (yyscanner);
-      YY_CURRENT_BUFFER_LVALUE =
-        cmCommandArgument_yy_create_buffer(yyin,YY_BUF_SIZE ,yyscanner);
-    }
+                if ( ! YY_CURRENT_BUFFER ) {
+                        cmCommandArgument_yyensure_buffer_stack (yyscanner);
+                        YY_CURRENT_BUFFER_LVALUE =
+                                cmCommandArgument_yy_create_buffer(yyin,YY_BUF_SIZE ,yyscanner);
+                }
 
-    cmCommandArgument_yy_load_buffer_state(yyscanner );
-    }
+                cmCommandArgument_yy_load_buffer_state(yyscanner );
+                }
 
-  for(;;)    /* loops until end-of-file is reached */
-    {
-    yy_cp = yyg->yy_c_buf_p;
+        for(;;)             /* loops until end-of-file is reached */
+                {
+                yy_cp = yyg->yy_c_buf_p;
 
-    /* Support of yytext. */
-    *yy_cp = yyg->yy_hold_char;
+                /* Support of yytext. */
+                *yy_cp = yyg->yy_hold_char;
 
-    /* yy_bp points to the position in yy_ch_buf of the start of
-     * the current run.
-     */
-    yy_bp = yy_cp;
+                /* yy_bp points to the position in yy_ch_buf of the start of
+                 * the current run.
+                 */
+                yy_bp = yy_cp;
 
-    yy_current_state = yyg->yy_start;
+                yy_current_state = yyg->yy_start;
 yy_match:
-    do
-      {
-      register YY_CHAR yy_c = yy_ec[YY_SC_TO_UI(*yy_cp)];
-      if ( yy_accept[yy_current_state] )
-        {
-        yyg->yy_last_accepting_state = yy_current_state;
-        yyg->yy_last_accepting_cpos = yy_cp;
-        }
-      while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
-        {
-        yy_current_state = (int) yy_def[yy_current_state];
-        if ( yy_current_state >= 20 )
-          yy_c = yy_meta[(unsigned int) yy_c];
-        }
-      yy_current_state = yy_nxt[yy_base[yy_current_state] + (unsigned int) yy_c];
-      ++yy_cp;
-      }
-    while ( yy_base[yy_current_state] != 21 );
+                do
+                        {
+                        register YY_CHAR yy_c = yy_ec[YY_SC_TO_UI(*yy_cp)];
+                        if ( yy_accept[yy_current_state] )
+                                {
+                                yyg->yy_last_accepting_state = yy_current_state;
+                                yyg->yy_last_accepting_cpos = yy_cp;
+                                }
+                        while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
+                                {
+                                yy_current_state = (int) yy_def[yy_current_state];
+                                if ( yy_current_state >= 20 )
+                                        yy_c = yy_meta[(unsigned int) yy_c];
+                                }
+                        yy_current_state = yy_nxt[yy_base[yy_current_state] + (unsigned int) yy_c];
+                        ++yy_cp;
+                        }
+                while ( yy_base[yy_current_state] != 21 );
 
 yy_find_action:
-    yy_act = yy_accept[yy_current_state];
-    if ( yy_act == 0 )
-      { /* have to back up */
-      yy_cp = yyg->yy_last_accepting_cpos;
-      yy_current_state = yyg->yy_last_accepting_state;
-      yy_act = yy_accept[yy_current_state];
-      }
-
-    YY_DO_BEFORE_ACTION;
-
-do_action:  /* This label is used only to access EOF actions. */
-
-    switch ( yy_act )
-  { /* beginning of action switch */
-      case 0: /* must back up */
-      /* undo the effects of YY_DO_BEFORE_ACTION */
-      *yy_cp = yyg->yy_hold_char;
-      yy_cp = yyg->yy_last_accepting_cpos;
-      yy_current_state = yyg->yy_last_accepting_state;
-      goto yy_find_action;
+                yy_act = yy_accept[yy_current_state];
+                if ( yy_act == 0 )
+                        { /* have to back up */
+                        yy_cp = yyg->yy_last_accepting_cpos;
+                        yy_current_state = yyg->yy_last_accepting_state;
+                        yy_act = yy_accept[yy_current_state];
+                        }
+
+                YY_DO_BEFORE_ACTION;
+
+do_action:      /* This label is used only to access EOF actions. */
+
+                switch ( yy_act )
+        { /* beginning of action switch */
+                        case 0: /* must back up */
+                        /* undo the effects of YY_DO_BEFORE_ACTION */
+                        *yy_cp = yyg->yy_hold_char;
+                        yy_cp = yyg->yy_last_accepting_cpos;
+                        yy_current_state = yyg->yy_last_accepting_state;
+                        goto yy_find_action;
 
 case 1:
 YY_RULE_SETUP
-
+#line 62 "cmCommandArgumentLexer.in.l"
 { 
   //std::cerr << __LINE__ << " here: [" << yytext << "]" << std::endl;
   yyextra->AllocateParserType(yylvalp, yytext+1, strlen(yytext)-2); 
   return cal_NCURLY; 
 } 
 case 2:
 YY_RULE_SETUP
-
+#line 68 "cmCommandArgumentLexer.in.l"
 { 
   //std::cerr << __LINE__ << " here: [" << yytext << "]" << std::endl;
   yyextra->AllocateParserType(yylvalp, yytext+1, strlen(yytext)-2); 
   return cal_ATNAME; 
 } 
 case 3:
 YY_RULE_SETUP
-
+#line 74 "cmCommandArgumentLexer.in.l"
 {
   //std::cerr << __LINE__ << " here: [" << yytext << "]" << std::endl;
   //yyextra->AllocateParserType(yylvalp, yytext, strlen(yytext)); 
@@ -817,7 +826,7 @@ YY_RULE_SETUP
 }
 case 4:
 YY_RULE_SETUP
-
+#line 81 "cmCommandArgumentLexer.in.l"
 {
   //std::cerr << __LINE__ << " here: [" << yytext << "]" << std::endl;
   //yyextra->AllocateParserType(yylvalp, yytext, strlen(yytext)); 
@@ -826,7 +835,7 @@ YY_RULE_SETUP
 }
 case 5:
 YY_RULE_SETUP
-
+#line 88 "cmCommandArgumentLexer.in.l"
 {
   //std::cerr << __LINE__ << " here: [" << yytext << "]" << std::endl;
   //yyextra->AllocateParserType(yylvalp, yytext, strlen(yytext)); 
@@ -835,15 +844,15 @@ YY_RULE_SETUP
 }
 case 6:
 YY_RULE_SETUP
-
+#line 95 "cmCommandArgumentLexer.in.l"
 { 
   //std::cerr << __LINE__ << " here: [" << yytext << "]" << std::endl;
   yyextra->AllocateParserType(yylvalp, yytext, strlen(yytext)); 
   return cal_NAME; 
 }
 case 7:
 YY_RULE_SETUP
-
+#line 101 "cmCommandArgumentLexer.in.l"
 {
   if ( !yyextra->HandleEscapeSymbol(yylvalp, *(yytext+1)) )
     {
@@ -854,364 +863,401 @@ YY_RULE_SETUP
 case 8:
 /* rule 8 can match eol */
 YY_RULE_SETUP
-
+#line 109 "cmCommandArgumentLexer.in.l"
 { 
   //std::cerr << __LINE__ << " here: [" << yytext << "]" << std::endl;
   yyextra->AllocateParserType(yylvalp, yytext, strlen(yytext)); 
   return cal_SYMBOL; 
 }
 case 9:
 YY_RULE_SETUP
-
+#line 115 "cmCommandArgumentLexer.in.l"
 {
   //yyextra->AllocateParserType(yylvalp, yytext, strlen(yytext)); 
   yylvalp->str = yyextra->DOLLARVariable;
   return cal_DOLLAR; 
 }
 case 10:
 YY_RULE_SETUP
-
+#line 121 "cmCommandArgumentLexer.in.l"
 {
   //yyextra->AllocateParserType(yylvalp, yytext, strlen(yytext)); 
   yylvalp->str = yyextra->LCURLYVariable;
   return cal_LCURLY; 
 }
 case 11:
 YY_RULE_SETUP
-
+#line 127 "cmCommandArgumentLexer.in.l"
 {
   //yyextra->AllocateParserType(yylvalp, yytext, strlen(yytext)); 
   yylvalp->str = yyextra->BSLASHVariable;
   return cal_BSLASH; 
 }
 case 12:
 YY_RULE_SETUP
-
+#line 133 "cmCommandArgumentLexer.in.l"
 ECHO;
-  YY_BREAK
-
+        YY_BREAK
+#line 913 "cmCommandArgumentLexer.cxx"
 case YY_STATE_EOF(INITIAL):
-  yyterminate();
-
-  case YY_END_OF_BUFFER:
-    {
-    /* Amount of text matched not including the EOB char. */
-    int yy_amount_of_matched_text = (int) (yy_cp - yyg->yytext_ptr) - 1;
-
-    /* Undo the effects of YY_DO_BEFORE_ACTION. */
-    *yy_cp = yyg->yy_hold_char;
-    YY_RESTORE_YY_MORE_OFFSET
-
-    if ( YY_CURRENT_BUFFER_LVALUE->yy_buffer_status == YY_BUFFER_NEW )
-      {
-      /* We're scanning a new file or input source.  It's
-       * possible that this happened because the user
-       * just pointed yyin at a new source and called
-       * cmCommandArgument_yylex().  If so, then we have to assure
-       * consistency between YY_CURRENT_BUFFER and our
-       * globals.  Here is the right place to do so, because
-       * this is the first action (other than possibly a
-       * back-up) that will match for the new input source.
-       */
-      yyg->yy_n_chars = YY_CURRENT_BUFFER_LVALUE->yy_n_chars;
-      YY_CURRENT_BUFFER_LVALUE->yy_input_file = yyin;
-      YY_CURRENT_BUFFER_LVALUE->yy_buffer_status = YY_BUFFER_NORMAL;
-      }
-
-    /* Note that here we test for yy_c_buf_p "<=" to the position
-     * of the first EOB in the buffer, since yy_c_buf_p will
-     * already have been incremented past the NUL character
-     * (since all states make transitions on EOB to the
-     * end-of-buffer state).  Contrast this with the test
-     * in input().
-     */
-    if ( yyg->yy_c_buf_p <= &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars] )
-      { /* This was really a NUL. */
-      yy_state_type yy_next_state;
-
-      yyg->yy_c_buf_p = yyg->yytext_ptr + yy_amount_of_matched_text;
-
-      yy_current_state = yy_get_previous_state( yyscanner );
-
-      /* Okay, we're now positioned to make the NUL
-       * transition.  We couldn't have
-       * yy_get_previous_state() go ahead and do it
-       * for us because it doesn't know how to deal
-       * with the possibility of jamming (and we don't
-       * want to build jamming into it because then it
-       * will run more slowly).
-       */
-
-      yy_next_state = yy_try_NUL_trans( yy_current_state , yyscanner);
-
-      yy_bp = yyg->yytext_ptr + YY_MORE_ADJ;
-
-      if ( yy_next_state )
-        {
-        /* Consume the NUL. */
-        yy_cp = ++yyg->yy_c_buf_p;
-        yy_current_state = yy_next_state;
-        goto yy_match;
-        }
-
-      else
-        {
-        yy_cp = yyg->yy_c_buf_p;
-        goto yy_find_action;
-        }
-      }
-
-    else switch ( yy_get_next_buffer( yyscanner ) )
-      {
-      case EOB_ACT_END_OF_FILE:
-        {
-        yyg->yy_did_buffer_switch_on_eof = 0;
-
-        if ( cmCommandArgument_yywrap(yyscanner ) )
-          {
-          /* Note: because we've taken care in
-           * yy_get_next_buffer() to have set up
-           * yytext, we can now set up
-           * yy_c_buf_p so that if some total
-           * hoser (like flex itself) wants to
-           * call the scanner after we return the
-           * YY_NULL, it'll still work - another
-           * YY_NULL will get returned.
-           */
-          yyg->yy_c_buf_p = yyg->yytext_ptr + YY_MORE_ADJ;
-
-          yy_act = YY_STATE_EOF(YY_START);
-          goto do_action;
-          }
-
-        else
-          {
-          if ( ! yyg->yy_did_buffer_switch_on_eof )
-            YY_NEW_FILE;
-          }
-        break;
-        }
-
-      case EOB_ACT_CONTINUE_SCAN:
-        yyg->yy_c_buf_p =
-          yyg->yytext_ptr + yy_amount_of_matched_text;
-
-        yy_current_state = yy_get_previous_state( yyscanner );
-
-        yy_cp = yyg->yy_c_buf_p;
-        yy_bp = yyg->yytext_ptr + YY_MORE_ADJ;
-        goto yy_match;
-
-      case EOB_ACT_LAST_MATCH:
-        yyg->yy_c_buf_p =
-        &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars];
-
-        yy_current_state = yy_get_previous_state( yyscanner );
-
-        yy_cp = yyg->yy_c_buf_p;
-        yy_bp = yyg->yytext_ptr + YY_MORE_ADJ;
-        goto yy_find_action;
-      }
-    break;
-    }
-
-  default:
-    YY_FATAL_ERROR(
-      "fatal flex scanner internal error--no action found" );
-  } /* end of action switch */
-    } /* end of scanning one token */
+        yyterminate();
+
+        case YY_END_OF_BUFFER:
+                {
+                /* Amount of text matched not including the EOB char. */
+                int yy_amount_of_matched_text = (int) (yy_cp - yyg->yytext_ptr) - 1;
+
+                /* Undo the effects of YY_DO_BEFORE_ACTION. */
+                *yy_cp = yyg->yy_hold_char;
+                YY_RESTORE_YY_MORE_OFFSET
+
+                if ( YY_CURRENT_BUFFER_LVALUE->yy_buffer_status == YY_BUFFER_NEW )
+                        {
+                        /* We're scanning a new file or input source.  It's
+                         * possible that this happened because the user
+                         * just pointed yyin at a new source and called
+                         * cmCommandArgument_yylex().  If so, then we have to assure
+                         * consistency between YY_CURRENT_BUFFER and our
+                         * globals.  Here is the right place to do so, because
+                         * this is the first action (other than possibly a
+                         * back-up) that will match for the new input source.
+                         */
+                        yyg->yy_n_chars = YY_CURRENT_BUFFER_LVALUE->yy_n_chars;
+                        YY_CURRENT_BUFFER_LVALUE->yy_input_file = yyin;
+                        YY_CURRENT_BUFFER_LVALUE->yy_buffer_status = YY_BUFFER_NORMAL;
+                        }
+
+                /* Note that here we test for yy_c_buf_p "<=" to the position
+                 * of the first EOB in the buffer, since yy_c_buf_p will
+                 * already have been incremented past the NUL character
+                 * (since all states make transitions on EOB to the
+                 * end-of-buffer state).  Contrast this with the test
+                 * in input().
+                 */
+                if ( yyg->yy_c_buf_p <= &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars] )
+                        { /* This was really a NUL. */
+                        yy_state_type yy_next_state;
+
+                        yyg->yy_c_buf_p = yyg->yytext_ptr + yy_amount_of_matched_text;
+
+                        yy_current_state = yy_get_previous_state( yyscanner );
+
+                        /* Okay, we're now positioned to make the NUL
+                         * transition.  We couldn't have
+                         * yy_get_previous_state() go ahead and do it
+                         * for us because it doesn't know how to deal
+                         * with the possibility of jamming (and we don't
+                         * want to build jamming into it because then it
+                         * will run more slowly).
+                         */
+
+                        yy_next_state = yy_try_NUL_trans( yy_current_state , yyscanner);
+
+                        yy_bp = yyg->yytext_ptr + YY_MORE_ADJ;
+
+                        if ( yy_next_state )
+                                {
+                                /* Consume the NUL. */
+                                yy_cp = ++yyg->yy_c_buf_p;
+                                yy_current_state = yy_next_state;
+                                goto yy_match;
+                                }
+
+                        else
+                                {
+                                yy_cp = yyg->yy_c_buf_p;
+                                goto yy_find_action;
+                                }
+                        }
+
+                else switch ( yy_get_next_buffer( yyscanner ) )
+                        {
+                        case EOB_ACT_END_OF_FILE:
+                                {
+                                yyg->yy_did_buffer_switch_on_eof = 0;
+
+                                if ( cmCommandArgument_yywrap(yyscanner ) )
+                                        {
+                                        /* Note: because we've taken care in
+                                         * yy_get_next_buffer() to have set up
+                                         * yytext, we can now set up
+                                         * yy_c_buf_p so that if some total
+                                         * hoser (like flex itself) wants to
+                                         * call the scanner after we return the
+                                         * YY_NULL, it'll still work - another
+                                         * YY_NULL will get returned.
+                                         */
+                                        yyg->yy_c_buf_p = yyg->yytext_ptr + YY_MORE_ADJ;
+
+                                        yy_act = YY_STATE_EOF(YY_START);
+                                        goto do_action;
+                                        }
+
+                                else
+                                        {
+                                        if ( ! yyg->yy_did_buffer_switch_on_eof )
+                                                YY_NEW_FILE;
+                                        }
+                                break;
+                                }
+
+                        case EOB_ACT_CONTINUE_SCAN:
+                                yyg->yy_c_buf_p =
+                                        yyg->yytext_ptr + yy_amount_of_matched_text;
+
+                                yy_current_state = yy_get_previous_state( yyscanner );
+
+                                yy_cp = yyg->yy_c_buf_p;
+                                yy_bp = yyg->yytext_ptr + YY_MORE_ADJ;
+                                goto yy_match;
+
+                        case EOB_ACT_LAST_MATCH:
+                                yyg->yy_c_buf_p =
+                                &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars];
+
+                                yy_current_state = yy_get_previous_state( yyscanner );
+
+                                yy_cp = yyg->yy_c_buf_p;
+                                yy_bp = yyg->yytext_ptr + YY_MORE_ADJ;
+                                goto yy_find_action;
+                        }
+                break;
+                }
+
+        default:
+                YY_FATAL_ERROR(
+                        "fatal flex scanner internal error--no action found" );
+        } /* end of action switch */
+                } /* end of scanning one token */
 } /* end of cmCommandArgument_yylex */
 
 /* yy_get_next_buffer - try to read in a new buffer
  *
  * Returns a code representing an action:
- *  EOB_ACT_LAST_MATCH -
- *  EOB_ACT_CONTINUE_SCAN - continue scanning from current position
- *  EOB_ACT_END_OF_FILE - end of file
+ *      EOB_ACT_LAST_MATCH -
+ *      EOB_ACT_CONTINUE_SCAN - continue scanning from current position
+ *      EOB_ACT_END_OF_FILE - end of file
  */
 static int yy_get_next_buffer (yyscan_t yyscanner)
 {
     struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
-  register char *dest = YY_CURRENT_BUFFER_LVALUE->yy_ch_buf;
-  register char *source = yyg->yytext_ptr;
-  register int number_to_move, i;
-  int ret_val;
-
-  if ( yyg->yy_c_buf_p > &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars + 1] )
-    YY_FATAL_ERROR(
-    "fatal flex scanner internal error--end of buffer missed" );
-
-  if ( YY_CURRENT_BUFFER_LVALUE->yy_fill_buffer == 0 )
-    { /* Don't try to fill the buffer, so this is an EOF. */
-    if ( yyg->yy_c_buf_p - yyg->yytext_ptr - YY_MORE_ADJ == 1 )
-      {
-      /* We matched a single character, the EOB, so
-       * treat this as a final EOF.
-       */
-      return EOB_ACT_END_OF_FILE;
-      }
-
-    else
-      {
-      /* We matched some text prior to the EOB, first
-       * process it.
-       */
-      return EOB_ACT_LAST_MATCH;
-      }
-    }
+        register char *dest = YY_CURRENT_BUFFER_LVALUE->yy_ch_buf;
+        register char *source = yyg->yytext_ptr;
+        register int number_to_move, i;
+        int ret_val;
+
+        if ( yyg->yy_c_buf_p > &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars + 1] )
+                YY_FATAL_ERROR(
+                "fatal flex scanner internal error--end of buffer missed" );
+
+        if ( YY_CURRENT_BUFFER_LVALUE->yy_fill_buffer == 0 )
+                { /* Don't try to fill the buffer, so this is an EOF. */
+                if ( yyg->yy_c_buf_p - yyg->yytext_ptr - YY_MORE_ADJ == 1 )
+                        {
+                        /* We matched a single character, the EOB, so
+                         * treat this as a final EOF.
+                         */
+                        return EOB_ACT_END_OF_FILE;
+                        }
+
+                else
+                        {
+                        /* We matched some text prior to the EOB, first
+                         * process it.
+                         */
+                        return EOB_ACT_LAST_MATCH;
+                        }
+                }
+
+        /* Try to read more data. */
+
+        /* First move last chars to start of buffer. */
+        number_to_move = (int) (yyg->yy_c_buf_p - yyg->yytext_ptr) - 1;
+
+        for ( i = 0; i < number_to_move; ++i )
+                *(dest++) = *(source++);
+
+        if ( YY_CURRENT_BUFFER_LVALUE->yy_buffer_status == YY_BUFFER_EOF_PENDING )
+                /* don't do the read, it's not guaranteed to return an EOF,
+                 * just force an EOF
+                 */
+                YY_CURRENT_BUFFER_LVALUE->yy_n_chars = yyg->yy_n_chars = 0;
 
-  /* Try to read more data. */
-
-  /* First move last chars to start of buffer. */
-  number_to_move = (int) (yyg->yy_c_buf_p - yyg->yytext_ptr) - 1;
-
-  for ( i = 0; i < number_to_move; ++i )
-    *(dest++) = *(source++);
-
-  if ( YY_CURRENT_BUFFER_LVALUE->yy_buffer_status == YY_BUFFER_EOF_PENDING )
-    /* don't do the read, it's not guaranteed to return an EOF,
-     * just force an EOF
-     */
-    YY_CURRENT_BUFFER_LVALUE->yy_n_chars = yyg->yy_n_chars = 0;
+        else
+                {
+                        int num_to_read =
+                        YY_CURRENT_BUFFER_LVALUE->yy_buf_size - number_to_move - 1;
 
-  else
-    {
-      size_t nuto_read =
-      YY_CURRENT_BUFFER_LVALUE->yy_buf_size - number_to_move - 1;
+                while ( num_to_read <= 0 )
+                        { /* Not enough room in the buffer - grow it. */
 
-    while ( nuto_read <= 0 )
-      { /* Not enough room in the buffer - grow it. */
+                        /* just a shorter name for the current buffer */
+                        YY_BUFFER_STATE b = YY_CURRENT_BUFFER;
 
-      /* just a shorter name for the current buffer */
-      YY_BUFFER_STATE b = YY_CURRENT_BUFFER;
+                        int yy_c_buf_p_offset =
+                                (int) (yyg->yy_c_buf_p - b->yy_ch_buf);
 
-      int yy_c_buf_p_offset =
-        (int) (yyg->yy_c_buf_p - b->yy_ch_buf);
+                        if ( b->yy_is_our_buffer )
+                                {
+                                int new_size = b->yy_buf_size * 2;
 
-      if ( b->yy_is_our_buffer )
-        {
-        int new_size = b->yy_buf_size * 2;
+                                if ( new_size <= 0 )
+                                        b->yy_buf_size += b->yy_buf_size / 8;
+                                else
+                                        b->yy_buf_size *= 2;
 
-        if ( new_size <= 0 )
-          b->yy_buf_size += b->yy_buf_size / 8;
-        else
-          b->yy_buf_size *= 2;
+                                b->yy_ch_buf = (char *)
+                                        /* Include room in for 2 EOB chars. */
+                                        cmCommandArgument_yyrealloc((void *) b->yy_ch_buf,b->yy_buf_size + 2 ,yyscanner );
+                                }
+                        else
+                                /* Can't grow it, we don't own it. */
+                                b->yy_ch_buf = 0;
 
-        b->yy_ch_buf = (char *)
-          /* Include room in for 2 EOB chars. */
-          cmCommandArgument_yyrealloc((void *) b->yy_ch_buf,b->yy_buf_size + 2 ,yyscanner );
-        }
-      else
-        /* Can't grow it, we don't own it. */
-        b->yy_ch_buf = 0;
+                        if ( ! b->yy_ch_buf )
+                                YY_FATAL_ERROR(
+                                "fatal error - scanner input buffer overflow" );
 
-      if ( ! b->yy_ch_buf )
-        YY_FATAL_ERROR(
-        "fatal error - scanner input buffer overflow" );
+                        yyg->yy_c_buf_p = &b->yy_ch_buf[yy_c_buf_p_offset];
 
-      yyg->yy_c_buf_p = &b->yy_ch_buf[yy_c_buf_p_offset];
+                        num_to_read = YY_CURRENT_BUFFER_LVALUE->yy_buf_size -
+                                                number_to_move - 1;
 
-      nuto_read = YY_CURRENT_BUFFER_LVALUE->yy_buf_size -
-            number_to_move - 1;
+                        }
 
-      }
+                if ( num_to_read > YY_READ_BUF_SIZE )
+                        num_to_read = YY_READ_BUF_SIZE;
 
-    if ( nuto_read > YY_READ_BUF_SIZE )
-      nuto_read = YY_READ_BUF_SIZE;
+                /* Read in more data. */
+                YY_INPUT( (&YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[number_to_move]),
+                        yyg->yy_n_chars, (size_t) num_to_read );
 
-    /* Read in more data. */
-    YY_INPUT( (&YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[number_to_move]),
-      yyg->yy_n_chars, nuto_read );
+                YY_CURRENT_BUFFER_LVALUE->yy_n_chars = yyg->yy_n_chars;
+                }
 
-    YY_CURRENT_BUFFER_LVALUE->yy_n_chars = yyg->yy_n_chars;
-    }
+        if ( yyg->yy_n_chars == 0 )
+                {
+                if ( number_to_move == YY_MORE_ADJ )
+                        {
+                        ret_val = EOB_ACT_END_OF_FILE;
+                        cmCommandArgument_yyrestart(yyin  ,yyscanner);
+                        }
 
-  if ( yyg->yy_n_chars == 0 )
-    {
-    if ( number_to_move == YY_MORE_ADJ )
-      {
-      ret_val = EOB_ACT_END_OF_FILE;
-      cmCommandArgument_yyrestart(yyin  ,yyscanner);
-      }
-
-    else
-      {
-      ret_val = EOB_ACT_LAST_MATCH;
-      YY_CURRENT_BUFFER_LVALUE->yy_buffer_status =
-        YY_BUFFER_EOF_PENDING;
-      }
-    }
+                else
+                        {
+                        ret_val = EOB_ACT_LAST_MATCH;
+                        YY_CURRENT_BUFFER_LVALUE->yy_buffer_status =
+                                YY_BUFFER_EOF_PENDING;
+                        }
+                }
 
-  else
-    ret_val = EOB_ACT_CONTINUE_SCAN;
+        else
+                ret_val = EOB_ACT_CONTINUE_SCAN;
 
-  yyg->yy_n_chars += number_to_move;
-  YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars] = YY_END_OF_BUFFER_CHAR;
-  YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars + 1] = YY_END_OF_BUFFER_CHAR;
+        yyg->yy_n_chars += number_to_move;
+        YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars] = YY_END_OF_BUFFER_CHAR;
+        YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars + 1] = YY_END_OF_BUFFER_CHAR;
 
-  yyg->yytext_ptr = &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[0];
+        yyg->yytext_ptr = &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[0];
 
-  return ret_val;
+        return ret_val;
 }
 
-/* yy_get_previous_state - get the state just before the EOB char was
-   reached */
+/* yy_get_previous_state - get the state just before the EOB char was reached */
 
     static yy_state_type yy_get_previous_state (yyscan_t yyscanner)
 {
-  register yy_state_type yy_current_state;
-  register char *yy_cp;
+        register yy_state_type yy_current_state;
+        register char *yy_cp;
     struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
 
-  yy_current_state = yyg->yy_start;
-
-  for ( yy_cp = yyg->yytext_ptr + YY_MORE_ADJ; yy_cp < yyg->yy_c_buf_p; ++yy_cp )
-    {
-    register YY_CHAR yy_c = (*yy_cp ? yy_ec[YY_SC_TO_UI(*yy_cp)] : 1);
-    if ( yy_accept[yy_current_state] )
-      {
-      yyg->yy_last_accepting_state = yy_current_state;
-      yyg->yy_last_accepting_cpos = yy_cp;
-      }
-    while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
-      {
-      yy_current_state = (int) yy_def[yy_current_state];
-      if ( yy_current_state >= 20 )
-        yy_c = yy_meta[(unsigned int) yy_c];
-      }
-    yy_current_state = yy_nxt[yy_base[yy_current_state] + (unsigned int) yy_c];
-    }
-
-  return yy_current_state;
+        yy_current_state = yyg->yy_start;
+
+        for ( yy_cp = yyg->yytext_ptr + YY_MORE_ADJ; yy_cp < yyg->yy_c_buf_p; ++yy_cp )
+                {
+                register YY_CHAR yy_c = (*yy_cp ? yy_ec[YY_SC_TO_UI(*yy_cp)] : 1);
+                if ( yy_accept[yy_current_state] )
+                        {
+                        yyg->yy_last_accepting_state = yy_current_state;
+                        yyg->yy_last_accepting_cpos = yy_cp;
+                        }
+                while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
+                        {
+                        yy_current_state = (int) yy_def[yy_current_state];
+                        if ( yy_current_state >= 20 )
+                                yy_c = yy_meta[(unsigned int) yy_c];
+                        }
+                yy_current_state = yy_nxt[yy_base[yy_current_state] + (unsigned int) yy_c];
+                }
+
+        return yy_current_state;
 }
 
 /* yy_try_NUL_trans - try to make a transition on the NUL character
  *
  * synopsis
- *  next_state = yy_try_NUL_trans( current_state );
+ *      next_state = yy_try_NUL_trans( current_state );
  */
     static yy_state_type yy_try_NUL_trans  (yy_state_type yy_current_state , yyscan_t yyscanner)
 {
-  register int yy_is_jam;
+        register int yy_is_jam;
+    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner; /* This var may be unused depending upon options. */
+        register char *yy_cp = yyg->yy_c_buf_p;
+
+        register YY_CHAR yy_c = 1;
+        if ( yy_accept[yy_current_state] )
+                {
+                yyg->yy_last_accepting_state = yy_current_state;
+                yyg->yy_last_accepting_cpos = yy_cp;
+                }
+        while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
+                {
+                yy_current_state = (int) yy_def[yy_current_state];
+                if ( yy_current_state >= 20 )
+                        yy_c = yy_meta[(unsigned int) yy_c];
+                }
+        yy_current_state = yy_nxt[yy_base[yy_current_state] + (unsigned int) yy_c];
+        yy_is_jam = (yy_current_state == 19);
+
+        return yy_is_jam ? 0 : yy_current_state;
+}
+
+    static void yyunput (int c, register char * yy_bp , yyscan_t yyscanner)
+{
+        register char *yy_cp;
     struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
-  register char *yy_cp = yyg->yy_c_buf_p;
 
-  register YY_CHAR yy_c = 1;
-  if ( yy_accept[yy_current_state] )
-    {
-    yyg->yy_last_accepting_state = yy_current_state;
-    yyg->yy_last_accepting_cpos = yy_cp;
-    }
-  while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
-    {
-    yy_current_state = (int) yy_def[yy_current_state];
-    if ( yy_current_state >= 20 )
-      yy_c = yy_meta[(unsigned int) yy_c];
-    }
-  yy_current_state = yy_nxt[yy_base[yy_current_state] + (unsigned int) yy_c];
-  yy_is_jam = (yy_current_state == 19);
+    yy_cp = yyg->yy_c_buf_p;
+
+        /* undo effects of setting up yytext */
+        *yy_cp = yyg->yy_hold_char;
+
+        if ( yy_cp < YY_CURRENT_BUFFER_LVALUE->yy_ch_buf + 2 )
+                { /* need to shift things up to make room */
+                /* +2 for EOB chars. */
+                register int number_to_move = yyg->yy_n_chars + 2;
+                register char *dest = &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[
+                                        YY_CURRENT_BUFFER_LVALUE->yy_buf_size + 2];
+                register char *source =
+                                &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[number_to_move];
+
+                while ( source > YY_CURRENT_BUFFER_LVALUE->yy_ch_buf )
+                        *--dest = *--source;
 
-  return yy_is_jam ? 0 : yy_current_state;
+                yy_cp += (int) (dest - source);
+                yy_bp += (int) (dest - source);
+                YY_CURRENT_BUFFER_LVALUE->yy_n_chars =
+                        yyg->yy_n_chars = YY_CURRENT_BUFFER_LVALUE->yy_buf_size;
+
+                if ( yy_cp < YY_CURRENT_BUFFER_LVALUE->yy_ch_buf + 2 )
+                        YY_FATAL_ERROR( "flex scanner push-back overflow" );
+                }
+
+        *--yy_cp = (char) c;
+
+        yyg->yytext_ptr = yy_bp;
+        yyg->yy_hold_char = *yy_cp;
+        yyg->yy_c_buf_p = yy_cp;
 }
 
 #ifndef YY_NO_INPUT
@@ -1222,70 +1268,70 @@ static int yy_get_next_buffer (yyscan_t yyscanner)
 #endif
 
 {
-  int c;
+        int c;
     struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
 
-  *yyg->yy_c_buf_p = yyg->yy_hold_char;
-
-  if ( *yyg->yy_c_buf_p == YY_END_OF_BUFFER_CHAR )
-    {
-    /* yy_c_buf_p now points to the character we want to return.
-     * If this occurs *before* the EOB characters, then it's a
-     * valid NUL; if not, then we've hit the end of the buffer.
-     */
-    if ( yyg->yy_c_buf_p < &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars] )
-      /* This was really a NUL. */
-      *yyg->yy_c_buf_p = '\0';
-
-    else
-      { /* need more input */
-      int offset = yyg->yy_c_buf_p - yyg->yytext_ptr;
-      ++yyg->yy_c_buf_p;
-
-      switch ( yy_get_next_buffer( yyscanner ) )
-        {
-        case EOB_ACT_LAST_MATCH:
-          /* This happens because yy_g_n_b()
-           * sees that we've accumulated a
-           * token and flags that we need to
-           * try matching the token before
-           * proceeding.  But for input(),
-           * there's no matching to consider.
-           * So convert the EOB_ACT_LAST_MATCH
-           * to EOB_ACT_END_OF_FILE.
-           */
-
-          /* Reset buffer status. */
-          cmCommandArgument_yyrestart(yyin ,yyscanner);
-
-          /*FALLTHROUGH*/
-
-        case EOB_ACT_END_OF_FILE:
-          {
-          if ( cmCommandArgument_yywrap(yyscanner ) )
-            return EOF;
-
-          if ( ! yyg->yy_did_buffer_switch_on_eof )
-            YY_NEW_FILE;
+        *yyg->yy_c_buf_p = yyg->yy_hold_char;
+
+        if ( *yyg->yy_c_buf_p == YY_END_OF_BUFFER_CHAR )
+                {
+                /* yy_c_buf_p now points to the character we want to return.
+                 * If this occurs *before* the EOB characters, then it's a
+                 * valid NUL; if not, then we've hit the end of the buffer.
+                 */
+                if ( yyg->yy_c_buf_p < &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars] )
+                        /* This was really a NUL. */
+                        *yyg->yy_c_buf_p = '\0';
+
+                else
+                        { /* need more input */
+                        int offset = yyg->yy_c_buf_p - yyg->yytext_ptr;
+                        ++yyg->yy_c_buf_p;
+
+                        switch ( yy_get_next_buffer( yyscanner ) )
+                                {
+                                case EOB_ACT_LAST_MATCH:
+                                        /* This happens because yy_g_n_b()
+                                         * sees that we've accumulated a
+                                         * token and flags that we need to
+                                         * try matching the token before
+                                         * proceeding.  But for input(),
+                                         * there's no matching to consider.
+                                         * So convert the EOB_ACT_LAST_MATCH
+                                         * to EOB_ACT_END_OF_FILE.
+                                         */
+
+                                        /* Reset buffer status. */
+                                        cmCommandArgument_yyrestart(yyin ,yyscanner);
+
+                                        /*FALLTHROUGH*/
+
+                                case EOB_ACT_END_OF_FILE:
+                                        {
+                                        if ( cmCommandArgument_yywrap(yyscanner ) )
+                                                return EOF;
+
+                                        if ( ! yyg->yy_did_buffer_switch_on_eof )
+                                                YY_NEW_FILE;
 #ifdef __cplusplus
-          return yyinput(yyscanner);
+                                        return yyinput(yyscanner);
 #else
-          return input(yyscanner);
+                                        return input(yyscanner);
 #endif
-          }
+                                        }
 
-        case EOB_ACT_CONTINUE_SCAN:
-          yyg->yy_c_buf_p = yyg->yytext_ptr + offset;
-          break;
-        }
-      }
-    }
+                                case EOB_ACT_CONTINUE_SCAN:
+                                        yyg->yy_c_buf_p = yyg->yytext_ptr + offset;
+                                        break;
+                                }
+                        }
+                }
 
-  c = *(unsigned char *) yyg->yy_c_buf_p;  /* cast for 8-bit char's */
-  *yyg->yy_c_buf_p = '\0';  /* preserve yytext */
-  yyg->yy_hold_char = *++yyg->yy_c_buf_p;
+        c = *(unsigned char *) yyg->yy_c_buf_p; /* cast for 8-bit char's */
+        *yyg->yy_c_buf_p = '\0';        /* preserve yytext */
+        yyg->yy_hold_char = *++yyg->yy_c_buf_p;
 
-  return c;
+        return c;
 }
 #endif  /* ifndef YY_NO_INPUT */
 
@@ -1298,14 +1344,14 @@ static int yy_get_next_buffer (yyscan_t yyscanner)
 {
     struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
 
-  if ( ! YY_CURRENT_BUFFER ){
+        if ( ! YY_CURRENT_BUFFER ){
         cmCommandArgument_yyensure_buffer_stack (yyscanner);
-    YY_CURRENT_BUFFER_LVALUE =
+                YY_CURRENT_BUFFER_LVALUE =
             cmCommandArgument_yy_create_buffer(yyin,YY_BUF_SIZE ,yyscanner);
-  }
+        }
 
-  cmCommandArgument_yy_init_buffer(YY_CURRENT_BUFFER,input_file ,yyscanner);
-  cmCommandArgument_yy_load_buffer_state(yyscanner );
+        cmCommandArgument_yy_init_buffer(YY_CURRENT_BUFFER,input_file ,yyscanner);
+        cmCommandArgument_yy_load_buffer_state(yyscanner );
 }
 
 /** Switch to a different input buffer.
@@ -1316,70 +1362,71 @@ static int yy_get_next_buffer (yyscan_t yyscanner)
 {
     struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
 
-  /* TODO. We should be able to replace this entire function body
-   * with
-   *    cmCommandArgument_yypop_buffer_state();
-   *    cmCommandArgument_yypush_buffer_state(new_buffer);
+        /* TODO. We should be able to replace this entire function body
+         * with
+         *              cmCommandArgument_yypop_buffer_state();
+         *              cmCommandArgument_yypush_buffer_state(new_buffer);
      */
-  cmCommandArgument_yyensure_buffer_stack (yyscanner);
-  if ( YY_CURRENT_BUFFER == new_buffer )
-    return;
-
-  if ( YY_CURRENT_BUFFER )
-    {
-    /* Flush out information for old buffer. */
-    *yyg->yy_c_buf_p = yyg->yy_hold_char;
-    YY_CURRENT_BUFFER_LVALUE->yy_buf_pos = yyg->yy_c_buf_p;
-    YY_CURRENT_BUFFER_LVALUE->yy_n_chars = yyg->yy_n_chars;
-    }
-
-  YY_CURRENT_BUFFER_LVALUE = new_buffer;
-  cmCommandArgument_yy_load_buffer_state(yyscanner );
-
-  /* We don't actually know whether we did this switch during EOF
-   * (cmCommandArgument_yywrap()) processing, but the only time this flag is
-   * looked at is after cmCommandArgument_yywrap() is called, so it's safe to
-   * go ahead and always set it.
-   */
-  yyg->yy_did_buffer_switch_on_eof = 1;
+        cmCommandArgument_yyensure_buffer_stack (yyscanner);
+        if ( YY_CURRENT_BUFFER == new_buffer )
+                return;
+
+        if ( YY_CURRENT_BUFFER )
+                {
+                /* Flush out information for old buffer. */
+                *yyg->yy_c_buf_p = yyg->yy_hold_char;
+                YY_CURRENT_BUFFER_LVALUE->yy_buf_pos = yyg->yy_c_buf_p;
+                YY_CURRENT_BUFFER_LVALUE->yy_n_chars = yyg->yy_n_chars;
+                }
+
+        YY_CURRENT_BUFFER_LVALUE = new_buffer;
+        cmCommandArgument_yy_load_buffer_state(yyscanner );
+
+        /* We don't actually know whether we did this switch during
+         * EOF (cmCommandArgument_yywrap()) processing, but the only time this flag
+         * is looked at is after cmCommandArgument_yywrap() is called, so it's safe
+         * to go ahead and always set it.
+         */
+        yyg->yy_did_buffer_switch_on_eof = 1;
 }
 
 static void cmCommandArgument_yy_load_buffer_state  (yyscan_t yyscanner)
 {
     struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
-  yyg->yy_n_chars = YY_CURRENT_BUFFER_LVALUE->yy_n_chars;
-  yyg->yytext_ptr = yyg->yy_c_buf_p = YY_CURRENT_BUFFER_LVALUE->yy_buf_pos;
-  yyin = YY_CURRENT_BUFFER_LVALUE->yy_input_file;
-  yyg->yy_hold_char = *yyg->yy_c_buf_p;
+        yyg->yy_n_chars = YY_CURRENT_BUFFER_LVALUE->yy_n_chars;
+        yyg->yytext_ptr = yyg->yy_c_buf_p = YY_CURRENT_BUFFER_LVALUE->yy_buf_pos;
+        yyin = YY_CURRENT_BUFFER_LVALUE->yy_input_file;
+        yyg->yy_hold_char = *yyg->yy_c_buf_p;
 }
 
-/** Allocate and initialize an input buffer state.  @param file A readable
- * stream.  @param size The character buffer size in bytes. When in doubt,
- * use @c YY_BUF_SIZE.  @param yyscanner The scanner object.  @return the
- * allocated buffer state.
+/** Allocate and initialize an input buffer state.
+ * @param file A readable stream.
+ * @param size The character buffer size in bytes. When in doubt, use @c YY_BUF_SIZE.
+ * @param yyscanner The scanner object.
+ * @return the allocated buffer state.
  */
     YY_BUFFER_STATE cmCommandArgument_yy_create_buffer  (FILE * file, int  size , yyscan_t yyscanner)
 {
-  YY_BUFFER_STATE b;
+        YY_BUFFER_STATE b;
     
-  b = (YY_BUFFER_STATE) cmCommandArgument_yyalloc(sizeof( struct yy_buffer_state ) ,yyscanner );
-  if ( ! b )
-    YY_FATAL_ERROR( "out of dynamic memory in cmCommandArgument_yy_create_buffer()" );
+        b = (YY_BUFFER_STATE) cmCommandArgument_yyalloc(sizeof( struct yy_buffer_state ) ,yyscanner );
+        if ( ! b )
+                YY_FATAL_ERROR( "out of dynamic memory in cmCommandArgument_yy_create_buffer()" );
 
-  b->yy_buf_size = size;
+        b->yy_buf_size = size;
 
-  /* yy_ch_buf has to be 2 characters longer than the size given because
-   * we need to put in 2 end-of-buffer characters.
-   */
-  b->yy_ch_buf = (char *) cmCommandArgument_yyalloc(b->yy_buf_size + 2 ,yyscanner );
-  if ( ! b->yy_ch_buf )
-    YY_FATAL_ERROR( "out of dynamic memory in cmCommandArgument_yy_create_buffer()" );
+        /* yy_ch_buf has to be 2 characters longer than the size given because
+         * we need to put in 2 end-of-buffer characters.
+         */
+        b->yy_ch_buf = (char *) cmCommandArgument_yyalloc(b->yy_buf_size + 2 ,yyscanner );
+        if ( ! b->yy_ch_buf )
+                YY_FATAL_ERROR( "out of dynamic memory in cmCommandArgument_yy_create_buffer()" );
 
-  b->yy_is_our_buffer = 1;
+        b->yy_is_our_buffer = 1;
 
-  cmCommandArgument_yy_init_buffer(b,file ,yyscanner);
+        cmCommandArgument_yy_init_buffer(b,file ,yyscanner);
 
-  return b;
+        return b;
 }
 
 /** Destroy the buffer.
@@ -1390,16 +1437,16 @@ static void cmCommandArgument_yy_load_buffer_state  (yyscan_t yyscanner)
 {
     struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
 
-  if ( ! b )
-    return;
+        if ( ! b )
+                return;
 
-  if ( b == YY_CURRENT_BUFFER ) /* Not sure if we should pop here. */
-    YY_CURRENT_BUFFER_LVALUE = (YY_BUFFER_STATE) 0;
+        if ( b == YY_CURRENT_BUFFER ) /* Not sure if we should pop here. */
+                YY_CURRENT_BUFFER_LVALUE = (YY_BUFFER_STATE) 0;
 
-  if ( b->yy_is_our_buffer )
-    cmCommandArgument_yyfree((void *) b->yy_ch_buf ,yyscanner );
+        if ( b->yy_is_our_buffer )
+                cmCommandArgument_yyfree((void *) b->yy_ch_buf ,yyscanner );
 
-  cmCommandArgument_yyfree((void *) b ,yyscanner );
+        cmCommandArgument_yyfree((void *) b ,yyscanner );
 }
 
 #ifndef __cplusplus
@@ -1413,18 +1460,17 @@ extern int isatty (int );
     static void cmCommandArgument_yy_init_buffer  (YY_BUFFER_STATE  b, FILE * file , yyscan_t yyscanner)
 
 {
-  int oerrno = errno;
+        int oerrno = errno;
     struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
 
-  cmCommandArgument_yy_flush_buffer(b ,yyscanner);
+        cmCommandArgument_yy_flush_buffer(b ,yyscanner);
 
-  b->yy_input_file = file;
-  b->yy_fill_buffer = 1;
+        b->yy_input_file = file;
+        b->yy_fill_buffer = 1;
 
-    /* If b is the current buffer, then cmCommandArgument_yy_init_buffer was
-     * _probably_ called from cmCommandArgument_yyrestart() or through
-     * yy_get_next_buffer.  In that case, we don't want to reset the lineno
-     * or column.
+    /* If b is the current buffer, then cmCommandArgument_yy_init_buffer was _probably_
+     * called from cmCommandArgument_yyrestart() or through yy_get_next_buffer.
+     * In that case, we don't want to reset the lineno or column.
      */
     if (b != YY_CURRENT_BUFFER){
         b->yy_bs_lineno = 1;
@@ -1433,35 +1479,35 @@ extern int isatty (int );
 
         b->yy_is_interactive = file ? (isatty( fileno(file) ) > 0) : 0;
     
-  errno = oerrno;
+        errno = oerrno;
 }
 
-/** Discard all buffered characters. On the next scan, YY_INPUT will be
- * called.  @param b the buffer state to be flushed, usually @c
- * YY_CURRENT_BUFFER.  @param yyscanner The scanner object.
+/** Discard all buffered characters. On the next scan, YY_INPUT will be called.
+ * @param b the buffer state to be flushed, usually @c YY_CURRENT_BUFFER.
+ * @param yyscanner The scanner object.
  */
     void cmCommandArgument_yy_flush_buffer (YY_BUFFER_STATE  b , yyscan_t yyscanner)
 {
     struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
-  if ( ! b )
-    return;
+        if ( ! b )
+                return;
 
-  b->yy_n_chars = 0;
+        b->yy_n_chars = 0;
 
-  /* We always need two end-of-buffer characters.  The first causes
-   * a transition to the end-of-buffer state.  The second causes
-   * a jam in that state.
-   */
-  b->yy_ch_buf[0] = YY_END_OF_BUFFER_CHAR;
-  b->yy_ch_buf[1] = YY_END_OF_BUFFER_CHAR;
+        /* We always need two end-of-buffer characters.  The first causes
+         * a transition to the end-of-buffer state.  The second causes
+         * a jam in that state.
+         */
+        b->yy_ch_buf[0] = YY_END_OF_BUFFER_CHAR;
+        b->yy_ch_buf[1] = YY_END_OF_BUFFER_CHAR;
 
-  b->yy_buf_pos = &b->yy_ch_buf[0];
+        b->yy_buf_pos = &b->yy_ch_buf[0];
 
-  b->yy_at_bol = 1;
-  b->yy_buffer_status = YY_BUFFER_NEW;
+        b->yy_at_bol = 1;
+        b->yy_buffer_status = YY_BUFFER_NEW;
 
-  if ( b == YY_CURRENT_BUFFER )
-    cmCommandArgument_yy_load_buffer_state(yyscanner );
+        if ( b == YY_CURRENT_BUFFER )
+                cmCommandArgument_yy_load_buffer_state(yyscanner );
 }
 
 /** Pushes the new state onto the stack. The new state becomes
@@ -1473,28 +1519,28 @@ extern int isatty (int );
 void cmCommandArgument_yypush_buffer_state (YY_BUFFER_STATE new_buffer , yyscan_t yyscanner)
 {
     struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
-  if (new_buffer == NULL)
-    return;
-
-  cmCommandArgument_yyensure_buffer_stack(yyscanner);
-
-  /* This block is copied from cmCommandArgument_yy_switch_to_buffer. */
-  if ( YY_CURRENT_BUFFER )
-    {
-    /* Flush out information for old buffer. */
-    *yyg->yy_c_buf_p = yyg->yy_hold_char;
-    YY_CURRENT_BUFFER_LVALUE->yy_buf_pos = yyg->yy_c_buf_p;
-    YY_CURRENT_BUFFER_LVALUE->yy_n_chars = yyg->yy_n_chars;
-    }
-
-  /* Only push if top exists. Otherwise, replace top. */
-  if (YY_CURRENT_BUFFER)
-    yyg->yy_buffer_stack_top++;
-  YY_CURRENT_BUFFER_LVALUE = new_buffer;
-
-  /* copied from cmCommandArgument_yy_switch_to_buffer. */
-  cmCommandArgument_yy_load_buffer_state(yyscanner );
-  yyg->yy_did_buffer_switch_on_eof = 1;
+        if (new_buffer == NULL)
+                return;
+
+        cmCommandArgument_yyensure_buffer_stack(yyscanner);
+
+        /* This block is copied from cmCommandArgument_yy_switch_to_buffer. */
+        if ( YY_CURRENT_BUFFER )
+                {
+                /* Flush out information for old buffer. */
+                *yyg->yy_c_buf_p = yyg->yy_hold_char;
+                YY_CURRENT_BUFFER_LVALUE->yy_buf_pos = yyg->yy_c_buf_p;
+                YY_CURRENT_BUFFER_LVALUE->yy_n_chars = yyg->yy_n_chars;
+                }
+
+        /* Only push if top exists. Otherwise, replace top. */
+        if (YY_CURRENT_BUFFER)
+                yyg->yy_buffer_stack_top++;
+        YY_CURRENT_BUFFER_LVALUE = new_buffer;
+
+        /* copied from cmCommandArgument_yy_switch_to_buffer. */
+        cmCommandArgument_yy_load_buffer_state(yyscanner );
+        yyg->yy_did_buffer_switch_on_eof = 1;
 }
 
 /** Removes and deletes the top of the stack, if present.
@@ -1504,144 +1550,147 @@ void cmCommandArgument_yypush_buffer_state (YY_BUFFER_STATE new_buffer , yyscan_
 void cmCommandArgument_yypop_buffer_state (yyscan_t yyscanner)
 {
     struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
-  if (!YY_CURRENT_BUFFER)
-    return;
-
-  cmCommandArgument_yy_delete_buffer(YY_CURRENT_BUFFER ,yyscanner);
-  YY_CURRENT_BUFFER_LVALUE = NULL;
-  if (yyg->yy_buffer_stack_top > 0)
-    --yyg->yy_buffer_stack_top;
-
-  if (YY_CURRENT_BUFFER) {
-    cmCommandArgument_yy_load_buffer_state(yyscanner );
-    yyg->yy_did_buffer_switch_on_eof = 1;
-  }
+        if (!YY_CURRENT_BUFFER)
+                return;
+
+        cmCommandArgument_yy_delete_buffer(YY_CURRENT_BUFFER ,yyscanner);
+        YY_CURRENT_BUFFER_LVALUE = NULL;
+        if (yyg->yy_buffer_stack_top > 0)
+                --yyg->yy_buffer_stack_top;
+
+        if (YY_CURRENT_BUFFER) {
+                cmCommandArgument_yy_load_buffer_state(yyscanner );
+                yyg->yy_did_buffer_switch_on_eof = 1;
+        }
 }
 
 /* Allocates the stack if it does not exist.
  *  Guarantees space for at least one push.
  */
 static void cmCommandArgument_yyensure_buffer_stack (yyscan_t yyscanner)
 {
-  int nuto_alloc;
+        int num_to_alloc;
     struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
 
-  if (!yyg->yy_buffer_stack) {
+        if (!yyg->yy_buffer_stack) {
 
-    /* First allocation is just for 2 elements, since we don't know if this
-     * scanner will even need a stack. We use 2 instead of 1 to avoid an
-     * immediate realloc on the next call.
+                /* First allocation is just for 2 elements, since we don't know if this
+                 * scanner will even need a stack. We use 2 instead of 1 to avoid an
+                 * immediate realloc on the next call.
          */
-    nuto_alloc = 1;
-    yyg->yy_buffer_stack = (struct yy_buffer_state**)cmCommandArgument_yyalloc
-                (nuto_alloc * sizeof(struct yy_buffer_state*)
-                , yyscanner);
-    
-    memset(yyg->yy_buffer_stack, 0, nuto_alloc * sizeof(struct yy_buffer_state*));
-        
-    yyg->yy_buffer_stack_max = nuto_alloc;
-    yyg->yy_buffer_stack_top = 0;
-    return;
-  }
+                num_to_alloc = 1;
+                yyg->yy_buffer_stack = (struct yy_buffer_state**)cmCommandArgument_yyalloc
+                                                                (num_to_alloc * sizeof(struct yy_buffer_state*)
+                                                                , yyscanner);
+                
+                memset(yyg->yy_buffer_stack, 0, num_to_alloc * sizeof(struct yy_buffer_state*));
+                                
+                yyg->yy_buffer_stack_max = num_to_alloc;
+                yyg->yy_buffer_stack_top = 0;
+                return;
+        }
 
-  if (yyg->yy_buffer_stack_top >= (yyg->yy_buffer_stack_max) - 1){
+        if (yyg->yy_buffer_stack_top >= (yyg->yy_buffer_stack_max) - 1){
 
-    /* Increase the buffer to prepare for a possible push. */
-    int grow_size = 8 /* arbitrary grow size */;
+                /* Increase the buffer to prepare for a possible push. */
+                int grow_size = 8 /* arbitrary grow size */;
 
-    nuto_alloc = yyg->yy_buffer_stack_max + grow_size;
-    yyg->yy_buffer_stack = (struct yy_buffer_state**)cmCommandArgument_yyrealloc
-                (yyg->yy_buffer_stack,
-                nuto_alloc * sizeof(struct yy_buffer_state*)
-                , yyscanner);
+                num_to_alloc = yyg->yy_buffer_stack_max + grow_size;
+                yyg->yy_buffer_stack = (struct yy_buffer_state**)cmCommandArgument_yyrealloc
+                                                                (yyg->yy_buffer_stack,
+                                                                num_to_alloc * sizeof(struct yy_buffer_state*)
+                                                                , yyscanner);
 
-    /* zero only the new slots.*/
-    memset(yyg->yy_buffer_stack + yyg->yy_buffer_stack_max, 0, grow_size * sizeof(struct yy_buffer_state*));
-    yyg->yy_buffer_stack_max = nuto_alloc;
-  }
+                /* zero only the new slots.*/
+                memset(yyg->yy_buffer_stack + yyg->yy_buffer_stack_max, 0, grow_size * sizeof(struct yy_buffer_state*));
+                yyg->yy_buffer_stack_max = num_to_alloc;
+        }
 }
 
-/** Setup the input buffer state to scan directly from a user-specified
- * character buffer.  @param base the character buffer @param size the size
- * in bytes of the character buffer @param yyscanner The scanner object.
- * @return the newly allocated buffer state object.
+/** Setup the input buffer state to scan directly from a user-specified character buffer.
+ * @param base the character buffer
+ * @param size the size in bytes of the character buffer
+ * @param yyscanner The scanner object.
+ * @return the newly allocated buffer state object. 
  */
 YY_BUFFER_STATE cmCommandArgument_yy_scan_buffer  (char * base, yy_size_t  size , yyscan_t yyscanner)
 {
-  YY_BUFFER_STATE b;
+        YY_BUFFER_STATE b;
     
-  if ( size < 2 ||
-       base[size-2] != YY_END_OF_BUFFER_CHAR ||
-       base[size-1] != YY_END_OF_BUFFER_CHAR )
-    /* They forgot to leave room for the EOB's. */
-    return 0;
-
-  b = (YY_BUFFER_STATE) cmCommandArgument_yyalloc(sizeof( struct yy_buffer_state ) ,yyscanner );
-  if ( ! b )
-    YY_FATAL_ERROR( "out of dynamic memory in cmCommandArgument_yy_scan_buffer()" );
-
-  b->yy_buf_size = size - 2;  /* "- 2" to take care of EOB's */
-  b->yy_buf_pos = b->yy_ch_buf = base;
-  b->yy_is_our_buffer = 0;
-  b->yy_input_file = 0;
-  b->yy_n_chars = b->yy_buf_size;
-  b->yy_is_interactive = 0;
-  b->yy_at_bol = 1;
-  b->yy_fill_buffer = 0;
-  b->yy_buffer_status = YY_BUFFER_NEW;
-
-  cmCommandArgument_yy_switch_to_buffer(b ,yyscanner );
-
-  return b;
+        if ( size < 2 ||
+             base[size-2] != YY_END_OF_BUFFER_CHAR ||
+             base[size-1] != YY_END_OF_BUFFER_CHAR )
+                /* They forgot to leave room for the EOB's. */
+                return 0;
+
+        b = (YY_BUFFER_STATE) cmCommandArgument_yyalloc(sizeof( struct yy_buffer_state ) ,yyscanner );
+        if ( ! b )
+                YY_FATAL_ERROR( "out of dynamic memory in cmCommandArgument_yy_scan_buffer()" );
+
+        b->yy_buf_size = size - 2;      /* "- 2" to take care of EOB's */
+        b->yy_buf_pos = b->yy_ch_buf = base;
+        b->yy_is_our_buffer = 0;
+        b->yy_input_file = 0;
+        b->yy_n_chars = b->yy_buf_size;
+        b->yy_is_interactive = 0;
+        b->yy_at_bol = 1;
+        b->yy_fill_buffer = 0;
+        b->yy_buffer_status = YY_BUFFER_NEW;
+
+        cmCommandArgument_yy_switch_to_buffer(b ,yyscanner );
+
+        return b;
 }
 
-/** Setup the input buffer state to scan a string. The next call to
- * cmCommandArgument_yylex() will scan from a @e copy of @a str.  @param str
- * a NUL-terminated string to scan @param yyscanner The scanner object.
- * @return the newly allocated buffer state object.  @note If you want to
- * scan bytes that may contain NUL values, then use
- * cmCommandArgument_yy_scan_bytes() instead.
+/** Setup the input buffer state to scan a string. The next call to cmCommandArgument_yylex() will
+ * scan from a @e copy of @a str.
+ * @param yystr a NUL-terminated string to scan
+ * @param yyscanner The scanner object.
+ * @return the newly allocated buffer state object.
+ * @note If you want to scan bytes that may contain NUL values, then use
+ *       cmCommandArgument_yy_scan_bytes() instead.
  */
-YY_BUFFER_STATE cmCommandArgument_yy_scan_string (yyconst char * yy_str , yyscan_t yyscanner)
+YY_BUFFER_STATE cmCommandArgument_yy_scan_string (yyconst char * yystr , yyscan_t yyscanner)
 {
     
-  return cmCommandArgument_yy_scan_bytes(yy_str,strlen(yy_str) ,yyscanner);
+        return cmCommandArgument_yy_scan_bytes(yystr,strlen(yystr) ,yyscanner);
 }
 
-/** Setup the input buffer state to scan the given bytes. The next call to
- * cmCommandArgument_yylex() will scan from a @e copy of @a bytes.  @param
- * bytes the byte buffer to scan @param len the number of bytes in the buffer
- * pointed to by @a bytes.  @param yyscanner The scanner object.  @return the
- * newly allocated buffer state object.
+/** Setup the input buffer state to scan the given bytes. The next call to cmCommandArgument_yylex() will
+ * scan from a @e copy of @a bytes.
+ * @param bytes the byte buffer to scan
+ * @param len the number of bytes in the buffer pointed to by @a bytes.
+ * @param yyscanner The scanner object.
+ * @return the newly allocated buffer state object.
  */
-YY_BUFFER_STATE cmCommandArgument_yy_scan_bytes  (yyconst char * bytes, int  len , yyscan_t yyscanner)
+YY_BUFFER_STATE cmCommandArgument_yy_scan_bytes  (yyconst char * yybytes, int  _yybytes_len , yyscan_t yyscanner)
 {
-  YY_BUFFER_STATE b;
-  char *buf;
-  yy_size_t n;
-  int i;
+        YY_BUFFER_STATE b;
+        char *buf;
+        yy_size_t n;
+        int i;
     
-  /* Get memory for full buffer, including space for trailing EOB's. */
-  n = len + 2;
-  buf = (char *) cmCommandArgument_yyalloc(n ,yyscanner );
-  if ( ! buf )
-    YY_FATAL_ERROR( "out of dynamic memory in cmCommandArgument_yy_scan_bytes()" );
+        /* Get memory for full buffer, including space for trailing EOB's. */
+        n = _yybytes_len + 2;
+        buf = (char *) cmCommandArgument_yyalloc(n ,yyscanner );
+        if ( ! buf )
+                YY_FATAL_ERROR( "out of dynamic memory in cmCommandArgument_yy_scan_bytes()" );
 
-  for ( i = 0; i < len; ++i )
-    buf[i] = bytes[i];
+        for ( i = 0; i < _yybytes_len; ++i )
+                buf[i] = yybytes[i];
 
-  buf[len] = buf[len+1] = YY_END_OF_BUFFER_CHAR;
+        buf[_yybytes_len] = buf[_yybytes_len+1] = YY_END_OF_BUFFER_CHAR;
 
-  b = cmCommandArgument_yy_scan_buffer(buf,n ,yyscanner);
-  if ( ! b )
-    YY_FATAL_ERROR( "bad buffer in cmCommandArgument_yy_scan_bytes()" );
+        b = cmCommandArgument_yy_scan_buffer(buf,n ,yyscanner);
+        if ( ! b )
+                YY_FATAL_ERROR( "bad buffer in cmCommandArgument_yy_scan_bytes()" );
 
-  /* It's okay to grow etc. this buffer, and we should throw it
-   * away when we're done.
-   */
-  b->yy_is_our_buffer = 1;
+        /* It's okay to grow etc. this buffer, and we should throw it
+         * away when we're done.
+         */
+        b->yy_is_our_buffer = 1;
 
-  return b;
+        return b;
 }
 
 #ifndef YY_EXIT_FAILURE
@@ -1650,26 +1699,26 @@ YY_BUFFER_STATE cmCommandArgument_yy_scan_bytes  (yyconst char * bytes, int  len
 
 static void yy_fatal_error (yyconst char* msg , yyscan_t)
 {
-      (void) fprintf( stderr, "%s\n", msg );
-  exit( YY_EXIT_FAILURE );
+        (void) fprintf( stderr, "%s\n", msg );
+        exit( YY_EXIT_FAILURE );
 }
 
 /* Redefine yyless() so it works in section 3 code. */
 
 #undef yyless
 #define yyless(n) \
-  do \
-    { \
-    /* Undo effects of setting up yytext. */ \
+        do \
+                { \
+                /* Undo effects of setting up yytext. */ \
         int yyless_macro_arg = (n); \
         YY_LESS_LINENO(yyless_macro_arg);\
-    yytext[yyleng] = yyg->yy_hold_char; \
-    yyg->yy_c_buf_p = yytext + yyless_macro_arg; \
-    yyg->yy_hold_char = *yyg->yy_c_buf_p; \
-    *yyg->yy_c_buf_p = '\0'; \
-    yyleng = yyless_macro_arg; \
-    } \
-  while ( 0 )
+                yytext[yyleng] = yyg->yy_hold_char; \
+                yyg->yy_c_buf_p = yytext + yyless_macro_arg; \
+                yyg->yy_hold_char = *yyg->yy_c_buf_p; \
+                *yyg->yy_c_buf_p = '\0'; \
+                yyleng = yyless_macro_arg; \
+                } \
+        while ( 0 )
 
 /* Accessor  methods (get/set functions) to struct members. */
 
@@ -1780,12 +1829,7 @@ void cmCommandArgument_yyset_column (int  column_no , yyscan_t yyscanner)
 
         /* column is only valid if an input buffer exists. */
         if (! YY_CURRENT_BUFFER )
-          {
-          yy_fatal_error
-            ("cmCommandArgument_yyset_column called with no buffer" , 
-             yyscanner); 
-          }
-        
+           yy_fatal_error( "cmCommandArgument_yyset_column called with no buffer" , yyscanner); 
     
     yycolumn = column_no;
 }
@@ -1822,21 +1866,51 @@ void cmCommandArgument_yyset_debug (int  bdebug , yyscan_t yyscanner)
 
 /* Accessor methods for yylval and yylloc */
 
+/* User-visible API */
+
+/* cmCommandArgument_yylex_init is special because it creates the scanner itself, so it is
+ * the ONLY reentrant function that doesn't take the scanner as the last argument.
+ * That's why we explicitly handle the declaration, instead of using our macros.
+ */
+
+int cmCommandArgument_yylex_init(yyscan_t* ptr_yy_globals)
+
+{
+    if (ptr_yy_globals == NULL){
+        errno = EINVAL;
+        return 1;
+    }
+
+    *ptr_yy_globals = (yyscan_t) cmCommandArgument_yyalloc ( sizeof( struct yyguts_t ), NULL );
+
+    if (*ptr_yy_globals == NULL){
+        errno = ENOMEM;
+        return 1;
+    }
+
+    /* By setting to 0xAA, we expose bugs in yy_init_globals. Leave at 0x00 for releases. */
+    memset(*ptr_yy_globals,0x00,sizeof(struct yyguts_t));
+
+    return yy_init_globals ( *ptr_yy_globals );
+}
+
 static int yy_init_globals (yyscan_t yyscanner)
 {
     struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
     /* Initialization is the same as for the non-reentrant scanner.
-       This function is called once per scanner lifetime. */
+     * This function is called from cmCommandArgument_yylex_destroy(), so don't allocate here.
+     */
 
     yyg->yy_buffer_stack = 0;
     yyg->yy_buffer_stack_top = 0;
     yyg->yy_buffer_stack_max = 0;
     yyg->yy_c_buf_p = (char *) 0;
-    yyg->yy_init = 1;
+    yyg->yy_init = 0;
     yyg->yy_start = 0;
+
     yyg->yy_start_stack_ptr = 0;
     yyg->yy_start_stack_depth = 0;
-    yyg->yy_start_stack = (int *) 0;
+    yyg->yy_start_stack =  NULL;
 
 /* Defined in main.c */
 #ifdef YY_STDINIT
@@ -1853,57 +1927,33 @@ static int yy_init_globals (yyscan_t yyscanner)
     return 0;
 }
 
-/* User-visible API */
-
-/* cmCommandArgument_yylex_init is special because it creates the scanner
- * itself, so it is the ONLY reentrant function that doesn't take the scanner
- * as the last argument.  That's why we explicitly handle the declaration,
- * instead of using our macros.
- */
-
-int cmCommandArgument_yylex_init(yyscan_t* ptr_yy_globals)
-
-{
-    if (ptr_yy_globals == NULL){
-        errno = EINVAL;
-        return 1;
-    }
-
-    *ptr_yy_globals = (yyscan_t) cmCommandArgument_yyalloc ( sizeof( struct yyguts_t ), NULL );
-
-    if (*ptr_yy_globals == NULL){
-        errno = ENOMEM;
-        return 1;
-    }
-
-    memset(*ptr_yy_globals,0,sizeof(struct yyguts_t));
-
-    return yy_init_globals ( *ptr_yy_globals );
-}
-
-/* cmCommandArgument_yylex_destroy is for both reentrant and non-reentrant
-   scanners. */
+/* cmCommandArgument_yylex_destroy is for both reentrant and non-reentrant scanners. */
 int cmCommandArgument_yylex_destroy  (yyscan_t yyscanner)
 {
     struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
 
     /* Pop the buffer stack, destroying each element. */
-  while(YY_CURRENT_BUFFER){
-    cmCommandArgument_yy_delete_buffer(YY_CURRENT_BUFFER ,yyscanner );
-    YY_CURRENT_BUFFER_LVALUE = NULL;
-    cmCommandArgument_yypop_buffer_state(yyscanner);
-  }
+        while(YY_CURRENT_BUFFER){
+                cmCommandArgument_yy_delete_buffer(YY_CURRENT_BUFFER ,yyscanner );
+                YY_CURRENT_BUFFER_LVALUE = NULL;
+                cmCommandArgument_yypop_buffer_state(yyscanner);
+        }
 
-  /* Destroy the stack itself. */
-  cmCommandArgument_yyfree(yyg->yy_buffer_stack ,yyscanner);
-  yyg->yy_buffer_stack = NULL;
+        /* Destroy the stack itself. */
+        cmCommandArgument_yyfree(yyg->yy_buffer_stack ,yyscanner);
+        yyg->yy_buffer_stack = NULL;
 
     /* Destroy the start condition stack. */
         cmCommandArgument_yyfree(yyg->yy_start_stack ,yyscanner );
         yyg->yy_start_stack = NULL;
 
+    /* Reset the globals. This is important in a non-reentrant scanner so the next time
+     * cmCommandArgument_yylex() is called, initialization will occur. */
+    yy_init_globals( yyscanner);
+
     /* Destroy the main struct (reentrant only). */
     cmCommandArgument_yyfree ( yyscanner , yyscanner );
+    yyscanner = NULL;
     return 0;
 }
 
@@ -1912,66 +1962,50 @@ int cmCommandArgument_yylex_destroy  (yyscan_t yyscanner)
  */
 
 #ifndef yytext_ptr
-static void yy_flex_strncpy (char* s1, yyconst char * s2, int n , 
-                             yyscan_t yyscanner)
+static void yy_flex_strncpy (char* s1, yyconst char * s2, int n , yyscan_t yyscanner)
 {
-  register int i;
-    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
-  for ( i = 0; i < n; ++i )
-    s1[i] = s2[i];
+        register int i;
+        for ( i = 0; i < n; ++i )
+                s1[i] = s2[i];
 }
 #endif
 
 #ifdef YY_NEED_STRLEN
 static int yy_flex_strlen (yyconst char * s , yyscan_t yyscanner)
 {
-  register int n;
-    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
-  for ( n = 0; s[n]; ++n )
-    ;
+        register int n;
+        for ( n = 0; s[n]; ++n )
+                ;
 
-  return n;
+        return n;
 }
 #endif
 
 void *cmCommandArgument_yyalloc (yy_size_t  size , yyscan_t)
 {
-  return (void *) malloc( size );
+        return (void *) malloc( size );
 }
 
 void *cmCommandArgument_yyrealloc  (void * ptr, yy_size_t  size , yyscan_t)
 {
-  /* The cast to (char *) in the following accommodates both
-   * implementations that use char* generic pointers, and those
-   * that use void* generic pointers.  It works with the latter
-   * because both ANSI C and C++ allow castless assignment from
-   * any pointer type to void*, and deal with argument conversions
-   * as though doing an assignment.
-   */
-  return (void *) realloc( (char *) ptr, size );
+        /* The cast to (char *) in the following accommodates both
+         * implementations that use char* generic pointers, and those
+         * that use void* generic pointers.  It works with the latter
+         * because both ANSI C and C++ allow castless assignment from
+         * any pointer type to void*, and deal with argument conversions
+         * as though doing an assignment.
+         */
+        return (void *) realloc( (char *) ptr, size );
 }
 
 void cmCommandArgument_yyfree (void * ptr , yyscan_t)
 {
-  free( (char *) ptr );  
-  /* see cmCommandArgument_yyrealloc() for (char *) cast */
+        free( (char *) ptr );   /* see cmCommandArgument_yyrealloc() for (char *) cast */
 }
 
 #define YYTABLES_NAME "yytables"
 
-#undef YY_NEW_FILE
-#undef YY_FLUSH_BUFFER
-#undef yy_set_bol
-#undef yy_new_buffer
-#undef yy_set_interactive
-#undef yytext_ptr
-#undef YY_DO_BEFORE_ACTION
-
-#ifdef YY_DECL_IS_OURS
-#undef YY_DECL_IS_OURS
-#undef YY_DECL
-#endif
-
+#line 133 "cmCommandArgumentLexer.in.l"
 
 
 