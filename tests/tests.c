#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include "ocgeo.h"

#if _WIN32
#  define C_RED(s)     s
#  define C_GREEN(s)   s
#else
#  define C_RED(s)     "\033[31;1m" s "\033[0m"
#  define C_GREEN(s)   "\033[32;1m" s "\033[0m"
#endif

#define TEST(s, x) \
    do { \
        if (x) { \
            printf(C_GREEN("PASS") " %s\n", s); \
            count_pass++; \
        } else { \
            printf(C_RED("FAIL") " %s\n", s); \
            count_fail++; \
        } \
    } while (0)

static inline bool
d_eq_scale(double a, double b, int p) 
{
    return lroundf(a*p) == lroundf(b*p);
}
#define d_eq_1(a,b) d_eq_scale(a,b,10)
#define d_eq_2(a,b) d_eq_scale(a,b,100)
#define d_eq_5(a,b) d_eq_scale(a,b,10000)
#define d_eq_6(a,b) d_eq_scale(a,b,100000)
#define d_eq_7(a,b) d_eq_scale(a,b,1000000)

#define TEST_DEC_TO_DEG(dec, d, m, s) \
    dd = ocgeo_decimal_coords_to_degrees(dec); \
    TEST("decimal_coords_to_degrees(" #dec "): match " #d "°" #m "'" #s "\"" ,\
        dd.degrees == d && dd.minutes == m &&  d_eq_2(dd.seconds,s))

#define TEST_DEG_TO_DEC(x) \
    dd = ocgeo_decimal_coords_to_degrees(x); \
    TEST("mapping " #x " to degrees and back", \
        d_eq_6(x,ocgeo_degrees_to_decimal_coords(dd)))

static int count_pass = 0;
static int count_fail = 0;

int main(int argc, char* argv[])
{

    ocgeo_dms_t dd;

    ocgeo_params_t params = ocgeo_default_params();
    ocgeo_response_t response;
    params.no_record = true;
    params.no_annotations = true;
    const char* query = "Syena, Aswan Governorate, Egypt";
    ocgeo_forward(query, "4372eff77b8343cebfc843eb4da4ddc4", &params, &response);
    TEST("Testing 402 response", response.status.code == OCGEO_CODE_QUOTA_ERROR);
    ocgeo_response_cleanup(&response);
    ocgeo_forward(query, "2e10e5e828262eb243ec0b54681d699a", &params, &response);
    TEST("Testing 403 response", response.status.code == OCGEO_CODE_FORBIDDEN);
    ocgeo_response_cleanup(&response);
    ocgeo_forward(query, "d6d0f0065f4348a4bdfe4587ba02714b", &params, &response);
    TEST("Testing 429 response", response.status.code == OCGEO_CODE_MANY_REQUESTS);
    ocgeo_response_cleanup(&response);

    params.no_annotations = false;
    ocgeo_forward(query, "6d0e711d72d74daeb2b0bfd2a5cdfdba", &params, &response);
    TEST("Testing 200 response", response.status.code == OCGEO_CODE_OK);
    TEST("Testing getting results", response.total_results > 0 && response.results != NULL);
    ocgeo_result_t* result = response.results;
    TEST("Testing currency annotation", result->currency != NULL &&
        strcmp(result->currency->iso_code, "EUR")==0 &&
        strcmp(result->currency->name, "Euro")==0 &&
        strcmp(result->currency->thousands_separator, ".")==0 &&
        strcmp(result->currency->decimal_mark, ",")==0);
    TEST("Testing road info annotation", result->roadinfo != NULL &&
        strcmp(result->roadinfo->speed_in, "km/h")==0 &&
        strcmp(result->roadinfo->drive_on, "right")==0);
    
    bool ok;
    const char* lat = ocgeo_response_get_str(result, "annotations.DMS.lat", &ok);
    TEST("Testing adv API, getting DMS lat", ok);
    int callingCode = ocgeo_response_get_int(result, "annotations.callingcode", &ok);
    TEST("Testing adv API, getting calling code", ok && callingCode == 49);
    printf("\t\tDMS lat = \"%s\", calling code=%d\n", lat, callingCode);

    TEST("Testing adv API, getting bounds", 
        d_eq_7(ocgeo_response_get_dbl(result, "bounds.northeast.lat", &ok), 51.9528202));
    TEST("Testing adv API, getting nonexistent string field", 
        ocgeo_response_get_str(result, "annotations.NON-EXISTENT", &ok) == NULL && !ok);

    ocgeo_response_cleanup(&response);


    TEST_DEC_TO_DEG(-10.1245839, -10, 7, 28.5);
    TEST_DEC_TO_DEG(-89.5006900, -89, 30, 2.48);
    TEST_DEC_TO_DEG(32.8247971, 32, 49, 29.27);
    TEST_DEC_TO_DEG(21.8545795, 21, 51, 16.49);

    TEST_DEG_TO_DEC(-10.1245839);
    TEST_DEG_TO_DEC(-89.5006900);

    printf("\n%d failed, %d pass\n", count_fail, count_pass);
    return 0;
}
