//
// Created by D. A. Hosek on 3.2.21.
//

#include <algorithm>
#include <utility>
#include <ostream>

#include "GFReader.h"
const int_fast32_t PRE  = 247;
const int_fast32_t POST = 248;

OpCode::OpCode(std::shared_ptr<CharacterContext> characterContext) : character_context(std::move(characterContext)) {}

Argument::Argument(std::shared_ptr<FileContext> fileContext) : file_context(std::move(fileContext)) {}


class paint : public OpCode {
public:
    using OpCode::OpCode;

    void operator()(std::int_fast32_t d) const override {
        if (character_context->is_black()) {
            character_context->bitmap.add_blackline(character_context->m, character_context->n, d);
        }
        character_context->m += d;
        character_context->toggle_color();
    }

};

class boc : public OpCode {
public:
    boc(const std::shared_ptr<CharacterContext> &characterContext, std::shared_ptr<FileContext> fileContext) :
            OpCode(characterContext), file_context(std::move(fileContext)) {}

    void operator()(std::int_fast32_t char_code) const override {
        std::int_fast32_t p = file_context->read4();
        std::int_fast32_t min_m = file_context->read4();
        std::int_fast32_t max_m = file_context->read4();
        std::int_fast32_t min_n = file_context->read4();
        std::int_fast32_t max_n = file_context->read4();
        boc_common(char_code, p, min_m, max_m, min_n, max_n);
    }

protected:
    void boc_common(uint_fast32_t code, int_fast32_t p, int_fast32_t min_m, int_fast32_t max_m, int_fast32_t min_n, int_fast32_t max_n) const {
        character_context->min_m = min_m;
        character_context->max_m = max_m;
        character_context->min_n = min_n;
        character_context->max_n = max_n;
        character_context->m = min_m;
        character_context->n = max_n;
        character_context->make_white();
        character_context->code = code;
    }

    std::shared_ptr<FileContext> file_context;
};

class boc1 : public boc {
public:
    using boc::boc;

    void operator()(std::int_fast32_t char_code) const override {
        std::uint_fast32_t del_m = file_context->read1();
        std::uint_fast32_t max_m = file_context->read1();
        std::uint_fast32_t del_n = file_context->read1();
        std::uint_fast32_t max_n = file_context->read1();
        boc_common(char_code, -1, max_m - del_m, max_m, max_n - del_n, max_n);
    }
};

class eoc: public OpCode {
public:
    eoc(const std::shared_ptr<CharacterContext> &characterContext,
        std::shared_ptr<SpecialContext> specialContext, std::shared_ptr<FontContext> fontContext)
            : OpCode(characterContext), special_context(std::move(specialContext)), font_context(std::move(fontContext)) {}

    void operator()(std::int_fast32_t argument) const override {
        // at end of character, copy the special_context into the character_context, then put the character_context
        // into the end of the vector of character in the font_context
        character_context->specialContext = SpecialContext(special_context);
        special_context->clear();
        font_context->add_character(character_context);
    }
private:
    std::shared_ptr<SpecialContext> special_context;
    std::shared_ptr<FontContext> font_context;
};

class skip: public OpCode {
public:
    using OpCode::OpCode;
    void operator()(std::int_fast32_t argument) const override {
        character_context->next_line(argument);
        character_context->make_white();
    }
};

class new_row: public OpCode {
public:
    using OpCode::OpCode;
    void operator()(std::int_fast32_t argument) const override {
        character_context->next_line(argument);
        character_context->make_black();
    }
};

class xxx : public OpCode {
public:
    xxx(const std::shared_ptr<CharacterContext> &characterContext, std::shared_ptr<FileContext> fileContext,
        std::shared_ptr<SpecialContext> specialContext) : OpCode(characterContext), file_context(std::move(fileContext)),
                                                                 special_context(std::move(specialContext)) {}

    void operator()(std::int_fast32_t string_length) const override {
        std::string special = file_context->read_string(string_length);
        std::cout << special << std::endl;
        special_context->add_special(special);
    }

private:
    std::shared_ptr<FileContext> file_context;
    std::shared_ptr<SpecialContext> special_context;
};

