#include "common.h"
#include "thingspeak.h"

#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#define DEVICE_NAME "UA-IOTENSR"

uint8_t ble_addr_type;

static int wf_prv_ssid_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    printf("SSID recieved (size: %d): %s\n", ctxt->om->om_len, ctxt->om->om_data);

    char buffer[ctxt->om->om_len + 1];
    memcpy(buffer, ctxt->om->om_data, ctxt->om->om_len);
    buffer[ctxt->om->om_len] = '\0';
    printf("SSID placed in buffer: %s\n", buffer);

    ssid_provisioned = true;

    strcpy((char *)wifi_config.sta.ssid, buffer);
    printf("SSID placed in config: %s\n", wifi_config.sta.ssid);

    return 0;
}

static int wf_prv_user_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    printf("Username recieved (size: %d): %s\n", ctxt->om->om_len, ctxt->om->om_data);

    char buffer[ctxt->om->om_len + 1];
    memcpy(buffer, ctxt->om->om_data, ctxt->om->om_len);
    buffer[ctxt->om->om_len] = '\0';
    printf("Username in buffer: %s\n", buffer);

    user_provisioned = true;

    esp_wifi_sta_wpa2_ent_set_username((const unsigned char *)buffer, ctxt->om->om_len);
    return 0;
}

static int wf_prv_pass_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    ////printf("Password recieved (size: %d): %s\n", ctxt->om->om_len, ctxt->om->om_data);

    char buffer[ctxt->om->om_len + 1];
    memcpy(buffer, ctxt->om->om_data, ctxt->om->om_len);
    buffer[ctxt->om->om_len] = '\0';

    pass_provisioned = true;
    strcpy((char *)wifi_config.sta.password, buffer);
    esp_wifi_sta_wpa2_ent_set_password((const unsigned char *)buffer, ctxt->om->om_len);

    return 0;
}

static int ts_prv_writekey_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    printf("Write API Key recieved (size: %d): %s\n", ctxt->om->om_len, ctxt->om->om_data);

    memcpy(write_key, ctxt->om->om_data, ctxt->om->om_len);
    write_key[ctxt->om->om_len] = '\0';
    printf("Storing in buffer: %s\n", write_key);

    get_writekey(write_key);

    write_key_provisioned = true;

    return 0;
}

static int bs_chr_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    printf("Begin Sample flag recieved (size: %d): %s\n", ctxt->om->om_len, ctxt->om->om_data);

    int8_t buffer = *ctxt->om->om_data;

    if (buffer == 1)
    {
        xSemaphoreGive(beginSamplingSemaphore);
    }

    return 0;
}

static int wf_conn_flag_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    switch (ctxt->op)
    {
        case BLE_GATT_ACCESS_OP_READ_CHR:
            // Write flag to mbuf to be read
            os_mbuf_append(ctxt->om, &wififlag, sizeof(wififlag));
            break;

        case BLE_GATT_ACCESS_OP_WRITE_CHR:
            // Read flag from mbuf
            wififlag = *ctxt->om->om_data;
            break;
    }

    return 0;
}

