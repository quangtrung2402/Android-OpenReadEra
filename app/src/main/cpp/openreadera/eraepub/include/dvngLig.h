//
// Created by Tarasus on 27/8/2020.
//

#ifndef _DVNGLIG_H
#define _DVNGLIG_H

//virtual charcodes range helpers
#define DEVANAGARI_START 0xE001
#define DEVANAGARI_END   0xE1C5
//from 0xE1C6 to 0xE200 is not reserved space
#define BANGLA_START     0xE201
#define BANGLA_END       0xE50F

class dvngLig
{
public:
    lChar16 a = 0;
    lChar16 b = 0;
    lChar16 c = 0;
    lChar16 d = 0;
    lChar16 e = 0;
    lChar16 f = 0;
    lChar16 g = 0;
    lChar16 h = 0;
    lChar16 i = 0;
    lChar16 j = 0;
    std::string a_mod;
    std::string b_mod;
    std::string c_mod;
    std::string d_mod;
    std::string e_mod;
    std::string f_mod;
    std::string g_mod;
    std::string h_mod;
    std::string i_mod;
    std::string j_mod;
    lUInt32 key_hash;
    int len = 0;
    int glyphindex = -1; //index of glyph in current font

    bool banglaRa = false; //last char of ligature is 0x09CD virama (in bengali it is Ra)

    dvngLig(){};

    dvngLig(int glyphIndex, std::string str);

    dvngLig(std::string str);

    dvngLig(lString16 str);

    bool operator == (const dvngLig& rhs);

    bool isNull();

private:
    std::vector<std::string> parse(const std::string &s, char delimeter);

    void parseStdStr(std::string str);

};

struct Comparator
{
    using is_transparent = std::true_type;

    // standard comparison (between two instances of dvngLig)
    bool operator()(const dvngLig& lhs, const dvngLig& rhs) const
    {
        //LE("l = %d r = %d",lhs.key_hash, rhs.key_hash);
        return lhs.key_hash < rhs.key_hash;
    }
};


#endif //_DVNGLIG_H
