#ifndef OC_GEOCODE_H
#define OC_GEOCODE_H

#ifdef __cplusplus
extern "C" {
#endif

/* HTTP Status code used */
#define OCGEO_CODE_OK (200)
#define OCGEO_CODE_INV_REQUEST (400)	/*Invalid request (bad request; a required parameter is missing; invalid coordinates; invalid version; invalid format) */
#define OCGEO_CODE_AUTH_ERROR (401) 	/* Unable to authenticate - missing, invalid, or unknown API key */
#define OCGEO_CODE_QUOTA_ERROR (402) 	/* Valid request but quota exceeded (payment required) */
#define OCGEO_CODE_FORBIDDEN (403) 		/* Forbidden (API key blocked) */
#define OCGEO_CODE_INV_ENDPOINT (404) 	/* Invalid API endpoint */
#define OCGEO_CODE_INV_METHOD (405) 	/* Method not allowed (non-GET request) */
#define OCGEO_CODE_TIMEOUT (408)  		/* Timeout; you can try again */
#define OCGEO_CODE_LONG_REQUEST (410) 	/* Request too long */
#define OCGEO_CODE_MANY_REQUESTS (429)	/* Too many requests (too quickly, rate limiting) */
#define OCGEO_CODE_INTERNAL_ERROR (503)	/* Internal server error  */

typedef struct ocgeo_status {
	int code;
	char* message;
} ocgeo_status_t;

typedef struct ocgeo_rate_info {
    int limit;
    int remaining;
    int reset;
} ocgeo_rate_info_t;

/*
 * A point using the geographic coordinate reference system, using the World 
 * Geodetic System 1984 (WGS 84), with longitude and latitude units of decimal
 * degrees
 */
typedef struct ocgeo_latlng {
	double lat;
	double lng;
} ocgeo_latlng_t;


/*
 * A "bounding box" providing the SouthWest and NorthEast points:
 */
typedef struct ocgeo_latlng_bounds {
	ocgeo_latlng_t northeast;
	ocgeo_latlng_t southwest;
} ocgeo_latlng_bounds_t;

/*
 * A matching result in the API's response. 
 */
typedef struct ocgeo_result {
	int confidence;
	char* formatted;

	/* "bounds", may be "null" (invalid): */
	ocgeo_latlng_bounds_t bounds;

	/* "components": */
	char* ISO_alpha2;
	char* ISO_alpha3;
	char* type;
	char* city;
	char* city_district;
	char* continent;
	char* country;
	char* country_code;
	char* county;
	char* house_number;
	char* neighbourhood;
	char* political_union;
	char* postcode;
	char* road;
	char* state;
	char* state_district;
	char* suburb;
} ocgeo_result_t;

typedef struct ocgeo_response {
	/* Returned status */
	ocgeo_status_t status;

	/* Rate information. If not returned (e.g. for paying customers)
	   all its fields should be 0.
	 */
	ocgeo_rate_info_t rateInfo;

	int total_results;
	ocgeo_result_t* results;

	void* internal;
} ocgeo_response_t;

typedef struct ocgeo_params {
	void* callback_data;
	void (*dbg_callback)(const char*, void*);

	/*
	 * Normal parameters : 
	 */

	/* When set to 1 the API attempts to abbreviate and shorten the formatted string */
	int abbrv; 
	
	/* Used only for forward geocoding. This value will restrict the possible 
	   results to a defined bounding box. */
	ocgeo_latlng_bounds_t bounds;
	
	/* Used only for forward geocoding. Restricts results to the specified 
	   country/territory or countries. The country code is a two letter code as 
	   defined by the ISO 3166-1 Alpha 2 standard. E.g. gb for the
	   United Kingdom, fr for France, us for United States. */
	char* countrycode;

	/* An IETF format language code (such as es for Spanish or pt-BR for 
	   Brazilian Portuguese), or native in which case we will attempt to return
	   the response in the local language(s). */
	char* language;

	/* The maximum number of results we should return. Default is 10. 
	   Maximum allowable value is 100.*/
	int limit;

	/* An integer from 1-10. Only results with at least this confidence will be returned. */
	int min_confidence;
	
	/* When set to 1 results will not contain annotations. */
	int no_annotations;
	
	/* When set to 1 results will not be deduplicated. */
	int no_dedupe;
	
	/* When set to 1 the query contents are not logged. Please use if you have 
	concerns about privacy and want us to have no record of your query.*/
	int no_record;

	/* When set to 1 results are 'pretty' printed for easier reading. Useful for debugging. */
	int pretty;

	/* Used only for forward geocoding. Provides the geocoder with a hint to 
	   bias results in favour of those closer to the specified location. Please 
	   note though, this is just one of many factors in the internal scoring we 
	   use for ranking results. */
	ocgeo_latlng_t proximity;

	/* When set to 1 the behaviour of the geocoder is changed to 
	   attempt to match the nearest road (as opposed to address). */
	int roadinfo;
} ocgeo_params_t;

extern ocgeo_params_t ocgeo_default_params(void);
extern ocgeo_response_t* ocgeo_forward(const char* query, const char* api_key, ocgeo_params_t* params, ocgeo_response_t* response);
extern ocgeo_response_t* ocgeo_reverse(double lat, double lng, const char* api_key, ocgeo_params_t* params, ocgeo_response_t* response);
extern void ocgeo_response_cleanup(ocgeo_response_t* r);

/* Some utils: */

static inline
int ocgeo_response_ok(ocgeo_response_t* response)
{
    if (response == NULL)
        return 0;
    return response->status.code == OCGEO_CODE_OK;
}

static inline
int ocgeo_is_valid_latlng(ocgeo_latlng_t coords)
{
	return -90.0 <=coords.lat && coords.lat <= 90.0 &&
		-180.0 <=coords.lng && coords.lng <= 180.0;
}

static inline
int ocgeo_is_valid_bounds(ocgeo_latlng_bounds_t* bbox)
{
	return bbox != NULL
		&& ocgeo_is_valid_latlng(bbox->northeast)
	 	&& ocgeo_is_valid_latlng(bbox->southwest);
}

extern char* ocgeo_version;

#ifdef __cplusplus
}
#endif

#endif
