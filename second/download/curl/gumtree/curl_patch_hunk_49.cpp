  *                                  _   _ ____  _     
  *  Project                     ___| | | |  _ \| |    
  *                             / __| | | | |_) | |    
  *                            | (__| |_| |  _ <| |___ 
  *                             \___|\___/|_| \_\_____|
  *
- * Copyright (C) 1998 - 2003, Daniel Stenberg, <daniel@haxx.se>, et al.
+ * Copyright (C) 1998 - 2004, Daniel Stenberg, <daniel@haxx.se>, et al.
  *
  * This software is licensed as described in the file COPYING, which
  * you should have received as part of this distribution. The terms
  * are also available at http://curl.haxx.se/docs/copyright.html.
  * 
  * You may opt to use, copy, modify, merge, publish, distribute and/or sell
  * copies of the Software, and permit persons to whom the Software is
  * furnished to do so, under the terms of the COPYING file.
  *
  * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
  * KIND, either express or implied.
  *
- * $Id: sendf.h,v 1.21 2003/01/16 21:08:13 bagder Exp $
+ * $Id: sendf.h,v 1.22 2004/01/07 09:19:35 bagder Exp $
  ***************************************************************************/
 
 CURLcode Curl_sendf(int fd, struct connectdata *, const char *fmt, ...);
 void Curl_infof(struct SessionHandle *, const char *fmt, ...);
 void Curl_failf(struct SessionHandle *, const char *fmt, ...);
 
