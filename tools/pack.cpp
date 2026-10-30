// Builds the installer: pack.exe <setup-stub.exe> <payload-dir> <output.exe>
// Output = stub + LZMS-compressed files + footer (read by src/setup/setup.cpp).
#ifndef UNICODE
#define UNICODE
#endif
#define NOMINMAX
#include <windows.h>
#include <compressapi.h>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#pragma pack(push, 1)
struct PackFooter {
    char magic[8];
    uint64_t payloadOffset;
    uint64_t totalRaw;
    uint32_t fileCount;
    uint32_t reserved;
};
#pragma pack(pop)

static const size_t kBlock = 8u << 20;

static void ListFiles(const std::wstring& root, const std::wstring& rel, std::vector<std::wstring>& out) {
    WIN32_FIND_DATAW fd;
    std::wstring pattern = root + (rel.empty() ? L"" : L"\\" + rel) + L"\\*";
    HANDLE h = FindFirstFileW(pattern.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        std::wstring name = fd.cFileName;
        if (name == L"." || name == L"..") continue;
        std::wstring r = rel.empty() ? name : rel + L"\\" + name;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ListFiles(root, r, out);
        else out.push_back(r);
    } while (FindNextFileW(h, &fd));
    FindClose(h);
}

static bool ReadAll(const std::wstring& path, std::vector<BYTE>& data) {
    HANDLE f = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return false;
    LARGE_INTEGER sz;
    GetFileSizeEx(f, &sz);
    data.resize((size_t)sz.QuadPart);
    DWORD got = 0;
    bool ok = data.empty() || (ReadFile(f, data.data(), (DWORD)data.size(), &got, nullptr) && got == data.size());
    CloseHandle(f);
    return ok;
}

static bool Put(HANDLE f, const void* p, size_t n) {
    DWORD put = 0;
    return WriteFile(f, p, (DWORD)n, &put, nullptr) && put == n;
}

int wmain(int argc, wchar_t** argv) {
    if (argc != 4) {
        fwprintf(stderr, L"usage: pack <stub.exe> <payload-dir> <output.exe>\n");
        return 2;
    }
    std::vector<BYTE> stub;
    if (!ReadAll(argv[1], stub)) {
        fwprintf(stderr, L"cannot read stub %ls\n", argv[1]);
        return 1;
    }
    std::wstring root = argv[2];
    std::vector<std::wstring> files;
    ListFiles(root, L"", files);
    std::sort(files.begin(), files.end());
    if (files.empty()) {
        fwprintf(stderr, L"payload is empty\n");
        return 1;
    }
    HANDLE out = CreateFileW(argv[3], GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
    if (out == INVALID_HANDLE_VALUE) {
        fwprintf(stderr, L"cannot create %ls\n", argv[3]);
        return 1;
    }
    COMPRESSOR_HANDLE comp = nullptr;
    if (!CreateCompressor(COMPRESS_ALGORITHM_LZMS, nullptr, &comp)) {
        fwprintf(stderr, L"CreateCompressor failed\n");
        return 1;
    }
    Put(out, stub.data(), stub.size());
    uint64_t totalRaw = 0, totalStored = 0;
    std::vector<BYTE> data, packed;
    for (auto& rel : files) {
        if (!ReadAll(root + L"\\" + rel, data)) {
            fwprintf(stderr, L"cannot read %ls\n", rel.c_str());
            return 1;
        }
        uint16_t len = (uint16_t)rel.size();
        uint64_t size = data.size();
        uint32_t blocks = (uint32_t)((data.size() + kBlock - 1) / kBlock);
        Put(out, &len, sizeof len);
        Put(out, rel.c_str(), rel.size() * sizeof(wchar_t));
        Put(out, &size, sizeof size);
        Put(out, &blocks, sizeof blocks);
        uint64_t stored = 0;
        for (uint32_t b = 0; b < blocks; ++b) {
            size_t off = (size_t)b * kBlock, n = std::min(kBlock, data.size() - off);
            packed.resize(n + n / 2 + 65536);
            SIZE_T outSize = 0;
            uint8_t method = 1;
            if (!Compress(comp, data.data() + off, n, packed.data(), packed.size(), &outSize) || outSize >= n) method = 0;
            uint32_t rawLen = (uint32_t)n, storedLen = method ? (uint32_t)outSize : (uint32_t)n;
            Put(out, &rawLen, 4);
            Put(out, &storedLen, 4);
            Put(out, &method, 1);
            Put(out, method ? packed.data() : data.data() + off, storedLen);
            stored += storedLen;
        }
        totalRaw += size;
        totalStored += stored;
        wprintf(L"  %-44ls %9.1f MB -> %7.1f MB\n", rel.c_str(), size / 1048576.0, stored / 1048576.0);
    }
    CloseCompressor(comp);
    PackFooter f{};
    memcpy(f.magic, "YT2PCPK1", 8);
    f.payloadOffset = stub.size();
    f.totalRaw = totalRaw;
    f.fileCount = (uint32_t)files.size();
    Put(out, &f, sizeof f);
    CloseHandle(out);
    wprintf(L"  %u files, %.1f MB -> %.1f MB\n", f.fileCount, totalRaw / 1048576.0, totalStored / 1048576.0);
    return 0;
}
