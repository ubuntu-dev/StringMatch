
#include <stdio.h>
#include <stdlib.h>
#include <cstdio>
#include <cstdlib>
#include <string.h>

#include "Kmp.h"
#include "support/StopWatch.h"

using namespace StringMatch;

#ifndef _DEBUG
const size_t Iterations = 5000000;
#else
const size_t Iterations = 10000;
#endif


//
// See: http://volnitsky.com/project/str_search/index.html
//
static const char * szSearchText[] = {
	"Here is a sample example.",

	"8'E . It consists of a number of low-lying, largely mangrove covered islands covering an area of around 665 km^2. "
	"The population of Bakassi is the subject of some dispute, but is generally put at between 150,000 and 300,000 people."
};

static const char * szPatterns[] = {
	"sample",
	"example",

	"islands",
	"around",
	"subject",
	"between",
	"people",

	"between 150,000"
};

void kmp_benchmark()
{
	test::StopWatch sw;
	int sum;
	static const size_t iters = Iterations / (__CountOf(szSearchText) * __CountOf(szPatterns));

	printf("---------------------\n");
	printf("  kmp_benchmark()\n");
	printf("---------------------\n\n");

	AnsiString::kmp::Pattern pattern[__CountOf(szPatterns)];
	for (int i = 0; i < __CountOf(szPatterns); ++i) {
		pattern[i].prepare(szPatterns[i]);
	}

	StringRef search_text[__CountOf(szSearchText)];
	for (int i = 0; i < __CountOf(szSearchText); ++i) {
		search_text[i].set_ref(szSearchText[i], strlen(szSearchText[i]));
	}

	sum = 0;
	sw.start();
	for (size_t loop = 0; loop < iters; ++loop) {
		for (int i = 0; i < __CountOf(szSearchText); ++i) {
			const char * text = search_text[i].c_str();
			size_t text_len = search_text[i].size();
			for (int j = 0; j < __CountOf(szPatterns); ++j) {
				int index_of = pattern[j].match(text, text_len);
				sum += index_of;
			}
		}
	}
	sw.stop();

	printf("sum: %12d, time spent: %0.3f ms\n\n", sum, sw.getElapsedMillisec());	
}

void kmp_test()
{
    const char pattern_text_1[] = "sample";
    char pattern_text_2[] = "a sample";

    test::StopWatch sw;
    int sum, index_of;

    AnsiString::kmp::Matcher matcher;

    AnsiString::kmp::Pattern pattern;
    pattern.prepare("example");    

    sum = 0;
    sw.start();    
    for (size_t i = 0; i < Iterations; ++i) {
        index_of = pattern.match("Here is a sample example.");
        sum += index_of;
    }
    sw.stop();
    pattern.display_test(index_of, sum, sw.getElapsedMillisec());

    AnsiString::kmp::Pattern pattern1(pattern_text_1);
    //pattern1.prepare(pattern_text_1);

    sum = 0;
    sw.start();
    for (size_t i = 0; i < Iterations; ++i) {
        index_of = pattern1.match("Here is a sample example.");
        sum += index_of;
    }
    sw.stop();
    pattern1.display_test(index_of, sum, sw.getElapsedMillisec());

    AnsiString::kmp::Pattern pattern2(pattern_text_2);
    //pattern2.prepare(pattern_text_2);

    sum = 0;
    sw.start();
    for (size_t i = 0; i < Iterations; ++i) {
        index_of = pattern2.match("Here is a sample example.");
        sum += index_of;
    }
    sw.stop();
    pattern2.display_test(index_of, sum, sw.getElapsedMillisec());
}

int main(int argc, char * argv[])
{
	kmp_test();
	kmp_benchmark();

    ::system("pause");
    return 0;
}