static const struct ble_gatt_svc_def gat_svcs[] = {
    {
        // *WiFi Provisioning Service
        // *UUID: 57694669-2050-726F-7669-73696F6E0000
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID128_DECLARE(0x00, 0x00, 0x6E, 0x6F, 0x69, 0x73, 0x69, 0x76, 0x6F, 0x72, 0x50, 0x20, 0x69, 0x46, 0x69, 0x57),
        .characteristics = (struct ble_gatt_chr_def[]){
            // *WiFi Provisioning Service Characteristics
            {
                // *SSID Characteristic
                .uuid = BLE_UUID128_DECLARE(0x01, 0x00, 0x6E, 0x6F, 0x69, 0x73, 0x69, 0x76, 0x6F, 0x72, 0x50, 0x20, 0x69, 0x46, 0x69, 0x57),
                .flags = BLE_GATT_CHR_F_WRITE,
                .access_cb = wf_prv_ssid_cb
            },

            {
                // *Username Characteristic
                .uuid = BLE_UUID128_DECLARE(0x02, 0x00, 0x6E, 0x6F, 0x69, 0x73, 0x69, 0x76, 0x6F, 0x72, 0x50, 0x20, 0x69, 0x46, 0x69, 0x57),
                .flags = BLE_GATT_CHR_F_WRITE,
                .access_cb = wf_prv_user_cb
            },

            {
                // *Password Characteristic
                .uuid = BLE_UUID128_DECLARE(0x03, 0x00, 0x6E, 0x6F, 0x69, 0x73, 0x69, 0x76, 0x6F, 0x72, 0x50, 0x20, 0x69, 0x46, 0x69, 0x57),
                .flags = BLE_GATT_CHR_F_WRITE,
                .access_cb = wf_prv_pass_cb
            },

            {
                // *WiFi Connect Flag Characteristic
                .uuid = BLE_UUID128_DECLARE(0x04, 0x00, 0x6E, 0x6F, 0x69, 0x73, 0x69, 0x76, 0x6F, 0x72, 0x50, 0x20, 0x69, 0x46, 0x69, 0x57),
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
                .access_cb = wf_conn_flag_cb
            },

            {0}
        }
    },

    {
        // *ThingSpeak Provisioning Service
        // *UUID: 54532050-726F-7669-7369-6F6E00000000
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID128_DECLARE(0x00, 0x00, 0x00, 0x00, 0x6E, 0x6F, 0x69, 0x73, 0x69, 0x76, 0x6F, 0x72, 0x50, 0x20, 0x53, 0x54),
        .characteristics = (struct ble_gatt_chr_def[]){ 
            // *ThingSpeak Provisioning Service Characteristics
            {
                // *Write API Key Characteristic
                .uuid = BLE_UUID128_DECLARE(0x01, 0x00, 0x00, 0x00, 0x6E, 0x6F, 0x69, 0x73, 0x69, 0x76, 0x6F, 0x72, 0x50, 0x20, 0x53, 0x54),
                .flags = BLE_GATT_CHR_F_WRITE,
                .access_cb = ts_prv_writekey_cb
            },

            {0}
        }
    },

    {
        // *Sample Begin Flag Service
        // *UUID: 5C58C20C-B466-4A2E-A8AB-B66BADBBA000
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID128_DECLARE(0x00, 0xA0, 0xBB, 0xAD, 0x6B, 0xB6, 0xAB, 0xA8, 0x2E, 0x4A, 0x66, 0xB4, 0x0C, 0xC2, 0x58, 0x5C),
        .characteristics = (struct ble_gatt_chr_def[]){
            // *Begin Sampling Sampling Characteristic
            {
                .uuid = BLE_UUID128_DECLARE(0x01, 0xA0, 0xBB, 0xAD, 0x6B, 0xB6, 0xAB, 0xA8, 0x2E, 0x4A, 0x66, 0xB4, 0x0C, 0xC2, 0x58, 0x5C),
                .flags = BLE_GATT_CHR_F_WRITE,
                .access_cb = bs_chr_cb
            },

            {0}
        }
    },

    {0} 
};

static int ble_gap_event(struct ble_gap_event *event, void *arg);

static void ble_app_advertise(void)
{
    struct ble_hs_adv_fields fields;
    memset(&fields, 0, sizeof(fields));

    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_DISC_LTD;
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    fields.name = (uint8_t *)ble_svc_gap_device_name();
    fields.name_len = strlen(ble_svc_gap_device_name());
    fields.name_is_complete = 1;

    ble_gap_adv_set_fields(&fields);

    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
}

static int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI("GAP", "BLE_GAP_EVENT_CONNECT %s", event->connect.status == 0 ? "OK" : "Failed");

        if (event->connect.status != 0)
        {
            ble_app_advertise();
        }

        break;

    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI("GAP", "BLE_GAP_EVENT_DISCONNECT");
        ble_app_advertise();
        break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI("GAP", "BLE_GAP_EVENT_ADV_COMPLETE");
        ble_app_advertise();
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
        ESP_LOGI("GAP", "BLE_GAP_EVENT_SUBSCRIBE");
        break;

    default:
        break;
    }
    return 0;
}

static void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type);
    ble_app_advertise();
}

static void host_task(void *param)
{
    nimble_port_run();
    nimble_port_freertos_deinit();
}

void initializeBLE()
{
    printOLED("BLE initializing...");
    nvs_flash_init();

    esp_nimble_hci_and_controller_init();
    nimble_port_init();

    ble_svc_gap_device_name_set(DEVICE_NAME);
    ble_svc_gap_init();
    ble_svc_gatt_init();

    ble_gatts_count_cfg(gat_svcs);
    ble_gatts_add_svcs(gat_svcs);

    ble_hs_cfg.sync_cb = ble_app_on_sync;
    nimble_port_freertos_init(host_task);

    user_provisioned = false;
    ssid_provisioned = false;
    pass_provisioned = false;
    write_key_provisioned = false;
    memset(&wifi_config, 0, sizeof(wifi_config));
    wififlag = 0;
}

void deinitializeBLE(){
    int ret = nimble_port_stop();
    if (ret == 0) {
        nimble_port_deinit();

        ret = esp_nimble_hci_and_controller_deinit();
            if (ret != ESP_OK) {
                ESP_LOGE("NimBLE", "esp_nimble_hci_and_controller_deinit() failed with error: %d", ret);
            }
    }
    ESP_LOGI("NimBLE", "BLE denitialized!\n");
}