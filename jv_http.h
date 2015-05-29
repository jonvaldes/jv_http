/* jv_http - v0.1 public domain http library http://github.com/jonvaldes/jv_http
                               no warranty implied; use it at your own risk

   Do this:
     #define JV_HTTP_IMPLEMENTATION
   before you include this file in *one* C or C++ file to create the implementation.

   i.e. it should look like this:
   	 #include ...
     #include ...
     #include ...
     #define JV_HTTP_IMPLEMENTATION
     #include "jv_http.h"

Usage:

    As a gentle introduction, here's the code needed to send a simple analytics
    request:

        jvh_env jvhEnvironment;
        jvh_init(&jvhEnvironment);

        jvh_response Response;
        jvh_simple_get(&jvhEnvironment, "mywebsite.com", "80", "/analytics?ID=11111", &Response));
        if(Response.status_code != 200){
            // Something went wrong!
        }
        jvh_close(&Response);
        jvh_stop(&jvhEnvironment);

    There is also a full example in the section named "Example program". That 
    should be enough to get you started. 

    Also, in the "Library configuration" section just below, you'll find all 
    available options to modify the library to your needs, like the user-agent
    or the memory allocation functions

License:
   This software is in the public domain. Where that dedication is not
   recognized, you are granted a perpetual, irrevocable license to copy
   and modify this file however you want.

Acknowledgements:
   This library was inspired by Sean T. Barrett's excellent "stb" libraries, available at http://nothings.org
*/

#ifndef JV_INCLUDE_HTTP_H
#define JV_INCLUDE_HTTP_H

// ----------------------------------------------
// Library configuration
//
#ifdef JV_HTTP_STATIC
#define JVHDEF static
#else
#define JVHDEF extern
#endif

// Substitute these two to use your own memory management functions
#ifndef JV_HTTP_MALLOC
#define JV_HTTP_MALLOC(sz) malloc(sz)
#define JV_HTTP_FREE(p) free(p)
#endif

#ifndef JV_HTTP_RESPONSE_BUFFER_LEN
#define JV_HTTP_RESPONSE_BUFFER_LEN 8192 // Note: right now this has to be big
#endif                                   // enough to hold the entire HTTP header

#ifndef JV_HTTP_USER_AGENT
#define JV_HTTP_USER_AGENT "Mozilla/5.0 (Windows NT 6.1; WOW64) \
                            AppleWebKit/537.36 (KHTML, like Gecko) \
                            Chrome/42.0.2311.90 Safari/537.36"
#endif

#ifndef JV_HTTP_PROTOCOL
#define JV_HTTP_PROTOCOL "HTTP/1.0" // Note: We use 1.0 because 1.1 has chunked/gzipped
#endif                              // transports, and we don't support those yet

//
// ----------------------------------------------

#include <string.h>

#ifdef _MSC_VER

#define JV_PLATFORM_WINSOCK
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32")
#pragma comment(lib, "Mswsock")

#else // !_MSC_VER

#define JV_PLATFORM_BSDSOCK

#endif

struct jvh_env;
typedef enum {
    JVH_ERR_OK = 0,
    JVH_ERR_UNSUPPORTED,
    JVH_ERR_OUT_OF_MEMORY,
    JVH_ERR_PERMISSION_DENIED,

    JVH_ERR_INVALID_RESPONSE,

    JVH_ERR_CONN_REFUSED,
    JVH_ERR_CONN_ABORTED,
    JVH_ERR_CONN_RESET,

    JVH_ERR_HOST_UNREACHABLE,
    JVH_ERR_NET_UNREACHABLE,
    JVH_ERR_NET_DOWN,
    JVH_ERR_TIMEOUT,

    JVH_ERR_BAD_MEM_ADDRESS,
    JVH_ERR_INVALID_ARGUMENT,

    JVH_ERR_TOO_MANY_CONNS,

    JVH_ERR_UNKNOWN = 5000,
} jvh_error;

typedef struct {
    int status_code;
    char _buffer[JV_HTTP_RESPONSE_BUFFER_LEN];
    int _bytes_in_buffer;
    int _buffer_offset;
    char _internal[8]; // Enough bytes to hold internal platform-dependent data
} jvh_response;

