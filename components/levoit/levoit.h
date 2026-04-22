#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/core/helpers.h"
#include "esphome/components/uart/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <freertos/semphr.h>
#include <vector>

namespace esphome {
namespace levoit {

enum class LevoitDeviceModel : uint8_t { NONE, A451S };
enum class LevoitPacketType : uint8_t { SEND_MESSAGE = 0x22, ACK_MESSAGE = 0x12, STATUS_MESSAGE = 0x02, ERROR = 0x52 };
enum class LevoitPayloadType : uint32_t {
  STATUS_REQUEST      = 0x011041,
  AUTO_STATUS         = 0x011141,
  SET_AUTO_OFF        = 0x01E5A5,
  SET_WIFI_STATUS_LED = 0x0129A1,
  SET_POWER_STATE     = 0x0100A0,
  SET_DISPLAY_STATE   = 0x0105A1,
  SET_RGB_LIGHT       = 0x01C240,
  SET_HUMIDITY_LEVEL  = 0x01E8A2,
  SET_MIST_LEVEL      = 0x0160A2,
  SET_WARM_LEVEL      = 0x011241,
};

enum class LevoitState : uint32_t {
  POWER               = 1 << 0,
  TANK                = 1 << 1,
  // FAN_AUTO            = 1 << 2,
  // FAN_SLEEP           = 1 << 3,
  // DISPLAY             = 1 << 4,
  // DISPLAY_LOCK        = 1 << 5,
  // FAN_SPEED1          = 1 << 6,
  HUMIDITY            = 1 << 7,
  // FAN_SPEED3          = 1 << 8,
  // FAN_SPEED4          = 1 << 9,
  // NIGHTLIGHT_OFF      = 1 << 10,
  // NIGHTLIGHT_LOW      = 1 << 11,
  // NIGHTLIGHT_HIGH     = 1 << 12,
  // AUTO_DEFAULT        = 1 << 13,
  // AUTO_QUIET          = 1 << 14,
  // AUTO_EFFICIENT      = 1 << 15,
  // AIR_QUALITY_CHANGE  = 1 << 16,
  // PM25_NAN            = 1 << 17,
  // PM25_CHANGE         = 1 << 18,
  // WIFI_CONNECTED      = 1 << 19,
  // HA_CONNECTED        = 1 << 20,
  // FILTER_RESET        = 1 << 21,
  // WIFI_LIGHT_SOLID    = 1 << 22,
  // WIFI_LIGHT_FLASH    = 1 << 23,
  // WIFI_LIGHT_OFF      = 1 << 24
};

struct LevoitStateListener {
  uint32_t mask;
  std::function<void(uint32_t currentBits)> func;
};

#define MAX_PAYLOAD_SIZE 32
struct LevoitCommand {
  LevoitPayloadType payloadType;
  LevoitPacketType packetType;
  uint8_t payload[MAX_PAYLOAD_SIZE] = {0};
  uint8_t payload_len = 0;
};
static_assert(std::is_trivially_copyable<LevoitCommand>::value,
              "LevoitCommand must be trivially copyable for FreeRTOS queue");

using PayloadTypeOverrideMap = std::unordered_map<LevoitDeviceModel, std::unordered_map<LevoitPayloadType, uint32_t>>;

static const PayloadTypeOverrideMap MODEL_SPECIFIC_PAYLOAD_TYPES = {
    // ... add other device models and their overrides here ...
};

class Levoit : public Component, public uart::UARTDevice {
 public:
  LevoitDeviceModel device_model_ = LevoitDeviceModel::A451S;
  float get_setup_priority() const override { return setup_priority::LATE; }
  void setup() override;
  void dump_config() override;
  void set_device_model(std::string model);
  void set_command_delay(int delay);
  void set_command_timeout(int timeout);
  void set_status_poll_seconds(int interval);
  void register_state_listener(uint32_t changeMask, const std::function<void(uint32_t currentBits)> &func);
  void set_request_state(uint32_t onMask, uint32_t offMask, bool acquireMutex = true);
  uint32_t get_model_specific_payload_type(LevoitPayloadType type);

 protected:
  QueueHandle_t rx_queue_;
  QueueHandle_t tx_queue_;
  SemaphoreHandle_t stateChangeMutex_;
  TaskHandle_t procTxQueueTaskHandle_;
  TaskHandle_t maintTaskHandle_;
  uint32_t current_state_ = 0;
  uint32_t req_on_state_ = 0;
  uint32_t req_off_state_ = 0;
  uint32_t command_delay_;
  uint32_t command_timeout_;
  uint32_t last_command_timestamp_ = 0;
  uint32_t last_rx_char_timestamp_ = 0;
  uint32_t status_poll_seconds;
  uint8_t sequenceNumber_ = 0;
  std::vector<uint8_t> rx_message_;
  std::vector<LevoitStateListener> state_listeners_;
  void rx_queue_task_();
  void process_rx_queue_task_();
  void process_tx_queue_task_();
  void maint_task_();
  void command_sync_();
  void send_command_(const LevoitCommand &command);
  void process_raw_command_(LevoitCommand command);
  void send_raw_command(LevoitCommand command);
  void set_bit_(uint32_t &state, bool condition, LevoitState bit);
  bool validate_message_();
  void handle_payload_(LevoitPayloadType type, uint8_t *payload, size_t len);
  
};

}  // namespace levoit
}  // namespace esphome
