#define STR_HELPER(x) #x
#define STRING(x) STR_HELPER(x)
#define JOIN(x, y) JOIN_AGAIN(x, y)
#define JOIN_AGAIN(x, y) x ## y

#if ~(~BUILD_NUMBER + 0) == 0 && ~(~BUILD_NUMBER + 1) == 1
  #warning "Build number not set"
  #undef BUILD_NUMBER
  #define SNAPSHOT -SNAPSHOT
  #define ASSET_VERSION 29
  #define BUILD_NUMBER ASSET_VERSION
#else
  #define ASSET_VERSION BUILD_NUMBER
  #define SNAPSHOT
#endif
