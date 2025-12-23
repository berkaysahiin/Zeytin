#pragma once 

#define MAKE_SINGLETON(ClassName) \
     public: \
         ClassName(const ClassName&) = delete; \
         ClassName& operator=(const ClassName&) = delete; \
         ClassName(ClassName&& other) = delete; \
         ClassName& operator=(ClassName&&) = delete;   \
         static ClassName& get() { \
             static ClassName instance; \
             return instance; \
         } \

#define CONSTRUCT_SINGLETON(ClassName) (void)ClassName :: get(); 
#define GET_SINGLETON(ClassName) ClassName :: get(); 