JVHDEF jvh_error jvh_init(struct jvh_env *env);
JVHDEF jvh_error jvh_stop(struct jvh_env *env);

JVHDEF jvh_error jvh_request(struct jvh_env *env, const char *server_name, const char *port, jvh_response *response, const char *requestTemplate, ...);

JVHDEF jvh_error jvh_simple_req(struct jvh_env *env, const char *method, const char *server_name, const char *port, const char *url, const char *contents, jvh_response *response) {
    return jvh_request(env, server_name, port, response,
                       "%s %s " JV_HTTP_PROTOCOL "\r\n\
                        Host: %s\r\n\
                        User-Agent:" JV_HTTP_USER_AGENT "\r\n\r\n%s",
                       method, url, server_name, contents);
}

JVHDEF jvh_error jvh_simple_get(struct jvh_env *env, const char *server_name, const char *port, const char *url, jvh_response *response) {
    return jvh_simple_req(env, "GET", server_name, port, url, "", response);
}

JVHDEF jvh_error jvh_recv_chunk(jvh_response *response, char *return_buffer, int buffer_size, int *bytes_read);
JVHDEF jvh_error jvh_close(jvh_response *response);

#ifdef JV_PLATFORM_WINSOCK
// -----------------
//  Windows structs
//
typedef struct jvh_env {
    WSADATA wsa;
} jvh_env;

#define JV_HTTP_STATIC_ASSERT(COND, MSG) typedef char static_assertion_##MSG[(COND) ? 1 : -1]

JV_HTTP_STATIC_ASSERT(sizeof(SOCKET) <= sizeof(((jvh_response *)0)->_internal), jvh_response_internal_field_must_be_big_enough_for_SOCKET);
//
// -----------------

#else // JV_PLATFORM_WINSOCK
// ---------------------
//  BSD Sockets structs
//

// @TODO

//
// ---------------------

#endif // !JV_PLATFORM_WINSOCK

// ===========================================================================
//  Example program:
// ===========================================================================

#ifdef JV_HTTP_TEST

#define CHECK(x)                                              \
    if((err = x) != JVH_ERR_OK) {                             \
        printf("ERROR: code %d at line %d\n", err, __LINE__); \
        exit(-1);                                             \
    }

#define RESPONSE_SIZE 2048

int main(int argc, wchar_t *argv[]) {
    jvh_error err;
    jvh_env env;
    CHECK(jvh_init(&env));
    jvh_response r;
    CHECK(jvh_simple_get(&env, "hombrealto.com", "80", "/", &r));
    printf("Response code: %d\n", r.status_code);
    printf("Response contents:\n");
    char buf[RESPONSE_SIZE + 1];
    int bytes_read;
    CHECK(jvh_recv_chunk(&r, buf, RESPONSE_SIZE, &bytes_read));
    while(bytes_read != 0) {
        buf[bytes_read] = 0;
        printf("%s", buf);
        CHECK(jvh_recv_chunk(&r, buf, RESPONSE_SIZE, &bytes_read));
    }
    CHECK(jvh_close(&r));
    CHECK(jvh_stop(&env));

#undef CHECK
    return 0;
}

#define JV_HTTP_IMPLEMENTATION

#endif // JV_HTTP_TEST

// ===========================================================================
// ===========================================================================

#ifdef JV_HTTP_IMPLEMENTATION

#ifdef JV_PLATFORM_WINSOCK
// -----------------
//  Windows implementation
// -----------------

