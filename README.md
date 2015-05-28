# jv_http
Single-header library for simple HTTP requests

This library does simple HTTP requests, with custom headers if desired. It's 
designed to be as easy to integrate as possible. Look at the file itself to 
see some usage explanations and a test case.

Limitations:
----
 - Right now only the winsock version is implemented, but a BSD socket 
version is planned too.
 - Chunked and gzipped transport are not implemented, and neither is HTTPS. 
 Thus, this library is limited to work in the HTTP 1.0 subset, and it still 
 doesn't implement the whole standard. Only just enough to get by.

Example use
----
Without error checking, a simple ping request (for analytics, for example) 
would be done like this:

    jvh_env jvhEnvironment;
    jvh_init(&jvhEnvironment);

    jvh_response Response;
    jvh_simple_get(&jvhEnvironment, "mywebsite.com", "80", "/analytics?ID=11111", &Response));
	if(Response.status_code != 200){
		// Something went wrong!
	}
    jvh_close(&Response);
    jvh_stop(&jvhEnvironment);


For a more complete example that checks the response body, there is a full 
test case inside the library source file.

To build the test case, do:

    cl.exe -D JV_HTTP_TEST -TC "jv_http.h" -link

This will produce a test program that downloads a url and prints it to stdout.

All comments and improvements are welcome.
