//
// Created by Tarasus on 27/8/2020.
//

#include <string>
#include <eraepub/include/lvstring.h>
#include <vector>
#include <ore_log.h>
#include "include/dvngLig.h"


dvngLig::dvngLig(int glyphIndex, std::string str)
{
    parseStdStr(str);
    glyphindex = glyphIndex;
}

dvngLig::dvngLig(std::string str)
{
    parseStdStr(str);
}

dvngLig::dvngLig(lString16 str)
{
    if(str.length()<2 || str.length()>10)
    {
        len = 0;
        return;
    }
    switch (str.length())
    {
       case 10: this->j = str[9];
        case 9: this->i = str[8];
        case 8: this->h = str[7];
        case 7: this->g = str[6];
        case 6: this->f = str[5];
        case 5: this->e = str[4];
        case 4: this->d = str[3];
        case 3: this->c = str[2];
        case 2: this->b = str[1];
        case 1: this->a = str[0]; break;
        default: break;
    }
    this->len = str.length();
    this->banglaRa = ( str[str.length()-1] == 0x09CD);

    if(banglaRa)
    {
        if(str[1] == 0x09CD  && (str[0] == 0x09F0 || str[0] == 0x09B0 ))
           banglaRa = false;
    }

    char hashstr[50];
    sprintf(hashstr, "%X%X%X%X%X%X%X%X%X%X", a, b, c, d, e, f, g, h, i, j);
    key_hash = lString16(hashstr, strlen(hashstr)).getHash();
}

bool dvngLig::operator==(const dvngLig &rhs)
{
    if(this->len != rhs.len) return false;
    if(this->a != rhs.a) return false;
    if(this->b != rhs.b) return false;
    if(this->c != rhs.c) return false;
    if(this->d != rhs.d) return false;
    if(this->e != rhs.e) return false;
    if(this->f != rhs.f) return false;
    if(this->g != rhs.g) return false;
    if(this->h != rhs.h) return false;
    if(this->i != rhs.i) return false;
    if(this->j != rhs.j) return false;
    return true;
}

bool dvngLig::isNull()
{
    return len <= 0;
}

std::vector <std::string> dvngLig::parse(const std::string &s, char delimeter)
{
    std::vector<std::string> output;

    std::string::size_type prev_pos = 0, pos = 0;

    while((pos = s.find(delimeter, pos)) != std::string::npos)
    {
        std::string substring( s.substr(prev_pos, pos-prev_pos) );

        output.push_back(substring);

        prev_pos = ++pos;
    }

    output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word

    return output;
}

void dvngLig::parseStdStr(std::string str)
{
    //LE("str = %s",str.c_str());
    std::vector<std::string> words = parse(str,' ');
    len = words.size();
    if(len < 2 || len > 10)
    {
        LE("something's wrong, len = %d",len);
        return;
    }

    std::string item;
    for (int iter = 0; iter < words.size(); iter++)
    {
        item = words.at(iter);
        std::string mod;
        int mod_index = item.find(".");
        if(mod_index != std::string::npos)
        {
            mod = item.substr(mod_index);
            item = item.substr(0,mod_index);
        }
        //LE("item = [%s], mod = [%s]",item.c_str(),mod.c_str());
        switch (iter)
        {
            case 0: a = std::stoul(item, nullptr, 16); a_mod = mod ; break;
            case 1: b = std::stoul(item, nullptr, 16); b_mod = mod ; break;
            case 2: c = std::stoul(item, nullptr, 16); c_mod = mod ; break;
            case 3: d = std::stoul(item, nullptr, 16); d_mod = mod ; break;
            case 4: e = std::stoul(item, nullptr, 16); e_mod = mod ; break;
            case 5: f = std::stoul(item, nullptr, 16); f_mod = mod ; break;
            case 6: g = std::stoul(item, nullptr, 16); g_mod = mod ; break;
            case 7: h = std::stoul(item, nullptr, 16); h_mod = mod ; break;
            case 8: i = std::stoul(item, nullptr, 16); i_mod = mod ; break;
            case 9: j = std::stoul(item, nullptr, 16); j_mod = mod ; break;
            default: LE("something's wrong, iter = %d",iter); break;
        }
    }
    this->banglaRa = (std::stoul(item, nullptr, 16) == 0x09CD);
    if(banglaRa && (str == "0x09F0 0x09CD" || str == "0x09B0 0x09CD" ))
        banglaRa = false;
    char hashstr[50];
    sprintf(hashstr, "%X%X%X%X%X%X%X%X%X%X", a, b, c, d, e, f, g, h, i, j);
    key_hash = lString16(hashstr,strlen(hashstr)).getHash();
}
