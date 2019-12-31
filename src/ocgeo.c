#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "cJSON.h"
#include "sds.h"
#include "ocgeo.h"

#ifndef OCG_API_SERVER
#define OCG_API_SERVER "https://api.opencagedata.com/geocode/v1/json"
#endif

#ifndef OCGEO_VERSION
#define OCGEO_VERSION 0.1.0
#endif

char* ocgeo_version = OCGEO_VERSION;

static ocgeo_latlng_t ocgeo_invalid_bounds = {.lat = -91.0, .lng=-181};

struct http_response {
    sds data;
};

static size_t
write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    struct http_response* r = userdata;
    r->data = sdscatlen(r->data, ptr, size*nmemb);
    return nmemb;
}

#define JSON_INT_VALUE(json) ((json) == NULL || cJSON_IsNull(json) ? 0 : (json)->valueint)
#define JSON_OBJ_GET_STR(obj,name) (cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(obj,name)))
#define JSON_OBJ_GET_INT(obj,name) (cJSON_GetObjectItemCaseSensitive(obj,name)->valueint)

static inline void
parse_latlng(cJSON* json, ocgeo_latlng_t* latlng)
{
    latlng->lat = cJSON_GetObjectItemCaseSensitive(json,"lat")->valuedouble;
    latlng->lng = cJSON_GetObjectItemCaseSensitive(json,"lng")->valuedouble;
}

static int
parse_response_json(cJSON* json, ocgeo_response_t* response)
{
    cJSON* obj = NULL;

    memset(response, 0, sizeof(ocgeo_response_t));
    response->results = NULL;
    response->internal = json;

    obj = cJSON_GetObjectItemCaseSensitive(json, "status");
    assert(obj);
    response->status.code = JSON_OBJ_GET_INT(obj,"code");
    response->status.message = JSON_OBJ_GET_STR(obj,"message");

    /* Rate information, may not returned (e.g. for paying customers): */
    obj = cJSON_GetObjectItem(json, "rate");
    if (obj) {
        response->rateInfo.limit = JSON_OBJ_GET_INT(obj,"limit");
        response->rateInfo.remaining = JSON_OBJ_GET_INT(obj,"remaining");
        response->rateInfo.reset = JSON_OBJ_GET_INT(obj,"reset");
    }

    obj = cJSON_GetObjectItem(json, "total_results");
    assert(obj);
    response->total_results = JSON_INT_VALUE(obj);
    if (response->total_results <= 0) {
        return 0;
    }

    response->results = calloc(response->total_results, sizeof(ocgeo_result_t));
    obj = cJSON_GetObjectItemCaseSensitive(json, "results");
    assert(obj);

    cJSON* result_js;
    int k = 0;
    for (result_js = obj->child; result_js!= NULL; result_js = result_js->next, k++) {
        ocgeo_result_t* result = response->results + k;
        result->confidence = JSON_OBJ_GET_INT(result_js,"confidence");
        result->formatted = JSON_OBJ_GET_STR(result_js,"formatted");

        cJSON* bounds_js = cJSON_GetObjectItemCaseSensitive(result_js, "bounds");
        // assert(bounds_js);
        if (bounds_js) {
            parse_latlng(cJSON_GetObjectItem(bounds_js, "northeast"), &result->bounds.northeast);
            parse_latlng(cJSON_GetObjectItem(bounds_js, "southwest"), &result->bounds.southwest);
        }
        else {
            result->bounds.northeast = ocgeo_invalid_bounds;
            result->bounds.southwest = ocgeo_invalid_bounds;
        }

        cJSON* comp_js = cJSON_GetObjectItemCaseSensitive(result_js, "components");
        assert(comp_js);
        result->ISO_alpha2 = JSON_OBJ_GET_STR(comp_js, "ISO_3166-1_alpha-2");
        result->ISO_alpha3 = JSON_OBJ_GET_STR(comp_js, "ISO_3166-1_alpha-3");
        result->type = JSON_OBJ_GET_STR(comp_js, "_type");
        result->city = JSON_OBJ_GET_STR(comp_js, "city");
        result->city_district = JSON_OBJ_GET_STR(comp_js, "city_district");
        result->continent = JSON_OBJ_GET_STR(comp_js, "continent");
        result->country = JSON_OBJ_GET_STR(comp_js, "country");
        result->country_code = JSON_OBJ_GET_STR(comp_js, "country_code");
        result->county = JSON_OBJ_GET_STR(comp_js, "county");
        result->house_number = JSON_OBJ_GET_STR(comp_js, "house_number");
        result->neighbourhood = JSON_OBJ_GET_STR(comp_js, "neighbourhood");
        result->political_union = JSON_OBJ_GET_STR(comp_js, "political_union");
        result->postcode = JSON_OBJ_GET_STR(comp_js, "postcode");
        result->road = JSON_OBJ_GET_STR(comp_js, "road");
        result->state = JSON_OBJ_GET_STR(comp_js, "state");
        result->state_district = JSON_OBJ_GET_STR(comp_js, "state_district");
        result->suburb = JSON_OBJ_GET_STR(comp_js, "suburb");
    }
    return 0;
}

