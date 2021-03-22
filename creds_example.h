/* 
 *  to use this example, rename this file to creds.h
 *  and populate the values below
 */

const char ssid[] = "";
const char pass[] = "";

char osHost[] = ""; //ex: objectstorage.us-phoenix-1.oraclecloud.com
char streamingHost[] = ""; //ex: streaming.us-phoenix-1.oci.oraclecloud.com
char demoStreamOcid[] = "";

char tenancyOcid[] = "";
char userOcid[] = "";
char keyFingerprint[] = "";
char* apiKey = \
"-----BEGIN RSA PRIVATE KEY-----\n"\
"MI [redacted] 4h\n"\
"-----END RSA PRIVATE KEY-----\n";

/* obtain the root ca cert from the API endpoint 
 * ex: openssl s_client -connect objectstorage.us-phoenix-1.oraclecloud.com:443 -showcerts
 */
char* objectStorageRootCert = \
"-----BEGIN CERTIFICATE-----\n"\
"MI [redacted] Bh\n"\
"-----END CERTIFICATE-----\n";

char* streamingServiceRootCert = \
"-----BEGIN CERTIFICATE-----\n" \
"MII [redacted] Bh\n" \
"-----END CERTIFICATE-----\n";
