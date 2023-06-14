#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_STR 256
#define B64T "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/="

void mset(int *dst, int v, size_t size)
{
	for (int i=0; i < size; ++i) {
		dst[i] = v;
	}
}

void print_binary(int *bs, size_t size)
{
	for (int i=0; i<size; ++i) printf("%d", bs[i]);
	printf("\n");
}

void stob(int bs[], char *ss, size_t ss_size)
{
	for (int i=0; i<ss_size; ++i) {
		int ones_count = 0;
		int j = 1; // first byte of 8 needs to be 0
		char c = ss[i];
		while (c >= 0 && ones_count < 1) {
			if (c <= 1) ++ones_count;
			bs[8*i+j] = c%2;
			c/=2;
			++j;
		} 
	}
}

void btos(char *ss, int bs[], size_t bs_size, size_t padding)
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
	for (int i=0; i<padding; ++i) ss[k+i] = '=';
}

int main(int argc, char **argv)
{
	struct stat stats;
	fstat(fileno(stdin), &stats);
	int stats_mode = stats.st_mode;
	FILE *stream = fdopen(STDIN_FILENO, "r");
	if (S_ISFIFO(stats_mode)) { // piped in
		printf("I'm fifo - len=%d\n", 0);
	} else if (S_ISCHR(stats_mode)) { // REPL
		int is_newline = 0;
		char buf[MAX_STR];
		int ctr = 0;
		while (1) {
			char c = fgetc(stream);
			if (c != '\n') {
				buf[ctr] = c;
				++ctr;
			} else {
				size_t bs_size = ctr*8;
				size_t padding = ((6 - (bs_size%6))%6)/2;
				bs_size += padding;
				int bs[bs_size];
				mset(bs, 0, bs_size+padding);
				print_binary(bs, bs_size);
				stob(bs, buf, ctr);
				print_binary(bs, bs_size);
				size_t b64_size = bs_size/6 + padding + (padding > 0); // non-zero padding means extra char
				char b64[b64_size];
				mset(b64, 0, b64_size);
				print_binary(b64, b64_size);
				btos(b64, bs, bs_size, padding);
				printf("%s\n", b64);
				ctr = 0;
			}
		}
	} else { // file directed in as stdin, eg ./a.out < file
		fseek(stream, 0, SEEK_END);
		int len = ftell(stream);
		rewind(stream);
		printf("Directed to me - len=%d\n", len);
	}
	return fclose(stream);
}
