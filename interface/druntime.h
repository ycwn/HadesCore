

#ifndef CORE_DRUNTIME_H
#define CORE_DRUNTIME_H


typedef struct {

	size_t  length;
	char   *string;

} druntime_string;


typedef struct {

	size_t  num;
	void   *item;

} druntime_array;


#define druntime_array_at(arr, T, n) (((T*)(arr)->item) + (n))



#endif

