/* MIT License

Copyright (c) 2024 Jason C. Fain

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR ALL_TAGS =  PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

#pragma once
#include <stdint.h>
#include <map>
#include <string>

namespace Tags
{
    // Increase backing type if additional tags are required
    using tag_t = uint32_t;

    enum
    {
        Main = 0,
        Display,
        Temperature,
        Battery,
        Settings,
        Wifi,
        Udp,
        WebSocketServer,
        WebSocketClient,
        SecureWebSocketServer,
        SecureWebSocketClient,
        Https,
        Web,
        SystemCommand,
        BLE,
        BLEConfiguration,
        Bluetooth,
        Servo,
        TCode,
        Motor,
        Motion,
        Voice,
        Button,
        Mdns,
        SettingsFactory,
        Tasks,
        Network,
        PinMap,
        Filesystem,
        MessageQueue,
        Power,
        LAST,
    };
    const tag_t INVALID = LAST;
    constexpr tag_t ALL_TAGS = ~(0);
    static_assert((LAST <= (sizeof(tag_t) * 8), "Too many Tags defined for backing type, increase size of tag_t"));

    uint32_t as_mask(tag_t tag)
    {
        if (tag >= LAST)
        {
            return 0;
        }
        return (0x1 << static_cast<uint32_t>(tag));
    }

    bool is_set(uint32_t mask, tag_t tag)
    {
        return (mask & as_mask(tag)) != 0;
    }

    void set_tags(uint32_t &mask, tag_t tag)
    {
        mask |= as_mask(tag);
    }

    void unset_tags(uint32_t &mask, tag_t tag)
    {
        mask &= ~as_mask(tag);
    }

    // This will return the lowest tag matching the mask
    tag_t from_mask(uint32_t mask)
    {
        for (uint32_t i = 0; i < static_cast<uint32_t>(LAST); ++i)
        {
            if (mask & (0x1 << i))
            {
                return static_cast<tag_t>(i);
            }
        }
        return 0;
    }

    constexpr const char *TAG_STRINGS[] = {
        "main",
        "display",
        "temperature",
        "battery",
        "settings",
        "wifi",
        "udp",
        "websocket-server",
        "websocket-base",
        "secure-websocket-server",
        "secure-websocket-client",
        "https",
        "web",
        "system-command",
        "ble",
        "ble-configuration",
        "bluetooth",
        "servo",
        "tcode",
        "motor",
        "motion",
        "voice",
        "button",
        "mdns",
        "settings-factory",
        "tasks",
        "network",
        "pin-map",
        "filesystem",
        "message-queue",
        "power",
    };

    constexpr const char* AvailableTags[] = {
        "main",
        "display",
        "temperature",
        "battery",
        "settings",
        "wifi",
        "udp",
        "websocket-server",
        "websocket-base",
        "secure-websocket-server",
        "secure-websocket-client",
        "https",
        "web",
        "system-command",
        "ble",
        "ble-configuration",
        "bluetooth",
        "servo",
        "tcode",
        "motor",
        "motion",
        "voice",
        "button",
        "mdns",
        "settings-factory",
        "tasks",
        "network",
        "pin-map",
        "filesystem",
        "message-queue",
        "power",
    };

    std::string as_str(uint32_t tag_mask)
    {
        std::string result = "";
        for (tag_t index = static_cast<tag_t>(0); index < LAST; index = static_cast<tag_t>(index + 1))
        {
            if (is_set(tag_mask, index))
            {
                try
                {
                    if (!result.empty())
                    {
                        result += ", ";
                    }
                    result += TAG_STRINGS[static_cast<uint32_t>(index)];
                }
                catch (::std::bad_alloc &)
                {
                    // We're out of memory, return what we have
                    return result;
                }
            }
        }
        return result;
    }

    // Accepts a comma, or whitespace separated list of tags
    bool from_str(const char *str, tag_t &found_tags)
    {
        // We need a copy of the buffer on the heap
        // since strtok_r modifies the string
        // Use strtok_r to be reentrant/thread safe vs
        char *workingbuffer = strdup(str);
        char *working_ptr;
        char *token;
        const char *delims = ", \t\n"; // basically all whitespace and comma

        found_tags = 0;

        if (!workingbuffer)
        {
            // Out of memory... give up
            return false;
        }
        while ((token = strtok_r(workingbuffer, delims, &working_ptr)) != nullptr)
        {
            // Linear search
            tag_t tag;
            for (tag = static_cast<tag_t>(0); tag < LAST; tag = static_cast<tag_t>(tag + 1))
            {
                if (strcmp(TAG_STRINGS[static_cast<uint32_t>(tag)], token) == 0)
                {
                    found_tags |= as_mask(tag);
                    break;
                }
            }
            // no token found, token is invaild... abort
            if (tag == LAST)
            {
                free(workingbuffer);
                return false;
            }
        }
        free(workingbuffer);
        return true;
    }
}
