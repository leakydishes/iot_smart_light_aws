#include <pgmspace.h>

#define THINGNAME "ENTER_THING_NAME"

const char WIFI_SSID[] = "ENTER_YOUR_NETWORK";
const char WIFI_PASSWORD[] = "ENTER_YOUR_PASSWORD";
const char AWS_IOT_ENDPOINT[] = "12345-ats.iot.somewhere.amazonaws.com";



// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)EOF";

// Device Certificate
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)KEY";

// Device Private Key
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
-----END RSA PRIVATE KEY-----
)KEY";