jvh_error jvh__translate_wsaerror(int err) {
    // clang-format off
    switch(err) {
        case 0:                     return JVH_ERR_OK;
        case WSAEACCES:             return JVH_ERR_PERMISSION_DENIED;
        case WSAEHOSTUNREACH:       return JVH_ERR_HOST_UNREACHABLE;
        case WSAENETUNREACH:        return JVH_ERR_NET_UNREACHABLE;
        case WSA_NOT_ENOUGH_MEMORY: return JVH_ERR_OUT_OF_MEMORY;
        case WSAEAFNOSUPPORT:       // v
        case WSAEOPNOTSUPP:         // v
        case WSAEPROTONOSUPPORT:    // v
        case WSAESOCKTNOSUPPORT:    // v
        case WSAVERNOTSUPPORTED:    return JVH_ERR_UNSUPPORTED;
        case WSAETIMEDOUT:          return JVH_ERR_TIMEOUT;
        case WSAECONNABORTED:       return JVH_ERR_CONN_ABORTED;
        case WSAECONNREFUSED:       return JVH_ERR_CONN_REFUSED;
        case WSAENETRESET:          // v
        case WSAECONNRESET:         return JVH_ERR_CONN_RESET;
        case WSAEINVALIDPROCTABLE:  // v
        case WSAEFAULT:             return JVH_ERR_BAD_MEM_ADDRESS;
        case WSAEPROTOTYPE:         // v
        case WSAEINVAL:             return JVH_ERR_INVALID_ARGUMENT;
        case WSAENETDOWN:           return JVH_ERR_NET_DOWN;
        case WSAEPROCLIM:           // v
        case WSAEMFILE:             return JVH_ERR_TOO_MANY_CONNS;

        // @TODO: Keep assigning jvh_error_codes to these errors
        case WSAEADDRINUSE:
        case WSAEADDRNOTAVAIL:
        case WSAEALREADY:
        case WSAEINPROGRESS:
        case WSAEINTR:
        case WSAEISCONN:
        case WSAEMSGSIZE:
        case WSAENOBUFS:
        case WSAENOTCONN:
        case WSAENOTSOCK:
        case WSAEPROVIDERFAILEDINIT:
        case WSAESHUTDOWN:
        case WSAEWOULDBLOCK:
        case WSAHOST_NOT_FOUND:
        case WSANOTINITIALISED:
        case WSANO_RECOVERY:
        case WSASYSNOTREADY:
        case WSATRY_AGAIN:
        case WSATYPE_NOT_FOUND:

        //
        case WSAEINVALIDPROVIDER:   // v
        default:                    return JVH_ERR_UNKNOWN;
    }
    // clang-format on
}

JVHDEF jvh_error jvh_init(jvh_env *env) {
    int errcode = WSAStartup(MAKEWORD(2, 2), &env->wsa);
    return jvh__translate_wsaerror(errcode);
}

JVHDEF jvh_error jvh_stop(jvh_env *env) {
    int errcode = WSACleanup();
    return jvh__translate_wsaerror(errcode);
}

#define RESP_SOCKET(resp) (*(SOCKET *)resp->_internal)

