#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_STR 256
#define B64T "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"

// TODO: cleanup funcs and naming/refactor
// TODO: test and bugfix

// borrowed from tsoding
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

int my_strlen(const char *s)
{
	int i = 0;
	while (*s++ != '\0')
		++i;
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

void estob(int bs[], char *ss, size_t ss_size)
{
	// first byte of 8 needs to be 0;
	for (int i=0; i<ss_size; ++i) {
		char c = ss[i];
		for (int j=7; j>=0; --j) {
			bs[8*i+7-j] = (c & (1 << j)) ? 1 : 0;
		}
	}
}

void ebtos(char *ss, int bs[], size_t bs_size, size_t padding)
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
	printf("%x, %d %d\n", buf[0]-buf[1], bs_size, ctr);
	bs_size += 2*padding;
	int bs[bs_size];
	mseti(bs, 0, bs_size);
	estob(bs, buf, ctr);
	print_binary(bs, bs_size);
	size_t b64_size = bs_size/6 + padding;
	char b64[b64_size+1]; // +1 for c-str
	msetc(b64, '\0', b64_size+1);
	ebtos(b64, bs, bs_size, padding);
	// printer(b64, b64_size);
	printf("%s", b64);
}

void dstob(int bs[], char *ss, size_t ss_size)
{
	for (int i=0; i<ss_size; ++i) {
		// naive method: find the index of the letter
		// TODO: implement faster searching algorithm
		int idx = -1;
		for (int j=0; j<64; ++j){
			if (ss[i] == B64T[j]) {
				idx = j;
				break;
			}
		}
		int ones_count = 0;
		int j = 5; // it's filled in reverse order
		while(idx >= 0 && ones_count < 1) {
			if (idx <= 1) ++ones_count;
			bs[6*i+j] = idx%2;
			idx/=2;
			--j;
		} 
	}
}

void dbtos(char *ss, int bs[], size_t bs_size)
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
	dstob(bs, buf, ctr);
	// print_binary(bs, bs_size);
	char ss[ss_size+1]; // +1 for c-str
	msetc(ss,'\0', ss_size+1);
	dbtos(ss, bs, bs_size);
	// printer(ss, ss_size);
	printf("%s", ss);
}


int main(int argc, char **argv)
{
	shift(&argc, &argv); // shift program name
	const char *action = "";
	if (argc > 0) {
		action = shift(&argc, &argv);
	}
	const int is_encode = !my_strcmp("encode", action);
	const int is_decode = !my_strcmp("decode", action);

	if (!is_encode && !is_decode) {
		printf("unknown action: %s\n", action);
		printf("allowed: encode decode\n");
		exit(1);
	}
	
	// <action> the provided cmdline args
	int from_cmd = 0;
	while (argc > 0) {
		++from_cmd;
		const char *arg = shift(&argc, &argv);
		size_t arglen = my_strlen(arg);
		if (is_encode) encode((char *)arg, arglen);
		else if (is_decode) decode((char *)arg, arglen);
	}
	if (from_cmd) {
		printf("\n");
		exit(0);
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
				if (buf[ctr-1] == '\n') --ctr; // if last char is newline, then drop it
				if (is_encode) encode(buf, ctr);
				else if (is_decode) decode(buf, ctr);
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
				else if (is_decode) decode(buf, ctr);
				ctr=0;
				putchar('\n');
			}
		}
	} else if (S_ISREG(stats_mode)) {
		fseek(stream, 0, SEEK_END);
		int len = ftell(stream);
		rewind(stream);
		char buf[len];
		fgets(buf, len, stream);
		int ctr = len-1; // -1 len, because I guess of EOF
		if (buf[ctr-1] == '\n') --ctr; // if last char is newline, then drop it
		if (is_encode) encode(buf, len-1);
		else if (is_decode) decode(buf, len-1);
	}
	putchar('\n');
	return fclose(stream);
}
