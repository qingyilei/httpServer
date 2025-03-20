//
// Created by qing on 25-3-17.
//

#ifndef HTTPSERVER_FILE_UPLOAD_H
#define HTTPSERVER_FILE_UPLOAD_H
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <filesystem>

class FileUploader {
public:
    struct MappedFile {
        void* addr = MAP_FAILED;
        size_t length = 0;
        std::string path;
        int fd = -1;
    };

    static MappedFile map_file(const std::string& base_dir, const std::string& filename, size_t size) {        namespace fs = std::filesystem;

        fs::path full_path = fs::path(base_dir) / filename;
        std::string path_str = full_path.string();
        MappedFile mf;
        mf.path = path_str;
        mf.fd = open(full_path.c_str(), O_RDWR | O_CREAT, 0644);
        if (mf.fd == -1) throw std::runtime_error("open file failed");

        if (ftruncate(mf.fd, size) == -1) {
            close(mf.fd);
            throw std::runtime_error("ftruncate failed");
        }

        mf.addr = mmap(nullptr, size, PROT_WRITE, MAP_SHARED, mf.fd, 0);
        if (mf.addr == MAP_FAILED) {
            close(mf.fd);
            throw std::runtime_error("mmap failed");
        }

        mf.length = size;
        return mf;
    }

    static void unmap_file(MappedFile& mf) {
        if (mf.addr != MAP_FAILED) {
            munmap(mf.addr, mf.length);
            close(mf.fd);
        }
    }
};

#endif //HTTPSERVER_FILE_UPLOAD_H
