 /**
- * collectd - src/utils_cms_putnotif.c
- * Copyright (C) 2008  Florian octo Forster
+ * collectd - src/utils_cmd_putnotif.c
+ * Copyright (C) 2008       Florian octo Forster
  *
- * This program is free software; you can redistribute it and/or modify it
- * under the terms of the GNU General Public License as published by the
- * Free Software Foundation; only version 2 of the License is applicable.
+ * Permission is hereby granted, free of charge, to any person obtaining a
+ * copy of this software and associated documentation files (the "Software"),
+ * to deal in the Software without restriction, including without limitation
+ * the rights to use, copy, modify, merge, publish, distribute, sublicense,
+ * and/or sell copies of the Software, and to permit persons to whom the
+ * Software is furnished to do so, subject to the following conditions:
  *
- * This program is distributed in the hope that it will be useful, but
- * WITHOUT ANY WARRANTY; without even the implied warranty of
- * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
- * General Public License for more details.
+ * The above copyright notice and this permission notice shall be included in
+ * all copies or substantial portions of the Software.
  *
- * You should have received a copy of the GNU General Public License along
- * with this program; if not, write to the Free Software Foundation, Inc.,
- * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
+ * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
+ * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
+ * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
+ * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
+ * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
+ * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
+ * DEALINGS IN THE SOFTWARE.
  *
- * Author:
- *   Florian octo Forster <octo at verplant.org>
+ * Authors:
+ *   Florian octo Forster <octo at collectd.org>
  **/
 
 #include "collectd.h"
 #include "common.h"
 #include "plugin.h"
 
 #include "utils_parse_option.h"
-#include "utils_cmd_putnotif.h"
 
 #define print_to_socket(fh, ...) \
   do { \
     if (fprintf (fh, __VA_ARGS__) < 0) { \
       char errbuf[1024]; \
       WARNING ("handle_putnotif: failed to write to socket #%i: %s", \