class yyy : public OpCode {
public:
    yyy(const std::shared_ptr<CharacterContext> &characterContext,
        std::shared_ptr<SpecialContext> specialContext) : OpCode(characterContext),
                                                                 special_context(std::move(specialContext)) {}

    void operator()(std::int_fast32_t argument) const override {
        special_context->add_numspecial(argument);
    }
private:
    std::shared_ptr<SpecialContext> special_context;
};

class char_loc : public OpCode {
public:
    char_loc(const std::shared_ptr<CharacterContext> &characterContext, std::shared_ptr<FileContext> fileContext) :
            OpCode(characterContext), file_context(std::move(fileContext)) {}


    void operator()(std::int_fast32_t c) const override {
        std::int_fast32_t dx = file_context->read4();
        std::int_fast32_t dy = file_context->read4();
        std::int_fast32_t w = file_context->read4();
        std::int_fast32_t p = file_context->read4();

        char_loc_common(c, dx, dy, w, p);
    }

protected:
    void char_loc_common(uint_fast32_t c, int_fast32_t dx, int_fast32_t dy, int_fast32_t w, int_fast32_t p) const {
        // We won't actually do anything with this data
    }

    std::shared_ptr<FileContext> file_context;
};

class char_loc0 : public char_loc {
public:
    using char_loc::char_loc;

    void operator()(std::int_fast32_t c) const override {
        std::int_fast32_t dx = file_context->read1() * 0x10000;
        std::int_fast32_t w = file_context->read4();
        std::int_fast32_t p = file_context->read4();

        char_loc_common(c, dx, 0, w, p);
    }
};

class pre : public OpCode {
public:
    pre(const std::shared_ptr<CharacterContext> &characterContext, std::shared_ptr<FileContext> fileContext,
        std::shared_ptr<FontContext> fontContext) : OpCode(characterContext), file_context(std::move(fileContext)),
                                                           font_context(std::move(fontContext)) {}

    void operator()(std::int_fast32_t gf_version) const override {
        if (gf_version != 131) {
            throw std::invalid_argument("Version of the GF file is not 131");
        }
        int_fast32_t str_len = file_context->read1();
        font_context->title = file_context->read_string(str_len);
    }
private:
    std::shared_ptr<FileContext> file_context;
    std::shared_ptr<FontContext> font_context;
};

class post : public OpCode {
public:
    post(const std::shared_ptr<CharacterContext> &characterContext, std::shared_ptr<FileContext> fileContext,
         std::shared_ptr<SpecialContext> specialContext, std::shared_ptr<FontContext> fontContext)
            : OpCode(characterContext), file_context(std::move(fileContext)), special_context(std::move(specialContext)),
              font_context(std::move(fontContext)) {}

    void operator()(std::int_fast32_t p) const override {
        font_context->ds = file_context->read4();
        font_context->cs = file_context->read4();
        font_context->hppp = file_context->read4();
        font_context->vppp = file_context->read4();
        font_context->min_m = file_context->read4();
        font_context->max_m = file_context->read4();
        font_context->min_n = file_context->read4();
        font_context->max_n = file_context->read4();

        font_context->specialContext = this->special_context;

        font_context->complete = true;
    }
private:
    std::shared_ptr<FileContext> file_context;
    std::shared_ptr<SpecialContext> special_context;
    std::shared_ptr<FontContext> font_context;
};

class post_post : public OpCode {
public:
    post_post(std::shared_ptr<CharacterContext> characterContext,
              std::shared_ptr<FileContext> fileContext) : OpCode(std::move(characterContext)), file_context(std::move(fileContext)) {}

    void operator()(std::int_fast32_t argument) const override {
        // We won't actually get here since we'll stop when we reach a post command
    }
private:
    std::shared_ptr<FileContext> file_context;
};

class illegal : public OpCode {
public:
    using OpCode::OpCode;

    void operator()(std::int_fast32_t argument) const override {
        //TODO
    }
};

