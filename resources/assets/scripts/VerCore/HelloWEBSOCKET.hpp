#ifndef HELLOENGINE_HELLOWEBSOCKET_HPP
#define HELLOENGINE_HELLOWEBSOCKET_HPP

#include <string>
#include <functional>
#include <cstdio>
#include <cstring>
#include <cstdint>

#ifndef SHA_LBLOCK
#define SHA_LBLOCK      16
#endif
#ifndef SHA_CBLOCK
#define SHA_CBLOCK      (SHA_LBLOCK*4)
#endif

#ifndef SHA_DIGEST_LENGTH
#define SHA_DIGEST_LENGTH 20
#endif

class HelloWS {
public:
    HelloWS(const std::string& address, int port);
    ~HelloWS();
    void run(std::function<void(HelloWS*, int, const std::string&, int)> program, int max_connection = 100);
    void send(int client_socket, const std::string& message);
    std::string recv(int client_socket);
    void close_server();

private:
    void handle_client(int client_socket, std::function<void(HelloWS*, int, const std::string&, int)> program);
    std::string ws_encode(const std::string& data, int opcode = 1 /* OPCODE_TEXT */, bool mask = true);
    std::string get_masked(const std::string& data);

    //------------------------------------------------------------
    //                  Encode to Base64
    //----------------
    const char* G_sBase64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int EncodeBase64(unsigned char *t, const unsigned char *f, int n) {
        int i, j, s;
        unsigned int val;
        for (i = 0, j = 0; i < n; i += 3) {
            val = f[i];
            val <<= 8;
            if (i + 1 < n) val |= f[i + 1];
            val <<= 8;
            if (i + 2 < n) val |= f[i + 2];

            t[j++] = G_sBase64Chars[(val >> 18) & 0x3F];
            t[j++] = G_sBase64Chars[(val >> 12) & 0x3F];
            if (i + 1 < n) {
                t[j++] = G_sBase64Chars[(val >> 6) & 0x3F];
            } else {
                t[j++] = '=';
            }
            if (i + 2 < n) {
                t[j++] = G_sBase64Chars[val & 0x3F];
            } else {
                t[j++] = '=';
            }
        }

        t[j] = '\0'; // Null-terminating the output string
        return j; // Returns the length of the encoded string
    }
    //------------------------------------------------------------

    std::string server_address;
    int server_port;
    int sockfd;
    bool running_serv = true;
};

#endif
