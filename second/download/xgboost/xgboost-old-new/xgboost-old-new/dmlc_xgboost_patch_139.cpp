@@ -119,38 +119,38 @@ inline void Allreduce(void *sendrecvbuf,
 // temporal memory for global and local model
 std::string global_buffer, local_buffer;
 // wrapper for serialization
-struct ReadWrapper : public ISerializable {
+struct ReadWrapper : public Serializable {
   std::string *p_str;
   explicit ReadWrapper(std::string *p_str)
       : p_str(p_str) {}
-  virtual void Load(IStream &fi) {
+  virtual void Load(Stream *fi) {
     uint64_t sz;
-    utils::Assert(fi.Read(&sz, sizeof(sz)) != 0,
+    utils::Assert(fi->Read(&sz, sizeof(sz)) != 0,
                  "Read pickle string");
     p_str->resize(sz);
     if (sz != 0) {
-      utils::Assert(fi.Read(&(*p_str)[0], sizeof(char) * sz) != 0,
+      utils::Assert(fi->Read(&(*p_str)[0], sizeof(char) * sz) != 0,
                     "Read pickle string");
     }
   }
-  virtual void Save(IStream &fo) const {
+  virtual void Save(Stream *fo) const {
     utils::Error("not implemented");
   }  
 };
-struct WriteWrapper : public ISerializable {
+struct WriteWrapper : public Serializable {
   const char *data;
   size_t length;
   explicit WriteWrapper(const char *data,
                         size_t length)
       : data(data), length(length) {
   }
-  virtual void Load(IStream &fi) {
+  virtual void Load(Stream *fi) {
     utils::Error("not implemented");
   }
-  virtual void Save(IStream &fo) const {
+  virtual void Save(Stream *fo) const {
     uint64_t sz = static_cast<uint16_t>(length);
-    fo.Write(&sz, sizeof(sz));
-    fo.Write(data, length * sizeof(char));
+    fo->Write(&sz, sizeof(sz));
+    fo->Write(data, length * sizeof(char));
   }
 };
 }  // namespace wrapper