static ocgeo_response_t*
do_request(int is_fwd, const char* q, const char* api_key, 
           ocgeo_params_t* params, ocgeo_response_t* response)
{
    if (params == NULL) {
        ocgeo_params_t params = ocgeo_default_params();
        return do_request(is_fwd, q, api_key, &params, response);
    }

    CURL *curl = curl_easy_init();
    if (curl == NULL) {
        return NULL;
    }

    // Build URL:
    char* q_escaped = curl_easy_escape(curl, q, 0);
    sds url = sdsempty();
    url = sdscatprintf(url, "%s?q=%s&key=%s", OCG_API_SERVER, q_escaped, api_key);
    curl_free(q_escaped);
    if (params->abbrv)
        url = sdscat(url, "&abbrv=1");
    if (is_fwd && params->countrycode)
        url = sdscatprintf(url, "&countrycode=%s", params->countrycode);
    if (params->language)
        url = sdscatprintf(url, "&language=%s", params->language);
    if (params->limit)
        url = sdscatprintf(url, "&limit=%d", params->limit);
    if (params->min_confidence)
        url = sdscatprintf(url, "&min_confidence=%d", params->min_confidence);
    if (params->no_annotations)
        url = sdscat(url, "&no_annotations=1");
    if (params->no_dedupe)
        url = sdscat(url, "&no_dedupe=1");
    if (params->no_record)
        url = sdscat(url, "&no_record=1");
    if (is_fwd && params->roadinfo)
        url = sdscat(url, "&roadinfo=1");
    if (is_fwd && ocgeo_is_valid_latlng(params->proximity))
        url = sdscatprintf(url, "&proximity=%.8F,%.8F", params->proximity.lat, params->proximity.lng);

    /* The value of the bounds parameter should be specified as two coordinate
       points forming the south-west and north-east corners of a bounding box.
       For example: bounds=-0.563160,51.280430,0.278970,51.683979 (min lon, min
       lat, max lon, max lat). */
    if (ocgeo_is_valid_bounds(&params->bounds))
        url = sdscatprintf(url, "&bounds=%.8F,%.8F,%.8F,%.8F", 
            params->bounds.southwest.lng, params->bounds.southwest.lat, 
            params->bounds.northeast.lng, params->bounds.northeast.lat);

    fprintf(stderr, "URL=%s\n", url);

    struct http_response r; r.data = sdsempty();
    sds user_agent = sdsempty();
    user_agent = sdscatprintf(user_agent, "c-ocgeo/%s (%s)", ocgeo_version, curl_version());
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agent);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &r);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    sdsfree(user_agent);

    if (res != CURLE_OK) {
        sdsfree(r.data);
        return NULL;
    }

    cJSON* json = cJSON_Parse(r.data);
    sdsfree(r.data);

    if (json == NULL)
        return NULL;

    if (params->dbg_callback != NULL) {
        char* str = cJSON_Print(json);
        params->dbg_callback(str, params->callback_data);
        cJSON_free(str);
    }

    parse_response_json(json, response);
    return response;
}

ocgeo_params_t ocgeo_default_params(void)
{

    ocgeo_params_t params = {
        .dbg_callback = NULL, .callback_data = NULL,
        .countrycode = NULL, .language = NULL,
        .no_annotations = 1
    };
    params.proximity = ocgeo_invalid_bounds;
    params.bounds.southwest = ocgeo_invalid_bounds;
    params.bounds.northeast = ocgeo_invalid_bounds;
    return params;
}

ocgeo_response_t* ocgeo_forward(const char* q, const char* api_key,
        ocgeo_params_t* params, ocgeo_response_t* response)
{
    return do_request(1, q, api_key, params, response);
}

ocgeo_response_t* ocgeo_reverse(double lat, double lng, const char* api_key,
        ocgeo_params_t* params, ocgeo_response_t* response)
{
    sds q = sdsempty();
    q = sdscatprintf(q, "%.8F,%.8F", lat, lng);
    ocgeo_response_t* r = do_request(0, q, api_key, params, response);
    sdsfree(q);
    return r;
}

void ocgeo_response_cleanup(ocgeo_response_t* r)
{
    if (r == NULL)
        return;
    free(r->results);
    r->results = NULL;
    cJSON_Delete(r->internal);
    r->internal = NULL;
}
