#if ~(~BUILD_NUMBER + 0) == 0 && ~(~BUILD_NUMBER + 1) == 1
#warning "Build number not set"
#define BUILD_NUMBER 11
#endif
#define STR_HELPER(x) #x
#define STRING(x) STR_HELPER(x)
