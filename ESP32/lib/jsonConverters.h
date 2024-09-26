#pragma once
#include <vector>
#include <map>
#include "ArduinoJson.h"
#include "enum.h"

void convertFromJson(JsonVariantConst src, TCodeVersion& dst) {
    dst = static_cast<TCodeVersion>(src.as<uint8_t>());
}
bool convertToJson(const TCodeVersion& src, JsonVariant dst) {
    return dst.set(static_cast<uint8_t>(src));
}
bool canConvertFromJson(JsonVariantConst src, const TCodeVersion&) {
  return src.is<uint8_t>();
}

void convertFromJson(JsonVariantConst src, LogLevel& dst) {
    dst = static_cast<LogLevel>(src.as<uint8_t>());
}
bool convertToJson(const LogLevel& src, JsonVariant dst) {
    return dst.set(static_cast<uint8_t>(src));
}
bool canConvertFromJson(JsonVariantConst src, const LogLevel&) {
  return src.is<uint8_t>();
}

void convertFromJson(JsonVariantConst src, BuildFeature& dst) {
    dst = static_cast<BuildFeature>(src.as<uint8_t>());
}
bool convertToJson(const BuildFeature& src, JsonVariant dst) {
    return dst.set(static_cast<uint8_t>(src));
}
bool canConvertFromJson(JsonVariantConst src, const BuildFeature&) {
  return src.is<uint8_t>();
}

void convertFromJson(JsonVariantConst src, BoardType& dst) {
    dst = static_cast<BoardType>(src.as<uint8_t>());
}
bool convertToJson(const BoardType& src, JsonVariant dst) {
    return dst.set(static_cast<uint8_t>(src));
}
bool canConvertFromJson(JsonVariantConst src, const BoardType&) {
  return src.is<uint8_t>();
}

void convertFromJson(JsonVariantConst src, MotorType& dst) {
    dst = static_cast<MotorType>(src.as<uint8_t>());
}
bool convertToJson(const MotorType& src, JsonVariant dst) {
    return dst.set(static_cast<uint8_t>(src));
}
bool canConvertFromJson(JsonVariantConst src, const MotorType&) {
  return src.is<uint8_t>();
}


void convertFromJson(JsonVariantConst src, DeviceType& dst) {
    dst = static_cast<DeviceType>(src.as<uint8_t>());
}
bool convertToJson(const DeviceType& src, JsonVariant dst) {
    return dst.set(static_cast<uint8_t>(src));
}
bool canConvertFromJson(JsonVariantConst src, const DeviceType&) {
  return src.is<uint8_t>();
}

void convertFromJson(JsonVariantConst src, BLDCEncoderType& dst) {
    dst = static_cast<BLDCEncoderType>(src.as<uint8_t>());
}
bool convertToJson(const BLDCEncoderType& src, JsonVariant dst) {
    return dst.set(static_cast<uint8_t>(src));
}
bool canConvertFromJson(JsonVariantConst src, const BLDCEncoderType&) {
  return src.is<uint8_t>();
}

void convertFromJson(JsonVariantConst src, BLEDeviceType& dst) {
    dst = static_cast<BLEDeviceType>(src.as<uint8_t>());
}
bool convertToJson(const BLEDeviceType& src, JsonVariant dst) {
    return dst.set(static_cast<uint8_t>(src));
}
bool canConvertFromJson(JsonVariantConst src, const BLEDeviceType&) {
  return src.is<uint8_t>();
}

void convertFromJson(JsonVariantConst src, BLELoveDeviceType& dst) {
    dst = static_cast<BLELoveDeviceType>(src.as<uint8_t>());
}
bool convertToJson(const BLELoveDeviceType& src, JsonVariant dst) {
    return dst.set(static_cast<uint8_t>(src));
}
bool canConvertFromJson(JsonVariantConst src, const BLELoveDeviceType&) {
  return src.is<uint8_t>();
}

void convertFromJson(JsonVariantConst src, ESPTimerChannelNum& dst) {
    dst = static_cast<ESPTimerChannelNum>(src.as<int8_t>());
}
bool convertToJson(const ESPTimerChannelNum& src, JsonVariant dst) {
    return dst.set(static_cast<int8_t>(src));
}
bool canConvertFromJson(JsonVariantConst src, const ESPTimerChannelNum&) {
  return src.is<int8_t>();
}
// void convertFromJson(JsonVariantConst src, std::vector<const char*>& dst) {
//     for (const char* item : src.as<JsonArrayConst>())
//         dst.push_back(item);
// }
// bool convertToJson(const std::vector<const char*>& src, JsonVariant dst) {
    
//     JsonArray array = dst.to<JsonArray>();
//     for (const char* item : src)
//         if(!array.add(item))
//             return false;
//     return true;
// }

// void convertFromJson(JsonVariantConst src, std::vector<int>& dst) {
//     for (const int item : src.as<JsonArrayConst>())
//         dst.push_back(item);
// }
// bool convertToJson(const std::vector<int>& src, JsonVariant dst) {
    
//     JsonArray array = dst.to<JsonArray>();
//     for (const int item : src)
//         if(!array.add(item))
//             return false;
//     return true;
// }

namespace ArduinoJson {
    template <typename T>
    struct Converter<std::vector<T> > {
        static void toJson(const std::vector<T>& src, JsonVariant dst) {
            JsonArray array = dst.to<JsonArray>();
            for (T item : src)
                array.add(item);
        }

        static std::vector<T> fromJson(JsonVariantConst src) {
            std::vector<T> dst;
            for (T item : src.as<JsonArrayConst>())
                dst.push_back(item);
            return dst;
        }

        static bool checkJson(JsonVariantConst src) {
            JsonArrayConst array = src;
            bool result = array;
            for (JsonVariantConst item : array)
                result &= item.is<T>();
            return result;
        }
    };
}  // namespace ARDUINOJSON_NAMESPACE


// template <typename T>
// void convertFromJson(JsonVariantConst src, std::map<std::string, T>& dst) {
//     std::map<std::string, T> dst;
//     for (JsonPairConst item : src.as<JsonObjectConst>())
//       dst[item.key().c_str()] = item.value().as<T>();
//     return dst;
// }
// template <typename T>
// bool convertToJson(const std::map<std::string, T>& src, JsonVariant dst) {
//     JsonObjectConst obj = src;
//     bool result = obj;
//     for (JsonPairConst item : obj)
//       result &= item.value().is<T>();
//     return result;
// }
;