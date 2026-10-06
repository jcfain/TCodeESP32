#ifndef FILESYSTEM_HANDLER_H_
#define FILESYSTEM_HANDLER_H_

#include <FS.h>
#include <LittleFS.h>
#include "logging/LogHandler.h"
#include "logging/TagHandler.h"

class FilesystemHandler
{
    public:
        static void init()
        {
            if(!LittleFS.begin(true))
            {
                LogHandler::error(Tags::Filesystem, "An error has occurred while mounting LittleFS");
            }
            else
            {
                LogHandler::info(Tags::Filesystem, "LittleFS mounted successfully");
            }
        }

        static bool exists(const char* path)
        {
            return LittleFS.exists(path);
        }

        File open(const char* path, const char* mode = FILE_READ, const bool create = false)
        {
            return LittleFS.open(path, mode, create);
        }

        static int write(File &file, const uint8_t *buf, size_t size)
        {
            return file.write(buf, size);
        }

        static int read(File &file, uint8_t *buf, size_t size)
        {
            return file.read(buf, size);
        }
    };


#endif // FILESYSTEM_HANDLER_H_