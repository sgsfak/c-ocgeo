#include <stdio.h>
#include "ocgeo.h"

static
void debug(const char* json, void* data)
{
    fprintf(stderr, "%s\n", json);
}

int main(int argc, char* argv[])
{
    printf("Using version '%s' of the ocgeo lib.\n", ocgeo_version);

    if (argc <= 2) {
        fprintf(stderr, "Usage: example <api-key> <query>\n");
        return 1;
    }
    const char* api_key = argv[1];
    const char* query = argv[2];

    /* Initialize parameters with the default values: */
    struct ocgeo_params params = ocgeo_default_params();
    /* and then overwrite them if you wish: */
    params.language = "en";
    // params.min_confidence = 3;
    params.dbg_callback = debug;
    params.no_annotations = false;

    /* Make a (forward) request to the API using the API key you have: */
    struct ocgeo_response response;
    ocgeo_forward(query, api_key, &params, &response);

    /* Check the response : */
    if (ocgeo_response_ok(&response)) {
        /* Retrieve the response information: */
        printf("Got %d results:\n", response.total_results);
        int i=1;
        ocgeo_result_t* result;
        foreach_ocgeo_result(result, &response) {
            printf("%2d. %s (type: %s, category: %s, conf:%d)\n", i,
                result->formatted, result->type, result->category, result->confidence);
            ++i;
            printf("\tGeohash=%s, what3words=%s, calling code=%d\n",
                result->geohash != NULL ? result->geohash : "",
                result->what3words != NULL ? result->what3words : "",
                result->callingcode);
            if (ocgeo_is_valid_latlng(result->geometry))
                printf("\tGeometry: (%.7f,%.7f)\n", 
                    result->geometry.lat, result->geometry.lng);

            if (result->bounds)
                printf("\tBounding box: SW(lat,lon)=(%.7f,%.7f) NE(lat,lon)=(%.7f, %.7f)\n", 
                    result->bounds->southwest.lat, result->bounds->southwest.lng,
                    result->bounds->northeast.lat, result->bounds->northeast.lng);
            bool ok;
            const char* curr = ocgeo_response_get_str(result, "annotations.currency.alternate_symbols.0", &ok);
            if (ok) {
                printf("\t(First) Alt Currency:%s\n", curr);
            }
        }
    }
    else {
        /* Request failed, print error message: */
        printf("Request failed with status: %d Message: %s\n", response.status.code, response.status.message);
    }
    /* Finally "cleanup" the response (free its memory): */
    ocgeo_response_cleanup(&response);

    return 0;
}
