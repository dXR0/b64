#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_STR 256
#define B64T "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/="

// borrowed from tsoding
const char *shift(int *argc, char ***argv)
{
    // assert(*argc > 0);
    const char *result = *argv[0];
    *argc -= 1;
    *argv += 1;
    return result;
}

int my_strcmp(const char *s1, const char *s2)
{
	int i;
	for (i=0; s1[i] == s2[i]; ++i)
		if (s1[i] == '\0') return 0;
	return s1[i] - s2[i];
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

void print_binaryi(int *bs, size_t size)
{
	for (int i=0; i<size; ++i) printf("%d", bs[i]);
	putchar('\n');
}

void print_binaryc(char *bs, size_t size)
{
	for (int i=0; i<size; ++i) printf("%c", bs[i]);
	putchar('\n');
}

void stob(int bs[], char *ss, size_t ss_size)
{
	// first byte of 8 needs to be 0;
	for (int i=0; i<ss_size; ++i) {
		int ones_count = 0;
		int j = 7; // it's filled in reverse order
		char c = ss[i];
		while(c >= 0 && ones_count < 1) {
			if (c <= 1) ++ones_count;
			bs[8*i+j] = c%2;
			c/=2;
			--j;
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
	// print_binaryi(bs, bs_size);
	stob(bs, buf, ctr);
	// print_binaryi(bs, bs_size);
	size_t b64_size = bs_size/6 + padding + (padding > 0) + 1; // non-zero padding means extra char; +1 for '\0'
	char b64[b64_size];
	msetc(b64, '\0', b64_size);
	btos(b64, bs, bs_size, padding);
	// print_binaryc(b64, b64_size);
	printf("%s\n", b64);
}

int main(int argc, char **argv)
{
	shift(&argc, &argv); // shift program name
	const char *action = "";
	while (argc > 0) {
		action = shift(&argc, &argv);
	}
	const int is_encode = !my_strcmp(action, "encode");
	const int is_decode = !my_strcmp(action, "decode");

	if (!is_encode && !is_decode) {
		printf("unknown action\n");
		exit(1);
	}

	struct stat stats;
	fstat(fileno(stdin), &stats);
	int stats_mode = stats.st_mode;
	FILE *stream = fdopen(STDIN_FILENO, "r");
	// S_ISFIFO(stats_mode) - piped in
	// S_ISCHR(stats_mode) - REPL
	// S_ISREG(stats_mode) - file directed in as stdin, eg ./a.out < file
	if (S_ISFIFO(stats_mode)) {
		char buf[MAX_STR];
		int ctr = 0;
		while (1) {
			char c = fgetc(stream);
			if (c != EOF) {
				buf[ctr] = c;
				++ctr;
			} else {
				if (is_encode) encode(buf, ctr);
				break;
			}
		}
	} else if (S_ISCHR(stats_mode)) {
		int is_newline = 0;
		char buf[MAX_STR];
		int ctr = 0;
		while (1) {
			char c = fgetc(stream);
			if (c != '\n' && c != EOF) {
				buf[ctr] = c;
				++ctr;
			} else {
				if (is_encode) encode(buf, ctr);
				// else if (is_decode ) decode(buf, ctr);
				ctr=0;
			}
		}
	} else if (S_ISREG(stats_mode)) {
		fseek(stream, 0, SEEK_END);
		int len = ftell(stream);
		rewind(stream);
		char buf[len];
		fgets(buf, len, stream);
		if (is_encode) encode(buf, len-1); // -1 len, because I guess of EOF
	}
	return fclose(stream);
}
