@@ -1,14 +1,5 @@
-/*============================================================================
-  KWSys - Kitware System Library
-  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium
-
-  Distributed under the OSI-approved BSD License (the "License");
-  see accompanying file Copyright.txt for details.
-
-  This software is distributed WITHOUT ANY WARRANTY; without even the
-  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
-  See the License for more information.
-============================================================================*/
+/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
+   file Copyright.txt or https://cmake.org/licensing#kwsys for details.  */
 #include "kwsysPrivate.h"
 #include KWSYS_HEADER(Configure.hxx)
 
@@ -19,24 +10,23 @@
 // Work-around CMake dependency scanning limitation.  This must
 // duplicate the above list of headers.
 #if 0
-# include "Configure.hxx.in"
-# include "IOStream.hxx.in"
+#include "Configure.hxx.in"
+#include "IOStream.hxx.in"
 #endif
 
 // Implement the rest of this file only if it is needed.
 #if KWSYS_IOS_NEED_OPERATORS_LL
 
-# include <stdio.h>  // sscanf, sprintf
-# include <string.h> // memchr
+#include <stdio.h>  // sscanf, sprintf
+#include <string.h> // memchr
 
-# if defined(_MAX_INT_DIG)
-#  define KWSYS_IOS_INT64_MAX_DIG _MAX_INT_DIG
-# else
-#  define KWSYS_IOS_INT64_MAX_DIG 32
-# endif
+#if defined(_MAX_INT_DIG)
+#define KWSYS_IOS_INT64_MAX_DIG _MAX_INT_DIG
+#else
+#define KWSYS_IOS_INT64_MAX_DIG 32
+#endif
 
