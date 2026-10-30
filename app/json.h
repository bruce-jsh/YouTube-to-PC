#pragma once
// Minimal JSON reader for yt-dlp output (-J). Strings are kept as UTF-8.
#include <string>
#include <vector>
#include <utility>
#include <cstdlib>
#include <cstring>
#include <cstdio>

struct JValue {
    enum Type { Null, Bool, Num, Str, Arr, Obj };
    Type t = Null;
    bool b = false;
    double n = 0;
    std::string s;
    std::vector<JValue> a;
    std::vector<std::pair<std::string, JValue>> o;

    const JValue* find(const char* key) const {
        if (t != Obj) return nullptr;
        for (auto& p : o)
            if (p.first == key) return &p.second;
        return nullptr;
    }
    const JValue& operator[](const char* key) const {
        static const JValue nil;
        const JValue* v = find(key);
        return v ? *v : nil;
    }
    bool isNull() const { return t == Null; }
    std::string str(const char* def = "") const {
        if (t == Str) return s;
        if (t == Num) {
            char buf[64];
            snprintf(buf, sizeof buf, "%.15g", n);
            return buf;
        }
        return def;
    }
    double num(double def = 0) const {
        if (t == Num) return n;
        if (t == Str && !s.empty()) {
            char* end = nullptr;
            double v = strtod(s.c_str(), &end);
            if (end != s.c_str()) return v;
        }
        return def;
    }
};

class JParser {
public:
    explicit JParser(const std::string& src) : p_(src.data()), e_(src.data() + src.size()) {}
    bool parse(JValue& out) {
        ws();
        return value(out, 0);
    }

private:
    const char* p_;
    const char* e_;

    void ws() {
        while (p_ < e_ && (*p_ == ' ' || *p_ == '\t' || *p_ == '\n' || *p_ == '\r')) ++p_;
    }
    bool lit(const char* w) {
        size_t n = strlen(w);
        if ((size_t)(e_ - p_) < n || strncmp(p_, w, n) != 0) return false;
        p_ += n;
        return true;
    }
    bool value(JValue& v, int depth) {
        if (depth > 256 || p_ >= e_) return false;
        switch (*p_) {
        case '{': return object(v, depth);
        case '[': return array(v, depth);
        case '"': v.t = JValue::Str; return string(v.s);
        case 't': if (lit("true")) { v.t = JValue::Bool; v.b = true; return true; } return false;
        case 'f': if (lit("false")) { v.t = JValue::Bool; v.b = false; return true; } return false;
        case 'n': if (lit("null")) { v.t = JValue::Null; return true; } return false;
        case 'N': if (lit("NaN")) { v.t = JValue::Null; return true; } return false;
        case 'I': if (lit("Infinity")) { v.t = JValue::Num; v.n = 1e308; return true; } return false;
        default: return number(v);
        }
    }
    bool number(JValue& v) {
        if (*p_ == '-' && lit("-Infinity")) { v.t = JValue::Num; v.n = -1e308; return true; }
        char* end = nullptr;
        std::string tmp(p_, (size_t)std::min<ptrdiff_t>(e_ - p_, 64));
        v.n = strtod(tmp.c_str(), &end);
        if (end == tmp.c_str()) return false;
        p_ += (end - tmp.c_str());
        v.t = JValue::Num;
        return true;
    }
    static void put(std::string& out, unsigned cp) {
        if (cp < 0x80) out += (char)cp;
        else if (cp < 0x800) { out += (char)(0xC0 | (cp >> 6)); out += (char)(0x80 | (cp & 0x3F)); }
        else if (cp < 0x10000) { out += (char)(0xE0 | (cp >> 12)); out += (char)(0x80 | ((cp >> 6) & 0x3F)); out += (char)(0x80 | (cp & 0x3F)); }
        else { out += (char)(0xF0 | (cp >> 18)); out += (char)(0x80 | ((cp >> 12) & 0x3F)); out += (char)(0x80 | ((cp >> 6) & 0x3F)); out += (char)(0x80 | (cp & 0x3F)); }
    }
    bool hex4(unsigned& cp) {
        if (e_ - p_ < 4) return false;
        cp = 0;
        for (int i = 0; i < 4; ++i) {
            char c = *p_++;
            cp <<= 4;
            if (c >= '0' && c <= '9') cp |= (unsigned)(c - '0');
            else if (c >= 'a' && c <= 'f') cp |= (unsigned)(c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') cp |= (unsigned)(c - 'A' + 10);
            else return false;
        }
        return true;
    }
    bool string(std::string& out) {
        ++p_;  // opening quote
        while (p_ < e_) {
            char c = *p_++;
            if (c == '"') return true;
            if (c != '\\') { out += c; continue; }
            if (p_ >= e_) return false;
            char esc = *p_++;
            switch (esc) {
            case '"': out += '"'; break;
            case '\\': out += '\\'; break;
            case '/': out += '/'; break;
            case 'b': out += '\b'; break;
            case 'f': out += '\f'; break;
            case 'n': out += '\n'; break;
            case 'r': out += '\r'; break;
            case 't': out += '\t'; break;
            case 'u': {
                unsigned cp;
                if (!hex4(cp)) return false;
                if (cp >= 0xD800 && cp <= 0xDBFF && e_ - p_ >= 6 && p_[0] == '\\' && p_[1] == 'u') {
                    const char* save = p_;
                    p_ += 2;
                    unsigned lo;
                    if (hex4(lo) && lo >= 0xDC00 && lo <= 0xDFFF) cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
                    else p_ = save;
                }
                put(out, cp);
                break;
            }
            default: out += esc; break;
            }
        }
        return false;
    }
    bool array(JValue& v, int depth) {
        v.t = JValue::Arr;
        ++p_;
        ws();
        if (p_ < e_ && *p_ == ']') { ++p_; return true; }
        while (p_ < e_) {
            v.a.emplace_back();
            if (!value(v.a.back(), depth + 1)) return false;
            ws();
            if (p_ < e_ && *p_ == ',') { ++p_; ws(); continue; }
            if (p_ < e_ && *p_ == ']') { ++p_; return true; }
            return false;
        }
        return false;
    }
    bool object(JValue& v, int depth) {
        v.t = JValue::Obj;
        ++p_;
        ws();
        if (p_ < e_ && *p_ == '}') { ++p_; return true; }
        while (p_ < e_) {
            if (*p_ != '"') return false;
            v.o.emplace_back();
            if (!string(v.o.back().first)) return false;
            ws();
            if (p_ >= e_ || *p_ != ':') return false;
            ++p_;
            ws();
            if (!value(v.o.back().second, depth + 1)) return false;
            ws();
            if (p_ < e_ && *p_ == ',') { ++p_; ws(); continue; }
            if (p_ < e_ && *p_ == '}') { ++p_; return true; }
            return false;
        }
        return false;
    }
};
