#include <stdio.h>
#include "ocgeo.h"

static
void debug(const char* json, void* data)
{
    fprintf(stderr, "%s\n", json);
}

int main(int argc, char* argv[])
{

    if (argc <= 2) {
        fprintf(stderr, "Usage: example <api-key> <query>\n");
        return 1;
    }
    const char* api_key = argv[1];
    const char* query = argv[2];

    /* Initialize parameters with the default values: */
    struct ocgeo_params params;
    ocgeo_params_init(&params);
    /* and then overwrite them if you wish: */
    params.language = "en";
    // params.min_confidence = 3;
    params.dbg_callback = debug;

    /* Make a (forward) request to the API using the API key you have: */
    struct ocgeo_response response;
    ocgeo_forward(query, api_key, &params, &response);

    /* Check the response : */
    if (ocgeo_response_ok(&response)) {
        /* Retrieve the response information: */
        printf("Got %d results:\n", response.total_results);
        for (int i=0; i<response.total_results; ++i) {
            struct ocgeo_result* result = response.results + i;
            printf("%2d. %s (type: %s, conf:%d)\n", i+1, 
                result->formatted, result->type, result->confidence);
            if (ocgeo_is_valid_bounds(&result->bounds))
            printf("\tBounding box: NE=(%.7f,%.7f) SW=(%.7f, %.7f)\n", 
                result->bounds.northeast.lat, result->bounds.northeast.lng, 
                result->bounds.southwest.lat, result->bounds.southwest.lng);
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
