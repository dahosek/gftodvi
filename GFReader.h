//
// Created by D. A. Hosek on 3.2.21.
//

#ifndef GFTODVI_GFREADER_H
#define GFTODVI_GFREADER_H
#include <utility>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <array>
#include <functional>
#include <iostream>
#include "Bitmap.h"

class SpecialContext {
public:
    SpecialContext(std::shared_ptr<SpecialContext> sharedPtr) {
        special_list = sharedPtr->special_list;
    }

    SpecialContext() {}

    inline void add_special(std::string special) {
        special_list.emplace_back(make_pair(special, std::vector<int_fast32_t>()));
    }
    inline void add_numspecial(int_fast32_t val) {
        special_list.back().second.emplace_back(val);
    }
    inline void clear() {
        special_list.clear();
    }

    const std::vector<std::pair<std::string, std::vector<int_fast32_t>>> &getSpecialList() const {
        return special_list;
    }

private:
    std::vector<std::pair<std::string, std::vector<int_fast32_t>>> special_list {};
};

class CharacterContext {
public:
    CharacterContext(const std::shared_ptr<CharacterContext>& sharedPtr, SpecialContext specialContext)
            : specialContext(std::move(specialContext)) {
        code = sharedPtr->code;
        min_m = sharedPtr->min_m;
        max_m = sharedPtr->max_m;
        min_n = sharedPtr->min_n;
        max_n = sharedPtr->max_n;
        bitmap = sharedPtr->bitmap;
    }

    CharacterContext()  {}

    std::int_fast32_t code;
    std::int_fast32_t m;
    std::int_fast32_t n;
    std::int_fast32_t min_m;
    std::int_fast32_t max_m;
    std::int_fast32_t min_n;
    std::int_fast32_t max_n;
    enum {black, white} color = white;
    Bitmap bitmap;
    SpecialContext specialContext;

    inline void toggle_color() {
        color = color == black ? white : black;
    }
    inline void next_line(uint_fast32_t d) {
        n -= (d + 1);
        m = min_m;
    }
    inline void make_white() {
        color = white;
    }
    inline void make_black() {
        color = black;
    }
    inline bool is_black() const {
        return color == black;
    }
};

static_assert(CHAR_BIT == 8); // Unlikely to be false, and if it is, you'll have to deal with it yourself.

class FileContext {
public:
    FileContext(std::shared_ptr<std::istream> stream) : stream(std::move(stream)) {}

    // 32 bit signed
    std::int_fast32_t read4() {
        stream->read(buffer, 4);
        // Just in case int32_t is not the same as int_fast_32_t, we do an extra cast here to make sure signed values
        // come out right.
        return static_cast<int32_t>(static_cast<uint8_t>(buffer[0]) << 24
               | static_cast<uint8_t>(buffer[1]) << 16
               | static_cast<uint8_t>(buffer[2]) << 8
               | static_cast<uint8_t>(buffer[3]));
    }
    // 24 bit unsigned
    std::int_fast32_t read3() {
        stream->read(buffer, 3);
        return static_cast<uint8_t>(buffer[0]) << 16
               | static_cast<uint8_t>(buffer[1]) << 8
               | static_cast<uint8_t>(buffer[2]);
    }
    // 16 bit unsigned
    std::int_fast32_t read2() {
        stream->read(buffer, 2);
        return static_cast<uint8_t>(buffer[0]) << 8
         | static_cast<uint8_t>(buffer[1]);
    }
    // 8 bit unsigned
    std::int_fast32_t read1() {
        return static_cast<uint8_t>(stream->get());
    }
    // string
    std::string read_string(int_fast32_t size) {
        std::string return_value(size, ' ');
        stream->read(&return_value[0], size);
        return return_value;
    }
private:
    std::shared_ptr<std::istream> stream;
    char buffer[4];
};


class FontContext {
public:
    void add_character(std::shared_ptr<CharacterContext> character_context) {
        characters.emplace_back(CharacterContext(character_context, character_context->specialContext));
    }
    std::string title;
    std::vector<CharacterContext> characters = std::vector<CharacterContext>();
    int_fast32_t ds;
    int_fast32_t cs;
    int_fast32_t hppp;
    int_fast32_t vppp;
    int_fast32_t min_m;
    int_fast32_t max_m;
    int_fast32_t min_n;
    int_fast32_t max_n;
    SpecialContext specialContext;
    bool complete = false;
};

class OpCode {
public:
    OpCode(std::shared_ptr<CharacterContext> characterContext);

    virtual void operator()(std::int_fast32_t argument) const = 0;
protected:
    std::shared_ptr<CharacterContext> character_context;
};

class Argument {
public:
    Argument(std::shared_ptr<FileContext> fileContext);
    virtual std::int_fast32_t operator()() const = 0;

protected:
    std::shared_ptr<FileContext> file_context;
};

class GFReader {
public:
    GFReader(std::shared_ptr<std::istream> stream);
    FontContext read_file();

private:
    std::array<std::function<void()>, 256> opcodes;
    std::shared_ptr<FileContext> file_context;
    std::shared_ptr<CharacterContext> character_context;
    std::shared_ptr<SpecialContext> special_context;
    std::shared_ptr<FontContext> font_context;
};



#endif //GFTODVI_GFREADER_H