static int jvh__connect(struct jvh_env *env, const char *server_name, const char *port, jvh_response *response) {
    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *server_addr;

    // Resolve the server address and port
    int addr_info_error = getaddrinfo(server_name, port, &hints, &server_addr);
    if(addr_info_error != 0) {
        return jvh__translate_wsaerror(addr_info_error);
    }
    memset(response, 0, sizeof(jvh_response));

    RESP_SOCKET(response) = INVALID_SOCKET;

    // Attempt to connect to an address until one succeeds
    for(struct addrinfo *ptr = server_addr; ptr; ptr = ptr->ai_next) {
        // Create a SOCKET for connecting to server
        RESP_SOCKET(response) = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if(RESP_SOCKET(response) == INVALID_SOCKET) {
            return jvh__translate_wsaerror(WSAGetLastError());
        }

        // Connect to server.
        if(connect(RESP_SOCKET(response), ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR) {
            closesocket(RESP_SOCKET(response));
            RESP_SOCKET(response) = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(server_addr);

    if(RESP_SOCKET(response) == INVALID_SOCKET) {
        return jvh__translate_wsaerror(WSAGetLastError());
    }

    return JVH_ERR_OK;
}

static jvh_error jvh__send(jvh_response *response, const char *data, int datalen) {
    if(send(RESP_SOCKET(response), data, datalen, 0) == SOCKET_ERROR) {
        jvh_error err = jvh__translate_wsaerror(WSAGetLastError());
        closesocket(RESP_SOCKET(response));
        return err;
    }
    return JVH_ERR_OK;
}

JVHDEF jvh_error jvh_recv_chunk(jvh_response *response, char *return_buffer, int buffer_size, int *bytes_read) {
    // First check if we have something buffered in the response
    if(response->_buffer_offset != 0 && response->_buffer_offset < response->_bytes_in_buffer) {
        int left_in_buffer = response->_bytes_in_buffer - response->_buffer_offset;
        int bytes_to_copy = left_in_buffer;
        if(bytes_to_copy > buffer_size) {
            bytes_to_copy = buffer_size;
        }
        memcpy(return_buffer, response->_buffer + response->_buffer_offset, bytes_to_copy);
        response->_buffer_offset += bytes_to_copy;
        *bytes_read = bytes_to_copy;
        return JVH_ERR_OK;
    }

    // Ok, no buffered data, request more from server
    int _bytes_read = recv(RESP_SOCKET(response), return_buffer, buffer_size, 0);
    if(_bytes_read > 0) {
        *bytes_read = _bytes_read;
        return JVH_ERR_OK;
    } else if(_bytes_read == 0) {
        *bytes_read = 0;
        return JVH_ERR_OK;
    } else {
        *bytes_read = 0;
        jvh_error err = jvh__translate_wsaerror(WSAGetLastError());
        closesocket(RESP_SOCKET(response));
        return err;
    }
}

JVHDEF jvh_error jvh_close(jvh_response *response) {
    if(closesocket(RESP_SOCKET(response)) == SOCKET_ERROR) {
        return jvh__translate_wsaerror(WSAGetLastError());
    }
    return JVH_ERR_OK;
}

static int jvh__str_find_first(const char *s, char c, int max_len) {
    int result = -1;
    for(int i = 0; *s && i < max_len; s++, i++) {
        if(*s == c) {
            result = i;
            break;
        }
    }
    return result;
}

static jvh_error jvh__parse_headers(jvh_response *response) {
    int bytes_read;
    int errcode;
    if((errcode = jvh_recv_chunk(response, response->_buffer, JV_HTTP_RESPONSE_BUFFER_LEN, &bytes_read)) != 0) {
        return jvh__translate_wsaerror(errcode);
    }
    if(bytes_read == 0) {
        return JVH_ERR_INVALID_RESPONSE; // server didn't return anything
    }
    // First line of response is of the form: HTTP/1.1 200 OK
    // So we want to find the first space char
    int status_code_pos = jvh__str_find_first(response->_buffer, ' ', bytes_read);
    response->status_code = atoi(response->_buffer + status_code_pos);

    // Ignore subsequent lines until we reach the \r\n\r\n
    int cur_pos = 0;
    int next_line_pos = jvh__str_find_first(response->_buffer, '\n', bytes_read);
    while(next_line_pos - cur_pos > 2) {
        cur_pos = next_line_pos + 1;
        next_line_pos = cur_pos + jvh__str_find_first(response->_buffer + cur_pos, '\n', bytes_read - cur_pos);
    }

    response->_buffer_offset = next_line_pos + 1;
    response->_bytes_in_buffer = bytes_read;
    return JVH_ERR_OK;
}

JVHDEF jvh_error jvh_request(struct jvh_env *env, const char *server_name, const char *port, jvh_response *response, const char *request_template, ...) {
    int errcode = 0;
    jvh_error result = JVH_ERR_OK;
    char *reqData = NULL;

    response->status_code = -1;

    if((errcode = jvh__connect(env, server_name, port, response)) != 0) {
        goto handleErr;
    }
    va_list args;
    va_start(args, request_template);

    int reqDataLen = vsnprintf(NULL, 0, request_template, args);
    reqData = (char *)JV_HTTP_MALLOC(reqDataLen + 1);
    vsprintf(reqData, request_template, args);
    reqData[reqDataLen] = 0;

    if((errcode = jvh__send(response, reqData, reqDataLen)) != 0) {
        goto handleErr;
    }

    if((errcode = jvh__parse_headers(response)) != 0) {
        goto handleErr;
    }

    goto tearDown;

handleErr:
    result = jvh__translate_wsaerror(errcode);
    jvh_close(response);

tearDown:
    JV_HTTP_FREE(reqData);
    return result;
}

#else
// ----------------------------
//  BSD Sockets implementation
// ----------------------------

// @TODO: do all this!
#endif // !JV_PLATFORM_WINSOCK

#endif // JV_HTTP_IMPLEMENTATION
#endif // JV_INCLUDE_HTTP_H
