/* Standalone ISO C consumer linked against the real shared library.
 * No C++ symbols, no JUCE dependency, no plugin SDK.
 */
#include "arverb/arverb_c.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#ifdef _MSC_VER
#  include <float.h>
#  define AR_FINITE(x) (_finite((double)(x)) != 0)
#else
#  define AR_FINITE(x) isfinite(x)
#endif

#define TEST(x) do { if (!(x)) { fprintf(stderr,"C ABI FAIL line %d: %s\n",__LINE__,#x); return 1; } } while(0)
int main(void) {
    TEST(arverb_abi_version() == ARVERB_ABI_VERSION);
    TEST(arverb_parameter_count() == ARVERB_PARAMETER_COUNT);
    TEST(strcmp(arverb_version(), ARVERB_EXPECTED_VERSION) == 0);
    arverb_destroy(NULL);
    TEST(arverb_prepare(NULL, 48000.0) == 0);
    TEST(arverb_load_preset(NULL, ARVERB_VOCAL_PLATE) == 0);
    TEST(arverb_set_parameter(NULL, ARVERB_MIX, .5f) == 0);
    TEST(arverb_get_parameter(NULL, ARVERB_MIX) == 0.f);
    arverb_handle *h = arverb_create();
    TEST(h != NULL);
    TEST(arverb_prepare(h, -48000.0) == 0);
    TEST(arverb_prepare(h, 48000.0) == 1);
    TEST(arverb_load_preset(h, ARVERB_VOCAL_PLATE) == 1);
    TEST(arverb_load_preset(h, 100u) == 0);
    TEST(arverb_set_parameter(h, ARVERB_MIX, NAN) == 0);
    TEST(arverb_set_parameter(h, 100u, .5f) == 0);
    TEST(arverb_set_parameter(h, ARVERB_MIX, 1.f) == 1);
    TEST(fabsf(arverb_get_parameter(h, ARVERB_MIX) - 1.f) < 1e-6f);
    float in_l[1024] = {0}, in_r[1024] = {0};
    float out_l[1024] = {0}, out_r[1024] = {0};
    in_l[0] = 1.f;
    for (int block = 0; block < 100; ++block) {
        arverb_process(h, in_l, in_r, out_l, out_r, 1024);
        for (int i = 0; i < 1024; ++i)
            TEST(AR_FINITE(out_l[i]) && AR_FINITE(out_r[i]));
        memset(in_l, 0, sizeof in_l);
    }
    arverb_reset(h);
    arverb_destroy(h);
    puts("PASS: ISO C shared library ABI, version, invalid inputs and audio");
    return 0;
}
