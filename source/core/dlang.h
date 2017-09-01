

#ifndef __CORE_DL_H
#define __CORE_DL_H


typedef struct {

	size_t  length;
	char   *string;

} dl_str;


typedef struct {

	size_t  num;
	void   *item;

} dl_array;


#define dl_array_at(arr, T, n) (((T*)(arr)->item) + (n))



#endif

