#include "error.h"
#include "socket_layer.h"
#include "string.h"
#include "util.h"

#include <netinet/in.h>

const int MAX_ANSWER_LENGTH = 2048 + 5;
const char *const end = "<EOF>";
const char *const smallFileAck = "small";
const char *const bigFileAck = "big";
const char *const ack = "ACK";

int main(int argc, char *argv[]) {

    if (argc != 3) {
        return ERR_NOT_ENOUGH_ARGUMENTS;
    } else {
        argc--;
        argv++; // skips ./
        FILE *file = fopen(argv[1], "rb");
        if (file == NULL) {
            return ERR_IO;
        }

        fseek(file, 0, SEEK_END);
        long length = ftell(file);
        if (length < 0) {
            return ERR_IO;
        } else if (length > 2048) {
            return ERR_INVALID_ARGUMENT;
        }

        uint16_t port = atouint16(argv[0]);

        int socketID = socket(AF_INET, SOCK_STREAM, 0);
        if (socketID == -1) {
            perror("fail in socket creation");
            return ERR_IO;
        }

        struct sockaddr_in server_address;
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(port);
        server_address.sin_addr.s_addr = htonl(INADDR_ANY);

        if (connect(socketID, &server_address, sizeof(server_address))) {
            perror("fail in connecting");
            return ERR_IO;
        }

        const char initMess[MAX_ANSWER_LENGTH + 1] = {0};
        sprintf(initMess, "%ld", length);
        strcat(initMess, end);

        // sends file length
        tcp_send(socketID, initMess, MAX_ANSWER_LENGTH);

        // file ACK
        const char fileAckMess[MAX_ANSWER_LENGTH + 1] = {0};
        tcp_read(socketID, fileAckMess, MAX_ANSWER_LENGTH);

        // TOO BIG
        if (!strncmp(fileAckMess, bigFileAck, 3)) {
            return 0;
        }

        // SMALL ENOUGH
        if (!strncmp(fileAckMess, smallFileAck, 3)) {
            // send file
            fseek(file, 0, SEEK_SET);
            const char fileMess[MAX_ANSWER_LENGTH + 1] = {0};

            if (fread(fileMess, length, 1, file) != 1) {
                return ERR_IO;
            }
            strcat(fileMess, end);
            tcp_send(socketID, fileMess, MAX_ANSWER_LENGTH);


            // end ACK
            const char lastMess[MAX_ANSWER_LENGTH + 1] = {0};
            tcp_read(socketID, lastMess, MAX_ANSWER_LENGTH);
            if (!strncmp(lastMess, ack, 3)) {
                return -1;
            }
            return 0;
        }

        return 0;
    }
}
