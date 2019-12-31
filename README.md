# c-ocgeo

This is a [C99](https://en.wikipedia.org/wiki/C99) library for accessing the [OpenCage Geocoder](https://opencagedata.com/) [API](https://opencagedata.com/api). 



## Building 

We assume a POSIX compliant environment. We have tested it in MacOSX and OpenBSD. The sole prerequisite is the availability of [libcurl](https://curl.haxx.se/libcurl/) that is used for making the HTTP requests. It additionally uses two very nice C libraries that are contained in source form in the code repository:

* [cJSON](https://github.com/DaveGamble/cJSON), an ultralightweight JSON parser in ANSI C
* [Simple Dynamic Strings (sds)](https://github.com/antirez/sds) (version 1.0.0) as a string library

There is a (GNU) Makefile for building it as a static library (`libocgeo.a`) but you are free to copy the source files and use you own build process. You may need to adjust the Makefile if for example the header files and library file of libcurl is not installed in some "well-known" directory (usually `/usr/local/include` and `/usr/local/lib` respectively).

## Usage

First of all it's important to check the [best practices for using the OpenCage API](https://opencagedata.com/api#bestpractices), and in particular [how to format forward geocoding queries](https://github.com/OpenCageData/opencagedata-roadmap/blob/master/query-formatting.md).

You can see the `example.c` file for an example. The recommended use is as follows:

* First of all, the user should have an API key by registering with the OpenCage Geocoder, otherwise the library can not be used (well, unless you use some [testing API keys](https://opencagedata.com/api#testingkeys)).

* Initialization of an `ocgeo_params` struct that contains various ([optional](https://opencagedata.com/api#forward-opt)) parameters for the request:

  ```C
  ocgeo_params_t params = ocgeo_default_params_init();
  ```

* The user can then overwrite the default parameters if s/he so wishes, e.g.

  ```C
  params.language = "hi"; // Hindi
  params.min_confidence = 5;
  params.countrycode = "in" // India
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

  



## Miscellaneous

* The library sends requests using the following User Agent HTTP header :

  ```
  c-ocgeo/<version> (<curl-version>)
  ```

  where `<version>` comes from the Git commit tag or hash and `<curl-version>` is the version of the libcurl used.