class get1byte : public Argument {
public:
    using Argument::Argument;

    int_fast32_t operator()() const override {
        return file_context->read1();
    }
};

class get2bytes : public Argument {
public:
    using Argument::Argument;

    int_fast32_t operator()() const override {
        return file_context->read2();
    }
};

class get3bytes : public Argument {
public:
    using Argument::Argument;

    int_fast32_t operator()() const override {
        return file_context->read3();
    }
};

class get4bytes : public Argument {
public:
    using Argument::Argument;

    int_fast32_t operator()() const override {
        return file_context->read4();
    }
};


GFReader::GFReader(std::shared_ptr<std::istream> stream)  {
    auto fileContext = std::make_shared<FileContext>(stream);
    auto characterContext = std::make_shared<CharacterContext>();
    auto specialContext = std::make_shared<SpecialContext>();
    auto fontContext = std::make_shared<FontContext>();

    this->file_context = fileContext;
    this->character_context = characterContext;
    this->special_context = specialContext;
    this->font_context = fontContext;

    paint     _paint (characterContext);
    boc       _boc (characterContext, fileContext);
    boc1      _boc1 (characterContext, fileContext);
    eoc       _eoc (characterContext, specialContext, fontContext);
    skip      _skip (characterContext);
    new_row   _new_line (characterContext);
    xxx       _xxx (characterContext, fileContext, specialContext);
    yyy       _yyy (characterContext, specialContext);
    char_loc  _char_loc (characterContext, fileContext);
    char_loc0 _char_loc0 (characterContext, fileContext);
    pre       _pre  (characterContext, fileContext, fontContext);
    post      _post (characterContext, fileContext, specialContext, fontContext);
    post_post _post_post (characterContext, fileContext);
    illegal   _illegal (characterContext);

    get1byte  _1byte (fileContext);
    get2bytes _2bytes (fileContext);
    get3bytes _3bytes (fileContext);
    get4bytes _4bytes (fileContext);

    int parm = 0;
    std::generate_n(opcodes.begin(), 64, [&parm, _paint](){ auto rv = [=](){_paint(parm);  }; ++parm; return rv; });
    opcodes[64] = [=](){_paint(_1byte());};
    opcodes[65] = [=](){_paint(_2bytes());};
    opcodes[66] = [=](){_paint(_3bytes());};
    opcodes[67] = [=](){_boc(_4bytes());};
    opcodes[68] = [=](){_boc1(_1byte());};
    opcodes[69] = [=](){_eoc(0);};
    opcodes[70] = [=](){_skip(_1byte());};
    opcodes[71] = [=](){_skip(_2bytes());};
    opcodes[72] = [=](){_skip(_3bytes());};
    opcodes[73] = [=](){_skip(_4bytes());};
    parm = 0;
    std::generate_n(opcodes.begin()+74, 165, [&parm, _new_line](){ auto rv = [=](){_new_line(parm);  }; ++parm; return rv; });
    opcodes[239] = [=](){_xxx(_1byte());};
    opcodes[240] = [=](){_xxx(_2bytes());};
    opcodes[241] = [=](){_xxx(_3bytes());};
    opcodes[242] = [=](){_xxx(_4bytes());};
    opcodes[243] = [=](){_yyy(_4bytes());};
    opcodes[244] = [](){};
    opcodes[245] = [=](){_char_loc(_1byte());};
    opcodes[246] = [=](){_char_loc0(_1byte());};
    opcodes[PRE] = [=](){_pre(_1byte());};
    opcodes[POST] = [=](){_post(_4bytes());};
    opcodes[249] = [=](){_post_post(_4bytes());};
    std::fill(opcodes.begin()+250, opcodes.end(), [=](){_illegal(0);});
}


FontContext GFReader::read_file() {
    int_fast32_t opcode = this->file_context->read1();
    if (opcode != PRE) {
        throw std::invalid_argument("Invalid GF file—first byte is not beginning of preamble");
    }
    do {
        opcodes[opcode]();
        opcode = this->file_context->read1();
    } while (!font_context->complete);

    return *(this->font_context);
}