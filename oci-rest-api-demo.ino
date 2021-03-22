#include "creds.h"
#include "ArduinoJson.h"
#include "mbedtls/base64.h"
#include "oci.h"

OciProfile ociProfile(tenancyOcid, userOcid, keyFingerprint, apiKey);
Oci oci(ociProfile);

void setup() {
  Serial.begin(115200);
  Serial.println("Connecting to " + String(ssid));
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.println(F("."));
  }
  Serial.print("Connected.");

  // Serial.println("\n*** list Object Storage buckets ***\n");
  listBuckets();

  Serial.println("\n*** get an OSS cursor ***\n");
  getCursor();
  
  Serial.println("\n*** post a message to OSS ***\n");
  postMessage();
  
  Serial.println("\n*** retrieve messages from OSS ***\n");
  getMessages();
  
  Serial.println("\n*** retrieve messages from OSS (again) ***\n");
  getMessages();
}

void loop() {
  delay(5000);
}

void listBuckets() {
  char* osPath = "/n/toddrsharp/b/?compartmentId=ocid1.compartment.oc1..aaaaaaaa7lzppsdxt6j56zhpvy6u5gyrenwyc2e2h4fak5ydvv6kt7anizbq";

  /* headers to add to the request */
  Header reqHeaders[] = { {"opc-request-id", "test"} };

  /* headers to retrieve from the result (name only) */
  Header resHeaders[] = { {"opc-request-id"} };

  /* 
    create a request to retrieve OS buckets
    to ensure a secure connection, pass the Root CA Cert 
    as the 6th argument
  */
  OciApiRequest listBucketsRequest(osHost, osPath, oci.HTTP_METHOD_GET, reqHeaders, 1, objectStorageRootCert);

  /* 
    create an object to store the result
    pass the array of headers to retrieve from the response
  */
  OciApiResponse listBucketsResponse(resHeaders, 1);
  
  oci.apiCall(listBucketsRequest, listBucketsResponse);

  Serial.println(resHeaders[0].headerValue);
  
  if( listBucketsResponse.statusCode == 200 ) {
    Serial.println("List Buckets Response:");
    DynamicJsonDocument doc(6000);
    deserializeJson(doc, listBucketsResponse.response);
    serializeJsonPretty(doc, Serial);  
  }
  else {
    Serial.println(listBucketsResponse.errorMsg);
  }
}

void postMessage() {
  
  /* base64 encode the message value */
  const char *input = "{\"msg\": \"hello, world!\"}";
  unsigned char output[64]; 
  size_t msgOutLen;
  mbedtls_base64_encode(output, 64, &msgOutLen, (const unsigned char*) input, strlen((char*) ((const unsigned char*) input)));

  /* construct a JSON object to contain the message to POST */
  char message[150] = "{ \"messages\": [ { \"key\": null, \"value\": \"";
  strcat(message, (char*) output);
  strcat(message, "\" } ] }");

  // the path to the API endpoint containing the stream OCID
  char postMsgPath[120] = "/20180418/streams/";
  strcat(postMsgPath, demoStreamOcid);
  strcat(postMsgPath, "/messages");

  /* 
    the post message request.
    pass empty array headers if none needed
    pass NULL in place of cert for an 
    insecure HTTPS request
    the POST message body is included
    in the post message request
  */
  OciApiRequest postMessageRequest(streamingHost, postMsgPath, oci.HTTP_METHOD_POST, {}, 0, NULL, message);
  OciApiResponse postMessageResponse;
  oci.apiCall(postMessageRequest, postMessageResponse);

  if( postMessageResponse.statusCode == 200 ) {
    Serial.println("Post Message Response:");
    DynamicJsonDocument doc(6000);
    deserializeJson(doc, postMessageResponse.response);
    serializeJsonPretty(doc, Serial);  
  }
  else {
    Serial.println(postMessageResponse.errorMsg);
  }  
}

char* cursor;

void getCursor() {
  char cursorPath[130] = "/20180418/streams/";
  strcat(cursorPath, demoStreamOcid);
  strcat(cursorPath, "/cursors");

  char createCursorBody[] = "{\"partition\": \"0\", \"type\": \"LATEST\"}";
  OciApiRequest getCursorRequest(streamingHost, cursorPath, oci.HTTP_METHOD_POST, {}, 0, streamingServiceRootCert, createCursorBody);
  OciApiResponse getCursorResponse;
  oci.apiCall(getCursorRequest, getCursorResponse);

  if( getCursorResponse.statusCode == 200 ) {
    Serial.println("Get Cursor Response:");
    DynamicJsonDocument doc(6000);
    deserializeJson(doc, getCursorResponse.response);
    serializeJsonPretty(doc, Serial);
    int cursorLen = strlen(doc["value"])+1;
    cursor = (char*)malloc(cursorLen);
    strncpy(cursor, (const char*) doc["value"], cursorLen);
  }
  else {
    Serial.println(getCursorResponse.errorMsg);
  }
}

void getMessages() {
  char getMsgPath[600] = "/20180418/streams/";
  strcat(getMsgPath, demoStreamOcid);
  strcat(getMsgPath, "/messages?cursor=");
  strcat(getMsgPath, cursor);
  strcat(getMsgPath, "&limit=2");
  
  Header getMsgsHeaders[] = { {"opc-next-cursor"} };
  OciApiRequest getMsgsRequest(streamingHost, getMsgPath, oci.HTTP_METHOD_GET, getMsgsHeaders, 1);
  OciApiResponse getMsgsResponse;
  oci.apiCall(getMsgsRequest, getMsgsResponse);
  
  if( getMsgsResponse.statusCode == 200 ) {
    Serial.println("Get Messages Response:");
    DynamicJsonDocument doc(6000);
    deserializeJson(doc, getMsgsResponse.response);
    serializeJsonPretty(doc, Serial);
    int newCursorLen = strlen(getMsgsHeaders[0].headerValue)+1;
    cursor = (char*)malloc(newCursorLen);
    strncpy(cursor, (const char*) getMsgsHeaders[0].headerValue, newCursorLen);
  }
  else {
    Serial.println(getMsgsResponse.errorMsg);
  } 
}
