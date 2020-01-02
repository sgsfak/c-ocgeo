# c-ocgeo

This is a [C99](https://en.wikipedia.org/wiki/C99) library for accessing the [OpenCage Geocoder](https://opencagedata.com/) [API](https://opencagedata.com/api). 



## Building 

We assume a POSIX compliant environment. We have tested it in MacOSX, OpenBSD, and Linux. It should work in Windows too although not tested yet. The sole prerequisite is the availability of [libcurl](https://curl.haxx.se/libcurl/) that is used for making the HTTP requests. It additionally uses two very nice C libraries that are contained in source form in the code repository:

* [cJSON](https://github.com/DaveGamble/cJSON), an ultralightweight JSON parser in ANSI C
* [Simple Dynamic Strings (sds)](https://github.com/antirez/sds) (version 1.0.0) as a string library

There is a (GNU) Makefile for building it as a static library (`libocgeo.a`) but you are free to copy the source files and use you own build process. You may need to adjust the Makefile if for example the header files and library file of libcurl is not installed in some "well-known" directory (usually `/usr/local/include` and `/usr/local/lib` respectively).

## Usage

First of all it's important to check the [best practices for using the OpenCage API](https://opencagedata.com/api#bestpractices), and in particular [how to format forward geocoding queries](https://github.com/OpenCageData/opencagedata-roadmap/blob/master/query-formatting.md).

You can see the `example.c` file for an example. The recommended use is as follows:

* First of all, the user should have an API key by registering with the OpenCage Geocoder, otherwise the library can not be used (well, unless you use some [testing API keys](https://opencagedata.com/api#testingkeys)).

* Initialization of an `ocgeo_params` struct that contains various ([optional](https://opencagedata.com/api#forward-opt)) parameters for the request:

  ```C
  ocgeo_params_t params = ocgeo_default_params();
  ```

* The user can then overwrite the default parameters if s/he so wishes, e.g.

  ```C
  params.language = "hi"; // Hindi
  params.min_confidence = 5;
  params.countrycode = "in"; // India
  ```

* Make a "forward" request providing the API key and your query:

  ```C
  ocgeo_response_t response;
  const char* query = "Syena, Aswan Governorate, Egypt";
  ocgeo_forward(query, api_key, &params, &response);
  ```

  Or, make a "reverse" lookup request providing a latitude and longitude:

  ```C
  double lat = 24.0875;
  double lon = 32.898889;
  ocgeo_reverse(lat, lon, api_key, &params, &response);
  ```

* Check the returned code for possible errors:

  ```C
  if (ocgeo_response_ok(&response)) {
    // Successful!
    // ...
  }
  else {
    printf("API returned error: %s\n", response.status.message);
  }
  ```

* If successful, iterate over the results:

  ```C
  printf("Got %d results:\n", response.total_results);
  for (int i=0; i<response.total_results; ++i) {
     ocgeo_result_t* result = response.results + i;
     printf("%2d. %s (type: %s, conf:%d)\n", i+1, 
         result->formatted, result->type, result->confidence);
  }
  ```

* Finally (*do not forget this!*) cleanup the response to free the memory used:

  ```C
  ocgeo_response_cleanup(&response);
  ```

  

## Design

* A decimal latitude or longitude is represented as `double` This is to ensure that more [precision](https://en.wikipedia.org/wiki/Decimal_degrees#Precision) is possible in specifying geographic coordinates.
* The caller is responsible for the management of the memory. The design of the `ocgeo_params_t` parameters struct permits the declaration of corresponding variables in the stack or the heap. The library cannot shun the dynamic allocation for internal fields of the response (`ocgeo_response_t`) structure and thus the caller should always call `ocgeo_response_cleanup` after any request.
* Some fields of the response (`ocgeo_response_t`) structure are optional. The caller should always check for NULL values in the pointers therein.

## Miscellaneous

* The library sends requests using the following User Agent HTTP header :

  ```
  c-ocgeo/<version> (<curl-version>)
  ```

  where `<version>` comes from the Git commit tag or hash and `<curl-version>` is the version of the libcurl used.

