@@ -1,46 +1,98 @@
-#ifndef XGBOOST_UTILS_THREAD_H
-#define XGBOOST_UTILS_THREAD_H
 /*!
+ * Copyright by Contributors
  * \file thread.h
- * \brief this header include the minimum necessary resource for multi-threading
+ * \brief this header include the minimum necessary resource
+ * for multi-threading that can be compiled in windows, linux, mac
  * \author Tianqi Chen
- * Acknowledgement: this file is adapted from SVDFeature project, by same author. 
- *  The MAC support part of this code is provided by Artemy Kolchinsky
  */
+#ifndef XGBOOST_UTILS_THREAD_H_ // NOLINT(*)
+#define XGBOOST_UTILS_THREAD_H_ // NOLINT(*)
+
 #ifdef _MSC_VER
-#include "utils.h"
 #include <windows.h>
 #include <process.h>
+#include "./utils.h"
 namespace xgboost {
 namespace utils {
 /*! \brief simple semaphore used for synchronization */
 class Semaphore {
  public :
   inline void Init(int init_val) {
     sem = CreateSemaphore(NULL, init_val, 10, NULL);
-    utils::Assert(sem != NULL, "create Semaphore error");
+    utils::Check(sem != NULL, "create Semaphore error");
   }
   inline void Destroy(void) {
     CloseHandle(sem);
   }
   inline void Wait(void) {
-    utils::Assert(WaitForSingleObject(sem, INFINITE) == WAIT_OBJECT_0, "WaitForSingleObject error");
+    utils::Check(WaitForSingleObject(sem, INFINITE) == WAIT_OBJECT_0, "WaitForSingleObject error");
   }
   inline void Post(void) {
-    utils::Assert(ReleaseSemaphore(sem, 1, NULL)  != 0, "ReleaseSemaphore error");
+    utils::Check(ReleaseSemaphore(sem, 1, NULL) != 0, "ReleaseSemaphore error");
   }
+
  private:
   HANDLE sem;
 };
+
+/*! \brief mutex under windows */
+class Mutex {
+ public:
+  inline void Init(void) {
+    utils::Check(InitializeCriticalSectionAndSpinCount(&mutex, 0x00000400) != 0,
+                   "Mutex::Init fail");
+  }
+  inline void Lock(void) {
+    EnterCriticalSection(&mutex);
+  }
+  inline void Unlock(void) {
+    LeaveCriticalSection(&mutex);
+  }
+  inline void Destroy(void) {
+    DeleteCriticalSection(&mutex);
+  }
+
+ private:
+  friend class ConditionVariable;
+  CRITICAL_SECTION mutex;
+};
+
+// conditional variable that uses pthread
+class ConditionVariable {
+ public:
+  // initialize conditional variable
+  inline void Init(void) {
+    InitializeConditionVariable(&cond);
+  }
+  // destroy the thread
+  inline void Destroy(void) {
+    // DeleteConditionVariable(&cond);
+  }
+  // wait on the conditional variable
+  inline void Wait(Mutex *mutex) {
+    utils::Check(SleepConditionVariableCS(&cond, &(mutex->mutex), INFINITE) != 0,
+                 "ConditionVariable:Wait fail");
+  }
+  inline void Broadcast(void) {
+    WakeAllConditionVariable(&cond);
+  }
+  inline void Signal(void) {
+    WakeConditionVariable(&cond);
+  }
+
+ private:
+  CONDITION_VARIABLE cond;
+};
+
 /*! \brief simple thread that wraps windows thread */
 class Thread {
  private:
   HANDLE    thread_handle;
-  unsigned  thread_id;            
+  unsigned  thread_id;
  public:
-  inline void Start(unsigned int __stdcall entry(void*), void *param) {
+  inline void Start(unsigned int __stdcall entry(void*p), void *param) {
     thread_handle = (HANDLE)_beginthreadex(NULL, 0, entry, param, 0, &thread_id);
-  }            
+  }
   inline int Join(void) {
     WaitForSingleObject(thread_handle, INFINITE);
     return 0;
@@ -54,39 +106,41 @@ inline void ThreadExit(void *status) {
 }  // namespace utils
 }  // namespace xgboost
 #else
-// thread interface using g++     
-extern "C" {
+// thread interface using g++
 #include <semaphore.h>
 #include <pthread.h>
-}
+#include <errno.h>
 namespace xgboost {
 namespace utils {
 /*!\brief semaphore class */
 class Semaphore {
   #ifdef __APPLE__
+
  private:
   sem_t* semPtr;
-  char sema_name[20];            
+  char sema_name[20];
+
  private:
   inline void GenRandomString(char *s, const int len) {
-    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ" ;
+    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
     for (int i = 0; i < len; ++i) {
       s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
     }
     s[len] = 0;
   }
+
  public:
   inline void Init(int init_val) {
-    sema_name[0]='/'; 
-    sema_name[1]='s'; 
-    sema_name[2]='e'; 
-    sema_name[3]='/'; 
+    sema_name[0] = '/';
+    sema_name[1] = 's';
+    sema_name[2] = 'e';
+    sema_name[3] = '/';
     GenRandomString(&sema_name[4], 16);
-    if((semPtr = sem_open(sema_name, O_CREAT, 0644, init_val)) == SEM_FAILED) {
+    if ((semPtr = sem_open(sema_name, O_CREAT, 0644, init_val)) == SEM_FAILED) {
       perror("sem_open");
       exit(1);
     }
-    utils::Assert(semPtr != NULL, "create Semaphore error");
+    utils::Check(semPtr != NULL, "create Semaphore error");
   }
   inline void Destroy(void) {
     if (sem_close(semPtr) == -1) {
@@ -103,53 +157,93 @@ class Semaphore {
   }
   inline void Post(void) {
     sem_post(semPtr);
-  }               
+  }
   #else
+
  private:
   sem_t sem;
+
  public:
   inline void Init(int init_val) {
-    sem_init(&sem, 0, init_val);
+    if (sem_init(&sem, 0, init_val) != 0) {
+      utils::Error("Semaphore.Init:%s", strerror(errno));
+    }
   }
   inline void Destroy(void) {
-    sem_destroy(&sem);
+    if (sem_destroy(&sem) != 0) {
+      utils::Error("Semaphore.Destroy:%s", strerror(errno));
+    }
   }
   inline void Wait(void) {
-    sem_wait(&sem);
+    if (sem_wait(&sem) != 0) {
+      utils::Error("Semaphore.Wait:%s", strerror(errno));
+    }
   }
   inline void Post(void) {
-    sem_post(&sem);
+    if (sem_post(&sem) != 0) {
+      utils::Error("Semaphore.Post:%s", strerror(errno));
+    }
   }
-  #endif  
+  #endif
 };
 
-// helper for c thread
-// used to strictly call c++ function from pthread
-struct ThreadContext {
-  void *(*entry)(void*);
-  void *param;
+// mutex that works with pthread
+class Mutex {
+ public:
+  inline void Init(void) {
+    pthread_mutex_init(&mutex, NULL);
+  }
+  inline void Lock(void) {
+    pthread_mutex_lock(&mutex);
+  }
+  inline void Unlock(void) {
+    pthread_mutex_unlock(&mutex);
+  }
+  inline void Destroy(void) {
+    pthread_mutex_destroy(&mutex);
+  }
+
+ private:
+  friend class ConditionVariable;
+  pthread_mutex_t mutex;
 };
-extern "C" {
-  inline void *RunThreadContext(void *ctx_) {
-    ThreadContext *ctx = reinterpret_cast<ThreadContext*>(ctx_);
-    void *ret = (*ctx->entry)(ctx->param);
-    delete ctx;
-    return ret;
+
+// conditional variable that uses pthread
+class ConditionVariable {
+ public:
+  // initialize conditional variable
+  inline void Init(void) {
+    pthread_cond_init(&cond, NULL);
   }
-}
+  // destroy the thread
+  inline void Destroy(void) {
+    pthread_cond_destroy(&cond);
+  }
+  // wait on the conditional variable
+  inline void Wait(Mutex *mutex) {
+    pthread_cond_wait(&cond, &(mutex->mutex));
+  }
+  inline void Broadcast(void) {
+    pthread_cond_broadcast(&cond);
+  }
+  inline void Signal(void) {
+    pthread_cond_signal(&cond);
+  }
+
+ private:
+  pthread_cond_t cond;
+};
+
 /*!\brief simple thread class */
 class Thread {
  private:
-  pthread_t thread;                
-
+  pthread_t thread;
  public :
-  inline void Start(void *entry(void*), void *param) {
+  inline void Start(void * entry(void*), void *param) { // NOLINT(*)
     pthread_attr_t attr;
     pthread_attr_init(&attr);
     pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
-    ThreadContext *ctx = new ThreadContext();
-    ctx->entry = entry; ctx->param = param;
-    pthread_create(&thread, &attr, RunThreadContext, ctx);
+    pthread_create(&thread, &attr, entry, param);
   }
   inline int Join(void) {
     void *status;
@@ -159,9 +253,8 @@ class Thread {
 inline void ThreadExit(void *status) {
   pthread_exit(status);
 }
-
 }  // namespace utils
 }  // namespace xgboost
 #define XGBOOST_THREAD_PREFIX void *
-#endif
-#endif
+#endif  // Linux
+#endif  // XGBOOST_UTILS_THREAD_H_  NOLINT(*)