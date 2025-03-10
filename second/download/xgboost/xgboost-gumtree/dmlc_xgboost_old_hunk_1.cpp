  }
};

/*! \brief model parameters */
struct GBTreeModelParam : public dmlc::Parameter<GBTreeModelParam> {
  /*! \brief number of trees */
  int num_trees;
  /*! \brief number of roots */
  int num_roots;
  /*! \brief number of features to be used by trees */
  int num_feature;
  /*! \brief pad this space, for backward compatibility reason.*/
  int pad_32bit;
  /*! \brief deprecated padding space. */
  int64_t num_pbuffer_deprecated;
  /*!
   * \brief how many output group a single instance can produce
   *  this affects the behavior of number of output we have:
   *    suppose we have n instance and k group, output will be k * n
   */
  int num_output_group;
  /*! \brief size of leaf vector needed in tree */
  int size_leaf_vector;
  /*! \brief reserved parameters */
  int reserved[32];
  /*! \brief constructor */
  GBTreeModelParam() {
    std::memset(this, 0, sizeof(GBTreeModelParam));
    static_assert(sizeof(GBTreeModelParam) == (4 + 2 + 2 + 32) * sizeof(int),
                  "64/32 bit compatibility issue");
  }
  // declare parameters, only declare those that need to be set.
  DMLC_DECLARE_PARAMETER(GBTreeModelParam) {
    DMLC_DECLARE_FIELD(num_output_group).set_lower_bound(1).set_default(1)
        .describe("Number of output groups to be predicted,"\
                  " used for multi-class classification.");
    DMLC_DECLARE_FIELD(num_roots).set_lower_bound(1).set_default(1)
        .describe("Tree updater sequence.");
    DMLC_DECLARE_FIELD(num_feature).set_lower_bound(0)
        .describe("Number of features used for training and prediction.");
    DMLC_DECLARE_FIELD(size_leaf_vector).set_lower_bound(0).set_default(0)
        .describe("Reserved option for vector tree.");
  }
};

// cache entry
struct CacheEntry {
