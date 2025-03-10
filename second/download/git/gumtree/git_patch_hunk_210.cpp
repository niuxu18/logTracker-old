 #include "cache.h"
-#include "commit.h"
 #include "prio-queue.h"
 
+static inline int compare(struct prio_queue *queue, int i, int j)
+{
+	int cmp = queue->compare(queue->array[i].data, queue->array[j].data,
+				 queue->cb_data);
+	if (!cmp)
+		cmp = queue->array[i].ctr - queue->array[j].ctr;
+	return cmp;
+}
+
+static inline void swap(struct prio_queue *queue, int i, int j)
+{
+	struct prio_queue_entry tmp = queue->array[i];
+	queue->array[i] = queue->array[j];
+	queue->array[j] = tmp;
+}
+
 void prio_queue_reverse(struct prio_queue *queue)
 {
 	int i, j;
 
 	if (queue->compare != NULL)
 		die("BUG: prio_queue_reverse() on non-LIFO queue");
-	for (i = 0; i <= (j = (queue->nr - 1) - i); i++) {
-		struct commit *swap = queue->array[i];
-		queue->array[i] = queue->array[j];
-		queue->array[j] = swap;
-	}
+	for (i = 0; i <= (j = (queue->nr - 1) - i); i++)
+		swap(queue, i, j);
 }
 
 void clear_prio_queue(struct prio_queue *queue)
 {
 	free(queue->array);
 	queue->nr = 0;
 	queue->alloc = 0;
 	queue->array = NULL;
+	queue->insertion_ctr = 0;
 }
 
 void prio_queue_put(struct prio_queue *queue, void *thing)
 {
-	prio_queue_compare_fn compare = queue->compare;
 	int ix, parent;
 
 	/* Append at the end */
 	ALLOC_GROW(queue->array, queue->nr + 1, queue->alloc);
-	queue->array[queue->nr++] = thing;
-	if (!compare)
+	queue->array[queue->nr].ctr = queue->insertion_ctr++;
+	queue->array[queue->nr].data = thing;
+	queue->nr++;
+	if (!queue->compare)
 		return; /* LIFO */
 
 	/* Bubble up the new one */
 	for (ix = queue->nr - 1; ix; ix = parent) {
 		parent = (ix - 1) / 2;
-		if (compare(queue->array[parent], queue->array[ix],
-			    queue->cb_data) <= 0)
+		if (compare(queue, parent, ix) <= 0)
 			break;
 
-		thing = queue->array[parent];
-		queue->array[parent] = queue->array[ix];
-		queue->array[ix] = thing;
+		swap(queue, parent, ix);
 	}
 }
 
 void *prio_queue_get(struct prio_queue *queue)
 {
-	void *result, *swap;
+	void *result;
 	int ix, child;
-	prio_queue_compare_fn compare = queue->compare;
 
 	if (!queue->nr)
 		return NULL;
-	if (!compare)
-		return queue->array[--queue->nr]; /* LIFO */
+	if (!queue->compare)
+		return queue->array[--queue->nr].data; /* LIFO */
 
-	result = queue->array[0];
+	result = queue->array[0].data;
 	if (!--queue->nr)
 		return result;
 
 	queue->array[0] = queue->array[queue->nr];
 
 	/* Push down the one at the root */
 	for (ix = 0; ix * 2 + 1 < queue->nr; ix = child) {
 		child = ix * 2 + 1; /* left */
-		if ((child + 1 < queue->nr) &&
-		    (compare(queue->array[child], queue->array[child + 1],
-			     queue->cb_data) >= 0))
+		if (child + 1 < queue->nr &&
+		    compare(queue, child, child + 1) >= 0)
 			child++; /* use right child */
 
-		if (compare(queue->array[ix], queue->array[child],
-			    queue->cb_data) <= 0)
+		if (compare(queue, ix, child) <= 0)
 			break;
 
-		swap = queue->array[child];
-		queue->array[child] = queue->array[ix];
-		queue->array[ix] = swap;
+		swap(queue, child, ix);
 	}
 	return result;
 }
