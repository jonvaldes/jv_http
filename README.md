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

    jvh_env env;
    jvh_init(&env);

    jvh_response r;
    jvh_simple_get(&env, "mywebsite.com", "80", "/analytics?ID=11111", &r));
	if(r.status_code != 200){
		// Something went wrong!
	}
    jvh_close(&r);
    jvh_stop(&env);


A more complete example that checks the response body:

    #define RESPONSE_SIZE 2048
    jvh_error err;
    jvh_env env;
    jvh_init(&env);
    jvh_response r;
    jvh_simple_get(&env, "hombrealto.com", "80", "/", &r);
    char buf[RESPONSE_SIZE + 1];
    int bytes_read;
    jvh_recv_chunk(&r, buf, RESPONSE_SIZE, &bytes_read);
    while(bytes_read != 0) {
        buf[bytes_read] = 0;
        printf("%s", buf);
        jvh_recv_chunk(&r, buf, RESPONSE_SIZE, &bytes_read);
    }
    jvh_close(&r);
    jvh_stop(&env);

To build the test case, do:

    cl.exe -D JV_HTTP_TEST -TC "jv_http.h" -link

This will produce a test program that downloads a url and prints it to stdout.

All comments and improvements are welcome.
