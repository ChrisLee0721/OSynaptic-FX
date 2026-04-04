#include <stdio.h>
#include <string.h>
#include "osfx_core.h"
int main(void) {
    osfx_fusion_state st;
    osfx_core_sensor_input sensors[10];
    uint8_t packet[4096];
    int packet_len = 0;
    uint8_t cmd = 0;
    size_t i;
    osfx_packet_meta meta;
    uint8_t body[2048];
    size_t body_len = 0;
    memset(sensors, 0, sizeof(sensors));
    sensors[0] = (osfx_core_sensor_input){"TEMP1","OK",23.5,"cel","wx4g0ec1","msg_temp1","https://r/1"};
    sensors[1] = (osfx_core_sensor_input){"TEMP2","OK",24.1,"cel","wx4g0ec2","msg_temp2","https://r/2"};
    sensors[2] = (osfx_core_sensor_input){"PRESS1","OK",101.3,"kPa","wx4g0ec3","msg_press1","https://r/3"};
    sensors[3] = (osfx_core_sensor_input){"PRESS2","OK",99.8,"kPa","wx4g0ec4","msg_press2","https://r/4"};
    sensors[4] = (osfx_core_sensor_input){"HUM1","OK",55.0,"%","wx4g0ec5","msg_hum1","https://r/5"};
    sensors[5] = (osfx_core_sensor_input){"HUM2","OK",57.2,"%","wx4g0ec6","msg_hum2","https://r/6"};
    sensors[6] = (osfx_core_sensor_input){"FLOW1","OK",12.7,"L/min","wx4g0ec7","msg_flow1","https://r/7"};
    sensors[7] = (osfx_core_sensor_input){"FLOW2","OK",13.1,"L/min","wx4g0ec8","msg_flow2","https://r/8"};
    sensors[8] = (osfx_core_sensor_input){"VIB1","OK",0.82,"g","wx4g0ec9","msg_vib1","https://r/9"};
    sensors[9] = (osfx_core_sensor_input){"VIB2","OK",0.79,"g","wx4g0eca","msg_vib2","https://r/10"};
    osfx_fusion_state_init(&st);
    if (!osfx_core_encode_multi_sensor_packet_auto(
            &st, 500U, 2U, 1710001234ULL, "NODE_A", "ONLINE",
            sensors, 10U, packet, sizeof(packet), &packet_len, &cmd)) {
        printf("encode_failed\n");
        return 1;
    }
    if (!osfx_packet_decode_meta(packet, (size_t)packet_len, &meta)) {
        printf("decode_meta_failed\n");
        return 1;
    }
    if (!osfx_packet_extract_body(packet, (size_t)packet_len, NULL, 0, body, sizeof(body), &body_len, NULL)) {
        printf("extract_body_failed\n");
        return 1;
    }
    body[body_len] = '\0';
    printf("cmd=%u\n", (unsigned)cmd);
    printf("packet_len=%d\n", packet_len);
    printf("source_aid=%u\n", (unsigned)meta.source_aid);
    printf("tid=%u\n", (unsigned)meta.tid);
    printf("timestamp_raw=%llu\n", (unsigned long long)meta.timestamp_raw);
    printf("body_len=%llu\n", (unsigned long long)body_len);
    printf("body=%s\n", (char*)body);
    printf("packet_hex=");
    for (i = 0; i < (size_t)packet_len; ++i) {
        printf("%02X", packet[i]);
    }
    printf("\n");
    return 0;
}
