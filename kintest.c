#include <kinetic_admin_client.h>
#include <kinetic_client.h>
#include <syscall.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <openssl/sha.h>

#define DISK "192.168.0.136"

KineticStatus writeToDisk(KineticSession *session, size_t start, size_t end, size_t increment, int sleeptime) {
    KineticStatus status = 0;
    assert(session);

    for(size_t size = start; size < end; size = size + increment) {

        // Create a unique key per write
        const char key[] = "warmUpKey";
        ByteBuffer put_key_buf = ByteBuffer_Malloc( strlen(key) + 15);
        ByteBuffer_Append(&put_key_buf, key, strlen(key));
        char keySuffix[15];
        snprintf(keySuffix, sizeof(keySuffix), "%02d", (int)size);
        ByteBuffer_AppendCString(&put_key_buf, keySuffix);


        // Create a value
        ByteBuffer put_value_buf = ByteBuffer_Malloc(size);
        ByteBuffer_AppendDummyData(&put_value_buf, put_value_buf.array.len);

        /* Populate tag with SHA1 of value */
        ByteBuffer put_tag_buf = ByteBuffer_Malloc(SHA_DIGEST_LENGTH);
        uint8_t sha1[SHA_DIGEST_LENGTH];
        SHA1(put_value_buf.array.data, put_value_buf.bytesUsed, &sha1[0]);
        ByteBuffer_Append(&put_tag_buf, sha1, SHA_DIGEST_LENGTH);

        KineticEntry put_entry = {
            .key = put_key_buf,
            .value = put_value_buf,
            .tag = put_tag_buf,
            .force = true, // just in case
            .algorithm = KINETIC_ALGORITHM_SHA1,
            /* Set sync to WRITETHROUGH, which will wait to complete
             * until the drive has persistend the write. (WRITEBACK
             * returns as soon as the drive has buffered the write.) */
            .synchronization = KINETIC_SYNCHRONIZATION_WRITEBACK,
        };

        status = KineticClient_Put(session, &put_entry, NULL);

        ByteBuffer_Free(put_value_buf);
        ByteBuffer_Free(put_tag_buf);
        ByteBuffer_Free(put_key_buf);

        fprintf(stdout,"\rCurrent value size = %d", (int)size);
        fflush(stdout);

        if (KINETIC_STATUS_SUCCESS != status) {
            fprintf(stderr, "\nWritefailed: %s %s at %d\n", Kinetic_GetStatusDescription(status), Kinetic_GetStatusDescription(KineticClient_GetTerminationStatus(session)), (int)size);
            return status;
        }else{
            if(sleeptime > 0){
                sleep(sleeptime); // This actually triggers the bug 
            }
        }
    }

    printf("\n");
    return KINETIC_STATUS_SUCCESS;
}


KineticSession *createSession(KineticClient *client, bool usessl){
    assert(client);

    KineticSession *session = NULL;

    const char HmacKeyString[] = "asdfasdf";

    KineticSessionConfig sessionConfig = {
        .clusterVersion = 0,
        .identity = 1,
        .host = DISK,
        .hmacKey = ByteArray_CreateWithCString(HmacKeyString)
    };

    if(usessl){
        sessionConfig.useSsl = true;
        sessionConfig.port = 8443;
    }else{
        sessionConfig.useSsl = false;
        sessionConfig.port = 8123;
    }

    KineticStatus status = KineticClient_CreateSession(&sessionConfig, client, &session);
    assert(KINETIC_STATUS_SUCCESS == status);

    // This can be omitted
    status = KineticClient_NoOp(session);
    assert(KINETIC_STATUS_SUCCESS == status);
    return session;
}

void destroySession(KineticSession * session){
  KineticClient_DestroySession(session);   
}

KineticClient *createClient(){
    char name[20] = {0};
    sprintf(name, "%s", "kinetic.log");

    KineticClientConfig clientConfig = {
        .logFile = name,
        .logLevel = 1, // set this to 5 for verbose logs
    };

    KineticClient *client = KineticClient_Init(&clientConfig);
    assert(client);
    return client;
}

void destroyClient(KineticClient * client){
    KineticClient_Shutdown(client);
}

int main(int argc, const char *argv[]) {

    KineticClient *client = NULL;
    KineticSession *session = NULL;

    printf("Test 1\n\n");

    client = createClient();
    session = createSession(client, false);    

    // write value size 1KB to 1MB with 1K increment, no ssl, no sleep in between puts
    writeToDisk(session, 1024, 1024*1024, 1024, 0); // this works

    destroySession(session);
    destroyClient(client);


    printf("\nTest 2\n\n");

    client = createClient();
    session = createSession(client, false);

    // write value size 10KB to 30KB with 1K increment, no ssl, no sleep in between puts
    writeToDisk(session, 10240, 30720, 1024, 0); // this works

    destroySession(session);
    destroyClient(client);


    printf("\nTest 3\n\n");

    client = createClient();
    session = createSession(client, true);

    // write value size 10KB to 30KB with 1K increment, with ssl, no sleep in between puts
    writeToDisk(session, 10240, 30720, 1024, 0); // this works

    destroySession(session);
    destroyClient(client);


    printf("\nTest 4\n\n");

    client = createClient();
    session = createSession(client, true);
    
    // write value size 10KB to 30KB with 1K increment, with ssl, 1s sleep in between puts
    writeToDisk(session, 10240, 30720, 1024, 1); // this fails

    destroySession(session);
    destroyClient(client);


    printf("\nTest 5\n\n");

    client = createClient();
    session = createSession(client, true);
    
    // write value size 15KB to 30KB with 1K increment, with ssl, 0s sleep in between puts
    writeToDisk(session, 15360, 30720, 1024, 0); // this fails
    
    destroySession(session);
    destroyClient(client);


    printf("\nTest 6\n\n");

    client = createClient();
    session = createSession(client, true);
    // write value size 13KB to 30KB with 1K increment, with ssl, 1s sleep in between puts
    writeToDisk(session, 13312, 30720, 1024, 1); // this fails
    destroySession(session);
    destroyClient(client);


    printf("\nTest 7\n\n");

    client = createClient();
    session = createSession(client, true);
    // write value size 14000B to 30KB with 1B increment, with ssl, 1s sleep in between puts
    writeToDisk(session, 14000, 30720, 1, 1); // this fails
    destroySession(session);
    destroyClient(client);


    printf("\nTest 8\n\n");

    client = createClient();
    session = createSession(client, true);
    // write value size 13KB to 20KB with 1B increment, with ssl, no sleep in between puts
    writeToDisk(session, 13312, 20480, 1, 0); // this works
    destroySession(session);
    destroyClient(client);



}

