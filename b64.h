#ifndef B64_H_
#define B64_H_

#include <stdio.h>

const char *shift(int *argc, char ***argv);
int my_strcmp(const char *base, const char *comp);
size_t my_strlen(const char *s);
void mseti(int *dst, int v, size_t size);
void msetc(char *dst, int v, size_t size);
void print_binary(int *bs, size_t size);
void printer(char *ss, size_t size);
void e_stob(int bs[], char *ss, size_t ss_size);
void e_btos(char *ss, int bs[], size_t bs_size, size_t padding);
void encode(char *buf, int ctr);
int find_pos(char target);
void d_stob(int bs[], char *ss, size_t ss_size);
void d_btos(char *ss, int bs[], size_t bs_size);
size_t count_padding(char *buf, size_t ctr);
void decode(char *buf, int ctr);
#endif // B64_H_

#ifndef B64_IMPLEMENTATION
#define B64_IMPLEMENTATION

#include <stdlib.h>

#define MAX_STR 256
#define B64T "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"

const char *shift(int *argc, char ***argv)
{
	// assert(*argc > 0);
	const char *result = *argv[0];
	*argc -= 1;
	*argv += 1;
	return result;
}

int my_strcmp(const char *base, const char *comp)
{
	int i;
	for (i=0; base[i] == comp[i]; ++i)
		if (base[i] == '\0') return 0;
	return base[i] - comp[i];
}

size_t my_strlen(const char *s)
{
	int i = 0;
	return i;
}

void mseti(int *dst, int v, size_t size)
{
	for (int i=0; i < size; ++i) {
		dst[i] = v;
	}
}

void msetc(char *dst, int v, size_t size)
{
	for (int i=0; i < size; ++i) {
		dst[i] = v;
	}
}

void print_binary(int *bs, size_t size)
{
	for (int i=0; i<size; ++i) printf("%d", bs[i]);
	putchar('\n');
}

void printer(char *ss, size_t size)
{
	for (int i=0; i<size; ++i)
		putchar(ss[i]);
}

void e_stob(int bs[], char *ss, size_t ss_size)
{
	// first byte of 8 needs to be 0;
	for (int i=0; i<ss_size; ++i) {
		char c = ss[i];
		for (int j=7; j>=0; --j) {
			bs[8*i+7-j] = (c & (1 << j)) ? 1 : 0;
		}
	}
}

void e_btos(char *ss, int bs[], size_t bs_size, size_t padding)
{
	int k = 0;
	for (int i=0; i<bs_size; i+=6) {
		int res = bs[i+5];
		int pow = 2;
		for (int j=4; j>-1; --j) {
			res += bs[i+j]*pow;
			pow *= 2;
		}
		ss[k] = B64T[res];
		++k;
	}
	int kpadding = k+padding;
	for (; k<kpadding; ++k) ss[k] = '=';
}

void encode(char *buf, int ctr)
{
	size_t bs_size = ctr*8;
	size_t padding = ((6 - (bs_size%6))%6)/2;
	bs_size += 2*padding;
	int bs[bs_size];
	mseti(bs, 0, bs_size);
	e_stob(bs, buf, ctr);
	// print_binary(bs, bs_size);
	size_t b64_size = bs_size/6 + padding;
	char b64[b64_size+1]; // +1 for c-str
	msetc(b64, '\0', b64_size+1);
	e_btos(b64, bs, bs_size, padding);
	// printer(b64, b64_size);
	printf("%s", b64);
}

int find_pos(char target)
{
	if (target == '=') return -1;
	for (int j=0; j<64; ++j){
		if (target == B64T[j]) {
			return j;
		}
	}
	printf("invalid base64 character: '%c'\n", target);
	exit(1);
}

void d_stob(int bs[], char *ss, size_t ss_size)
{
	for (int i=0; i<ss_size; ++i) {
		int idx = find_pos(ss[i]);
		for (int j=5; j>=0; --j) {
			bs[6*i+5-j] = (idx & (1 << j)) ? 1 : 0;
		}
	}
}

void d_btos(char *ss, int bs[], size_t bs_size)
{
	int k = 0;
	for (int i=0; i<bs_size; i+=8) {
		int res = bs[i+7];
		int pow = 2;
		for (int j=6; j>-1; --j) {
			res += bs[i+j]*pow;
			pow *= 2;
		}
		ss[k] = res;
		++k;
	}
}

size_t count_padding(char *buf, size_t ctr)
{
	int count = 0;
	for (int i=0; i<ctr; ++i) {
		if (buf[i] == '=') ++count;
	}
	return count;
}	

void decode(char *buf, int ctr)
{
	size_t padding = count_padding(buf, ctr);
	size_t ss_size = 6*(ctr-padding)/8;
	size_t bs_size = ss_size*8;
	int bs[bs_size];
	mseti(bs, 0, bs_size);
	d_stob(bs, buf, ctr);
	// print_binary(bs, bs_size);
	char ss[ss_size+1]; // +1 for c-str
	msetc(ss,'\0', ss_size+1);
	d_btos(ss, bs, bs_size);
	// printer(ss, ss_size);
	printf("%s", ss);
}

#endif // B64_IMPLEMENTATION