-namespace KWSYS_NAMESPACE
-{
+namespace KWSYS_NAMESPACE {
 
 // Scan an input stream for an integer value.
 static int IOStreamScanStream(std::istream& is, char* buffer)
@@ -46,74 +36,68 @@ static int IOStreamScanStream(std::istream& is, char* buffer)
   char* end = buffer + KWSYS_IOS_INT64_MAX_DIG - 1;
 
   // Look for leading sign.
-  if(is.peek() == '+') { *out++ = '+'; is.ignore(); }
-  else if(is.peek() == '-') { *out++ = '-'; is.ignore(); }
+  if (is.peek() == '+') {
+    *out++ = '+';
+    is.ignore();
+  } else if (is.peek() == '-') {
+    *out++ = '-';
+    is.ignore();
+  }
 
   // Determine the base.  If not specified in the stream, try to
   // detect it from the input.  A leading 0x means hex, and a leading
   // 0 alone means octal.
   int base = 0;
   int flags = is.flags() & std::ios_base::basefield;
-  if(flags == std::ios_base::oct) { base = 8; }
-  else if(flags == std::ios_base::dec) { base = 10; }
-  else if(flags == std::ios_base::hex) { base = 16; }
+  if (flags == std::ios_base::oct) {
+    base = 8;
+  } else if (flags == std::ios_base::dec) {
+    base = 10;
+  } else if (flags == std::ios_base::hex) {
+    base = 16;
+  }
   bool foundDigit = false;
   bool foundNonZero = false;
-  if(is.peek() == '0')
-    {
+  if (is.peek() == '0') {
     foundDigit = true;
     is.ignore();
-    if((is.peek() == 'x' || is.peek() == 'X') && (base == 0 || base == 16))
-      {
+    if ((is.peek() == 'x' || is.peek() == 'X') && (base == 0 || base == 16)) {
       base = 16;
       foundDigit = false;
       is.ignore();
-      }
-    else if (base == 0)
-      {
+    } else if (base == 0) {
       base = 8;
-      }
     }
+  }
 
   // Determine the range of digits allowed for this number.
   const char* digits = "0123456789abcdefABCDEF";
   int maxDigitIndex = 10;
-  if(base == 8)
-    {
+  if (base == 8) {
     maxDigitIndex = 8;
-    }
-  else if(base == 16)
-    {
-    maxDigitIndex = 10+6+6;
-    }
+  } else if (base == 16) {
+    maxDigitIndex = 10 + 6 + 6;
+  }
 
   // Scan until an invalid digit is found.
-  for(;is.peek() != EOF; is.ignore())
-    {
-    if(memchr(digits, *out = (char)is.peek(), maxDigitIndex) != 0)
-      {
-      if((foundNonZero || *out != '0') && out < end)
-        {
+  for (; is.peek() != EOF; is.ignore()) {
+    if (memchr(digits, *out = (char)is.peek(), maxDigitIndex) != 0) {
+      if ((foundNonZero || *out != '0') && out < end) {
         ++out;
         foundNonZero = true;
-        }
-      foundDigit = true;
       }
-    else
-      {
+      foundDigit = true;
+    } else {
       break;
-      }
     }
+  }
 
   // Correct the buffer contents for degenerate cases.
-  if(foundDigit && !foundNonZero)
-    {
+  if (foundDigit && !foundNonZero) {
     *out++ = '0';
-    }
-  else if (!foundDigit)
-    {
+  } else if (!foundDigit) {
     out = buffer;
-    }
+  }
 
   // Terminate the string in the buffer.
   *out = '\0';
@@ -123,87 +107,109 @@ static int IOStreamScanStream(std::istream& is, char* buffer)
 
 // Read an integer value from an input stream.
 template <class T>
-std::istream&
-IOStreamScanTemplate(std::istream& is, T& value, char type)
+std::istream& IOStreamScanTemplate(std::istream& is, T& value, char type)
 {
   int state = std::ios_base::goodbit;
 
   // Skip leading whitespace.
   std::istream::sentry okay(is);
 
-  if(okay)
-    {
+  if (okay) {
     try {
-    // Copy the string to a buffer and construct the format string.
-    char buffer[KWSYS_IOS_INT64_MAX_DIG];
-#   if defined(_MSC_VER)
-    char format[] = "%I64_";
-    const int typeIndex = 4;
-#   else
-    char format[] = "%ll_";
-    const int typeIndex = 3;
-#   endif
-    switch(IOStreamScanStream(is, buffer))
-      {
-      case 8: format[typeIndex] = 'o'; break;
-      case 0: // Default to decimal if not told otherwise.
-      case 10: format[typeIndex] = type; break;
-      case 16: format[typeIndex] = 'x'; break;
+      // Copy the string to a buffer and construct the format string.
+      char buffer[KWSYS_IOS_INT64_MAX_DIG];
+#if defined(_MSC_VER)
+      char format[] = "%I64_";
+      const int typeIndex = 4;
+#else
+      char format[] = "%ll_";
+      const int typeIndex = 3;
+#endif
+      switch (IOStreamScanStream(is, buffer)) {
+        case 8:
+          format[typeIndex] = 'o';
+          break;
+        case 0: // Default to decimal if not told otherwise.
+        case 10:
+          format[typeIndex] = type;
+          break;
+        case 16:
+          format[typeIndex] = 'x';
+          break;
       };
 
-    // Use sscanf to parse the number from the buffer.
-    T result;
-    int success = (sscanf(buffer, format, &result) == 1)?1:0;
+      // Use sscanf to parse the number from the buffer.
+      T result;
+      int success = (sscanf(buffer, format, &result) == 1) ? 1 : 0;
 
-    // Set flags for resulting state.
-    if(is.peek() == EOF) { state |= std::ios_base::eofbit; }
-    if(!success) { state |= std::ios_base::failbit; }
-    else { value = result; }
-    } catch(...) { state |= std::ios_base::badbit; }
+      // Set flags for resulting state.
+      if (is.peek() == EOF) {
+        state |= std::ios_base::eofbit;
+      }
+      if (!success) {
+        state |= std::ios_base::failbit;
+      } else {
+        value = result;
+      }
+    } catch (...) {
+      state |= std::ios_base::badbit;
     }
+  }
 
   is.setstate(std::ios_base::iostate(state));
   return is;
 }
 
 // Print an integer value to an output stream.
 template <class T>
-std::ostream&
-IOStreamPrintTemplate(std::ostream& os, T value, char type)
+std::ostream& IOStreamPrintTemplate(std::ostream& os, T value, char type)
 {
   std::ostream::sentry okay(os);
-  if(okay)
-    {
+  if (okay) {
     try {
-    // Construct the format string.
-    char format[8];
-    char* f = format;
-    *f++ = '%';
-    if(os.flags() & std::ios_base::showpos) { *f++ = '+'; }
-    if(os.flags() & std::ios_base::showbase) { *f++ = '#'; }
-#   if defined(_MSC_VER)
-    *f++ = 'I'; *f++ = '6'; *f++ = '4';
-#   else
-    *f++ = 'l'; *f++ = 'l';
-#   endif
-    long bflags = os.flags() & std::ios_base::basefield;
-    if(bflags == std::ios_base::oct) { *f++ = 'o'; }
-    else if(bflags != std::ios_base::hex) { *f++ = type; }
-    else if(os.flags() & std::ios_base::uppercase) { *f++ = 'X'; }
-    else { *f++ = 'x'; }
-    *f = '\0';
-
-    // Use sprintf to print to a buffer and then write the
-    // buffer to the stream.
-    char buffer[2*KWSYS_IOS_INT64_MAX_DIG];
-    sprintf(buffer, format, value);
-    os << buffer;
-    } catch(...) { os.clear(os.rdstate() | std::ios_base::badbit); }
+      // Construct the format string.
+      char format[8];
+      char* f = format;
+      *f++ = '%';
+      if (os.flags() & std::ios_base::showpos) {
+        *f++ = '+';
+      }
+      if (os.flags() & std::ios_base::showbase) {
+        *f++ = '#';
+      }
+#if defined(_MSC_VER)
+      *f++ = 'I';
+      *f++ = '6';
+      *f++ = '4';
+#else
+      *f++ = 'l';
+      *f++ = 'l';
+#endif
+      long bflags = os.flags() & std::ios_base::basefield;
+      if (bflags == std::ios_base::oct) {
+        *f++ = 'o';
+      } else if (bflags != std::ios_base::hex) {
+        *f++ = type;
+      } else if (os.flags() & std::ios_base::uppercase) {
+        *f++ = 'X';
+      } else {
+        *f++ = 'x';
+      }
+      *f = '\0';
+
+      // Use sprintf to print to a buffer and then write the
+      // buffer to the stream.
+      char buffer[2 * KWSYS_IOS_INT64_MAX_DIG];
+      sprintf(buffer, format, value);
+      os << buffer;
+    } catch (...) {
+      os.clear(os.rdstate() | std::ios_base::badbit);
     }
+  }
   return os;
 }
 
-# if !KWSYS_IOS_HAS_ISTREAM_LONG_LONG
+#if !KWSYS_IOS_HAS_ISTREAM_LONG_LONG
 // Implement input stream operator for IOStreamSLL.
 std::istream& IOStreamScan(std::istream& is, IOStreamSLL& value)
 {
@@ -215,9 +221,9 @@ std::istream& IOStreamScan(std::istream& is, IOStreamULL& value)
 {
   return IOStreamScanTemplate(is, value, 'u');
 }
-# endif
+#endif
 
-# if !KWSYS_IOS_HAS_OSTREAM_LONG_LONG
+#if !KWSYS_IOS_HAS_OSTREAM_LONG_LONG
 // Implement output stream operator for IOStreamSLL.
 std::ostream& IOStreamPrint(std::ostream& os, IOStreamSLL value)
 {
@@ -229,14 +235,13 @@ std::ostream& IOStreamPrint(std::ostream& os, IOStreamULL value)
 {
   return IOStreamPrintTemplate(os, value, 'u');
 }
-# endif
+#endif
 
 } // namespace KWSYS_NAMESPACE
 
 #else
 
-namespace KWSYS_NAMESPACE
-{
+namespace KWSYS_NAMESPACE {
 
 // Create one public symbol in this object file to avoid warnings from
 // archivers.